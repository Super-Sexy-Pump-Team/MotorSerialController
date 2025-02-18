#ifndef MUX_H
#define MUX_H

#include <Arduino.h>

// MUX control pins
#define MUX_A 11
#define MUX_B 0
#define MUX_C 0

// MUX inhibit pin
#define MUX_INH 10

// Max number of channels used on the MUX (2, 4, or 8)
#define MUX_CHANNELS 2

void MUX_init(void);

void MUX_select(uint8_t channel);

void MUX_disable(void);

#endif