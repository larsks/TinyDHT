#include <stdio.h>

#include "serial.h"
#include "TinyDHT.h"

DHT dht;

int main() {
    dht_temperature_t t;
    dht_humidity_t h;
    millis_t now;

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
            sprintf(buf, "[%lu] read failed!", millis());
            serial_println(buf);
            goto next_measure;
        }

        // get sensor values and write them out
        t = dht_read_temperature(&dht, F);
        h = dht_read_humidity(&dht);
        sprintf(buf, "[%lu] t: %d h: %d", millis(), t, h);
        serial_println(buf);

next_measure:

        // wait two seconds before next read
        now =  millis();
        while (millis() - now <= 2000);
    }
    serial_disable();
}
