#include "CD4051.hpp"

/**
 * @brief Initialize the MUX
 * @param void
 * @retval void
 */
void MUX_init(void){
  pinMode(MUX_INH, OUTPUT);

  pinMode(MUX_A, OUTPUT);
  if (MUX_CHANNELS == 2) return;

  pinMode(MUX_B, OUTPUT);
  if (MUX_CHANNELS == 4) return;

  pinMode(MUX_C, OUTPUT);

}

/**
 * @brief Select a channel on the MUX
 * @param channel Channel to select (0-7)
 * @retval void
 */
void MUX_select(uint8_t channel){
  if (channel >= MUX_CHANNELS) return;

  digitalWrite(MUX_INH, LOW);

  digitalWrite(MUX_A, (channel & 0x01) ? HIGH : LOW);
  if (MUX_CHANNELS == 2) return;

  digitalWrite(MUX_B, (channel & 0x02) ? HIGH : LOW);
  if (MUX_CHANNELS == 4) return;

  digitalWrite(MUX_C, (channel & 0x04) ? HIGH : LOW);
  
}

/**
 * @brief Disable the MUX output
 * @param void
 * @retval void
 */
void MUX_disable(void){
  digitalWrite(MUX_INH, HIGH);
}