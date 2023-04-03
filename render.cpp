#include <Bela.h>
#include <AddressableLeds.h>
#include <vector>

// rgb triplets, can be resized to match number of LEDs _before_ `task()` starts.
// after that, its content (but not size!) can be changed from anywhere
std::vector<uint8_t> colors = {{
	0x00, 0xC0, 0x00,
	0xC0, 0x00, 0x00,
	0x00, 0x00, 0xC0,
	0x00, 0x00, 0x00,
}};

void task(void*) {
	// uint8_t kNumColors = colors.size() / kBytesPerRgb;
	uint8_t kNumLeds = 16;
	std::vector<uint8_t> rgb(kNumLeds * kBytesPerRgb);
	for (unsigned int n = 0; n < 1000000 && !Bela_stopRequested(); ++n) {
		for(unsigned int l = 0; l < kNumLeds; ++l) {
			printf("RGB[%d]: ", l);
			for(unsigned int b = 0;  b < kBytesPerRgb; ++b) {
				uint8_t val = colors[(( n + l) * kBytesPerRgb + b) % colors.size()];;
				rgb[l * kBytesPerRgb + b] = val;
				printf("%#02x ", val);
			}
			printf("\n");
		}
		neopixelSend(rgb);
		usleep(500000);
	}
}

bool setup(BelaContext *context, void *userData)
{
	spi.setup({.device = "/dev/spidev2.1", // Device to open
		.speed = kSpiClock, // Clock speed in Hz
		.delay = 0, // Delay after last transfer before deselecting the device, won't matter
		.numBits = kSpiWordLength, // No. of bits per transaction word,
		.mode = Spi::MODE3 // SPI mode
	});
	size_t numLeds = 12; // TODO: adjust depending on num of leds in use
	colors.resize(kBytesPerRgb * numLeds);
	Bela_runAuxiliaryTask(task, 90);
	return true;
}

void render(BelaContext *context, void *userData)
{
// TODO: set elements of `colors`
}

void cleanup(BelaContext *context, void *userData)
{}
