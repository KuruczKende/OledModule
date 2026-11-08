/*
 * Screens.h
 *
 *  Created on: Oct 2, 2026
 *      Author: kuruc
 */

#ifndef INC_SCREENS_H_
#define INC_SCREENS_H_

/*
 * Screen management API.
 *
 * The application keeps a stack of active screens and routes the user input
 * through the currently visible screen instance. New screens can be added here
 * and initialized in Screens.c.
 */

#include "DispItf.h"
#include "IoExpander.h"

struct _C_SCREEN;
typedef struct _C_SCREEN{
  C_UI_SCREEN cUI;
  bool (*bofpButtonEvent)(struct _C_SCREEN* pcScreen, C_BUTTONS_EVENT* pcEvent);
  uint8_t u8InnerState;
  uint8_t u8ScreenData;
}C_SCREEN;

/**
 * Initializes the application test screen.
 *
 * @param pcScreen Screen instance to populate with the demo UI.
 * @param ptr Optional per-screen context pointer.
 */
void testScreenInit(C_SCREEN* pcScreen, void* ptr);

// TODO Add new screen above

/**
 * Available screen IDs managed by the screen stack.
 */
typedef enum{
  eSCREEN_TEST,
  eSCREEN_IRON,

  // TODO Add new screen above
  eSCREEN_NUM
}E_SCREENS;

/**
 * Physical button IDs used by the UI input layer.
 */
enum{
  eBUTTON_UP = 0,
  eBUTTON_DOWN,
  eBUTTON_LEFT,
  eBUTTON_RIGHT,
  eBUTTON_MIDDLE,
  eBUTTON_A,
  eBUTTON_B,
  // TODO Modify buttons

  eBUTTON_NUM
};

typedef void (*fpv_t)(C_SCREEN* pcScreen, void*);

// TODO modify the Screens.c too

/**
 * Initializes the hardware and all registered screens.
 */
void vSCREEN_Init();

/**
 * Draws the current screen on the display.
 */
void vSCREEN_Show();

/**
 * Polls input events and updates the currently active screen.
 */
void vSCREEN_Process();

/**
 * Replaces the top screen in the stack without changing the stack depth.
 */
bool boSCREEN_Switch(E_SCREENS eScreen);

/**
 * Pushes a new screen onto the stack.
 */
bool boSCREEN_Open(E_SCREENS eScreen);

/**
 * Pops the currently active screen from the stack.
 */
bool boSCREEN_Close();

#endif /* INC_SCREENS_H_ */
