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
    DDRA |= 1<<PORTA0;
    PORTA &= ~(1<<PORTA0);
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

#define SIGNAL_LOW (!(DHTPINREG & (1<<dht->pin)))
#define SIGNAL_HIGH (DHTPINREG & (1<<dht->pin))

bool dht_read(DHT *dht) {
    uint8_t counter = 0;
    millis_t currenttime;

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

    PORTA |= 1<<PORTA0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // pull low for 5 ms
        DHTDDR |= 1<<dht->pin;          // configure dht->pin as output
        DHTPORTREG &= ~(1<<dht->pin);
        delay(5);
    
        // pull high for 30 us
        DHTPORTREG |= 1<<dht->pin;
        delayMicroseconds(30);

        DHTDDR &= ~(1<<dht->pin);       // configure dht->pin as input

        // wait for start signal

        // signal goes low for ~ 80us
        counter = 0;
        while (SIGNAL_LOW) {
            if (counter++ > 100) goto failed;
            delayMicroseconds(1);
        }

        // signal goes high ~ 80us
        counter = 0;
        while (SIGNAL_HIGH) {
            if (counter++ > 100) goto failed;
            delayMicroseconds(1);
        }

        // start receiving data bits
        for (int i=0; i<5; i++) {
            for (int j=0; j<8; j++) { 

                // signal goes low to indicate start of bit
                counter = 0;
                while (SIGNAL_LOW) {
                    if (counter++ > 100) goto failed;
                    delayMicroseconds(1);
                }

                // signal goes high to indicate bit value:
                // ~30us = 0
                // ~70us = 1
                counter = 0;
                while (SIGNAL_HIGH) {
                    if (counter++ > 100) goto failed;
                    delayMicroseconds(1);
                }

                dht->data[i] <<= 1;
                dht->data[i] |= (counter > 15) ? 1 : 0;
                dht->debug[(8*i) + j] = counter;
            }
        }
    }

    if (dht->data[4] != ((
                    dht->data[0] +
                    dht->data[1] +
                    dht->data[2] +
                    dht->data[3]) & 0xFF)) {
        goto failed;
    }

succeeded:
    PORTA &= ~(1<<PORTA0);
    return true;

failed:
    PORTA &= ~(1<<PORTA0);
    return false;
}
