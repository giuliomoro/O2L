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
#include <Bela.h>
#include <AddressableLeds.h>
#include <vector>
#include <libraries/Trill/Trill.h>
#include <cmath>
#include <MiscUtilities.h>

static Trill gTrill;
static AddressableLeds gLeds;
static constexpr uint8_t kBytesPerRgb = AddressableLeds::kBytesPerRgb;

void task(void*) {
	uint8_t kNumLeds = 16;
	std::vector<uint8_t> rgb(kNumLeds * kBytesPerRgb);
	while(!Bela_stopRequested())
	{
		// draw the Trill Bar's location on a strip of addressable LEDs
		gTrill.readI2C();
		size_t numTouches = gTrill.getNumTouches();
		std::fill(rgb.begin(), rgb.end(), 0);
		for(size_t n = 0; n < numTouches; ++n)
		{
			float size = gTrill.touchSize(n);
			float location = gTrill.touchLocation(n); 
			// find nearest LED
			size_t quantisedLocation = std::round(location * (kNumLeds - 1));
			size_t c = n % kBytesPerRgb; //first touch on red, second touch on green, third touch on blue etc
			c = 0;
			uint8_t intensity = std::min(255.f, size * 255); // clip intensity to 255 (max)
			rgb[quantisedLocation * kBytesPerRgb + c] = intensity;
		}
		gLeds.send(rgb, 0);
		usleep(10000);
	}
}

bool setup(BelaContext *context, void *userData)
{
	if(gTrill.setup(1, Trill::BAR, 0x22))
		return false;
	if(gLeds.setup("/dev/spidev2.1"))
		return false;
	// this one is only needed on Bela and requires the mods mentioned above
	// It will print an error on BelaMini, but nothing to worry about
	PinmuxUtils::set("P9_30", "spi");
	Bela_runAuxiliaryTask(task);
	return true;
}

void render(BelaContext *context, void *userData)
{
// TODO: set elements of `colors`
}

void cleanup(BelaContext *context, void *userData)
{}
