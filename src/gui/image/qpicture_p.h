/****************************************************************************
**
** Definition of the QPicturePrivate class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPICTURE_P_H
#define QPICTURE_P_H

#include <qatomic.h>
#include <qbuffer.h>
#include <qobjectdefs.h>
#include <qrect.h>
#include <private/qobject_p.h>

class QPaintEngine;

extern const char  *mfhdr_tag;
extern const Q_UINT16 mfhdr_maj;
extern const Q_UINT16 mfhdr_min;

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
        PdcDrawPoints = 22, // ptarr,ival,ival
        PdcDrawWinFocusRect = 23, // rect,color
        PdcDrawTiledPixmap = 24, // rect,pixmap,point

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

class QPicturePrivate : public QPaintCommands
{
    Q_DECLARE_PUBLIC(QPicture)
    friend class QPicturePaintEngine;
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &s, const QPicture &r);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &s, QPicture &r);

public:
    inline QPicturePrivate() : q_ptr(0) { ref = 1; }
    QAtomic ref;

protected:
    bool checkFormat();
    void resetFormat();

    QBuffer pictb;
    int        trecs;
    bool formatOk;
    int        formatMajor;
    int        formatMinor;
    QRect brect;
    QPaintEngine *paintEngine;

    QPicture *q_ptr;
};

#endif // QPICTURE_P_H
