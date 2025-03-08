#ifndef MUX_H
#define MUX_H

#include <Arduino.h>

extern uint8_t _MUX_CHANNELS;
extern uint8_t _MUX_INH;
extern uint8_t _MUX_A;
extern uint8_t _MUX_B;
extern uint8_t _MUX_C;

void MUX_init(uint8_t muxInh, uint8_t muxA, uint8_t muxB, uint8_t muxC);

void MUX_select(uint8_t channel);

void MUX_disable(void);

#endif