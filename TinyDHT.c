/* TinyDHT library 
   Integer version of the Adafruit DHT library for the
   Trinket and Gemma mini microcontrollers
   MIT license
   written by Adafruit Industries
   */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "millis.h"
#include "TinyDHT.h"

void dht_new(DHT *dht, uint8_t pin, uint8_t type, uint8_t count) {
    dht->pin = pin;
    dht->type = type;
    dht->count = count;
    dht->firstreading = true;
}

void dht_begin(DHT *dht) {
    DHTDDR &= ~(1<<dht->pin);   // configure dht->pin as input
    DHTPORTREG |= 1<<dht->pin;  // enable internal pull-up
    dht->lastreadtime = 0;
}

dht_temperature_t dht_read_temperature(DHT *dht, TEMPSCALE scale) {
    int16_t f;

    if (dht_read(dht)) {
        switch (dht->type) {
            case DHT11:
                f = (int16_t) dht->data[2];
                if(scale == F)
                    f = convertCtoF(f); 

                return f;
            case DHT22:
            case DHT21:
                f = (int16_t)(dht->data[2] & 0x7F);
                f *= 256;
                f += (int16_t) dht->data[3];
                f /= 10;
                if (dht->data[2] & 0x80)
                    f *= -1;
                if(scale == F)
                    f = convertCtoF(f);

                return f;
        }
    }
    
    return BAD_TEMP;
}

int16_t convertCtoF(int16_t c) {
    return (c * 9) / 5 + 32;
}

dht_humidity_t dht_read_humidity(DHT *dht) {  //  0-100 %
    uint8_t f;
    uint16_t f2;  // bigger to allow for math operations
    if (dht_read(dht)) {
        switch (dht->type) {
            case DHT11:
                f = dht->data[0];
                return f;
            case DHT22:
            case DHT21:
                f2 = (uint16_t) dht->data[0];
                f2 *= 256;
                f2 += dht->data[1];
                f2 /= 10;
                f = (uint8_t) f2;
                return f;
        }
    }

    return BAD_HUM;
}

bool dht_read(DHT *dht) {
    uint8_t laststate = 1<<dht->pin;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    millis_t currenttime;

    // pull the pin high and wait 250 milliseconds
    DHTPORTREG |= 1<<dht->pin;
    delay(250);

    currenttime = millis();
    if (currenttime < dht->lastreadtime) {
        // ie there was a rollover
        dht->lastreadtime = 0;
    }
    if (!dht->firstreading && ((currenttime - dht->lastreadtime) < 2000)) {
        return true;
    }
    dht->firstreading = false;
    dht->lastreadtime = millis();

    dht->data[0] = dht->data[1] = dht->data[2] = dht->data[3] = dht->data[4] = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // pull line low for 5 ms
        DHTDDR |= 1<<dht->pin;
        DHTPORTREG &= ~(1<<dht->pin);
        delay(5);
    
        // pull high for 30 seconds
        DHTPORTREG |= 1<<dht->pin;
        delayMicroseconds(30);

        // configure dht->pin as input
        DHTDDR &= ~(1<<dht->pin);

        // read in timings
        for ( i=0; i< MAXTIMINGS; i++) {
            counter = 0;
            while ((DHTPINREG & (1<<dht->pin)) == laststate) {
                counter++;
                delayMicroseconds(1);
                if (counter == 255) {
                    break;
                }
            }
            laststate = DHTPINREG & 1<<dht->pin;

            if (counter == 255) break;

            // ignore first 3 transitions
            if ((i >= 4) && (i%2 == 0)) {
                // shove each bit into the storage bytes
                dht->data[j/8] <<= 1;
                if (counter > dht->count)
                    dht->data[j/8] |= 1;
                j++;
            }

        }
    }

    // check we read 40 bits and that the checksum matches
    if ((j >= 40) && 
            (dht->data[4] == ((dht->data[0] +
                               dht->data[1] +
                               dht->data[2] +
                               dht->data[3]) & 0xFF)) ) {
        return true;
    }

    return false;
}
