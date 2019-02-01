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

    dht_new(&dht, PORTB1, DHT22, DHT_DEFAULT_COUNT); 
    dht_begin(&dht);

    while (1) {
        if (dht_read(&dht)) {
            serial_println("read okay");
        } else {
            serial_println("read failed");
        }
        for (int i=0; i<6; i++) {
            sprintf(buf, "data[%d] = %d", i, dht.data[i]);
            serial_println(buf);
        }
        delay(2000);
    }
    serial_disable();
}
