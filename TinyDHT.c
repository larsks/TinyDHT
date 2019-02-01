/**
 * \file TinyDHT.c
 *
 * attiny84/85 driver to read temperature and humidity from a DHT22/11 sensor.
 *
 * This started out as a simple port of Adafruit's TinyDHT code from Arudino to
 * straight avr-libc, but it didn't worked and I ended up making a number of
 * changes to the code.
 *
 * The original file header was:
 *
 *     TinyDHT library 
 *     Integer version of the Adafruit DHT library for the
 *     Trinket and Gemma mini microcontrollers
 *     MIT license
 *     written by Adafruit Industries
 */

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "TinyDHT.h"

//! \defgroup PINCONFIG Pin Configuration
//!
//! TinyDHT uses pin `PORTB0` by default. If you can select a new pin
//! on `PORTB` by simply passing a new pin value to the `dht_new()`
//! function.
//!
//! If you wish to use a pin on `PORTA`, the code has to use a
//! different register to set the pin mode (`DDRA`), to set the pin 
//! value (`PORTA`), and to read the pin value (`PINA`).  Rather than
//! having you redefine them individually, you can set `DHTPORTNAME`
//! to `A`:
//!
//!     #define DHTPORTNAME 'A'
//!
//! This will set `DHTPORTREG`, `DHTPINREG`, and `DHTDDR` to the
//! appropriate values for a pin on `PORTA`.
//!
//! @{
#ifndef DHTPORTNAME
#define DHTPORTNAME 'B'
#endif

#if DHTPORTNAME == 'B'
#define DHTPORTREG PORTB
#define DHTPINREG PINB
#define DHTDDR DDRB
#else
#define DHTPORTREG PORTA
#define DHTPINREG PINA
#define DHTDDR DDRA
#endif
//! @}

//! A boolean expression that is `true` if `dht->pin` is low
#define SIGNAL_LOW (!(DHTPINREG & (1<<dht->pin)))
//! A boolean expression that is `true` if `dht->pin` is high
#define SIGNAL_HIGH (DHTPINREG & (1<<dht->pin))

void dht_new(DHT *dht, uint8_t pin, uint8_t type) {
    dht->pin = pin;
    dht->type = type;
    dht->valid = false;
}

void dht_begin(DHT *dht) {
    DHTDDR &= ~(1<<dht->pin);   // configure dht->pin as input
    DHTPORTREG |= 1<<dht->pin;  // enable internal pull-up
    dht->valid = false;
}

dht_temperature_t dht_read_temperature(DHT *dht, TEMPSCALE scale) {
    int16_t f;

    if (dht->valid) {
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
    if (dht->valid) {
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
    uint8_t counter = 0;

    // zero out data field
    dht->data[0] = dht->data[1] = dht->data[2] = dht->data[3] = dht->data[4] = 0;

    // pull low for 5 ms
    DHTDDR |= 1<<dht->pin;          // configure dht->pin as output
    DHTPORTREG &= ~(1<<dht->pin);
    _delay_ms(5);
    
    // Perform our reads inside an `ATOMIC_BLOCK` to prevent interrupts
    // from disrupting the timing.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // pull high for 30 us
        DHTPORTREG |= 1<<dht->pin;
        _delay_us(30);

        DHTDDR &= ~(1<<dht->pin);       // configure dht->pin as input

        /*
         * Wait for start signal
         */

        // signal goes low for ~ 80us
        counter = 0;
        while (SIGNAL_LOW) {
            if (counter++ > 100) goto failed;
            _delay_us(1);
        }

        // signal goes high ~ 80us
        counter = 0;
        while (SIGNAL_HIGH) {
            if (counter++ > 100) goto failed;
            _delay_us(1);
        }

        /*
         * Start receiving data
         */

        // the DHT will response with 2 bytes of temperature data,
        // 2 bytes of humidity data, and a 1 byte checksum, for a total
        // of 5 bytes.
        for (int i=0; i<5; i++) {

            // collect the bits
            for (int j=0; j<8; j++) {

                // signal goes low for ~ 50us to indicate start of bit
                counter = 0;
                while (SIGNAL_LOW) {
                    if (counter++ > 100) goto failed;
                    _delay_us(1);
                }

                // signal goes high to indicate bit value:
                // ~30us = 0
                // ~70us = 1
                counter = 0;
                while (SIGNAL_HIGH) {
                    if (counter++ > 100) goto failed;
                    _delay_us(1);
                }

                dht->data[i] <<= 1;
                dht->data[i] |= (counter > 12) ? 1 : 0;
            }

        }
    }

    // verify checksum
    if (dht->data[4] != ((
                    dht->data[0] +
                    dht->data[1] +
                    dht->data[2] +
                    dht->data[3]) & 0xFF)) {
        goto failed;
    }

    dht->valid = true;
    return true;

failed:
    return false;
}
