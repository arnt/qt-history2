/****************************************************************************
**
** Implementation of QColorDialog classes for mac.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcolordialog.h"
#if !defined(QT_NO_COLORDIALOG) && defined(Q_WS_MAC)
#include <qapplication.h>
#include <qdesktopwidget.h>
#include "qt_mac.h"
#include <string.h>

const unsigned char * p_str(const QString &s); //qglobal.cpp

QRgb macGetRgba(QRgb initial, bool *ok, QWidget *parent, const char*)
{
    Point p = { -1, -1 };
    const uchar *pstr = p_str("Choose a color");
    static const int sw = 420, sh = 300;
    if(parent) {
        parent = parent->topLevelWidget();
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
        rval = GetColor(place, pstr, &rgb, &rgbout);
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
    memcpy(cpInfo.prompt, pstr, pstr[0]+1);
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
    free((void *)pstr);
    if(ok)
        (*ok) = rval;
    if(!rval)
        return initial;
    initial = qRgba(rgbout.red / 256, rgbout.green / 256,
                    rgbout.blue / 256, qAlpha(initial));
    return initial;
}

QColor macGetColor(const QColor& initial, QWidget *parent, const char *name)
{
    bool ok;
    QRgb rgb = macGetRgba(initial.rgb(), &ok, parent, name);

    QColor ret;
    if(ok)
        ret = QColor(rgb);
    return ret;
}
#endif
