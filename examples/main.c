#include <stdio.h>
#include <util/delay.h>

#include "serial.h"
#include "TinyDHT.h"

DHT dht;

int main() {
    dht_temperature_t t;
    dht_humidity_t h;

    // for string formatting
    char buf[40];

    // enable serial support
    serial_init();
    serial_enable();

    serial_println("*");
    serial_println("* TinyDHT Example");
    serial_println("*");

    // initialize the DHT driver
    dht_new(&dht, PORTB1, DHT22);
    dht_begin(&dht);

    while (1) {
        // read sensor
        if (! dht_read(&dht)) {
            serial_println("* read failed!");
            goto next_measure;
        }

        // get sensor values and write them out
        t = dht_read_temperature(&dht, F);
        h = dht_read_humidity(&dht);
        sprintf(buf, "t: %d h: %d", t, h);
        serial_println(buf);

next_measure:
        _delay_ms(3000);
    }
    serial_disable();
}
