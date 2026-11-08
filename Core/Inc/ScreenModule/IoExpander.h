/*
 * IoExpander.h
 *
 *  Created on: Oct 22, 2026
 *      Author: kuruc
 */

#ifndef INC_IOEXPANDER_H_
#define INC_IOEXPANDER_H_

/*
 * GPIO expander abstraction for digital input/output handling.
 *
 * The project uses a single I2C-backed port expander to manage push buttons,
 * LEDs and generic GPIO lines as software-friendly objects.
 */

#include <stdbool.h>

/**
 * Logical state of a debounced button.
 */
typedef enum{
  eIOE_BUTTON_RELEASED = 0u,
  eIOE_BUTTON_PRESSING,
  eIOE_BUTTON_PRESSED,
  eIOE_BUTTON_RELEASING
}E_BUTTON_STATE;

/**
 * Events generated while processing a button state machine.
 */
typedef enum{
  eBUTTON_PRESSED    = 0x01,
  eBUTTON_RELEASED   = 0x02,
  eBUTTON_CLICKED    = 0x04,
  eBUTTON_DOUBLE_CLICKED = 0x08,
  eBUTTON_PRESSING   = 0x10, // repetitive

}E_BUTTON_EVENTS;

typedef enum{
  eLED_BLINK_STATIC,
  eLED_BLINK_MORSE,
  eLED_BLINK_BLINK,

}E_LED_BLINK_TYPE;

/**
 * Available physical pins exposed by the port expander.
 */
typedef enum{
  eIOE_PIN_INVALID = 0,
  eIOE_PIN_P0,
  eIOE_PIN_P1,
  eIOE_PIN_P2,
  eIOE_PIN_P3,
  eIOE_PIN_P4,
  eIOE_PIN_P5,
  eIOE_PIN_P6,
  eIOE_PIN_P7,

  eIOE_PIN_MAX
}E_IOE_PINS;
typedef enum{
  eIOE_PIN_TYPE_GEN = 0,
  eIOE_PIN_TYPE_BUTTON,
  eIOE_PIN_TYPE_LED,
  eIOE_PIN_TYPE_NUM
}E_OIE_PIN_TYPES;

typedef struct{
  E_IOE_PINS ePin;
  E_OIE_PIN_TYPES eType;
  uint8_t u8Identifier; // separated for leds, buttons and for every type
}C_PIN_CONFIG;

typedef struct{
  E_IOE_PINS ePin;
  bool boActiveOnHigh;
  uint16_t u16PressingJitterTime;
  uint16_t u16ReleasingJitterTime;
  uint16_t u16PressingStartDelayTime;
  uint16_t u16PressingPeriodTime;
  uint16_t u16MinClickTime;
  uint16_t u16MaxClickTime;
  uint16_t u16MaxBetweenClicksTime;
}C_PIN_CONFIG_BUTTON;

typedef struct{
  E_IOE_PINS ePin;
  bool boActiveOnHigh;
}C_PIN_CONFIG_LED;

typedef struct{
  uint8_t u8Button;
  E_BUTTON_EVENTS eEvent;
}C_BUTTONS_EVENT;

/**
 * Resets the port expander state and clears all dynamic tracking data.
 */
bool boIOE_Init();

/**
 * Registers a button using a hardware pin and timing configuration.
 */
bool boAddButton(uint8_t u8Id, C_PIN_CONFIG_BUTTON* pcConfig);

/**
 * Registers a LED object and binds it to a physical expander pin.
 */
bool boAddLed(uint8_t u8Id, C_PIN_CONFIG_LED* pcConfig);

/**
 * Registers a generic configurable pin for non-button, non-LED use cases.
 */
bool boAddGenericPin(uint8_t u8Id, E_IOE_PINS ePin);

/**
 * Reads the expander state, updates button/LED logic and writes back changes.
 */
void vIOE_Update();

/* Led control */

bool boIOE_SetLedBlink(uint8_t u8Id, uint16_t u16BlinkSpeed);

bool boIOE_SetLedMorse(uint8_t u8Id, const char *pchMorseCode, uint16_t u16Speed);


bool boIOE_SetLedStatic(uint8_t u8Id, bool boOn);

/* Button state and events */

E_BUTTON_STATE eIOE_GetButtonState(uint8_t u8Id);

uint16_t u16IOE_GetButtonEvents(uint8_t u8Id);

uint16_t u16IOE_GetAndClearAllButtonEvents(uint8_t u8Id);

void vIOE_ClearButtonEvents(uint8_t u8Id, uint16_t u16Mask);

C_BUTTONS_EVENT* pcIOE_GetEvents(uint8_t* u8Len);

#endif /* INC_IOEXPANDER_H_ */
