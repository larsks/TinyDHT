This is a fork of [Adafruit's TinyDHT library][1] that you can compile using `avr-gcc`.  The library returns integer values to save space on the memory constrained ATTiny85 by not requiring the floating point library.  It has been tested with a DHT22 and both an attiny84 and an attiny85.

[1]: https://github.com/adafruit/TinyDHT

## Differences from adafruit/TinyDHT

A significant difference from Adafruit's version of the code is that I have removed the use of the `millis()` function, since this is not available out of the box when using straight C. This makes it the responsibility of the calling code to ensure that it doesn't attempt to read the sensor more often than once every two seconds. This changes the calling conventions, since it is no longer possible for `dht_read_temperature()` and `dht_read_humidity()` to Do the Right Thing without outside help.  To read values from the sensor, you code should look something like this:

    DHT dht;
    dht_temperature_t t;
    dht_humidity_t h;

    if (dht_read(&dht)) {
      // specify C to return the temperature in Celsius,
      // F to return the temperature in Farenheit
      t = dht_read_temperature(&dht, C);
      h = dht_read_humidity(&dht);
    }

## A warning about clock settings

This code doesn't appear to work at 1Mhz.  You will need to configure your attiny device to run at 8Mhz or faster.

## Examples

See the `examples/` directory for some sample code that will periodically read the temperature and humidity and report them via a serial connection.  This makes use of my [avr-serial][] library.

[avr-serial]: https://github.com/larsks/avr-serial

## Additional documentation

The Doxygen docs for this code are available online at <http://oddbit.com/TinyDHT/>.
