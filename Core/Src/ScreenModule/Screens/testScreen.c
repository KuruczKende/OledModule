/*
 * testScreen.c
 *
 *  Created on: Oct 2, 2026
 *      Author: kuruc
 */
#include "ScreenModule/Screens.h"

/**
 * Demo screen used to exercise the OLED drawing pipeline and
 * button-driven value changes during development.
 */

  uint8_t au8TestValue[3] = {12, 34, 56};
  uint8_t u8ScrollSpeed;

  uint8_t u8WindowOffsetX = 0;
  uint8_t u8WindowOffsetY = 0;


  /* Offset callbacks */
  int16_t TestWindowOffsetX(uint8_t *pu8Ref)
  {
    return 0;
  }

  int16_t TestWindowOffsetY(uint8_t *pu8Ref)
  {
    return 0;
  }


  /* Text getters */
  uint8_t TestValue0(void *pvCtx)
  {
    return au8TestValue[0];
  }

  uint8_t TestValue1(void *pvCtx)
  {
    return au8TestValue[1];
  }

  uint8_t TestValue2(void *pvCtx)
  {
    return au8TestValue[2];
  }

  const char* TestMenuText(void *pvCtx)
  {
    return "Menu:";
  }


  /* Window elements */
  C_UI_DRAWABLE cTestWindowElements[] =
  {
      {
        .eType = eUI_DRAWABLE_TYPE_SHAPE,
        .i16PosX = 0,
        .i16PosY = 0,
        .boInversed = true,
        .pu8ConditionRef = NULL,
        .u8ConditionRefValue = 0,
        .cShape = {
            .eType = eUI_SHAPE_TYPE_FILLED_RECTANGLE,
            .x1 = 21,
            .y1 = 30
        },
      },

      {
        .eType = eUI_DRAWABLE_TYPE_TEXT,
        .i16PosX = 0,
        .i16PosY = 0,
        .boInversed = false,
        .pu8ConditionRef = NULL,
        .u8ConditionRefValue = 0,

        .cText = {
          .eType = eUI_TEXT_TYPE_VALUE_U8,
          .u8Get = TestValue0,
          .pvCtx = NULL,
          .Font = &Font_7x10
        }
      },

    {
      .eType = eUI_DRAWABLE_TYPE_TEXT,
      .i16PosX = 0,
      .i16PosY = 10,
      .boInversed = false,
      .pu8ConditionRef = NULL,
      .u8ConditionRefValue = 1,

      .cText = {
        .eType = eUI_TEXT_TYPE_VALUE_U8,
        .u8Get = TestValue1,
        .pvCtx = NULL,
        .Font = &Font_7x10
      }
    },

    {
      .eType = eUI_DRAWABLE_TYPE_TEXT,
      .i16PosX = 0,
      .i16PosY = 20,
      .boInversed = false,
      .pu8ConditionRef = NULL,
      .u8ConditionRefValue = 2,

      .cText = {
        .eType = eUI_TEXT_TYPE_VALUE_U8,
        .u8Get = TestValue2,
        .pvCtx = NULL,
        .Font = &Font_7x10
      }
    }
  };


  /* Test window */
  C_UI_WINDOW cTestWindow =
  {
    .u8Width = 80,
    .u8Height = 50,

    .fps16OffsetX = TestWindowOffsetX,
    .fps16OffsetY = TestWindowOffsetY,

    .pu8OffsetRef = &u8WindowOffsetX,

    .pcElements = cTestWindowElements,
    .u8ElementNum = sizeof(cTestWindowElements) /
                    sizeof(cTestWindowElements[0])
  };


  /* Screen elements */
  C_UI_DRAWABLE cTestScreenElements[] =
  {
    {
      .eType = eUI_DRAWABLE_TYPE_TEXT,
      .i16PosX = 5,
      .i16PosY = 5,
      .boInversed = false,
      .pu8ConditionRef = NULL,
      .u8ConditionRefValue = 0,

      .cText = {
        .eType = eUI_TEXT_TYPE_VALUE_STR,
        .pchGet = TestMenuText,
        .pvCtx = NULL,
        .Font = &Font_7x10
      }
    },

    {
      .eType = eUI_DRAWABLE_TYPE_WINDOW,
      .i16PosX = 5,
      .i16PosY = 15,
      .boInversed = false,
      .pu8ConditionRef = NULL,
      .u8ConditionRefValue = 0,

      .cWindow = {
        .u8Width = 80,
        .u8Height = 50,

        .fps16OffsetX = TestWindowOffsetX,
        .fps16OffsetY = TestWindowOffsetY,

        .pu8OffsetRef = NULL,

        .pcElements = cTestWindowElements,
        .u8ElementNum = sizeof(cTestWindowElements) /
                        sizeof(cTestWindowElements[0])
      }
    }
  };

/**
 * Handles the interaction logic for the test screen.
 *
 * The screen exposes three editable values and reacts to press,
 * hold and click events to emulate a simple UI control panel.
 */
bool testButtonEvent(C_SCREEN* pcScreen, C_BUTTONS_EVENT* pcEvent){
  if (pcEvent->eEvent == eBUTTON_PRESSED && (pcEvent->u8Button == eBUTTON_LEFT || pcEvent->u8Button == eBUTTON_RIGHT)){
    u8ScrollSpeed = 1;
  }
  if (pcEvent->eEvent == eBUTTON_PRESSING) {
    switch (pcEvent->u8Button) {
    case eBUTTON_LEFT:
      if (au8TestValue[pcScreen->u8InnerState] > u8ScrollSpeed/5) {
        au8TestValue[pcScreen->u8InnerState]-=u8ScrollSpeed/5;
      }
      else if (au8TestValue[pcScreen->u8InnerState] > 0){
        au8TestValue[pcScreen->u8InnerState]=0;
      }
      if(u8ScrollSpeed<255)u8ScrollSpeed++;
      break;
    case eBUTTON_RIGHT:
      if (au8TestValue[pcScreen->u8InnerState] < 255 - u8ScrollSpeed/5) {
        au8TestValue[pcScreen->u8InnerState]+=u8ScrollSpeed/5;
      }
      else if (au8TestValue[pcScreen->u8InnerState] < 255){
        au8TestValue[pcScreen->u8InnerState]=255;
      }
      if(u8ScrollSpeed<255)u8ScrollSpeed++;
      break;
    default:
      break;
    }
  }
  if (pcEvent->eEvent == eBUTTON_CLICKED) {
    switch (pcEvent->u8Button) {
    case eBUTTON_UP:
      pcScreen->u8InnerState--;
      if (pcScreen->u8InnerState >= 3) {
        pcScreen->u8InnerState = 2;
      }
      break;
    case eBUTTON_DOWN:
      pcScreen->u8InnerState++;
      if (pcScreen->u8InnerState >= 3) {
        pcScreen->u8InnerState = 0;
      }
      break;
    case eBUTTON_LEFT:
      if (au8TestValue[pcScreen->u8InnerState] > 0) {
        au8TestValue[pcScreen->u8InnerState]--;
      }
      break;
    case eBUTTON_RIGHT:
      if (au8TestValue[pcScreen->u8InnerState] < 255) {
        au8TestValue[pcScreen->u8InnerState]++;
      }
      break;
    default:
      break;
    }
  }
  return true;
}


/**
 * Initializes the demo screen and binds the UI elements to the
 * editable test values stored in this module.
 */
void testScreenInit(C_SCREEN* pcScreen, void* ptr){
  /* Test values */
  cTestWindowElements[1].pu8ConditionRef = &pcScreen->u8InnerState;
  cTestWindowElements[2].pu8ConditionRef = &pcScreen->u8InnerState;
  cTestWindowElements[3].pu8ConditionRef = &pcScreen->u8InnerState;

  *pcScreen = (C_SCREEN){
    .cUI = (C_UI_SCREEN){
    .pcElements = cTestScreenElements,
    .u8ElementNum = sizeof(cTestScreenElements) /
                    sizeof(cTestScreenElements[0]),
    },
    .bofpButtonEvent = testButtonEvent,
    .u8InnerState = 0
  };
}
