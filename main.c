#include <stdio.h>

#include "serial.h"
#include "TinyDHT.h"

DHT dht;

int main() {
    dht_temperature_t t;
    dht_humidity_t h;
    char buf[40];
    millis_t now;

    serial_init();
    serial_enable();

    serial_println("*");
    serial_println("* DHT22 Sensor Example");
    serial_println("*");

    dht_new(&dht, PORTB1, DHT22);
    dht_begin(&dht);

    while (1) {
        if (! dht_read(&dht)) {
            sprintf(buf, "[%lu] read failed!", millis());
            serial_print(buf);
            continue;
        }

        t = dht_read_temperature(&dht, F);
        h = dht_read_humidity(&dht);
        sprintf(buf, "[%lu] t: %d h: %d", millis(), t, h);
        serial_println(buf);
        
        now =  millis();
        while (millis() - now <= 2000);
    }
    serial_disable();
}
