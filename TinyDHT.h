#ifndef DHT_H
#define DHT_H

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <util/delay.h>

/* Tiny DHT library
Uses integer math to save space on Trinket/Gemma

MIT license
written by Adafruit Industries
*/

#define delay(x) (_delay_ms(x))
#define delayMicroseconds(x) (_delay_us(x))

// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

// NAN code in DHT library takes space, define bad values here
#define BAD_HUM    -1  // Bad humitidy reading
#define BAD_TEMP -999  // Bad temperature reading

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

#define DHT_DEFAULT_COUNT 6

typedef enum TEMPSCALE {C, F} TEMPSCALE;
typedef int8_t dht_humidity_t;
typedef int16_t dht_temperature_t;

typedef struct DHT {
    bool firstreading;
    uint8_t data[6];
    uint8_t pin, type, count;
    unsigned long lastreadtime;
} DHT;

void dht_new(DHT *dht, uint8_t pin, uint8_t type, uint8_t count);
void dht_begin(DHT *dht);
dht_temperature_t dht_read_temperature(DHT *dht, TEMPSCALE scale);
dht_humidity_t dht_read_humidity(DHT *dht);
int16_t convertCtoF(int16_t);
bool dht_read(DHT *dht);

#endif
