/*
 * DispItf.h
 *
 *  Created on: Sep 30, 2026
 *      Author: kuruc
 */

#ifndef INC_DISPITF_H_
#define INC_DISPITF_H_

/*
 * Display interface abstraction used to compose UI widgets and draw screens.
 *
 * The drawing layer converts high-level objects such as labels, shapes and
 * nested windows into SH1106 pixel operations while honouring clipping bounds.
 */

#include "inttypes.h"
#include "stdbool.h"
#include "SH1106.h"

#define MAX_ELEMENT_IN_LIST (32u)

/**
 * Forward declaration for drawable elements used inside the UI tree.
 */
typedef struct C_UI_DRAWABLE_ C_UI_DRAWABLE;

typedef enum {
  eUI_TEXT_TYPE_LABEL,
  eUI_TEXT_TYPE_VALUE_U8,
  eUI_TEXT_TYPE_VALUE_I8,
  eUI_TEXT_TYPE_VALUE_U16,
  eUI_TEXT_TYPE_VALUE_STR,
} E_UI_TEXT_TYPE;

typedef struct {
  E_UI_TEXT_TYPE eType;
  union {
    const char *pchLabel;
    uint8_t (*u8Get)(void *pvCtx);
    int8_t (*i8Get)(void *pvCtx);
    uint16_t (*u16Get)(void *pvCtx);
    const char* (*pchGet)(void *pvCtx);
  };
  void *pvCtx;
  FontDef_t* Font;
} C_UI_TEXT;

typedef struct{
  FontDef_t* Font;
  struct{
    E_UI_TEXT_TYPE eType;
    union {
      const char *pchLabel;
      uint8_t (*u8Get)(void *pvCtx);
      int8_t (*i8Get)(void *pvCtx);
      const char* (*pchGet)(void *pvCtx);
    };
    void *pvCtx;
  }acTexts[16];
}C_UI_FORM_TEXT;

typedef enum{
  eUI_SHAPE_TYPE_POINT,
  eUI_SHAPE_TYPE_LINE,
  eUI_SHAPE_TYPE_RECTANGLE,
  eUI_SHAPE_TYPE_FILLED_RECTANGLE,
  eUI_SHAPE_TYPE_TRIANGLE,
  eUI_SHAPE_TYPE_FILLED_TRIANGLE,
  eUI_SHAPE_TYPE_CIRCLE,
  eUI_SHAPE_TYPE_FILLED_CIRCLE,
}E_UI_SHAPE_TYPE;

typedef struct {
  E_UI_SHAPE_TYPE eType;
  uint16_t x1,x2,y1,y2;
} C_UI_SHAPE;

// It draws it's inner element to the screen with setting the limited selection space
// It describes a window, and no element will be drawn outside this window.
// The elements will be drawn with an offset
typedef struct {
  uint8_t u8Width;
  uint8_t u8Height;

  int16_t (*fps16OffsetX)(uint8_t* pu8Ref);
  int16_t (*fps16OffsetY)(uint8_t* pu8Ref);
  uint8_t* pu8OffsetRef;

  C_UI_DRAWABLE* pcElements;
  uint8_t u8ElementNum;
} C_UI_WINDOW;

typedef enum{
  eUI_DRAWABLE_TYPE_TEXT,
  eUI_DRAWABLE_TYPE_SHAPE,
  eUI_DRAWABLE_TYPE_WINDOW
} E_UI_DRAWABLE_TYPE;

struct C_UI_DRAWABLE_{
  E_UI_DRAWABLE_TYPE eType;
  int16_t i16PosX;
  int16_t i16PosY;
  bool boInversed;
  uint8_t *pu8ConditionRef;
  uint8_t u8ConditionRefValue;

  union{
    C_UI_TEXT cText;
    C_UI_SHAPE cShape;
    C_UI_WINDOW cWindow;
  };

};

typedef struct{
  C_UI_DRAWABLE* pcElements;
  uint8_t u8ElementNum;
} C_UI_SCREEN;

/**
 * Draws all widgets contained in a screen to the OLED buffer.
 */
void vUI_DrawScreen(C_UI_SCREEN *pcScreen);


#endif /* INC_DISPITF_H_ */
