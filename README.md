An OSC to addressable LED (aka Neopixel or WS2812b) bridge for Linux using Bela's OscReceiver and UdpServer classes.

--------

An example of a program that can run stand-alone on Bela (or any Linux embedded machine with an spidev-compatible SPI port),
receive OSC and set LEDs on a Neopixel-like strip.
It comes with a Pd example of a 16-segment VU-meter and a basic SuperCollider example.

If running this on BelaMini, use P2.25 as the data line to the NeoPixels. On Bela, instead, you need to make a possibly
risky modification to free up the relevant pin P9.30, which will in turn disable the audio inputs. Such mod involves
bending out pin P9.30 (which is normally the data line coming from the codec's ADC). This is a potentially destructive
operation if not done carefully: it may be complicated to bend the pin back in place if you need the audio input again
in the future. If possible, try bending it out without causing a sharp angle, e.g.: using a pair of pliers to shape it
into a more rounded shape. If this pins should break now or in the future, you can probably bypass it with a jumper wire
soldered to the back of the beaglebone ... not ideal but better than nothing. You'll also need to install a modified
device tree overlay on your board. This is explained in some more detail at the top of the main.cpp file.

Either way, running this without external electronics requires a bit of luck, because according to the most NeoPixel-style
datasheets, the 3.3V signal from Bela is not high enough for the LED's data line when the LEDs are powered from 5V.
You may need additional electronics, either a power diode or a signal switching transistor, as noted
[here](https://forum.bela.io/d/3001-control-neopixel-with-pure-data/25).
Some tweaking to the code to achieve the desired signal timing may be required depending on the datasheet of the specific
device you are using, see [here](https://forum.bela.io/d/3001-control-neopixel-with-pure-data/25) for some examples of that.

You should test O2L (and any of the above mods and electronics) by running a Pd or Sc patch such as the enclosed
ones on the host, while running this program on the board from the IDE. Once that setup is tested and works fine,
in order to run a regular Bela program for audio/sensor processing alongside O2L, you will need to run the O2L
program as a service on the board. See [here](https://learn.bela.io/using-bela/bela-techniques/running-a-program-as-a-service/)
for details on that.