/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcolordialog.h"
#if !defined(QT_NO_COLORDIALOG) && defined(Q_WS_MAC)
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <private/qt_mac_p.h>
#include <string.h>

extern void qt_mac_to_pascal_string(QString s, Str255 str, TextEncoding encoding=0, int len=-1);  //qglobal.cpp

QRgb macGetRgba(QRgb initial, bool *ok, QWidget *parent)
{
    Point p = { -1, -1 };
    Str255 title;
    qt_mac_to_pascal_string("Choose a color", title);
    static const int sw = 420, sh = 300;
    if(parent) {
        parent = parent->window();
        p.h = (parent->x() + (parent->width() / 2)) - (sw / 2);
        p.v = (parent->y() + (parent->height() / 2)) - (sh / 2);
        QRect r = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(parent));
        if(p.h + sw > r.right())
            p.h -= (p.h + sw) - r.right() + 10;
        if(p.v + sh > r.bottom())
            p.v -= (p.v + sh) - r.bottom() + 10;
    } else if(QWidget *w = qApp->mainWidget()) {
        static int last_screen = -1;
        int scr = QApplication::desktop()->screenNumber(w);
        if(last_screen != scr) {
            QRect r = QApplication::desktop()->screenGeometry(scr);
            p.h = (r.x() + (r.width() / 2)) - (sw / 2);
            p.v = (r.y() + (r.height() / 2)) - (sh / 2);
        }
    }
    RGBColor rgb, rgbout;
    rgb.red = qRed(initial) * 256;
    rgb.blue = qBlue(initial) * 256;
    rgb.green = qGreen(initial) * 256;
#if 1
    Point place;
    place.h = p.h == -1 ? 0 : p.h;
    place.v = p.v == -1 ? 0 : p.v;
    Boolean rval = false;
    {
        QMacBlockingFunction block;
        rval = GetColor(place, title, &rgb, &rgbout);
    }
#else
    ColorPickerInfo     cpInfo;
    // Set the input color to be an RGB color in system space.
    cpInfo.theColor.color.rgb.red = rgb.red;
    cpInfo.theColor.color.rgb.green = rgb.green;
    cpInfo.theColor.color.rgb.blue = rgb.blue;
    cpInfo.theColor.profile = 0L;
    cpInfo.dstProfile = 0L;
    cpInfo.flags = kColorPickerAppIsColorSyncAware | kColorPickerCanModifyPalette |
                   kColorPickerCanAnimatePalette;
    // Place dialog
    cpInfo.placeWhere = kCenterOnMainScreen;
    if(p.h != -1 || p.v != -1) {
        cpInfo.placeWhere = kAtSpecifiedOrigin;
        cpInfo.dialogOrigin = p;
    }
    cpInfo.pickerType = 0L; // Use the default picker.
    cpInfo.eventProc = 0L;
    cpInfo.colorProc = 0L;
    cpInfo.colorProcData = 0L;
    memcpy(cpInfo.prompt, title, title[0]+1);
    Boolean rval = false;
    {
        QMacBlockingFunction block;
        rval = (PickColor(&cpInfo) == noErr && cpInfo.newColorChosen);
    }
    if(rval) {
        rval = true;
        if(!cpInfo.theColor.profile) {
            rgbout.red = cpInfo.theColor.color.rgb.red;
            rgbout.green = cpInfo.theColor.color.rgb.green;
            rgbout.blue = cpInfo.theColor.color.rgb.blue;
        } else {
            qDebug("not sure how to handle..");
        }
    }
#endif
    if(ok)
        (*ok) = rval;
    if(!rval)
        return initial;
    initial = qRgba(rgbout.red / 256, rgbout.green / 256,
                    rgbout.blue / 256, qAlpha(initial));
    return initial;
}

QColor macGetColor(const QColor& initial, QWidget *parent)
{
    bool ok;
    QRgb rgb = macGetRgba(initial.rgb(), &ok, parent);

    QColor ret;
    if(ok)
        ret = QColor(rgb);
    return ret;
}
#endif
