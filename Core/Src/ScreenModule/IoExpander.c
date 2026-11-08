/*
 * IoExpander.c
 *
 *  Created on: Oct 22, 2026
 *      Author: kuruc
 */

#include "i2c.h"
#include "ScreenModule/IoExpander.h"
#include <ctype.h>
#include <string.h>

/*
 * Port-expander driver.
 *
 * This module translates raw GPIO states into higher-level button, LED and
 * generic pin objects. The event queue is intentionally small and is used by
 * the screen layer to react to user input without storing history beyond the
 * current polling cycle.
 */

#define IOE_I2C &hi2c1
#define IOE_I2C_ADDR         (0x20<<1)

#define IOE_MAX_BUTTONS  8u
#define IOE_MAX_LEDS     8u
#define IOE_MAX_GEN_PINS 8u
#define BUFFERED_REG_NUM 1u

#define MORSE_SHORT           (pcLed->u16Speed)
#define MORSE_LONG            (pcLed->u16Speed * 3)
#define MORSE_SPACE           (pcLed->u16Speed * 4) //4+3=7
#define MORSE_ENTER           (pcLed->u16Speed * 10) // TODO check

extern I2C_HandleTypeDef hi2c1;

typedef struct{
  C_PIN_CONFIG_BUTTON cConfig;
  E_BUTTON_STATE eState;
  uint16_t u16Events;
  uint32_t u32Timer;
  uint16_t u16PressedCnt;
  uint8_t u8ClickedPhase;
  uint32_t u32ClickedTimer;
}C_DYNAMIC_BUTTON_DATA;

typedef struct{
  C_PIN_CONFIG_LED cConfig;
  E_LED_BLINK_TYPE eType;
  union{
    bool boOn;
    struct{ const char* pchMorseCode; uint16_t u16Speed; uint8_t u8CharIdx; uint8_t u8Phase;};
    uint16_t u16BlinkSpeed;
  };
  uint32_t u32Timer;
}C_DYNAMIC_LED_DATA;

typedef struct{
  E_OIE_PIN_TYPES eType;
  uint8_t u8Id;
}C_TYPE_AND_ID;

struct _C_MORSE_CHAR{
  uint8_t u8Len;
  bool aboLong[10];
};

struct _C_IOE_PIN{
  uint8_t u8Register;
  uint8_t u8Bit;
} acIoePins[eIOE_PIN_MAX] = {
    [eIOE_PIN_P0] = {.u8Register = 0x00, .u8Bit = 0},
    [eIOE_PIN_P1] = {.u8Register = 0x00, .u8Bit = 1},
    [eIOE_PIN_P2] = {.u8Register = 0x00, .u8Bit = 2},
    [eIOE_PIN_P3] = {.u8Register = 0x00, .u8Bit = 3},
    [eIOE_PIN_P4] = {.u8Register = 0x00, .u8Bit = 4},
    [eIOE_PIN_P5] = {.u8Register = 0x00, .u8Bit = 5},
    [eIOE_PIN_P6] = {.u8Register = 0x00, .u8Bit = 6},
    [eIOE_PIN_P6] = {.u8Register = 0x00, .u8Bit = 7},
};
uint8_t au8RegBuffer[BUFFERED_REG_NUM];

static const struct _C_MORSE_CHAR acMorse[26+10] = {
    /* Letters: index = character - 'a' */
    ['a' - 'a'] = {.u8Len = 2, .aboLong = {false, true}},              /* A .-     */
    ['b' - 'a'] = {.u8Len = 4, .aboLong = {true, false, false, false}},/* B -...   */
    ['c' - 'a'] = {.u8Len = 4, .aboLong = {true, false, true, false}}, /* C -.-.   */
    ['d' - 'a'] = {.u8Len = 3, .aboLong = {true, false, false}},       /* D -..    */
    ['e' - 'a'] = {.u8Len = 1, .aboLong = {false}},                    /* E .      */
    ['f' - 'a'] = {.u8Len = 4, .aboLong = {false, false, true, false}},/* F ..-.   */
    ['g' - 'a'] = {.u8Len = 3, .aboLong = {true, true, false}},        /* G --.    */
    ['h' - 'a'] = {.u8Len = 4, .aboLong = {false, false, false, false}},/* H ....  */
    ['i' - 'a'] = {.u8Len = 2, .aboLong = {false, false}},             /* I ..     */
    ['j' - 'a'] = {.u8Len = 4, .aboLong = {false, true, true, true}},  /* J .---   */
    ['k' - 'a'] = {.u8Len = 3, .aboLong = {true, false, true}},        /* K -.-    */
    ['l' - 'a'] = {.u8Len = 4, .aboLong = {false, true, false, false}},/* L .-..   */
    ['m' - 'a'] = {.u8Len = 2, .aboLong = {true, true}},               /* M --     */
    ['n' - 'a'] = {.u8Len = 2, .aboLong = {true, false}},              /* N -.     */
    ['o' - 'a'] = {.u8Len = 3, .aboLong = {true, true, true}},         /* O ---    */
    ['p' - 'a'] = {.u8Len = 4, .aboLong = {false, true, true, false}}, /* P .--.   */
    ['q' - 'a'] = {.u8Len = 4, .aboLong = {true, true, false, true}},  /* Q --.-   */
    ['r' - 'a'] = {.u8Len = 3, .aboLong = {false, true, false}},       /* R .-.    */
    ['s' - 'a'] = {.u8Len = 3, .aboLong = {false, false, false}},      /* S ...    */
    ['t' - 'a'] = {.u8Len = 1, .aboLong = {true}},                     /* T -      */
    ['u' - 'a'] = {.u8Len = 3, .aboLong = {false, false, true}},       /* U ..-    */
    ['v' - 'a'] = {.u8Len = 4, .aboLong = {false, false, false, true}},/* V ...-   */
    ['w' - 'a'] = {.u8Len = 3, .aboLong = {false, true, true}},        /* W .--    */
    ['x' - 'a'] = {.u8Len = 4, .aboLong = {true, false, false, true}}, /* X -..-   */
    ['y' - 'a'] = {.u8Len = 4, .aboLong = {true, false, true, true}},  /* Y -.--   */
    ['z' - 'a'] = {.u8Len = 4, .aboLong = {true, true, false, false}}, /* Z --..   */

    /* Digits: index = character - '0' + 26 */
    ['0' - '0' + 26] = {.u8Len = 5, .aboLong = {true, true, true, true, true}},   /* 0 ----- */
    ['1' - '0' + 26] = {.u8Len = 5, .aboLong = {false, true, true, true, true}},  /* 1 .---- */
    ['2' - '0' + 26] = {.u8Len = 5, .aboLong = {false, false, true, true, true}}, /* 2 ..--- */
    ['3' - '0' + 26] = {.u8Len = 5, .aboLong = {false, false, false, true, true}},/* 3 ...-- */
    ['4' - '0' + 26] = {.u8Len = 5, .aboLong = {false, false, false, false, true}},/* 4 ....- */
    ['5' - '0' + 26] = {.u8Len = 5, .aboLong = {false, false, false, false, false}},/* 5 ..... */
    ['6' - '0' + 26] = {.u8Len = 5, .aboLong = {true, false, false, false, false}},/* 6 -.... */
    ['7' - '0' + 26] = {.u8Len = 5, .aboLong = {true, true, false, false, false}}, /* 7 --... */
    ['8' - '0' + 26] = {.u8Len = 5, .aboLong = {true, true, true, false, false}},  /* 8 ---.. */
    ['9' - '0' + 26] = {.u8Len = 5, .aboLong = {true, true, true, true, false}},   /* 9 ----. */
};

C_DYNAMIC_BUTTON_DATA acButtons[IOE_MAX_BUTTONS];
C_DYNAMIC_LED_DATA acLeds[IOE_MAX_LEDS];
E_IOE_PINS aeGenPins[IOE_MAX_GEN_PINS];
C_TYPE_AND_ID acPin2Device[eIOE_PIN_MAX];
C_BUTTONS_EVENT acButtonEventList[IOE_MAX_BUTTONS * 16];
uint8_t u8ButtonEventListLen;


bool boIOE_ReadRegister(uint8_t u8Register, uint8_t* pu8Data){
  if(HAL_I2C_Master_Transmit(IOE_I2C, IOE_I2C_ADDR, &u8Register, 1, 100) != HAL_OK) {
      return false;
    }
  if(HAL_I2C_Master_Receive(IOE_I2C, IOE_I2C_ADDR, pu8Data, 1, 100) != HAL_OK) {
      return false;
    }
  return true;
}

bool boIOE_WriteRegister(uint8_t u8Register, uint8_t u8Data){
  uint8_t au8AddrData[2];
  au8AddrData[0] = u8Register;
  au8AddrData[1] = u8Data;
  if(HAL_I2C_Master_Transmit(IOE_I2C, IOE_I2C_ADDR, au8AddrData, 2, 10) != HAL_OK) {
      return false;
    }
  return true;
}

bool boUpdateRegisters(bool boRead){
  bool boRet = true;
  if(boRead == true){
    for(uint8_t i=0;i<BUFFERED_REG_NUM;i++){
      if(boIOE_ReadRegister(i,&au8RegBuffer[i]) == false){
        boRet = false;
      }
    }
  }
  else{
    for(uint8_t i=0;i<BUFFERED_REG_NUM;i++){
      if(boIOE_WriteRegister(i,au8RegBuffer[i]) == false){
        boRet = false;
      }
    }
  }
  return boRet;
}

/**
 * Initializes the software state for all ports and clears the event buffer.
 */
bool boIOE_Init(){
  for(uint16_t i=eIOE_PIN_INVALID+1;i<eIOE_PIN_MAX;i++){
    acPin2Device[i].eType = eIOE_PIN_TYPE_NUM;
    acPin2Device[i].u8Id = 0;
  }
  for(uint16_t i=0;i<IOE_MAX_BUTTONS;i++)acButtons[i].cConfig.ePin = eIOE_PIN_INVALID;
  for(uint16_t i=0;i<IOE_MAX_LEDS;i++)acLeds[i].cConfig.ePin = eIOE_PIN_INVALID;
  for(uint16_t i=0;i<IOE_MAX_GEN_PINS;i++)aeGenPins[i] = eIOE_PIN_INVALID;
  u8ButtonEventListLen = 0;
  boUpdateRegisters(true);
  return true;
}

bool boRecordEvent(C_DYNAMIC_BUTTON_DATA* pcButton, E_BUTTON_EVENTS eEvent){
  pcButton->u16Events |= eEvent;
  uint8_t u8Idx = ((uint32_t)pcButton-(uint32_t)acButtons)/sizeof(C_DYNAMIC_BUTTON_DATA);
  acButtonEventList[u8ButtonEventListLen] = (C_BUTTONS_EVENT){.u8Button = u8Idx, .eEvent = eEvent};
  u8ButtonEventListLen++;
  return true;
}

/**
 * Binds a button ID to a physical expander pin and stores its timing config.
 */
bool boAddButton(uint8_t u8Id, C_PIN_CONFIG_BUTTON* pcConfig){
  // Check pin validity
  if ((pcConfig->ePin <= eIOE_PIN_INVALID) || (pcConfig->ePin >= eIOE_PIN_MAX)){
    return false;
  }
  // Check unused pin
  if (acPin2Device[pcConfig->ePin].eType != eIOE_PIN_TYPE_NUM){
    return false;
  }
  // Check id validity
  if (u8Id >= IOE_MAX_BUTTONS){
    return false;
  }
  // Check unused id
  if (acButtons[u8Id].cConfig.ePin != eIOE_PIN_INVALID){
    return false;
  }
  // Check button values
    // TODO


  // Everything validated, save config
  memcpy(&acButtons[u8Id].cConfig, pcConfig, sizeof(C_PIN_CONFIG_BUTTON));
  acButtons[u8Id].eState = eIOE_BUTTON_RELEASED;
  acButtons[u8Id].u16Events = 0;
  acPin2Device[pcConfig->ePin].eType = eIOE_PIN_TYPE_BUTTON;
  acPin2Device[pcConfig->ePin].u8Id = u8Id;
  return true;
}

bool boAddLed(uint8_t u8Id, C_PIN_CONFIG_LED* pcConfig){
  // Check pin validity
  if ((pcConfig->ePin <= eIOE_PIN_INVALID) || (pcConfig->ePin >= eIOE_PIN_MAX)){
    return false;
  }
  // Check unused pin
  if (acPin2Device[pcConfig->ePin].eType != eIOE_PIN_TYPE_NUM){
    return false;
  }
  // Check id validity
  if (u8Id >= IOE_MAX_LEDS){
    return false;
  }
  // Check unused id
  if (acLeds[u8Id].cConfig.ePin != eIOE_PIN_INVALID){
    return false;
  }
  // Check led values
    //Nothing to check yet

  // Everything validated, save config
  memcpy(&acLeds[u8Id].cConfig, pcConfig, sizeof(C_PIN_CONFIG_LED));
  acLeds[u8Id].eType = eLED_BLINK_STATIC;
  acLeds[u8Id].boOn = false;
  acPin2Device[pcConfig->ePin].eType = eIOE_PIN_TYPE_LED;
  acPin2Device[pcConfig->ePin].u8Id = u8Id;
  return true;
}

bool boAddGenericPin(uint8_t u8Id, E_IOE_PINS ePin){
  // Check pin validity
  if ((ePin <= eIOE_PIN_INVALID) || (ePin >= eIOE_PIN_MAX)){
    return false;
  }
  // Check unused pin
  if (acPin2Device[ePin].eType != eIOE_PIN_TYPE_NUM){
    return false;
  }
  // Check id validity
  if (u8Id >= IOE_MAX_GEN_PINS){
    return false;
  }
  // Check unused id
  if (aeGenPins[u8Id] != eIOE_PIN_INVALID){
    return false;
  }

  // Everything validated, save config
  aeGenPins[u8Id] = ePin;
  acPin2Device[ePin].eType = eIOE_PIN_TYPE_GEN;
  acPin2Device[ePin].u8Id = u8Id;
  return true;
}

void vIOE_UpdateButtonClick(C_DYNAMIC_BUTTON_DATA* pcButton, bool boDown){
  if ((boDown == true) && (pcButton->u8ClickedPhase%2 == 0)){
    if (pcButton->u8ClickedPhase == 0){
      // first press no time requirements
      pcButton->u8ClickedPhase++;
    }
    else{
      // out of time
      pcButton->u8ClickedPhase = 0;
    }
  }
  else if ((boDown == false) && (pcButton->u8ClickedPhase%2 == 1)){
    if (HAL_GetTick() - pcButton->u32ClickedTimer > pcButton->cConfig.u16MinClickTime) {
      pcButton->u8ClickedPhase++;
    } else {
      // before time: not long enough
      pcButton->u8ClickedPhase = 0;
    }
  }
  else{
    // Inconsistency
    pcButton->u8ClickedPhase = 0;
  }
  pcButton->u32ClickedTimer = HAL_GetTick();
}

void vIOE_UpdateButtonClickCheckTimeout(C_DYNAMIC_BUTTON_DATA* pcButton){
  if ((pcButton->eState == eIOE_BUTTON_RELEASED) || (pcButton->eState == eIOE_BUTTON_PRESSING)){
    if (HAL_GetTick() - pcButton->u32ClickedTimer > pcButton->cConfig.u16MaxBetweenClicksTime){
      if (pcButton->u8ClickedPhase == 2) {
        //simple click
        boRecordEvent(pcButton, eBUTTON_CLICKED);
      }
      else if (pcButton->u8ClickedPhase == 4) {
        //double click
        boRecordEvent(pcButton, eBUTTON_DOUBLE_CLICKED);
      }
      pcButton->u8ClickedPhase = 0;
      pcButton->u32ClickedTimer = HAL_GetTick();
    }
  }
  else {
    if (HAL_GetTick() - pcButton->u32ClickedTimer > pcButton->cConfig.u16MaxClickTime){
      // too long press, i think it ruins the before clicks
      pcButton->u8ClickedPhase = 0;
      pcButton->u32ClickedTimer = HAL_GetTick();
    }
  }
}

void vIOE_UpdateButton(uint8_t i){
  C_DYNAMIC_BUTTON_DATA* pcButton = &acButtons[acPin2Device[i].u8Id];
  if ( ((au8RegBuffer[acIoePins[pcButton->cConfig.ePin].u8Register] >> acIoePins[pcButton->cConfig.ePin].u8Bit) & 0x01) == (pcButton->cConfig.boActiveOnHigh?0x01:0x00) ){
    switch (pcButton->eState){
    case eIOE_BUTTON_RELEASED:
      pcButton->u32Timer = HAL_GetTick();
      pcButton->eState = eIOE_BUTTON_PRESSING;
      break;
    case eIOE_BUTTON_PRESSING:
      if(pcButton->u32Timer + pcButton->cConfig.u16PressingJitterTime <= HAL_GetTick()){
        pcButton->eState = eIOE_BUTTON_PRESSED;
        pcButton->u16PressedCnt = 0;
        pcButton->u32Timer = HAL_GetTick();
        vIOE_UpdateButtonClick(pcButton, true);
        boRecordEvent(pcButton, eBUTTON_PRESSED);
      }
      break;
    case eIOE_BUTTON_PRESSED:
      vIOE_UpdateButtonClickCheckTimeout(pcButton);
      if (pcButton->u32Timer + pcButton->cConfig.u16PressingStartDelayTime <= HAL_GetTick()){
        if (pcButton->u32Timer + pcButton->cConfig.u16PressingStartDelayTime + (pcButton->u16PressedCnt * pcButton->cConfig.u16PressingPeriodTime) <= HAL_GetTick()){
          pcButton->u16PressedCnt++;
          boRecordEvent(pcButton, eBUTTON_PRESSING);
        }
      }
      break;
    case eIOE_BUTTON_RELEASING:
      pcButton->u32Timer = HAL_GetTick();
      vIOE_UpdateButtonClickCheckTimeout(pcButton);
      break;
    }
  }
  else{
    switch (pcButton->eState){
    case eIOE_BUTTON_RELEASED:
      vIOE_UpdateButtonClickCheckTimeout(pcButton);
      break;
    case eIOE_BUTTON_PRESSING:
      pcButton->u32Timer = HAL_GetTick();
      vIOE_UpdateButtonClickCheckTimeout(pcButton);
      break;
    case eIOE_BUTTON_PRESSED:
      pcButton->u32Timer = HAL_GetTick();
      pcButton->eState = eIOE_BUTTON_RELEASING;
      break;
    case eIOE_BUTTON_RELEASING:
      if(pcButton->u32Timer + pcButton->cConfig.u16ReleasingJitterTime <= HAL_GetTick()){
        pcButton->eState = eIOE_BUTTON_RELEASED;
        vIOE_UpdateButtonClick(pcButton, false);
        boRecordEvent(pcButton, eBUTTON_RELEASED);
      }
      break;
    }
  }
}

static const struct _C_MORSE_CHAR* pcGetMorse(char chChar){
  if (isalpha(chChar) != 0){
    return &acMorse[(chChar | 0x20)-'a'];
  }
  else if (isdigit(chChar) != 0){
    return &acMorse[chChar - '0' + 'z' - 'a' + 1];
  }
  else{
    return NULL;
  }
}

void vSetLed(C_DYNAMIC_LED_DATA* pcLed, bool boOn){
  if(boOn == pcLed->cConfig.boActiveOnHigh)
    au8RegBuffer[acIoePins[pcLed->cConfig.ePin].u8Register] |= 0x01 << acIoePins[pcLed->cConfig.ePin].u8Bit;
  else
    au8RegBuffer[acIoePins[pcLed->cConfig.ePin].u8Register] &= ~(0x01 << acIoePins[pcLed->cConfig.ePin].u8Bit);
}

void vSwitchLed(C_DYNAMIC_LED_DATA* pcLed){
  au8RegBuffer[acIoePins[pcLed->cConfig.ePin].u8Register] ^= 0x01 << acIoePins[pcLed->cConfig.ePin].u8Bit;
}

void vIOE_UpdateLed(uint8_t i){
  C_DYNAMIC_LED_DATA* pcLed = &acLeds[acPin2Device[i].u8Id];
  switch (pcLed->eType) {
  case eLED_BLINK_STATIC:
    // Nothing
    break;
  case eLED_BLINK_MORSE:
    if (pcLed->u32Timer <= HAL_GetTick()){
      vSwitchLed(pcLed);
      char chCurent = pcLed->pchMorseCode[pcLed->u8CharIdx];
      const struct _C_MORSE_CHAR* p_cMorse = pcGetMorse(chCurent);
      if (chCurent == '\0'){
        // End of text
        pcLed->u8CharIdx = 0;
        pcLed->u8Phase = 0;
        pcLed->u32Timer = HAL_GetTick() + MORSE_ENTER;
        break;
      }
      else if (chCurent == ' '){
        pcLed->u8CharIdx++;
        pcLed->u8Phase = 0;
        pcLed->u32Timer = HAL_GetTick() + MORSE_SPACE;
        break;
      }
      else if (p_cMorse != NULL){
        if (pcLed->u8Phase%2 == 1){
          // Wait between blinks in char
          pcLed->u32Timer = HAL_GetTick() + MORSE_SHORT;
          pcLed->u8Phase++;
        }
        else if (p_cMorse->aboLong[pcLed->u8Phase/2] == true){
          //long
          pcLed->u32Timer = HAL_GetTick() + MORSE_LONG;
          pcLed->u8Phase++;
        }
        else{
          pcLed->u32Timer = HAL_GetTick() + MORSE_SHORT;
          pcLed->u8Phase++;
        }
        // Check phase at the end
        if (pcLed->u8Phase + 1 > p_cMorse->u8Len * 2 ){
          // end of char
          pcLed->u8CharIdx++;
          pcLed->u8Phase = 0;
          pcLed->u32Timer = HAL_GetTick() + MORSE_SPACE;
          break;
        }
      }
    }
    break;
  case eLED_BLINK_BLINK:
    if (pcLed->u32Timer <= HAL_GetTick()){
      vSwitchLed(pcLed);
      pcLed->u32Timer = HAL_GetTick() + pcLed->u16BlinkSpeed;
    }
    break;
  default:
    break;
  }
}

/**
 * Polls the expander, updates all registered inputs and flushes the result to
 * the physical device.
 */
void vIOE_Update(){
  // Check Button states, blink leds...
  boUpdateRegisters(true);
  for(uint8_t i = eIOE_PIN_INVALID+1;i<eIOE_PIN_MAX;i++){
    switch (acPin2Device[i].eType) {
    case eIOE_PIN_TYPE_GEN:
      // Nothing
      break;
    case eIOE_PIN_TYPE_BUTTON:
      vIOE_UpdateButton(i);
      break;
    case eIOE_PIN_TYPE_LED:
      vIOE_UpdateLed(i);
      break;
    default:
      break;
    }
  }
  boUpdateRegisters(false);
}

/**
 * Sets a LED to either permanent ON or OFF state without a blinking pattern.
 */
bool boIOE_SetLedStatic(uint8_t u8Id, bool boOn) {
  if ((u8Id >= IOE_MAX_LEDS) || (acLeds[u8Id].cConfig.ePin == eIOE_PIN_INVALID))
    return false;

  acLeds[u8Id].eType = eLED_BLINK_STATIC;
  acLeds[u8Id].boOn = boOn;
  boUpdateRegisters(true);
  vSetLed(&acLeds[u8Id], boOn);
  boUpdateRegisters(false);

  return true;
}
/**
 * Sets a LED to blink at a specified speed in milliseconds. A speed of 500ms will result in a 1Hz blink frequency.
 */
bool boIOE_SetLedBlink(uint8_t u8Id, uint16_t u16BlinkSpeed) {
  if ((u8Id >= IOE_MAX_LEDS) || (acLeds[u8Id].cConfig.ePin == eIOE_PIN_INVALID) || (u16BlinkSpeed == 0u))
    return false;

  C_DYNAMIC_LED_DATA *pcLed = &acLeds[u8Id];

  pcLed->eType = eLED_BLINK_BLINK;
  pcLed->u16BlinkSpeed = u16BlinkSpeed;
  pcLed->u32Timer = HAL_GetTick();

  boUpdateRegisters(true);
  vSetLed(pcLed, true);
  boUpdateRegisters(false);

  return true;
}
/**
 * Sets a LED to blink a Morse code pattern at a specified speed in milliseconds. The Morse code is provided as a string of characters, where each character is translated into its Morse representation. A speed of 500ms will result in a 1Hz blink frequency for the shortest Morse elements.
 */
bool boIOE_SetLedMorse(uint8_t u8Id, const char *pchMorseCode, uint16_t u16Speed) {
  if ((u8Id >= IOE_MAX_LEDS) || (acLeds[u8Id].cConfig.ePin == eIOE_PIN_INVALID) || (pchMorseCode == NULL) || (u16Speed == 0u))
    return false;

  C_DYNAMIC_LED_DATA *pcLed = &acLeds[u8Id];

  pcLed->eType = eLED_BLINK_MORSE;
  pcLed->pchMorseCode = pchMorseCode;
  pcLed->u16Speed = u16Speed;
  pcLed->u8CharIdx = 0u;
  pcLed->u8Phase = 0u;
  pcLed->u32Timer = HAL_GetTick();

  boUpdateRegisters(true);
  vSetLed(pcLed, false);
  boUpdateRegisters(false);

  return true;
}
/**
 * Returns the current state of a button, which can be one of the following:
 */
E_BUTTON_STATE eIOE_GetButtonState(uint8_t u8Id) {
  if ((u8Id >= IOE_MAX_BUTTONS) || (acButtons[u8Id].cConfig.ePin == eIOE_PIN_INVALID))
    return eIOE_BUTTON_RELEASED;
  return acButtons[u8Id].eState;
}
/**
 * Returns the accumulated events for a button since the last call to this function. The events are represented as a bitmask of E_BUTTON_EVENTS values. After calling this function, the event count for the button is reset to zero.
 */
uint16_t u16IOE_GetButtonEvents(uint8_t u8Id) {
  if ((u8Id >= IOE_MAX_BUTTONS) || (acButtons[u8Id].cConfig.ePin == eIOE_PIN_INVALID))
    return 0;

  return acButtons[u8Id].u16Events;
}
/**
 * Returns the accumulated events for a button since the last call to this function and clears the event count for that button. The events are represented as a bitmask of E_BUTTON_EVENTS values. After calling this function, the event count for the button is reset to zero.
 */
uint16_t u16IOE_GetAndClearAllButtonEvents(uint8_t u8Id) {
  if ((u8Id >= IOE_MAX_BUTTONS) || (acButtons[u8Id].cConfig.ePin == eIOE_PIN_INVALID))
    return 0;

  uint16_t u16Events = acButtons[u8Id].u16Events;
  acButtons[u8Id].u16Events = 0u;

  return u16Events;
}
/**
 * Clears specific events for a button based on the provided mask. The events to be cleared are represented as a bitmask of E_BUTTON_EVENTS values. After calling this function, the specified events for the button will be reset to zero.
 */
void vIOE_ClearButtonEvents(uint8_t u8Id, uint16_t u16Mask) {
  if ((u8Id >= IOE_MAX_BUTTONS) || (acButtons[u8Id].cConfig.ePin == eIOE_PIN_INVALID))
    return;

  acButtons[u8Id].u16Events &= ~u16Mask;
}
/**
 * Returns a pointer to the array of button events that have occurred since the last call to this function, along with the number of events in the array. The length of the event list is returned through the u8Len parameter. After calling this function, the event list is cleared and ready to accumulate new events.
 */
C_BUTTONS_EVENT* pcIOE_GetEvents(uint8_t* u8Len){
  *u8Len = u8ButtonEventListLen;
  u8ButtonEventListLen = 0;
  return acButtonEventList;
}
