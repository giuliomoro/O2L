// On BelaMini, this runs out of the box using P1.12
//
// On Bela, this only works if you sacrifice the audio input and perform a hw
// mod and load a dedicated device-tree overlay.
// Hardware mod:
// - remove the Bela cape
// - bend pin P9.30 out so that it doesn't enter the BeagleBone Black
// - insert a wire into P9.30 on the BeagleBone Black or solder it to its pad at the back of the BeagleBone Black
// - re-insert the Bela cape
// Loading device tree overlay:
// - get the latest from https://github.com/BelaPlatform/bb.org-overlays into /opt/bb.org-overlays
// - run `make all install`
// - reboot
// - use Mis
#include <AddressableLeds.h>
#include <vector>
#include <libraries/Trill/Trill.h>
#include <cmath>
#include <MiscUtilities.h>
#include <libraries/OscReceiver/OscReceiver.h>
#include <signal.h>

const int gLocalPort = 7562; //port for incoming OSC messages
uint8_t kNumLeds = 23; // number of LEDs on the strip

static AddressableLeds gLeds;
static OscReceiver oscReceiver;
static constexpr uint8_t kBytesPerRgb = AddressableLeds::kBytesPerRgb;

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
static std::vector<uint8_t> gRgb(kNumLeds * kBytesPerRgb);
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
	std::string baseAddr = "/leds/setRange";
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
				printf("\n");
				if(kOk == error)
					gLeds.send(gRgb);
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
	// this one is only needed on Bela and requires the mods mentioned above
	// It will print an error on BelaMini, but nothing to worry about
	PinmuxUtils::set("P9_30", "spi");

	if(gLeds.setup("/dev/spidev2.1"))
		return false;
	// OSC
	oscReceiver.setup(gLocalPort, parseMessage);

	gStop = false;
	// Set up interrupt handler to catch Control-C and SIGTERM
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);
	while(!gStop)
		usleep(200000);
	return true;
}
