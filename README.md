An OSC to addressable LED (aka Neopixel or WS2812b) bridge for Linux using Bela's OscReceiver and UdpServer classes.

--------

An example of a program that can run stand-alone on Bela or a BeagleBone-series single-board computer and receive OSC and set LEDs on a Neopixel-like strip.
It comes with a Pd example of a 16-segment VU-meter and a basic SuperCollider example.

It uses a PRU to bitbang the neopixel protocol on PRU GPIO pin. See ws281x.p for the pin in use and options for your board.
A suitable device tree overlay must be loaded to set the correct settings for the pin.
 
Running this without external electronics requires, in principle, a bit of luck, because according to most NeoPixel-style
datasheets, the 3.3V signal from Bela is not high enough for the LED's data line when the LEDs are powered from 5V.
You may need additional electronics, either a power diode or a signal switching transistor, as noted
[here](https://forum.bela.io/d/3001-control-neopixel-with-pure-data/25).
**However** we have found in practice that in the real world we yet have to find a strip that doesn't work with the 3.3V digital
signals coming from the GPIO pins, so you could start without an external level shifter and then only add it in if the LEDs
don't perform reliably without it.

Some tweaking to the code to achieve the desired signal timing may be required depending on the datasheet of the specific
device you are using, see [here](https://forum.bela.io/d/3001-control-neopixel-with-pure-data/25) for some examples of that.

You should test O2L (and any of the above mods and electronics) by running a Pd or Sc patch such as the enclosed
ones on the host, while running this program on the board from the IDE. Once that setup is tested and works fine,
in order to run a regular Bela program for audio/sensor processing alongside O2L, you will need to run the O2L
program as a service on the board. See [here](https://learn.bela.io/using-bela/bela-techniques/running-a-program-as-a-service/)
for details on that.
