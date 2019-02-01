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
            serial_println(buf);
            goto next_measure;
        }

        t = dht_read_temperature(&dht, F);
        h = dht_read_humidity(&dht);
        sprintf(buf, "[%lu] t: %d h: %d", millis(), t, h);
        serial_println(buf);

next_measure:
#ifdef DHT_DEBUG
        for (int i=0; i<5; i++) {
            for (int j=0; j<8; j++) {
                sprintf(buf, "%02d ", dht.debug[i*5 + j]);
                serial_print(buf);
            }
            serial_println("");
        }
#endif

        now =  millis();
        while (millis() - now <= 2000);
    }
    serial_disable();
}
