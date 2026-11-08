
/*
 * DispItf.c
 *
 *  Created on: Sep 30, 2026
 *      Author: kuruc
 */
#include "ScreenModule/DispItf.h"
#include "ScreenModule/SH1106.h"
#include <stdio.h>
#include <string.h>

/*
 * UI drawing helper functions.
 *
 * These routines convert the generic widget descriptions into SH1106 drawing
 * calls while preserving the logical origin and inversion semantics of each
 * drawable element.
 */

static void vUI_DrawText(C_UI_TEXT *pcText, int16_t x, int16_t y, bool boInversed);
static void vUI_DrawShape(C_UI_SHAPE *pcShape, int16_t x, int16_t y, bool boInversed);
static void vUI_DrawWindow(C_UI_WINDOW *pcWindow, int16_t x, int16_t y, bool boInversed);
static void vUI_DrawDrawable(C_UI_DRAWABLE *pcDrawable, int16_t x, int16_t y, bool boInversed);

/**
 * Draws a text element using the configured font and formatting mode.
 */
static void vUI_DrawText(C_UI_TEXT *pcText, int16_t x, int16_t y, bool boInversed){
  char chText[64];
  SH1106_COLOR_t c = boInversed?SH1106_COLOR_BLACK:SH1106_COLOR_WHITE;
  SH1106_GotoXY(x, y);
  switch (pcText->eType)
  {
    case eUI_TEXT_TYPE_LABEL:
      SH1106_LimitedPuts(pcText->pchLabel,pcText->Font,c);
      break;

    case eUI_TEXT_TYPE_VALUE_U8:
      sprintf(chText,"%u",pcText->u8Get(pcText->pvCtx));
      SH1106_LimitedPuts(chText,pcText->Font,c);
      break;

    case eUI_TEXT_TYPE_VALUE_I8:
      sprintf(chText,"%d",pcText->i8Get(pcText->pvCtx));
      SH1106_LimitedPuts(chText,pcText->Font,c);
      break;

    case eUI_TEXT_TYPE_VALUE_U16:
        sprintf(chText, "%u", pcText->u16Get(pcText->pvCtx));
        SH1106_LimitedPuts(chText, pcText->Font, c);
        break;

    case eUI_TEXT_TYPE_VALUE_STR:
      SH1106_LimitedPuts(pcText->pchGet(pcText->pvCtx),pcText->Font,c);
      break;

    default:
      break;
  }
}
static void vUI_DrawShape(C_UI_SHAPE *pcShape, int16_t x, int16_t y, bool boInversed){
  SH1106_COLOR_t c = boInversed?SH1106_COLOR_BLACK:SH1106_COLOR_WHITE;
  switch (pcShape->eType)
  {
      case eUI_SHAPE_TYPE_POINT:
          SH1106_DrawLimitedPixel(x,y,c);
          break;

      case eUI_SHAPE_TYPE_LINE:
          SH1106_DrawLimitedLine(x,y,pcShape->x1,pcShape->y1,c);
          break;

      case eUI_SHAPE_TYPE_RECTANGLE:
          SH1106_DrawLimitedRectangle(x,y,pcShape->x1,pcShape->y1,c);
          break;

      case eUI_SHAPE_TYPE_FILLED_RECTANGLE:
          SH1106_DrawLimitedFilledRectangle(x,y,pcShape->x1,pcShape->y1,c);
          break;

      case eUI_SHAPE_TYPE_TRIANGLE:
          SH1106_DrawLimitedTriangle(x,y,pcShape->x1,pcShape->y1,pcShape->x2,pcShape->y2,c);
          break;

      case eUI_SHAPE_TYPE_FILLED_TRIANGLE:
          SH1106_DrawLimitedFilledTriangle(x,y,pcShape->x1,pcShape->y1,pcShape->x2,pcShape->y2,c);
          break;

      case eUI_SHAPE_TYPE_CIRCLE:
          SH1106_DrawLimitedCircle(x,y,pcShape->x1,c);
          break;

      case eUI_SHAPE_TYPE_FILLED_CIRCLE:
          SH1106_DrawLimitedFilledCircle(x,y,pcShape->x1,c);
          break;
  }
}
/**
 * Draws a nested window by applying a temporary clipping region and offset.
 */
static void vUI_DrawWindow(C_UI_WINDOW *pcWindow, int16_t x, int16_t y, bool boInversed){
  //Save selection
  int16_t sx1,sx2,sy1,sy2;
  SH1106_GetSelection(&sx1,&sy1,&sx2,&sy2);
  //Get offset
  int16_t ox=0,oy=0;
  if(pcWindow->fps16OffsetX != NULL) ox = x + pcWindow->fps16OffsetX(pcWindow->pu8OffsetRef);
  if(pcWindow->fps16OffsetY != NULL) oy = y + pcWindow->fps16OffsetY(pcWindow->pu8OffsetRef);

  SH1106_SetSelection(x,y,x+pcWindow->u8Width-1,y+pcWindow->u8Height-1);
  for(uint8_t i=0;i<pcWindow->u8ElementNum;i++){
    vUI_DrawDrawable(&pcWindow->pcElements[i],ox,oy,boInversed);
  }

  SH1106_SetSelection(sx1,sy1,sx2,sy2);
}

static void vUI_DrawDrawable(C_UI_DRAWABLE *pcDrawable, int16_t x, int16_t y, bool boInversed)
{
    if (pcDrawable->pu8ConditionRef != NULL)
    {
        if (*pcDrawable->pu8ConditionRef == pcDrawable->u8ConditionRefValue)
          boInversed = !boInversed;
    }

    x += pcDrawable->i16PosX;
    y += pcDrawable->i16PosY;

    switch (pcDrawable->eType)
    {
        case eUI_DRAWABLE_TYPE_TEXT:
            vUI_DrawText(&pcDrawable->cText, x, y, pcDrawable->boInversed!=boInversed);
            break;

        case eUI_DRAWABLE_TYPE_SHAPE:
            vUI_DrawShape(&pcDrawable->cShape, x, y, pcDrawable->boInversed!=boInversed);
            break;

        case eUI_DRAWABLE_TYPE_WINDOW:
            vUI_DrawWindow(&pcDrawable->cWindow, x, y, pcDrawable->boInversed!=boInversed);
            break;

        default:
            break;
    }
}

/**
 * Draws every drawable object contained in the given screen definition.
 */
void vUI_DrawScreen(C_UI_SCREEN *pcScreen){
  SH1106_SetSelection(0,0,SH1106_WIDTH-1,SH1106_HEIGHT-1);

  for(uint8_t i=0;i<pcScreen->u8ElementNum;i++){
    vUI_DrawDrawable(&pcScreen->pcElements[i],0,0,false);
  }
  SH1106_UpdateScreen();
}

