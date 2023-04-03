#include <vector>
#include <libraries/Spi/Spi.h>
#include <string>
#pragma once

class AddressableLeds
{
public:
	static constexpr uint8_t kBytesPerRgb = 3;
private:
	Spi spi;
	static constexpr unsigned int kSpiClock = 12000000;
	static constexpr unsigned int kSpiMinTransferSize = 160; // min number of bytes to trigger DMA transfer
	static constexpr unsigned int kSpiMaxTransferSize = 4096; // max number of bytes to be sent at once
	static constexpr unsigned int kSpiWordLength = 32;
	static constexpr uint8_t kSpiMsbFirst = 1;
	static constexpr float kSpiInterWordTime = 200;
	static constexpr float kSpiPeriodNs = 1000000000.0 / kSpiClock;
	static constexpr uint8_t kBitsPerByte = 8;
	// The specs (WS2812B-2020 datasheet) say:
	// T0H 0 code, high voltage time 220ns~380ns
	// T1H 1 code, high voltage time 580ns~1µs
	// T0L 0 code, low voltage time 580ns~1µs
	// T1L 1 code, low voltage time 220ns~420ns
	// RES Frame unit, low voltage time >280µs
	// WS2812B v1.0 on the other hand says:
	// TH+TL = 1.25µs ± 600ns
	// T0H 0 code, high voltage time 0.4us ±150ns
	// T1H 1 code, high voltage time 0.8us ±150ns
	// T0L 0 code, low voltage time 0.85us ±150ns
	// T1L 1 code, low voltage time 0.45us ±150ns
	// More recent datasheets say

	// We pick below some conservative values for WS2812B
	typedef struct {
		float min;
		float max;
	} T_t;

	const T_t TH[2] = {
		{.min = 200, .max = 400}, // T0H
		{.min = 700, .max = 900}, // T1H
	};

	const T_t TL[2] = {
		{.min = 750, .max = 950}, // T0L
		{.min = 350, .max = 550}, // T1L
	};

	// Additionally, there is need for adding a long enough "0" output before we start clocking out data.
	// The SPI peripheral in use seems to always set the MOSI high when not clocking out data, so we have
	// to start clocking out some zeros first.
	// Skipping this will result in 1-bit offset in the bit shifting. If you see random flashes
	// in your LEDs strip and/or the first LED remaning on when set to off
	// and/or the others have the colors "slightly" wrong, this could be the reason.
	// This could be avoided if the MOSI was to be LOW when idle, but this would require
	// an external transistor to invert it.
	static constexpr double kSpiLeadingZerosNs = 50000; // determined empirically
	static constexpr unsigned int kSpiLeadingZeros = 0.5 + kSpiLeadingZerosNs / kSpiPeriodNs;
	static bool readBitField(const uint8_t* data, uint32_t offset);
	static void writeBitField(uint8_t* data, uint32_t offset, bool value);
	ssize_t rgbToClk(const uint8_t* rgb, size_t numRgb, uint8_t* out, size_t numOut);
	void sendSpi(uint8_t* data, size_t length, bool verbose);
public:
	int send(const std::vector<uint8_t>& rgb, bool verbose = false);
	int setup(const std::string& device);
};
