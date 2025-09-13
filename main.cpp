// On BelaMini, this runs out of the box using P2.25
//
// On Bela, this runs on P8.35
// On BelaMini, you have to modify the pin and/or bank in ws281x.h,s
// for instance GPIO_PIN 20 would be P1.20
#include "pixel.hpp"
#include <vector>
#include <cmath>
#include <MiscUtilities.h>
#include <libraries/OscReceiver/OscReceiver.h>
#include <signal.h>

const int gLocalPort = 7562; //port for incoming OSC messages
uint8_t kNumLeds = 225; // number of LEDs on the strip

static PixelBone_Pixel strip(kNumLeds);
static OscReceiver oscReceiver;
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
static std::vector<char> gRgb(kNumLeds * kBytesPerRgb);
int parseMessage(oscpkt::Message msg, const char* address, void*)
{
	oscpkt::Message::ArgReader args = msg.arg();
	enum {
		kOk = 0,
		kWrongArguments,
		kUnmatchedPattern,
	} error = kOk;
	printf("Message from %s; %s\n", address, msg.addressPattern().c_str());
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
				strip.clear();
				for (uint32_t p = 0; p < kNumLeds; p++)
				{
					size_t k = p * kBytesPerRgb;
					strip.setPixelColor(p, PixelBone_Pixel::Color(gRgb[k + 0], gRgb[k + 1], gRgb[k + 2]));
				}
				strip.show();
				strip.wait();
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

int main(int argc, char* argv[])
{
	// OSC
	oscReceiver.setup(gLocalPort, parseMessage);

	gStop = false;
	// Set up interrupt handler to catch Control-C and SIGTERM
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);
	while(!gStop)
		usleep(200000);
	return 0;
}
