/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GRAPH_DATA_XY.c
Purpose     : Implementation of YT graph
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GRAPH_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
#define ABS(v) ((v>0) ? v : -v)

const I16 aTanSelfAdd[] = {
  0,       /* atan(0/16) */
  41,      /* atan(1/16) */
  81,      /* atan(2/16) */
  121,     /* atan(3/16) */
  160,     /* atan(4/16) */
  197,     /* atan(5/16) */
  234,     /* atan(6/16) */
  269,     /* atan(7/16) */
  302,     /* atan(8/16) */
  334,     /* atan(9/16) */
  364,     /* atan(10/16) */
  393,     /* atan(11/16) */
  419,     /* atan(12/16) */
  445,     /* atan(13/16) */
  469,     /* atan(14/16) */
  491,     /* atan(15/16) */
  512      /* atan(1) = 45° = 512/1024 */
};

typedef struct {
  GRAPH_DATA_OBJ GraphDataObj;
  WM_HMEM        hData;
  U8             PenSize;
  U8             LineStyle;
} GRAPH_DATA_XY_OBJ;


static void _SwapInt(int* pa, int* pb) {
  int t;
  t = *pa;
  *pa = *pb;
  *pb = t;
}

/*********************************************************************
*
*       Abs
*/
static int Abs(int v) {
  return ABS(v);
}


static int _atan0_45(int q) {
  int r;
  int i, Faktor;
  /* Now angle is reduced to 0° <= <= 90° ==>  0 <= <= 256*/
  q >>=2;    /* make sure we do not exceed 16 bits in calculations */
  i = q>>4;
  Faktor = (1<<4)-(q&((1<<4)-1));
  r = aTanSelfAdd[i]*Faktor;
  if (Faktor !=(1<<4)) {
    r += aTanSelfAdd[i+1]*((1<<4)-Faktor);
  }
  r = (r+(1<<3))/(1<<4);   /* divide  incl. rounding */
  return r;
}

static int _atan2(I32 x, I32 y) {
  U8 q =0;
  int angle;
/* first make sure we are in angle between 0 and 45° */
  if (x<0) {
    q=1;
    x=-x;
  }
  if (y<0) {
    q |= (1<<1);
    y=-y;
  }
  if (y>x) {
    int t = y;
    y=x;
    x=t;
    q |= (1<<2);
  }
  y <<=10;
  y += (x/2);
  y /= x;
  angle = _atan0_45(y);
  if (q&(1<<2)) { /* y/x reverse ? */
    angle = GUI_90DEG-angle;
  }
  if (q&1) {  /* xreverse ? */
    angle = GUI_180DEG-angle;
  }
  if (q&(1<<1)) { /* y-reverse ? */
    angle = GUI_360DEG-angle;
  }
  return angle;
}


static void _CalcOrto(int xDiff, int yDiff, I32 r, int*px, int*py) {
  I32 x,y;
  int Angle = _atan2(xDiff, yDiff);
  Angle+= GUI_90DEG;
  x = ( r*(I32)GUI_cos(Angle));
  y = ( r*(I32)GUI_sin(Angle));
  x = (x<0) ? -((-x+512)>>10) : ((x+512)>>10);
  y = (y<0) ? -((-y+512)>>10) : ((y+512)>>10);
  *px = x;
  *py = y;
}

static int _SetLineColor(int i) {
  switch (GUI_Context.LineStyle) {
  case GUI_LS_DASH:
    i = (i + 6) % 16;
    return (i < 12);
  case GUI_LS_DOT:
    i %= 4;
    return (i < 2);
  case GUI_LS_DASHDOT:
    i %= 20;
    if (i < 12) {
      return 1;
    } else if ((i >= 16) && (i < 18)) {
      return 1;
    }
    return 0;
  case GUI_LS_DASHDOTDOT:
    i %= 24;
    if (i < 12) {
      return 1;
    } else if ((i >= 16) && (i < 18)) {
      return 1;
    } else if ((i >= 20) && (i < 22)) {
      return 1;
    } else {
      return 0;
    }
  }
  return 0;
}

void GL_DrawLine1Ex(int x0, int y0, int x1, int y1, unsigned * pPixelCnt) {
  int xdiff = x1 - x0;
  int ydiff = y1 - y0;
  int xdiffby2;
  int i;
  char XYSwapped = 0;
  int Dir;
  int PixelCnt = * pPixelCnt;
/* check if no line */
  if (!(xdiff | ydiff)) {
    GL_DrawPoint(x0, y0);
    return;
  }
/* check if we swap x and y for calculation */
  if (Abs(xdiff) < Abs(ydiff)) {
    _SwapInt(&xdiff, &ydiff);
    _SwapInt(&x0, &y0);
    _SwapInt(&x1, &y1);
    XYSwapped = 1;
  }
/* make sure line direction is positive */
  if (xdiff != Abs(xdiff)) {
    xdiff = -xdiff;
    ydiff = -ydiff;
    _SwapInt(&x0, &x1);
    _SwapInt(&y0, &y1);
    Dir = -1;
  } else {
    Dir = 1;
  }
/* Draw horizontal or vertical line if possible */
  if (ydiff == 0) {
    if (XYSwapped) {
      if (GUI_Context.LineStyle == GUI_LS_SOLID) {
        LCD_DrawVLine(y0, x0, x1);
      } else {
        for (i = 0; i <= xdiff; i++) {
          if (!_SetLineColor(PixelCnt++)) {
            LCD_HL_DrawPixel(y0, x0 + i);
          }
        }
        *pPixelCnt = PixelCnt - 1;
      }
    } else {
      if (GUI_Context.LineStyle == GUI_LS_SOLID) {
        LCD_HL_DrawHLine(x0, y0, x1);
      } else {
        for (i = 0; i <= xdiff; i++) {
          if (!_SetLineColor(PixelCnt++)) {
            LCD_HL_DrawPixel(x0 + i, y0);
          }
        }
        *pPixelCnt = PixelCnt - 1;
      }
    }
    return;
  }
  xdiffby2 = xdiff / 2;
  if (ydiff < 0) {
    xdiffby2 = -xdiffby2;
  }
/* Draw pixel by pixel solid*/
  if (GUI_Context.LineStyle == GUI_LS_SOLID) {
    for (i = 0; i <= xdiff; i++) {
      I32 l = ((I32)ydiff) * i + xdiffby2;
      int y = (ABS(l) < 32767) ? (y0 + ((int)l) / xdiff) : (y0 + l / xdiff);
      if (!XYSwapped) {
        LCD_HL_DrawPixel(x0+ i , y);
      } else {
        LCD_HL_DrawPixel(y, x0 + i);
      }
    }
/* Draw pixel by pixel with fill style */
  } else {
    if (Dir < 0) {
      *pPixelCnt += xdiff;
    }
    for (i = 0; i <= xdiff; i++) {
      long l = ((long)ydiff) * i + xdiffby2;
      int y = (ABS(l) < 32767) ? (y0 + ((int)l) / xdiff) : (y0 + l / xdiff);
      if (!_SetLineColor((*pPixelCnt) += Dir)) {
        if (!XYSwapped) {
          LCD_HL_DrawPixel(x0 + i, y);
        } else {
          LCD_HL_DrawPixel(y, x0 + i);
        }
      }
    }
    if (Dir < 0) {
      *pPixelCnt += xdiff;
    }
    *pPixelCnt -= Dir;
  }
}

static void _DrawLineEnd(int x0, int y0) {
  switch (GUI_Context.PenShape) {
  case GUI_PS_ROUND:
    GL_DrawPoint(x0,y0);
    break;
  case GUI_PS_FLAT:
    break;
  }
}

void GL_DrawLineEx(int x0, int y0, int x1, int y1, unsigned * pPixelCnt) {
  if (GUI_Context.PenSize ==1) {
    GL_DrawLine1Ex(x0,y0,x1,y1,pPixelCnt);
  } else {
    int xdiff, ydiff;
    xdiff = x0-x1;
    ydiff = y0-y1;
    if (xdiff | ydiff) {
      GUI_POINT Poly[4];
      int xOff, yOff;
      int xOffP, yOffP, xOffM, yOffM;
      _CalcOrto(x0-x1,y0-y1, (I32)(GUI_Context.PenSize-1), &xOff, &yOff);
  /* Do rounding for offsets */
      if (xOff >0) {
        xOffP = (xOff+1)/2;
        xOffM =  xOff   /2;
      } else {
        xOffP =   xOff/2;
        xOffM =  (xOff-1)   /2;
      }
      if (yOff >0) {
        yOffP = (yOff+1)/2;
        yOffM =  yOff   /2;
      } else {
        yOffP =   yOff/2;
        yOffM =  (yOff-1)   /2;
      }
      Poly[0].x = x0+xOffP;
      Poly[0].y = y0+yOffP;
      Poly[1].x = x0-xOffM;
      Poly[1].y = y0-yOffM;
      Poly[2].x = x1-xOffM;
      Poly[2].y = y1-yOffM;
      Poly[3].x = x1+xOffP;
      Poly[3].y = y1+yOffP;
      GL_FillPolygon(&Poly[0], 4, 0,0);
      _DrawLineEnd(x0,y0);
      _DrawLineEnd(x1,y1);
    }
  }
}
/*********************************************************************
*
*       GL_DrawLineToEx
*/
void GL_DrawLineToEx(int x, int y, unsigned * pPixelCnt) {
  GL_DrawLineEx(GUI_Context.DrawPosX, GUI_Context.DrawPosY, x, y, pPixelCnt);
  GUI_Context.DrawPosX = x;
  GUI_Context.DrawPosY = y;
}

/*********************************************************************
*
*       _DrawPolyLine
*/
static void _DrawPolyLine(const GUI_POINT * pPoints, int NumPoints, int x0, int y0) {
  unsigned PixelCnt;
  PixelCnt = 0;
  WM_ADDORG(x0,y0);
  WM_ITERATE_START(NULL); {
    GL_MoveTo(pPoints->x + x0, y0 - pPoints->y);
    while (--NumPoints >0) {
      pPoints++;
      GL_DrawLineToEx(pPoints->x + x0, y0 - pPoints->y, &PixelCnt);
    }
  } WM_ITERATE_END();
}



static void _OnPaint(GRAPH_DATA_Handle hDataObj, GUI_RECT * pRect) {
  GUI_POINT         * pData;
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  GUI_USE_PARA(pRect);
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_ALLOC_h2p(hDataObj);
  pData = (GUI_POINT *)GUI_ALLOC_h2p(pDataXYObj->hData);
  if (pData) {
    GUI_RECT Rect;
    GRAPH_OBJ * pObj;
    int xPos, yPos;
    int BorderL, BorderT, BorderR, BorderB;
    pObj = GRAPH_H2P(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    BorderL = pObj->Props.BorderL;
    BorderT = pObj->Props.BorderT;
    BorderR = pObj->Props.BorderR;
    BorderB = pObj->Props.BorderB;
    WM_GetInsideRectExScrollbar(pDataXYObj->GraphDataObj.PaintObj.hGraph, &Rect);
    Rect.x0 += BorderL;
    Rect.y0 += BorderT;
    Rect.x1 -= BorderR;
    Rect.y1 -= BorderB;
    xPos     = Rect.x0 + pDataXYObj->GraphDataObj.OffX;
    yPos     = Rect.y1 - pDataXYObj->GraphDataObj.OffY;
    pObj = GRAPH_H2P(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    GUI_SetColor(pDataXYObj->GraphDataObj.Color);
    GUI_SetPenSize(pDataXYObj->PenSize);
    GUI_SetLineStyle(pDataXYObj->LineStyle);
    _DrawPolyLine(pData, pDataXYObj->GraphDataObj.NumItems, xPos + pObj->ScrollStateH.v, yPos + pObj->ScrollStateV.v);
  }
}

/*********************************************************************
*
*       _OnDelete
*/
static void _OnDelete(GRAPH_DATA_Handle hDataObj) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_ALLOC_h2p(hDataObj);
  if (pDataXYObj->hData) {
    GUI_ALLOC_Free(pDataXYObj->hData);
  }
}

/*********************************************************************
*
*       _SetOffX
*/
static void _SetOffX(GRAPH_DATA_OBJ * pDataObj, int Value) {
  if (pDataObj) {
    if (pDataObj->OffX != Value) {
      pDataObj->OffX = Value;
      WM_Invalidate(pDataObj->PaintObj.hGraph);
    }
  }
}

/*********************************************************************
*
*       _SetOffY
*/
static void _SetOffY(GRAPH_DATA_OBJ * pDataObj, int Value) {
  if (pDataObj) {
    if (pDataObj->OffY != Value) {
      pDataObj->OffY = Value;
      WM_Invalidate(pDataObj->PaintObj.hGraph);
    }
  }
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GRAPH_DATA_XY_Create
*/
GRAPH_DATA_Handle GRAPH_DATA_XY_Create(GUI_COLOR Color, unsigned MaxNumItems, GUI_POINT * pItems, unsigned NumItems) {
  GRAPH_DATA_Handle  hDataObj;
  WM_LOCK();
  hDataObj = GUI_ALLOC_AllocZero(sizeof(GRAPH_DATA_XY_OBJ));
  if (hDataObj) {
    GRAPH_DATA_XY_OBJ * pDataXYObj;
    WM_HMEM             hData;
    int                 BufferSize;
    BufferSize = sizeof(GUI_POINT) * MaxNumItems;
    hData = GUI_ALLOC_AllocZero(BufferSize);
    if (hData && pItems && NumItems) {
      void * p;
      p = GUI_ALLOC_h2p(hData);
      memcpy(p, pItems, sizeof(GUI_POINT) * NumItems);
    }
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_ALLOC_h2p(hDataObj);
    pDataXYObj->hData = hData;
    pDataXYObj->PenSize = 1;
    pDataXYObj->GraphDataObj.Color               = Color;
    pDataXYObj->GraphDataObj.MaxNumItems         = MaxNumItems;
    pDataXYObj->GraphDataObj.NumItems            = NumItems;
    pDataXYObj->GraphDataObj.PaintObj.pfOnPaint  = _OnPaint;
    pDataXYObj->GraphDataObj.PaintObj.pfOnDelete = _OnDelete;
  }
  WM_UNLOCK();
  return hDataObj;
}

/*********************************************************************
*
*       GRAPH_DATA_XY_AddPoint
*/
void GRAPH_DATA_XY_AddPoint(GRAPH_DATA_Handle hDataObj, GUI_POINT * pPoint) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  WM_LOCK();
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_ALLOC_h2p(hDataObj);
  if (pDataXYObj) {
    GUI_POINT * pData;
    pData = (GUI_POINT *)GUI_ALLOC_h2p(pDataXYObj->hData);
    GRAPH__AddValue(&pDataXYObj->GraphDataObj, pData, pPoint, sizeof(GUI_POINT));
  }
  WM_UNLOCK();
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetOffX
*/
void GRAPH_DATA_XY_SetOffX(GRAPH_DATA_Handle hDataObj, int Off) {
  WM_LOCK();
  _SetOffX((GRAPH_DATA_OBJ *)GUI_ALLOC_h2p(hDataObj), Off);
  WM_UNLOCK();
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetOffY
*/
void GRAPH_DATA_XY_SetOffY(GRAPH_DATA_Handle hDataObj, int Off) {
  WM_LOCK();
  _SetOffY((GRAPH_DATA_OBJ *)GUI_ALLOC_h2p(hDataObj), Off);
  WM_UNLOCK();
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetPenSize
*/
void GRAPH_DATA_XY_SetPenSize(GRAPH_DATA_Handle hDataObj, U8 PenSize) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  WM_LOCK();
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_ALLOC_h2p(hDataObj);
  if (pDataXYObj) {
    if (pDataXYObj->PenSize != PenSize) {
      pDataXYObj->PenSize = PenSize;
      WM_InvalidateWindow(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    }
  }
  WM_UNLOCK();
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetLineStyle
*/
void GRAPH_DATA_XY_SetLineStyle(GRAPH_DATA_Handle hDataObj, U8 LineStyle) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  WM_LOCK();
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_ALLOC_h2p(hDataObj);
  if (pDataXYObj) {
    if (pDataXYObj->LineStyle != LineStyle) {
      pDataXYObj->LineStyle = LineStyle;
      WM_InvalidateWindow(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    }
  }
  WM_UNLOCK();
}

#else                            /* Avoid problems with empty object modules */
  void GRAPH_DATA_XY_C(void);
  void GRAPH_DATA_XY_C(void) {}
#endif  /* #if GUI_WINSUPPORT */
	 	 			 		    	 				 	  			   	 	 	 	 	 	  	  	      	   		 	 	 		  		  	 		 	  	  			     			       	   	 			  		    	 	     	 				  	 					 	 			   	  	  			 				 		 	 	 			     			 
