/**
 * \file TinyDHT.h
 *
 * attiny84/85 driver to read temperature and humidity from a DHT22/11 sensor.
 *
 * This is a port of Adafruit's TinyDHT code from Arudino to
 * straight avr-libc.
 *
 * The original file header was:
 *
 *     TinyDHT library 
 *     Integer version of the Adafruit DHT library for the
 *     Trinket and Gemma mini microcontrollers
 *     MIT license
 *     written by Adafruit Industries
 */

#ifndef DHT_H
#define DHT_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

//! \defgroup SENSOR Sensor type
//!
//! Constants for setting your sensor model
//!
//! @{
#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21
//! @}

#define BAD_HUM    -1  //!< Bad humitidy reading
#define BAD_TEMP -999  //!< Bad temperature reading

#define DHT_HIGH_THRESHOLD 13

//! Used to provide a temperature scale to `dht_read_temperature()`.
typedef enum TEMPSCALE {C, F} TEMPSCALE;

//! Return type of `dht_read_humidity()` function
typedef int8_t dht_humidity_t;

//! Return type of `dht_read_temperature()` function
typedef int16_t dht_temperature_t;

//! Reperesents a single DHT sensor
typedef struct DHT {
    bool valid;         //!< `true` if most recent DHT read was successful
    uint8_t data[5];    //!< data collected from DHT sensor
    uint8_t pin,        //!< pin to which DHT sensor is attached
            type;       //!< one of (`DHT22`, `DHT21`, `DHT21`, `AM2301`)
#ifdef DHT_DEBUG
    uint8_t debug[50];
#endif
} DHT;

//! Initialize a new DHT object
void dht_new(DHT *dht, uint8_t pin, uint8_t type);

//! Prepare DHT for reading. Call this once before you start
//! reading values.
void dht_begin(DHT *dht);

//! Read values from the DHT sensor. This is based on the timing
//! specifications found in
//! <https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf>
bool dht_read(DHT *dht);

//! Return temperature component of previous `dht_read` operation.
dht_temperature_t dht_read_temperature(DHT *dht, TEMPSCALE scale);

//! Return humidity component of previous `dht_read` operation.
dht_humidity_t dht_read_humidity(DHT *dht);

//! Convert Celsius to Farenheit
int16_t convertCtoF(int16_t);

#endif
