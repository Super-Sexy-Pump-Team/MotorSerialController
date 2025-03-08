#include "CD4051.hpp"

uint8_t _MUX_CHANNELS = 0;
uint8_t _MUX_INH = 0;
uint8_t _MUX_A = 0;
uint8_t _MUX_B = 0;
uint8_t _MUX_C = 0;

/**
 * @brief Initialize the MUX
 * @param muxInh MUX INH pin
 * @param muxA MUX A pin
 * @param muxB MUX B pin
 * @param muxC MUX C pin
 * @retval void
 */
void MUX_init(uint8_t muxInh, uint8_t muxA, uint8_t muxB, uint8_t muxC){
  if (muxInh == 0) return;
  _MUX_INH = muxInh;
  pinMode(_MUX_INH, OUTPUT);

  if (muxA == 0) return;
  _MUX_A = muxA;
  _MUX_CHANNELS = 2;
  pinMode(_MUX_A, OUTPUT);

  if (muxB == 0) return;
  _MUX_B = muxB;
  _MUX_CHANNELS = 4;
  pinMode(_MUX_B, OUTPUT);

  if (muxC == 0) return;
  _MUX_C = muxC;
  _MUX_CHANNELS = 8;
  pinMode(_MUX_C, OUTPUT);

}

/**
 * @brief Select a channel on the MUX
 * @param channel Channel to select (0-7)
 * @retval void
 */
void MUX_select(uint8_t channel){
  if (channel >= _MUX_CHANNELS) return;

  if (_MUX_CHANNELS == 0) return;

  digitalWrite(_MUX_INH, LOW);

  digitalWrite(_MUX_A, (channel & 0x01) ? HIGH : LOW);
  if (_MUX_CHANNELS <= 2) return;

  digitalWrite(_MUX_B, (channel & 0x02) ? HIGH : LOW);
  if (_MUX_CHANNELS <= 4) return;

  digitalWrite(_MUX_C, (channel & 0x04) ? HIGH : LOW);
  
}

/**
 * @brief Disable the MUX output
 * @param void
 * @retval void
 */
void MUX_disable(void){
  if (_MUX_CHANNELS == 0) return;
  digitalWrite(_MUX_INH, HIGH);
}