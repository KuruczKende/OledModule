/*
 * Screens.c
 *
 *  Created on: Oct 28, 2026
 *      Author: kuruc
 */
#include "ScreenModule/Screens.h"

/*
 * Screen stack implementation.
 *
 * The application keeps a simple stack of active screens to support nested UI
 * flows while sharing one I/O expander and one display instance.
 */

#define MAX_SCRREN_IN_BUFFER (32)

static C_SCREEN acScreen[eSCREEN_NUM];
static E_SCREENS aeScreenBuffer[MAX_SCRREN_IN_BUFFER];
static uint8_t u8ScreenBufferSize;


/**
 * Initializes the I/O expander and registers all screens used by the UI.
 */
void vSCREEN_Init(){
  boIOE_Init();
  u8ScreenBufferSize = 0;

  // TODO initialize screens
  testScreenInit(&acScreen[eSCREEN_TEST], NULL);
  ironScreenInit(&acScreen[eSCREEN_IRON], NULL);

  // TODO initialize buttons
  C_PIN_CONFIG_BUTTON cButton;
  cButton.boActiveOnHigh = true;
  cButton.u16MaxBetweenClicksTime = 100;
  cButton.u16MaxClickTime = 500;
  cButton.u16MinClickTime = 100;
  cButton.u16PressingJitterTime = 20;
  cButton.u16ReleasingJitterTime = 20;
  cButton.u16PressingStartDelayTime = 550;
  cButton.u16PressingPeriodTime = 100;

  cButton.ePin = eIOE_PIN_P5;
  boAddButton(eBUTTON_UP,&cButton);

  cButton.ePin = eIOE_PIN_P3;
  boAddButton(eBUTTON_DOWN,&cButton);

  cButton.ePin = eIOE_PIN_P2;
  boAddButton(eBUTTON_LEFT,&cButton);

  cButton.ePin = eIOE_PIN_P4;
  boAddButton(eBUTTON_RIGHT,&cButton);

  cButton.ePin = eIOE_PIN_P6;
  boAddButton(eBUTTON_MIDDLE,&cButton);

  cButton.ePin = eIOE_PIN_P0;
  boAddButton(eBUTTON_A,&cButton);

  cButton.ePin = eIOE_PIN_P1;
  boAddButton(eBUTTON_B,&cButton);

}

/**
 * Renders the currently active screen to the OLED panel.
 */
void vSCREEN_Show(){
  if(u8ScreenBufferSize<1) return;

  vUI_DrawScreen(&acScreen[aeScreenBuffer[u8ScreenBufferSize-1]].cUI);
}

/**
 * Updates the active screen with fresh button events and redraws it.
 */
void vSCREEN_Process(){
  if(u8ScreenBufferSize<1) return;

  if (acScreen[aeScreenBuffer[u8ScreenBufferSize-1]].bofpButtonEvent != NULL){
    uint8_t u8Len;
    C_BUTTONS_EVENT* pcEvents = pcIOE_GetEvents(&u8Len);
    for(uint8_t i=0; i<u8Len; i++){
      acScreen[aeScreenBuffer[u8ScreenBufferSize-1]].bofpButtonEvent(&acScreen[aeScreenBuffer[u8ScreenBufferSize-1]], &pcEvents[i]);
    }
  }
  vUI_DrawScreen(&acScreen[aeScreenBuffer[u8ScreenBufferSize-1]].cUI);
}

/**
 * Replaces the current top-of-stack screen with the requested one.
 */
bool boSCREEN_Switch(E_SCREENS eScreen){
  if(u8ScreenBufferSize<1) return boSCREEN_Open(eScreen);

  if(eScreen >= eSCREEN_NUM) return false;
  aeScreenBuffer[u8ScreenBufferSize-1] = eScreen;
  return true;
}

/**
 * Pushes a screen onto the stack if there is room remaining.
 */
bool boSCREEN_Open(E_SCREENS eScreen){
  if(u8ScreenBufferSize+1 >= MAX_SCRREN_IN_BUFFER) return false;
  if(eScreen >= eSCREEN_NUM) return false;
  aeScreenBuffer[u8ScreenBufferSize] = eScreen;
  u8ScreenBufferSize++;
  return true;
}

/**
 * Pops the current screen from the stack if one is active.
 */
bool boSCREEN_Close(){
  if(u8ScreenBufferSize<1) return;
  u8ScreenBufferSize--;

}
