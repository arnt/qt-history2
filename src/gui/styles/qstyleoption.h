/****************************************************************************
**
** Definition of QStyleOption class.
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
#ifndef NEWSTYLE_H
#define NEWSTYLE_H

#include "qiconset.h"
#include "qslider.h"
#include "qstyle.h"
#include "qtabbar.h"

struct Q4StyleOption {
    int version;
    int type;
    QStyle::SFlags state;
    QRect rect;             // Right now we also use this as the rect for drawing the label too...
    QPalette palette;
    enum { Default, FocusRect, Button, Tab, MenuItem, Complex, Slider };
    enum { Type = Default };
    Q4StyleOption(int optionversion, int optiontype = Default);
    void init(const QWidget *w);
};

struct Q4StyleOptionFocusRect  : public Q4StyleOption {
    enum { Type = FocusRect };
    QColor backgroundColor;
    Q4StyleOptionFocusRect(int version) : Q4StyleOption(version, FocusRect) {}
};

struct Q4StyleOptionButton : public Q4StyleOption {
    enum { Type = Button };
    enum Extras { None = 0x00, Flat = 0x01, HasMenu = 0x02 };
    uint extras;
    QString text;
    QPixmap pixmap;
    QIconSet icon;
    Q4StyleOptionButton(int version) : Q4StyleOption(version, Button) {}
};

struct Q4StyleOptionTab : public Q4StyleOption {
    enum { Type = Tab };
    QTabBar::Shape tabshape;
    QString text;
    QIconSet iconset;
    Q4StyleOptionTab(int version) : Q4StyleOption(version, Tab) {}
};

struct Q4StyleOptionMenuItem : public Q4StyleOption {
    enum { Type = MenuItem };
    enum MenuItemType { Normal = 0x00, Separator = 0x01, Checkmark = 0x02, Hierarchy = 0x04 };
    uint extras;
    bool checked;
    QRect menurect;
    QString text;
    QIconSet icon;
    int maxIconWidth;
    int tabWidth;
    Q4StyleOptionMenuItem(int version) : Q4StyleOption(version, MenuItem) {}
};

struct Q4StyleOptionComplex : public Q4StyleOption
{
    enum { Type = Complex };
    QStyle::SCFlags parts;
    QStyle::SCFlags activeparts;
    Q4StyleOptionComplex(int version, int type) : Q4StyleOption(version, type) {}
};

struct Q4StyleOptionSlider : public Q4StyleOptionComplex {
    enum { Type = Slider };
    Qt::Orientation orientation;
    int minimum;
    int maximum;
    QSlider::TickSetting tickmarks;
    int tickinterval;
    bool useRightToLeft;
    // These two values are typically the same, but different if tracking is not enabled.
    int sliderposition;
    int slidervalue;
    Q4StyleOptionSlider(int version) : Q4StyleOptionComplex(version, Slider) {}
};

template <typename T>
T qt_cast(const Q4StyleOption *opt)
{
    if (opt->type == static_cast<T>(0)->Type)
        return static_cast<T>(const_cast<Q4StyleOption *>(opt));
    return 0;
}
#endif
