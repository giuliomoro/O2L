// This uses the PRU pin shown at the top of ws281x.p

#include "pixel.hpp"
#include <vector>
#include <cmath>
#include <MiscUtilities.h>
#include <signal.h>

#define USE_OSC

uint8_t kNumLeds = 225; // number of LEDs on the strip
const int gVerbose = 1;

static PixelBone_Pixel strip(kNumLeds);
static constexpr uint8_t kBytesPerRgb = 3;

static bool gStop;
// Handle Ctrl-C by requesting that the audio rendering stop
static void interrupt_handler(int var)
{
	gStop = true;
}

template <typename T>
static uint8_t clipForLed(T val)
{
	return val > 255 ? 255 : val;
}
static void writeLeds(const std::vector<char>& rgb, PixelBone_Pixel& strip)
{
	strip.clear();
	for (uint32_t p = 0; p < kNumLeds; p++)
	{
		size_t k = p * kBytesPerRgb;
		if(gVerbose >= 2) {
			printf("{%d %d %d}, ", rgb[k + 0], rgb[k + 1], rgb[k + 2]);
		}
		strip.setPixelColor(p, PixelBone_Pixel::Color(rgb[k + 0], rgb[k + 1], rgb[k + 2]));
	}
	if(gVerbose >= 2)
		printf("\n");
	strip.show();
	strip.wait();
}
static std::vector<char> gRgb(kNumLeds * kBytesPerRgb);

#ifdef USE_OSC
const int gLocalPort = 7562; //port for incoming OSC messages
#include <libraries/OscReceiver/OscReceiver.h>
static OscReceiver oscReceiver;
int parseMessage(oscpkt::Message msg, const char* address, void*)
{
	oscpkt::Message::ArgReader args = msg.arg();
	enum {
		kOk = 0,
		kWrongArguments,
		kUnmatchedPattern,
	} error = kOk;
	static size_t count = 0;
	if(gVerbose >= 1) {
		printf("Message %zu from %s; %s\n", count, address, msg.addressPattern().c_str());
		fflush(stdout);
	}
	count++;
	// check state (non-display) messages first
	std::string baseAddr = "/leds/setRaw";
	if (msg.partialMatch(baseAddr)) {
		enum {
			kR,
			kG,
			kB,
			kRgb,
		};
		size_t color;
		if(msg.match(baseAddr + "/r"))
			color = kR;
		else if(msg.match(baseAddr + "/g"))
			color = kG;
		else if(msg.match(baseAddr + "/b"))
			color = kB;
		else if(msg.match(baseAddr + "/rgb"))
			color = kRgb;
		else
			error = kUnmatchedPattern;
		if(kOk == error)
		{
			if(args.isBlob()) {
				args.popBlob(gRgb);
			} 
			else {
				size_t start;
				args.popNumber(start);
				start *= kBytesPerRgb;
				float gain;
				args.popNumber(gain);
				size_t numArgs = args.nbArgRemaining();
				if(!numArgs || (kRgb == color && numArgs % kBytesPerRgb) || !args.isOk())
					error = kWrongArguments;
				else {
					int n = 0;
					while(args.nbArgRemaining() && n < gRgb.size())
					{
						if(args.isNumber()) {
							float val;
							args.popNumber(val);
							if(!args.isOk()) {
								error = kWrongArguments;
								break;
							}
							uint8_t ledValue = clipForLed(val * gain);
							// now use the retrieved value
							if(kRgb == color)
							{
								// in kRgb mode, set each color per each LED in order
								gRgb[start + n] = ledValue;
								++n;
							} else {
								// in monochrome mode, set the corresponding color
								// and zero out the rest
								for(size_t c = 0; c < kBytesPerRgb; ++c)
								{	
									if(color == c)
										gRgb[start + n] = ledValue;
									else
										gRgb[start + n] = 0;
									++n;
								}
							}
						}
						else args.pop(); // ingore argument
					}
				}	
			}
			if(kOk == error)
			{
				writeLeds(gRgb, strip);
			}
		}
	} else
		error = kUnmatchedPattern;
	int ret = 0;
	if(error)
	{
		std::string str;
		switch(error){
			case kUnmatchedPattern:
				str = "no matching pattern available\n";
				break;
			case kWrongArguments:
				str = "unexpected types and/or length\n";
				break;
			case kOk:
				str = "";
				break;
		}
		fprintf(stderr, "An error occurred with message to: %s: %s\n", msg.addressPattern().c_str(), str.c_str());
		ret = 1;
	}
	return ret;
}
#endif // USE_OSC

int main(int argc, char* argv[])
{
#ifdef USE_OSC
	oscReceiver.setup(gLocalPort, parseMessage);
#endif

	gStop = false;
	// Set up interrupt handler to catch Control-C and SIGTERM
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);
	while(!gStop) {
#ifndef USE_OSC
		// demo colors
		static unsigned int count = 0;
		for(unsigned int n = 0; n < kNumLeds; ++n) {
			unsigned int mod = count % 3;
			uint8_t r = (0 == mod) * 10;
			uint8_t g = (1 == mod) * 10;
			uint8_t b = (2 == mod) * 10;
			gRgb[n * kBytesPerRgb + 0] = r;
			gRgb[n * kBytesPerRgb + 1] = g;
			gRgb[n * kBytesPerRgb + 2] = b;
		}
		count++;
		writeLeds(gRgb, strip);
#endif // !USE_OSC
		usleep(200000);
	}
	return 0;
}

void setup() {}
void loop() {}
