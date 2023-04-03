#include <AddressableLeds.h>
#include <string.h>
#include <cmath>
#include <stdio.h>

bool AddressableLeds::readBitField(const uint8_t* data, uint32_t offset)
{
	uint8_t position = offset % kBitsPerByte;
	unsigned int i = offset / kBitsPerByte;
	return data[i] & (1 << position);
}

void AddressableLeds::writeBitField(uint8_t* data, uint32_t offset, bool value)
{
	uint8_t position = offset % kBitsPerByte;
	unsigned int i = offset / kBitsPerByte;
	if(value)
		data[i] |= 1 << position;
	else
		data[i] &= ~(1 << position);
}

ssize_t AddressableLeds::rgbToClk(const uint8_t* rgb, size_t numRgb, uint8_t* out, size_t numOut)
{
	memset(out, 0, numOut * sizeof(out[0]));
	// writeBitField(out, 0, 1);
	uint32_t clk = 0;
	// emsure we have complete RGB sets
	numRgb = numRgb - (numRgb % 3);
	for(uint32_t inBit = 0; inBit < numRgb * kBitsPerByte; ++inBit) {
		// data comes in as RGB but needs to be shuffled into GRB for the WS2812B
		uint32_t actualInBit;
		uint8_t remainder = inBit % (3 * kBitsPerByte);
		if(remainder < kBitsPerByte)
			actualInBit = inBit + kBitsPerByte;
		else if(remainder < kBitsPerByte * 2)
			actualInBit = inBit - kBitsPerByte;
		else
			actualInBit = inBit;
		bool value = readBitField(rgb, actualInBit);
		const T_t* Ts[2] = {TH, TL};
		for(unsigned int t = 0; t < 2; ++t) {
			const T_t* T = Ts[t];
			bool highLow = (0 == t) ? true : false;
			float time = 0;
			while(time < T[value].min) {
				bool wordEnd = (kSpiWordLength - 1 == clk);
				writeBitField(out, clk, highLow);
				time += kSpiPeriodNs;
				if(wordEnd)
					time += kSpiInterWordTime;
				++clk;
				if(clk / kBitsPerByte > numOut)
					return -1;
			}
			if(time > T[value].max) {
				printf("Error: expected %f but got %f\n", T[value].max, time);
				return 0;
			}

		}
	}
	ssize_t numBytes = ((clk + kSpiWordLength - 1) / kSpiWordLength) * kSpiWordLength / kBitsPerByte; // round up to the next word
	if(numBytes > numOut) // just in case numOut was not a multiple, we now no longer fit
		return -1;
	return numBytes;
}

void AddressableLeds::sendSpi(uint8_t* data, size_t length, bool verbose)
{
	if(length < kSpiMinTransferSize)
		length = kSpiMinTransferSize;
	if (spi.transfer(data, NULL, length) == 0)
		verbose && printf("SPI: Transaction Complete. Sent %d bytes\n", length);
	else
		fprintf(stderr, "SPI: Transaction Failed\r\n");
}

int AddressableLeds::send(const std::vector<uint8_t>& rgb, bool verbose)
{
	std::vector<uint8_t> data(kSpiMaxTransferSize);
	if(verbose)
	{
		printf("data: ");
		for(unsigned int n = 0; n < rgb.size(); ++n) {
			if(0 == (n % 3))
				printf("\n");
			printf("%#02x ", rgb[n]);
		}
		printf("\n");
	}
	ssize_t len = kSpiLeadingZeros + rgbToClk(rgb.data(), rgb.size(), data.data() + kSpiLeadingZeros, data.size() - kSpiLeadingZeros);
	if(len < 0) {
		fprintf(stderr, "Error: message too long\n");
		return 1;
	} else {
		if(verbose)
			printf("Data is %d\n", len);
	}
	if(verbose)
	{
		// print data as it will be sent out
		unsigned int k = 0;
		for(unsigned int n = 0; n < len; ++n) {
			for(unsigned int c = 0; c < kBitsPerByte; ++c) {
				printf("%d", (bool)(data[n] & (1 << c)));
				++k;
				if(0 == (k % kBitsPerByte))
					printf(" ");
				if(0 == (k % kSpiWordLength))
					printf("| ");
			}
		}
		printf("\n");
	}

	// format data
	// SPI transmits the most significant byte first ("right justified in each word")
	// so if we have more than 1 byte per word we need to shuffle them around so that
	// they are output in the correct order
	unsigned int bytesPerWord = kSpiWordLength / kBitsPerByte;
	for(unsigned int n = 0; n < len; n += bytesPerWord) {
		for(unsigned int b = 0; b < bytesPerWord / 2; ++b) {
			uint8_t tmp = data[n + b];
			data[n + b] = data[n + bytesPerWord - 1 - b];
			data[n + bytesPerWord - 1 - b] = tmp;
		}
	}
	if(kSpiMsbFirst) {
		for(unsigned int n = 0; n < len; ++n)
		{
			data[n] = __builtin_bitreverse8(data[n]);
		}
	}

	sendSpi(data.data(), data.size(), verbose);
	return 0;
}

int AddressableLeds::setup(const std::string& device)
{
	int ret = spi.setup({.device = device.c_str(), // Device to open
		.speed = kSpiClock, // Clock speed in Hz
		.delay = 0, // Delay after last transfer before deselecting the device, won't matter
		.numBits = kSpiWordLength, // No. of bits per transaction word,
		.mode = Spi::MODE3 // SPI mode
	});
	return ret;
}
