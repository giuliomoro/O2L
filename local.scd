// OSC format is  as simple as sending to `/leds/setRaw/rgb` a set of floats or ints
// `offset gain led0r led0g led0b led1r led1g led1b...`,
// where `offset` is the offset from the first LED,
// gain is an overall gain control and then the rest are the values for each LED's red,
// green or blue components.
// If you want to send single-component data, use, e.g.:`/leds/setRaw/g` with data:
// `offset gain led0g led1g led2g` (same for `r` or `b`);

// simple example for rgb:
~displayOSC = NetAddr.new("bela.local", 7562);
// first pixel is green, second pixel is blue
~displayOSC.sendMsg('/leds/setRaw/rgb', 0, 1,    0, 255, 0,    0, 0, 255);
 //Values for LEDs should normally be betwen  0 to 255 and gain should be between 0 and 1.
 // However, you may find it easier to have LED values betwen 0 and 1 and
 // having gain between 0 and 255. Either way, these two values are together before being
 // sent to the pixel. So the below is equivalent to the above:
~displayOSC.sendMsg('/leds/setRaw/rgb', 0, 255,  0, 1.0, 0,    0, 0, 1.0);
