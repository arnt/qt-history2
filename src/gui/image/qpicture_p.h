#ifndef QPICTURE_P_H
#define QPICTURE_P_H

#include "qbuffer.h"
#include "qrect.h"
#include "qshared.h"

class QPaintEngine;

class QPaintCommands
{
public:
    enum PaintCommand {
	PdcNOP = 0, //  <void>
	PdcDrawPoint = 1, // point
	PdcDrawFirst = PdcDrawPoint,
	PdcMoveTo = 2, // point
	PdcLineTo = 3, // point
	PdcDrawLine = 4, // point,point
	PdcDrawRect = 5, // rect
	PdcDrawRoundRect = 6, // rect,ival,ival
	PdcDrawEllipse = 7, // rect
	PdcDrawArc = 8, // rect,ival,ival
	PdcDrawPie = 9, // rect,ival,ival
	PdcDrawChord = 10, // rect,ival,ival
	PdcDrawLineSegments = 11, // ptarr
	PdcDrawPolyline = 12, // ptarr
	PdcDrawPolygon = 13, // ptarr,ival
	PdcDrawCubicBezier = 14, // ptarr
	PdcDrawText = 15, // point,str
	PdcDrawTextFormatted = 16, // rect,ival,str
	PdcDrawPixmap = 17, // rect,pixmap
	PdcDrawImage = 18, // rect,image
	PdcDrawText2 = 19, // point,str
	PdcDrawText2Formatted = 20, // rect,ival,str
	PdcDrawTextItem = 21,
	PdcDrawLast = PdcDrawTextItem,

	// no painting commands below PdcDrawLast.

	PdcBegin = 30, //  <void>
	PdcEnd = 31, //  <void>
	PdcSave = 32, //  <void>
	PdcRestore = 33, //  <void>
	PdcSetdev = 34, // device - PRIVATE
	PdcSetBkColor = 40, // color
	PdcSetBkMode = 41, // ival
	PdcSetROP = 42, // ival
	PdcSetBrushOrigin = 43, // point
	PdcSetFont = 45, // font
	PdcSetPen = 46, // pen
	PdcSetBrush = 47, // brush
	PdcSetTabStops = 48, // ival
	PdcSetTabArray = 49, // ival,ivec
	PdcSetUnit = 50, // ival
	PdcSetVXform = 51, // ival
	PdcSetWindow = 52, // rect
	PdcSetViewport = 53, // rect
	PdcSetWXform = 54, // ival
	PdcSetWMatrix = 55, // matrix,ival
	PdcSaveWMatrix = 56,
	PdcRestoreWMatrix = 57,
	PdcSetClip = 60, // ival
	PdcSetClipRegion = 61, // rgn

	PdcReservedStart = 0, // codes 0-199 are reserved
	PdcReservedStop = 199 //   for Qt
    };
};

struct QPicturePrivate : public QShared, public QPaintCommands {
    bool checkFormat();
    void resetFormat();

    QBuffer pictb;
    int	trecs;
    bool formatOk;
    int	formatMajor;
    int	formatMinor;
    QRect brect;
    QPaintEngine *paintEngine;
};

#endif // QPICTURE_P_H
