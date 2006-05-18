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

#include "qplastiquestyle.h"

#if !defined(QT_NO_STYLE_PLASTIQUE) || defined(QT_PLUGIN)

static const bool UsePixmapCache = true;
static const bool AnimateBusyProgressBar = true;
static const bool AnimateProgressBar = false;
// #define QPlastique_MaskButtons
static const int ProgressBarFps = 25;

#include <qapplication.h>
#include <qbitmap.h>
#include <qabstractitemview.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdialogbuttonbox.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstyleoption.h>
#include <qtextedit.h>
#include <qtoolbar.h>
#include <qtoolbox.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include <limits.h>

// from windows style
static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsSepHeight        =  2; // separator item height
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  2; // menu item ver text margin
static const int windowsArrowHMargin     =  6; // arrow horizontal margin
static const int windowsTabSpacing       = 12; // space between text and tab
static const int windowsCheckMarkHMargin =  2; // horiz. margins of check mark
static const int windowsRightBorder      = 15; // right border on windows
static const int windowsCheckMarkWidth   = 12; // checkmarks width on windows

// checkbox, on
static const char * const qt_plastique_check[] = {
    "9 9 3 1",
    "X c #323232",
    "o c #979797",
    ". c None",
    "oXo...oXo",
    "XXXo.oXXX",
    "oXXXoXXXo",
    ".oXXXXXo.",
    "..oXXXo..",
    ".oXXXXXo.",
    "oXXXoXXXo",
    "XXXo.oXXX",
    "oXo...oXo"};

// checkbox, on and sunken
static const char * const qt_plastique_check_sunken[] = {
    "9 9 4 1",
    "X c #828282",
    "o c #bfbfbf",
    "* c #929292",
    ". c None",
    "oXo...oXo",
    "X*Xo.oX*X",
    "oX*XoX*Xo",
    ".oX*X*Xo.",
    "..oX*Xo..",
    ".oX*X*Xo.",
    "oX*XoX*Xo",
    "X*Xo.oX*X",
    "oXo...oXo"};

static const char * const qt_plastique_radio[] = {
    "13 13 5 1",
    "X c #828282",
    "o c None",
    "- c None",
    "* c None",
    ". c None",
    "...*XXXXX*...",
    "..XXo---oXX..",
    ".Xo-------oX.",
    "*X---------X*",
    "Xo---------oX",
    "X-----------X",
    "X-----------X",
    "X-----------X",
    "Xo---------oX",
    "*X---------X*",
    ".Xo-------oX.",
    "..XXo---oXX..",
    "...*XXXXX*..."};

static const char * const qt_plastique_radioborder[] = {
    "13 13 2 1",
    "X c #000000",
    ". c None",
    "....XXXXX....",
    "..XX.....XX..",
    ".X.........X.",
    ".X.........X.",
    "X...........X",
    "X...........X",
    "X...........X",
    "X...........X",
    "X...........X",
    ".X.........X.",
    ".X.........X.",
    "..XX.....XX..",
    "....XXXXX...."};

static const char * const qt_plastique_radio_outeralpha[] = {
    "13 13 2 1",
    "o c #000000",
    ". c None",
    "...o.....o...",
    ".............",
    ".............",
    "o...........o",
    ".............",
    ".............",
    ".............",
    ".............",
    ".............",
    "o...........o",
    ".............",
    ".............",
    "...o.....o..."};

static const char * const qt_plastique_radio_inneralpha[] = {
    "13 13 2 1",
    "o c #000000",
    ". c None",
    ".............",
    "....o...o....",
    "..o.......o..",
    ".............",
    ".o.........o.",
    ".............",
    ".............",
    ".............",
    ".o.........o.",
    ".............",
    "..o.......o..",
    "....o...o....",
    "............."};


static const char * const qt_plastique_radio_innerhover[] = {
    "11 11 4 1",
    "   c None",
    ".  c #567CB6",
    "+  c #7AA1DB",
    "@  c #ABC3E8",
    "   .+++.   ",
    " .++@@@++. ",
    " +@@   @@+ ",
    ".+@     @+.",
    "+@       @+",
    "+@       @+",
    "+@       @+",
    ".+@     @+.",
    " +@@   @@+ ",
    " .++@@@++. ",
    "   .+++.   "};

static const char * const qt_plastique_radio_check[] = {
    "13 13 3 1",
    ". c None",
    "g c #aa00aa",
    "* c #bb00bb",
    ".............",
    ".............",
    ".............",
    "....g***g....",
    "...g*****g...",
    "...*******...",
    "...*******...",
    "...*******...",
    "...g*****g...",
    "....g***g....",
    ".............",
    ".............",
    "............."};

static const char * const qt_plastique_slider_verticalhandle[] = {
    "15 11 4 1",
    "   c None",
    "+  c #979797",
    "@  c #C9C9C9",
    "$  c #C1C1C1",
    " $++++++++$    ",
    "$+        +$   ",
    "+  $$      +$  ",
    "+  $@       +$ ",
    "+            +$",
    "+             +",
    "+            +$",
    "+  $$       +$ ",
    "+  $@      +$  ",
    "$+        +$   ",
    " $++++++++$    "};

static const char * const qt_plastique_slider_verticalhandle_left[] = {
    "15 11 4 1",
    "   c None",
    "+  c #979797",
    "@  c #C9C9C9",
    "$  c #C1C1C1",
    "    $++++++++$ ",
    "   $+        +$",
    "  $+      $$  +",
    " $+       $@  +",
    "$+            +",
    "+             +",
    "$+            +",
    " $+       $$  +",
    "  $+      $@  +",
    "   $+        +$",
    "    $++++++++$ "};

static const char * const qt_plastique_slider_horizontalhandle[] = {
    "11 15 4 1",
    "   c None",
    "+  c #979797",
    "@  c #C9C9C9",
    "$  c #C1C1C1",
    " $+++++++$ ",
    "$+       +$",
    "+         +",
    "+ $$   $$ +",
    "+ $@   $@ +",
    "+         +",
    "+         +",
    "+         +",
    "+         +",
    "+         +",
    "$+       +$",
    " $+     +$ ",
    "  $+   +$  ",
    "   $+ +$   ",
    "    $+$    "};

static const char * const qt_plastique_slider_horizontalhandle_up[] = {
    "11 15 4 1",
    "   c None",
    "+  c #979797",
    "@  c #C9C9C9",
    "$  c #C1C1C1",
    "    $+$    ",
    "   $+ +$   ",
    "  $+   +$  ",
    " $+     +$ ",
    "$+       +$",
    "+         +",
    "+         +",
    "+         +",
    "+         +",
    "+         +",
    "+ $$   $$ +",
    "+ $@   $@ +",
    "+         +",
    "$+       +$",
    " $+++++++$ "};

static const char * const qt_scrollbar_button_arrow_left[] = {
    "4 7 2 1",
    "   c None",
    "*  c #BFBFBF",
    "   *",
    "  **",
    " ***",
    "****",
    " ***",
    "  **",
    "   *"};

static const char * const qt_scrollbar_button_arrow_right[] = {
    "4 7 2 1",
    "   c None",
    "*  c #BFBFBF",
    "*   ",
    "**  ",
    "*** ",
    "****",
    "*** ",
    "**  ",
    "*   "};

static const char * const qt_scrollbar_button_arrow_up[] = {
    "7 4 2 1",
    "   c None",
    "*  c #BFBFBF",
    "   *   ",
    "  ***  ",
    " ***** ",
    "*******"};

static const char * const qt_scrollbar_button_arrow_down[] = {
    "7 4 2 1",
    "   c None",
    "*  c #BFBFBF",
    "*******",
    " ***** ",
    "  ***  ",
    "   *   "};

static const char * const qt_scrollbar_button_left[] = {
    "16 16 6 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "#  c #FAFAFA",
    "<  c #FAFAFA",
    "*  c #FAFAFA",
    " .+++++++++++++.",
    ".+#############+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    ".+<<<<<<<<<<<<<+",
    " .+++++++++++++."};

static const char * const qt_scrollbar_button_right[] = {
    "16 16 6 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "#  c #FAFAFA",
    "<  c #FAFAFA",
    "*  c #FAFAFA",
    ".+++++++++++++. ",
    "+#############+.",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+<<<<<<<<<<<<<+.",
    ".+++++++++++++. "};

static const char * const qt_scrollbar_button_up[] = {
    "16 16 6 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "#  c #FAFAFA",
    "<  c #FAFAFA",
    "*  c #FAFAFA",
    " .++++++++++++. ",
    ".+############+.",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+<<<<<<<<<<<<<<+",
    ".++++++++++++++."};

static const char * const qt_scrollbar_button_down[] = {
    "16 16 6 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "#  c #FAFAFA",
    "<  c #FAFAFA",
    "*  c #FAFAFA",
    "++++++++++++++++",
    "+##############+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    "+#            <+",
    ".+<<<<<<<<<<<<+.",
    " .++++++++++++. "};

static const char * const qt_scrollbar_slider_pattern_vertical[] = {
    "10 18 3 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "..  ..  ..",
    ".+  .+  .+",
    "          ",
    "          ",
    "..  ..  ..",
    ".+  .+  .+",
    "          ",
    "          ",
    "..  ..  ..",
    ".+  .+  .+",
    "          ",
    "          ",
    "..  ..  ..",
    ".+  .+  .+",
    "          ",
    "          ",
    "..  ..  ..",
    ".+  .+  .+"};

static const char * const qt_scrollbar_slider_pattern_horizontal[] = {
    "18 10 3 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "..  ..  ..  ..  ..",
    ".+  .+  .+  .+  .+",
    "                  ",
    "                  ",
    "..  ..  ..  ..  ..",
    ".+  .+  .+  .+  .+",
    "                  ",
    "                  ",
    "..  ..  ..  ..  ..",
    ".+  .+  .+  .+  .+"};

static const char * const qt_toolbarhandle[] = {
    "6 6 4 1",
    "       c None",
    ".      c #C5C5C5",
    "+      c #EEEEEE",
    "@      c #FAFAFA",
    "..    ",
    ".+@   ",
    " @@   ",
    "   .. ",
    "   .+@",
    "    @@"};

static const char * const qt_simple_toolbarhandle[] = {
    "3 3 4 1",
    "       c None",
    ".      c #C5C5C5",
    "+      c #EEEEEE",
    "@      c #FAFAFA",
    ".. ",
    ".+@",
    " @@"};

static const char * const qt_titlebar_context_help[] = {
"27 27 5 1",
"  c None",
". c #0A0C12",
"+ c #1B202D",
"@ c #293144",
"# c #3C435D",
"                           ",
"                           ",
"                           ",
"                           ",
"                           ",
"                           ",
"                           ",
"                           ",
"           +@##@+          ",
"         .@@@.+@@..        ",
"         .##+  +@@+.       ",
"         .##@  @#@+.       ",
"         ....  +@+..       ",
"            .@+@@..        ",
"            +#@@+          ",
"            .##.           ",
"            .++.           ",
"            .++.           ",
"            +##+           ",
"            .@@.           ",
"                           ",
"                           ",
"                           ",
"                           ",
"                           ",
"                           ",
"                           "};

static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

static QString uniqueName(const QString &key, const QStyleOption *option, const QSize &size)
{
    QString tmp;
    const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
    tmp.sprintf("%s-%d-%d-%d-%dx%d", key.toLatin1().constData(), uint(option->state),
                complexOption ? uint(complexOption->activeSubControls) : uint(0),
                option->palette.serialNumber(), size.width(), size.height());
    return tmp;
}

static void qt_plastique_draw_gradient(QPainter *painter, const QRect &rect, const QColor &gradientStart,
                                       const QColor &gradientStop)
{
    QString gradientName;
    gradientName.sprintf("%dx%d-%x-%x", rect.width(), rect.height(), gradientStart.rgba(), gradientStop.rgba());
    QPixmap cache;
    if (!UsePixmapCache || !QPixmapCache::find(gradientName, cache)) {
        cache = QPixmap(rect.size());
        cache.fill(Qt::white);
        QPainter cachePainter(&cache);
        QRect pixmapRect(0, 0, rect.width(), rect.height());
        int x = pixmapRect.center().x();
        QLinearGradient gradient(x, pixmapRect.top(), x, pixmapRect.bottom());
        gradient.setColorAt(0, gradientStart);
        gradient.setColorAt(1, gradientStop);
        cachePainter.fillRect(pixmapRect, gradient);
        cachePainter.end();
        if (UsePixmapCache)
            QPixmapCache::insert(gradientName, cache);
    }
    painter->drawPixmap(rect, cache);
}

static void qt_plastique_drawFrame(QPainter *painter, const QStyleOption *option, const QWidget *widget)
{
    QRect rect = option->rect;
    QPen oldPen = painter->pen();

    QColor borderColor = option->palette.background().color().dark(178);
    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);
    QColor alphaCornerColor;
    if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
    } else {
        alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    }

    // outline / border
    painter->setPen(borderColor);
    painter->drawLine(rect.left() + 2, rect.top(), rect.right() - 2, rect.top());
    painter->drawLine(rect.left() + 2, rect.bottom(), rect.right() - 2, rect.bottom());
    painter->drawLine(rect.left(), rect.top() + 2, rect.left(), rect.bottom() - 2);
    painter->drawLine(rect.right(), rect.top() + 2, rect.right(), rect.bottom() - 2);
    painter->drawPoint(rect.left() + 1, rect.top() + 1);
    painter->drawPoint(rect.right() - 1, rect.top() + 1);
    painter->drawPoint(rect.left() + 1, rect.bottom() - 1);
    painter->drawPoint(rect.right() - 1, rect.bottom() - 1);

    painter->setPen(alphaCornerColor);
    painter->drawPoint(rect.left() + 1, rect.top());
    painter->drawPoint(rect.right() - 1, rect.top());
    painter->drawPoint(rect.left() + 1, rect.bottom());
    painter->drawPoint(rect.right() - 1, rect.bottom());
    painter->drawPoint(rect.left(), rect.top() + 1);
    painter->drawPoint(rect.right(), rect.top() + 1);
    painter->drawPoint(rect.left(), rect.bottom() - 1);
    painter->drawPoint(rect.right(), rect.bottom() - 1);

    // inner border
    if ((option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On))
        painter->setPen(option->palette.button().color().dark(118));
    else
        painter->setPen(gradientStartColor);
    painter->drawLine(rect.left() + 2, rect.top() + 1, rect.right() - 2, option->rect.top() + 1);
    painter->drawLine(rect.left() + 1, rect.top() + 2, rect.left() + 1, option->rect.bottom() - 2);

    if ((option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On))
        painter->setPen(option->palette.button().color().dark(110));
    else
        painter->setPen(gradientStopColor.dark(102));
    painter->drawLine(rect.left() + 2, rect.bottom() - 1, rect.right() - 2, rect.bottom() - 1);
    painter->drawLine(rect.right() - 1, rect.top() + 2, rect.right() - 1, rect.bottom() - 2);

    painter->setPen(oldPen);
}

static void qt_plastique_drawShadedPanel(QPainter *painter, const QStyleOption *option, bool base,
                                         const QWidget *widget)
{
    QRect rect = option->rect;
    QPen oldPen = painter->pen();

    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);

    // gradient fill
    if ((option->state & QStyle::State_Enabled) || !(option->state & QStyle::State_AutoRaise)) {
        if ((option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On)) {
            qt_plastique_draw_gradient(painter, rect.adjusted(1, 1, -1, -1),
                                       option->palette.button().color().dark(114),
                                       option->palette.button().color().dark(106));
        } else {
            qt_plastique_draw_gradient(painter, rect.adjusted(1, 1, -1, -1),
                                       base ? option->palette.background().color().light(105) : gradientStartColor,
                                       base ? option->palette.background().color().dark(102) : gradientStopColor);
        }
    }

    qt_plastique_drawFrame(painter, option, widget);

    painter->setPen(oldPen);
}

static void qt_plastique_draw_mdibutton(QPainter *painter, const QStyleOptionTitleBar *option, const QRect &tmp, bool hover)
{
    if (tmp.isNull())
        return;
    bool active = (option->titleBarState & QStyle::State_Active);

    // ### use palette colors instead
    QColor mdiButtonGradientStartColor;
    QColor mdiButtonGradientStopColor;
    if (active) {
        mdiButtonGradientStartColor = QColor(hover ? 0x7d8bb1 : 0x55689a);
        mdiButtonGradientStopColor = QColor(hover ? 0x939ebe : 0x7381ab);
    } else {
        mdiButtonGradientStartColor = QColor(hover ? 0x9e9e9e : 0x818181);
        mdiButtonGradientStopColor = QColor(hover ? 0xababab : 0x929292);
    }

    qt_plastique_draw_gradient(painter, tmp.adjusted(1, 1, -1, -1),
                               mdiButtonGradientStartColor, mdiButtonGradientStopColor);

    QColor mdiButtonBorderColor;
    if (active) {
        mdiButtonBorderColor = hover ? QColor(0x627097) : QColor(0x324577);
    } else {
        mdiButtonBorderColor = hover ? QColor(0x838383) : QColor(0x5e5e5e);
    }
    painter->setPen(QPen(mdiButtonBorderColor, 1));
    painter->drawLine(tmp.left() + 2, tmp.top(), tmp.right() - 2, tmp.top());
    painter->drawLine(tmp.left() + 2, tmp.bottom(), tmp.right() - 2, tmp.bottom());
    painter->drawLine(tmp.left(), tmp.top() + 2, tmp.left(), tmp.bottom() - 2);
    painter->drawLine(tmp.right(), tmp.top() + 2, tmp.right(), tmp.bottom() - 2);
    painter->drawPoint(tmp.left() + 1, tmp.top() + 1);
    painter->drawPoint(tmp.right() - 1, tmp.top() + 1);
    painter->drawPoint(tmp.left() + 1, tmp.bottom() - 1);
    painter->drawPoint(tmp.right() - 1, tmp.bottom() - 1);
}

#ifndef QT_NO_DOCKWIDGET
static QString elliditide(const QString &text, const QFontMetrics &fontMetrics, const QRect &rect, int *textWidth = 0)
{
    // Chop and insert ellide into title if text is too wide
    QString title = text;
    int width = textWidth ? *textWidth : fontMetrics.width(text);
    QString ellipsis = QLatin1String("...");
    if (width > rect.width()) {
        QString leftHalf = title.left(title.size() / 2);
        QString rightHalf = title.mid(leftHalf.size() + 1);
        while (!leftHalf.isEmpty() && !rightHalf.isEmpty()) {
            leftHalf.chop(1);
            int width = fontMetrics.width(leftHalf + ellipsis + rightHalf);
            if (width < rect.width()) {
                title = leftHalf + ellipsis + rightHalf;
                width = width;
                break;
            }
            rightHalf.remove(0, 1);
            width = fontMetrics.width(leftHalf + ellipsis + rightHalf);
            if (width < rect.width()) {
                title = leftHalf + ellipsis + rightHalf;
                width = width;
                break;
            }
        }
    }
    if (textWidth)
        *textWidth = width;
    return title;
}
#endif // QT_NO_DOCKWIDGET

#if !defined(QT_NO_DOCKWIDGET) || !defined(QT_NO_SPLITTER)
static void qt_plastique_draw_handle(QPainter *painter, const QStyleOption *option,
                                     const QRect &rect, Qt::Orientation orientation,
                                     const QWidget *widget)
{
    QColor borderColor = option->palette.background().color().dark(178);
    QColor alphaCornerColor;
    if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
    } else {
        alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    }

    QImage handle(qt_simple_toolbarhandle);
    handle.setColor(1, alphaCornerColor.rgba());
    handle.setColor(2, mergedColors(alphaCornerColor, option->palette.base().color()).rgba());
    handle.setColor(3, option->palette.base().color().rgba());

    const int spacing = 2;

    if (orientation == Qt::Vertical) {
        int nchunks = rect.width() / (handle.width() + spacing);
        for (int i = 0; i < nchunks; ++i)
            painter->drawImage(QPoint(rect.left() + i * (handle.width() + spacing), rect.top()), handle);
    } else {
        int nchunks = rect.height() / (handle.height() + spacing);
        for (int i = 0; i < nchunks; ++i)
            painter->drawImage(QPoint(rect.left(), rect.top() + i * (handle.height() + spacing)), handle);
    }
}
#endif

class QPlastiqueStylePrivate
{
public:
    QPlastiqueStylePrivate(QPlastiqueStyle *qq);
    virtual ~QPlastiqueStylePrivate();

#ifndef QT_NO_PROGRESSBAR
    QList<QProgressBar *> bars;
    int progressBarAnimateTimer;
    QTime timer;
    int animateStep;
#endif

    QPlastiqueStyle *q;
};

/*!
  \internal
 */
QPlastiqueStylePrivate::QPlastiqueStylePrivate(QPlastiqueStyle *qq) :
#ifndef QT_NO_PROGRESSBAR
    progressBarAnimateTimer(0), animateStep(0),
#endif
    q(qq)
{
}

/*!
  \internal
 */
QPlastiqueStylePrivate::~QPlastiqueStylePrivate()
{
}

/*!
    \class QPlastiqueStyle
    \brief The QPlastiqueStyle class provides a widget style similar to the
    Plastik style available in KDE.

    The Plastique style provides a default look and feel for widgets on X11
    that closely resembles the Plastik style, introduced by Sandro Giessl in
    KDE 3.2.

    \img qplastiquestyle.png
    \sa QWindowsXPStyle, QMacStyle, QWindowsStyle, QCDEStyle, QMotifStyle
*/

/*!
    Constructs a QPlastiqueStyle object.
*/
QPlastiqueStyle::QPlastiqueStyle()
    : QWindowsStyle(), d(new QPlastiqueStylePrivate(this))
{
}

/*!
    Destructs the QPlastiqueStyle object.
*/
QPlastiqueStyle::~QPlastiqueStyle()
{
    delete d;
}

/*!
  \reimp
*/
void QPlastiqueStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                    QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);

    QColor borderColor = option->palette.background().color().dark(178);
    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);

    QColor baseGradientStartColor = option->palette.base().color().dark(101);
    QColor baseGradientStopColor = option->palette.base().color().dark(106);

    QColor highlightedGradientStartColor = option->palette.button().color().light(101);
    QColor highlightedGradientStopColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 85);

    QColor highlightedBaseGradientStartColor = option->palette.base().color();
    QColor highlightedBaseGradientStopColor = mergedColors(option->palette.base().color().dark(105), option->palette.highlight().color(), 70);

    QColor highlightedDarkInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 35);
    QColor highlightedLightInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 58);

    QColor alphaCornerColor;
    if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
    } else {
        alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    }
    QColor alphaInnerColor = mergedColors(highlightedLightInnerBorderColor, gradientStartColor);

    QColor alphaInnerColorNoHover = mergedColors(borderColor, gradientStartColor);
    QColor alphaTextColor = mergedColors(option->palette.background().color(), option->palette.text().color());

    QColor alphaLightTextColor = mergedColors(option->palette.background().color().light(250), option->palette.text().color().light(250));

    QColor lightShadow = option->palette.button().color().light(105);

    QColor shadowGradientStartColor = option->palette.button().color().dark(115);
    QColor shadow = shadowGradientStartColor;

    switch (element) {
    case PE_IndicatorButtonDropDown:
        drawPrimitive(PE_PanelButtonTool, option, painter, widget);
        break;
    case PE_FrameDefaultButton:
        // Draws the frame around a default button (drawn in
        // PE_PanelButtonCommand).
        break;
#ifndef QT_NO_TABWIDGET
    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            if (twf->shape != QTabBar::RoundedNorth && twf->shape != QTabBar::RoundedWest &&
                twf->shape != QTabBar::RoundedSouth && twf->shape != QTabBar::RoundedEast) {
                QWindowsStyle::drawPrimitive(element, option, painter, widget);
                break;
            }

            int borderThickness = pixelMetric(PM_TabBarBaseOverlap, twf, widget);
            bool reverse = (twf->direction == Qt::RightToLeft);

            painter->save();

            QRect tabBarRect;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
                if (reverse)
                    tabBarRect = QRect(twf->rect.right() - twf->leftCornerWidgetSize.width() - twf->tabBarSize.width() + 1, twf->rect.top(), twf->tabBarSize.width(), borderThickness);
                else
                    tabBarRect = QRect(twf->rect.left() + twf->leftCornerWidgetSize.width(), twf->rect.top(), twf->tabBarSize.width(), borderThickness);
                break ;
            case QTabBar::RoundedWest:
                tabBarRect = QRect(twf->rect.left(), twf->rect.top() + twf->leftCornerWidgetSize.height(), borderThickness, twf->tabBarSize.height());
                break ;
            case QTabBar::RoundedEast:
                tabBarRect = QRect(twf->rect.right() - borderThickness + 1, twf->rect.top()  + twf->leftCornerWidgetSize.height(),
                                   borderThickness, twf->tabBarSize.height());
                break ;
            case QTabBar::RoundedSouth:
                if (reverse)
                    tabBarRect = QRect(twf->rect.right() - twf->leftCornerWidgetSize.width() - twf->tabBarSize.width() + 1,
                                       twf->rect.bottom() - borderThickness + 1, twf->tabBarSize.width(), borderThickness);
                else
                    tabBarRect = QRect(twf->rect.left() + twf->leftCornerWidgetSize.width(),
                                       twf->rect.bottom() - borderThickness + 1, twf->tabBarSize.width(), borderThickness);
                break ;
            default:
                break;
            }

            QRegion region(twf->rect);
            region -= tabBarRect;
            painter->setClipRegion(region);

            // Outer border
            QLine leftLine = QLine(twf->rect.topLeft() + QPoint(0, 2), twf->rect.bottomLeft() - QPoint(0, 2));
            QLine rightLine = QLine(twf->rect.topRight() + QPoint(0, 2), twf->rect.bottomRight() - QPoint(0, 2));
            QLine bottomLine = QLine(twf->rect.bottomLeft() + QPoint(2, 0), twf->rect.bottomRight() - QPoint(2, 0));
            QLine topLine = QLine(twf->rect.topLeft() + QPoint(2, 0), twf->rect.topRight() - QPoint(2, 0));

            painter->setPen(borderColor);
            painter->drawLine(topLine);

            // Inner border
            QLine innerLeftLine = QLine(leftLine.p1() + QPoint(1, 0), leftLine.p2() + QPoint(1, 0));
            QLine innerRightLine = QLine(rightLine.p1() - QPoint(1, 0), rightLine.p2() - QPoint(1, 0));
            QLine innerBottomLine = QLine(bottomLine.p1() - QPoint(0, 1), bottomLine.p2() - QPoint(0, 1));
            QLine innerTopLine = QLine(topLine.p1() + QPoint(0, 1), topLine.p2() + QPoint(0, 1));

            // Rounded Corner
            QPoint leftBottomOuterCorner = QPoint(innerLeftLine.p2() + QPoint(0, 1));
            QPoint leftBottomInnerCorner1 = QPoint(leftLine.p2() + QPoint(0, 1));
            QPoint leftBottomInnerCorner2 = QPoint(bottomLine.p1() - QPoint(1, 0));
            QPoint rightBottomOuterCorner = QPoint(innerRightLine.p2() + QPoint(0, 1));
            QPoint rightBottomInnerCorner1 = QPoint(rightLine.p2() + QPoint(0, 1));
            QPoint rightBottomInnerCorner2 = QPoint(bottomLine.p2() + QPoint(1, 0));
            QPoint rightTopOuterCorner = QPoint(innerRightLine.p1() - QPoint(0, 1));
            QPoint rightTopInnerCorner1 = QPoint(rightLine.p1() - QPoint(0, 1));
            QPoint rightTopInnerCorner2 = QPoint(topLine.p2() + QPoint(1, 0));
            QPoint leftTopOuterCorner = QPoint(innerLeftLine.p1() - QPoint(0, 1));
            QPoint leftTopInnerCorner1 = QPoint(leftLine.p1() - QPoint(0, 1));
            QPoint leftTopInnerCorner2 = QPoint(topLine.p1() - QPoint(1, 0));

            painter->setPen(borderColor);
            painter->drawLine(leftLine);
            painter->drawLine(rightLine);
            painter->drawLine(bottomLine);
            painter->drawPoint(leftBottomOuterCorner);
            painter->drawPoint(rightBottomOuterCorner);
            painter->drawPoint(rightTopOuterCorner);
            painter->drawPoint(leftTopOuterCorner);

            painter->setPen(lightShadow);
            painter->drawLine(innerLeftLine);
            painter->drawLine(innerTopLine);

            painter->setPen(shadow);
            painter->drawLine(innerRightLine);
            painter->drawLine(innerBottomLine);

            painter->setPen(alphaCornerColor);
            painter->drawPoint(leftBottomInnerCorner1);
            painter->drawPoint(leftBottomInnerCorner2);
            painter->drawPoint(rightBottomInnerCorner1);
            painter->drawPoint(rightBottomInnerCorner2);
            painter->drawPoint(rightTopInnerCorner1);
            painter->drawPoint(rightTopInnerCorner2);
            painter->drawPoint(leftTopInnerCorner1);
            painter->drawPoint(leftTopInnerCorner2);

            painter->restore();
        }
        break ;
#endif // QT_NO_TABWIDGET
#ifndef QT_NO_TABBAR
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            if (tbb->shape != QTabBar::RoundedNorth && tbb->shape != QTabBar::RoundedWest &&
                tbb->shape != QTabBar::RoundedSouth && tbb->shape != QTabBar::RoundedEast) {
                QWindowsStyle::drawPrimitive(element, option, painter, widget);
                break;
            }

            painter->save();

            QRegion region(tbb->rect);
            region -= tbb->tabBarRect;
            painter->setClipRegion(region);

            QLine topLine = QLine(tbb->rect.bottomLeft() - QPoint(0, 1), tbb->rect.bottomRight() - QPoint(0, 1));
            QLine bottomLine = QLine(tbb->rect.bottomLeft(), tbb->rect.bottomRight());

            if (tbb->shape == QTabBar::RoundedSouth)
                painter->setPen(alphaCornerColor);
            else
                painter->setPen(borderColor);
            painter->drawLine(topLine);

            if (tbb->shape != QTabBar::RoundedSouth)
                painter->setPen(lightShadow);
            else
                painter->setPen(borderColor);
            painter->drawLine(bottomLine);

            painter->restore();
        }
        break ;
#endif // QT_NO_TABBAR
#ifndef QT_NO_GROUPBOX
    case PE_FrameGroupBox:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            QStyleOptionFrameV2 frameV2(*frame);
            if (frameV2.features & QStyleOptionFrameV2::Flat) {
                QPen oldPen = painter->pen();
                painter->setPen(borderColor);
                painter->drawLine(frameV2.rect.topLeft(), frameV2.rect.topRight());
                painter->setPen(oldPen);
            } else {
                frameV2.state &= ~(State_Sunken | State_HasFocus);
                drawPrimitive(PE_Frame, &frameV2, painter, widget);
            }
        }
        break;
#endif // QT_NO_GROUPBOX
#ifndef QT_NO_LINEEDIT
    case PE_FrameLineEdit:
        if (widget && widget->parent()) {
            // Line edits use QPalette::Base as background role, so if we can
            // get the parent's background role, we'll plot that into the four
            // corner pixels.
            QColor backgroundColor = option->palette.color(widget->parentWidget()->backgroundRole());
            QPen oldPen = painter->pen();
            painter->setPen(backgroundColor);
            painter->drawPoint(option->rect.topLeft());
            painter->drawPoint(option->rect.topRight());
            painter->drawPoint(option->rect.bottomLeft());
            painter->drawPoint(option->rect.bottomRight());
            painter->setPen(oldPen);
            alphaCornerColor = mergedColors(backgroundColor, borderColor);
        }
        // fall through
#endif // QT_NO_LINEEDIT
    case PE_Frame:
#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3ToolBar")) {
            QPen oldPen = painter->pen();
            painter->setPen(option->palette.background().color().light(104));
            painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
            painter->setPen(alphaCornerColor);
            painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
            painter->setPen(oldPen);
            break;
        }
        if (widget && widget->inherits("Q3DockWindow")) {
            // Don't draw a frame around docked dock windows.
            break;
        }
#endif
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            painter->save();
            bool focus = (frame->state & State_Enabled) && (frame->state & State_HasFocus);
            bool groupbox = element == PE_FrameGroupBox;

            int lw = 1;
            int mlw = 1;

            // Don't show frames in tiny rects
            if (lw + mlw > frame->rect.width() || lw + mlw > frame->rect.height())
                break;

            // Outer border, left side and top side
            QColor color = focus ? highlightedDarkInnerBorderColor : borderColor;
            painter->fillRect(QRect(frame->rect.left() + lw + mlw,frame->rect.top(),
                                    frame->rect.width() - lw*2 - mlw*2,lw),color); // top line
            painter->fillRect(QRect(frame->rect.left(), frame->rect.top() + lw + mlw,
                                    lw, frame->rect.height() - lw*2 - mlw*2),color); // left line

            // Line ends
            QColor alphaLineEnds;
            if (element == PE_FrameLineEdit && widget && widget->parent()) {
                // Line edits use QPalette::Base as background role, so we use
                // the parent's background role to calculate the alpha line
                // end pixels.
                alphaLineEnds = mergedColors(frame->palette.color(widget->parentWidget()->backgroundRole()), color);
            } else {
                alphaLineEnds = mergedColors(frame->palette.background().color(), color);
            }
            painter->fillRect(QRect(frame->rect.left() + lw, frame->rect.top(), mlw, lw),
                              alphaLineEnds);
            painter->fillRect(QRect(frame->rect.right() - lw - mlw + 1, frame->rect.top(), mlw, lw),
                              alphaLineEnds);
            painter->fillRect(QRect(frame->rect.left(),frame->rect.top() + lw, lw, mlw),
                              alphaLineEnds);
            painter->fillRect(QRect(frame->rect.left(), frame->rect.bottom() - lw - mlw + 1, lw, mlw),
                              alphaLineEnds);

            // Outer border, right side and bottom side
            painter->fillRect(QRect(frame->rect.left() + lw + mlw, frame->rect.bottom() - lw + 1,
                                    frame->rect.width() - lw*2 - mlw*2, lw), color); // bottom line
            painter->fillRect(QRect(frame->rect.right() - lw + 1, frame->rect.top() + lw + mlw,
                                    lw, frame->rect.height() - lw*2 - mlw*2), color); // right line

            // Line ends
            painter->fillRect(QRect(frame->rect.left() + lw, frame->rect.bottom() - lw + 1, mlw, lw),
                              alphaLineEnds);
            painter->fillRect(QRect(frame->rect.right() - lw - mlw + 1, frame->rect.bottom() - lw + 1, mlw, lw),
                              alphaLineEnds);
            painter->fillRect(QRect(frame->rect.right() - lw + 1, frame->rect.top() + lw, lw, mlw),
                              alphaLineEnds);
            painter->fillRect(QRect(frame->rect.right() - lw + 1, frame->rect.bottom() - lw - mlw + 1, lw, mlw),
                              alphaLineEnds);

            // Only show inner frame for raised and sunken states
            if ((frame->state & State_Raised) || (frame->state & State_Sunken)) {
                if (frame->state & State_Raised) {
                    color = focus ? option->palette.highlight().color().light(101)
                            : option->palette.button().color().light(101);
                } else {
                    if (focus) {
                        color = mergedColors(option->palette.color(widget ? widget->backgroundRole() : QPalette::Base),
                                             option->palette.highlight().color().dark(130), 10);
                    } else {
                        color = mergedColors(option->palette.color(widget ? widget->backgroundRole() : QPalette::Base),
                                             borderColor, 30);
                    }
                }

                // Inner border, top and left (just the line ends drawn for group boxes)
                if (!groupbox) {
                    painter->fillRect(QRect(frame->rect.left() + lw + mlw,frame->rect.top() + lw,
                                            frame->rect.width() - lw*2 - mlw*2,mlw), color); // top line
                    painter->fillRect(QRect(frame->rect.left() + lw, frame->rect.top() + lw + mlw,
                                            mlw, frame->rect.height() - lw*2 - mlw*2),color); // left line
                }

                // Line ends
                QColor lineEndColor = focus ? highlightedDarkInnerBorderColor : borderColor.dark(112);
                painter->fillRect(QRect(frame->rect.left() + lw, frame->rect.top() + lw, mlw, mlw),
                                  lineEndColor);
                painter->fillRect(QRect(frame->rect.right() - lw - mlw + 1, frame->rect.top() + lw, mlw, mlw),
                                  lineEndColor);
                painter->fillRect(QRect(frame->rect.left() + lw, frame->rect.bottom() - lw - mlw + 1, mlw, mlw),
                                  lineEndColor);

                if (frame->state & State_Raised) {
                    color = focus ? option->palette.highlight().color().dark(130)
                            : option->palette.button().color().dark(130);
                } else {
                    if (focus) {
                        color = mergedColors(option->palette.color(widget ? widget->backgroundRole() : QPalette::Base),
                                             option->palette.highlight().color(), 10);
                    } else {
                        color = mergedColors(option->palette.color(widget ? widget->backgroundRole() : QPalette::Base),
                                             borderColor, 80);
                    }
                }

                // Inner border, bottom and right (just the line ends drawn for group boxes)
                if (!groupbox) {
                    painter->fillRect(QRect(frame->rect.left() + lw + mlw, frame->rect.bottom() - lw - mlw + 1,
                                            frame->rect.width() - lw*2 - mlw*2, mlw), color);
                    painter->fillRect(QRect(frame->rect.right() - lw - mlw + 1, frame->rect.top() + lw + mlw,
                                            mlw, frame->rect.height() - lw*2 - mlw*2), color);
                }

                // Line ends
                painter->fillRect(QRect(frame->rect.left() + lw, frame->rect.bottom() - lw - mlw + 1, mlw, mlw),
                                  lineEndColor);
                painter->fillRect(QRect(frame->rect.right() - lw - mlw + 1, frame->rect.bottom() - lw - mlw + 1, mlw, mlw),
                                  lineEndColor);
                painter->fillRect(QRect(frame->rect.right() - lw - mlw + 1, frame->rect.top() + lw, mlw, mlw),
                                  lineEndColor);
            } else {
                // just draw the inner corners
                painter->setPen(focus ? highlightedDarkInnerBorderColor : borderColor);
                painter->drawPoint(frame->rect.left() + 1, frame->rect.top() + 1);
                painter->drawPoint(frame->rect.right() - 1, frame->rect.top() + 1);
                painter->drawPoint(frame->rect.left() + 1, frame->rect.bottom() - 1);
                painter->drawPoint(frame->rect.right() - 1, frame->rect.bottom() - 1);
            }
            painter->restore();
        }
        break ;
    case PE_FrameDockWidget:
    case PE_FrameMenu:
    case PE_FrameStatusBar: {
        // Draws the frame around a popup menu.
        QPen oldPen = painter->pen();
        painter->setPen(borderColor);
        painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
        painter->setPen(alphaCornerColor);
        painter->drawPoint(option->rect.topLeft());
        painter->drawPoint(option->rect.topRight());
        painter->drawPoint(option->rect.bottomLeft());
        painter->drawPoint(option->rect.bottomRight());
        painter->setPen(oldPen);
        break;
    }
#ifdef QT3_SUPPORT
    case PE_Q3DockWindowSeparator: {
        QPen oldPen = painter->pen();
        painter->setPen(alphaCornerColor);
        QRect rect = option->rect;
        if (option->state & State_Horizontal) {
            painter->drawLine(rect.right(), rect.top() + 2, rect.right(), rect.bottom() - 1);
        } else {
            painter->drawLine(rect.left() + 2, rect.bottom(), rect.right() - 1, rect.bottom());
        }
        painter->setPen(oldPen);
        break;
    }
    case PE_Q3Separator: {
        QPen oldPen = painter->pen();
        painter->setPen(alphaCornerColor);
        if ((option->state & State_Horizontal) == 0)
            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        else
            painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
        painter->setPen(option->palette.background().color().light(104));
        if ((option->state & State_Horizontal) == 0)
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
        else
            painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
        painter->setPen(oldPen);
        break;
    }
#endif // QT3_SUPPORT
#ifndef QT_NO_MAINWINDOW
    case PE_PanelMenuBar:
        if (widget && qobject_cast<const QMainWindow *>(widget->parentWidget())
#ifdef QT3_SUPPORT
            || (widget->parentWidget() && widget->parentWidget()->inherits("Q3MainWindow"))
#endif
            ) {
            // Draws the light line above and the dark line below menu bars and
            // tool bars.
            QPen oldPen = painter->pen();
            if (element == PE_PanelMenuBar || (option->state & State_Horizontal)) {
                painter->setPen(alphaCornerColor);
                painter->drawLine(option->rect.left(), option->rect.bottom(),
                                  option->rect.right(), option->rect.bottom());
                painter->setPen(option->palette.background().color().light(104));
                painter->drawLine(option->rect.left(), option->rect.top(),
                                  option->rect.right(), option->rect.top());
            } else {
                painter->setPen(option->palette.background().color().light(104));
                painter->drawLine(option->rect.left(), option->rect.top(),
                                  option->rect.left(), option->rect.bottom());
                painter->setPen(alphaCornerColor);
                painter->drawLine(option->rect.right(), option->rect.top(),
                                  option->rect.right(), option->rect.bottom());
            }
            painter->setPen(oldPen);
        }
        break;
#endif // QT_NO_MAINWINDOW
    case PE_IndicatorHeaderArrow: {
        bool usedAntialiasing = painter->renderHints() & QPainter::Antialiasing;
        if (!usedAntialiasing)
            painter->setRenderHint(QPainter::Antialiasing);
        QWindowsStyle::drawPrimitive(element, option, painter, widget);
        if (!usedAntialiasing)
            painter->setRenderHint(QPainter::Antialiasing, false);
        break;
    }
    case PE_PanelButtonTool:
        // Draws a tool button (f.ex., in QToolBar and QTabBar)
        if ((option->state & State_Enabled) || !(option->state & State_AutoRaise))
            qt_plastique_drawShadedPanel(painter, option, true, widget);
        break;
#ifndef QT_NO_TOOLBAR
    case PE_IndicatorToolBarHandle: {
        QPixmap cache;
        QRect rect = option->rect;
#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3DockWindowHandle") && widget->parentWidget()->inherits("Q3DockWindow")) {
            if (!(option->state & State_Horizontal))
                rect.adjust(2, 0, -2, 0);
        }
#endif
        QString pixmapName = uniqueName("toolbarhandle", option, rect.size());
        if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
            cache = QPixmap(rect.size());
            cache.fill(Qt::blue);
            QPainter cachePainter(&cache);
            QRect cacheRect(QPoint(0, 0), rect.size());
            if (widget)
                cachePainter.fillRect(cacheRect, option->palette.brush(widget->backgroundRole()));
            else
                cachePainter.fillRect(cacheRect, option->palette.background());

            QImage handle(qt_toolbarhandle);
            handle.setColor(1, alphaCornerColor.rgba());
            handle.setColor(2, mergedColors(alphaCornerColor, option->palette.base().color()).rgba());
            handle.setColor(3, option->palette.base().color().rgba());

            if (option->state & State_Horizontal) {
                int nchunks = cacheRect.height() / handle.height();
                int indent = (cacheRect.height() - (nchunks * handle.height())) / 2;
                for (int i = 0; i < nchunks; ++i)
                    cachePainter.drawImage(QPoint(cacheRect.left() + 3, cacheRect.top() + indent + i * handle.height()),
                                           handle);
            } else {
                int nchunks = cacheRect.width() / handle.width();
                int indent = (cacheRect.width() - (nchunks * handle.width())) / 2;
                for (int i = 0; i < nchunks; ++i)
                    cachePainter.drawImage(QPoint(cacheRect.left() + indent + i * handle.width(), cacheRect.top() + 3),
                                           handle);
            }
            cachePainter.end();
            if (UsePixmapCache)
                QPixmapCache::insert(pixmapName, cache);
        }
        painter->drawPixmap(rect.topLeft(), cache);
        break;
    }
    case PE_IndicatorToolBarSeparator: {
        QPen oldPen = painter->pen();
        painter->setPen(alphaCornerColor);
        if (option->state & State_Horizontal) {
            painter->drawLine(option->rect.left(), option->rect.top() + 1, option->rect.left(), option->rect.bottom() - 2);
            painter->setPen(option->palette.base().color());
            painter->drawLine(option->rect.right(), option->rect.top() + 1, option->rect.right(), option->rect.bottom() - 2);
        } else {
            painter->drawLine(option->rect.left() + 1, option->rect.top(), option->rect.right() - 2, option->rect.top());
            painter->setPen(option->palette.base().color());
            painter->drawLine(option->rect.left() + 1, option->rect.bottom(), option->rect.right() - 2, option->rect.bottom());
        }
        painter->setPen(oldPen);
        break;
    }
#endif // QT_NO_TOOLBAR
    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            bool down = (button->state & State_Sunken) || (button->state & State_On);
            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);
            bool isDefault = (button->features & QStyleOptionButton::DefaultButton);
            bool isEnabled = (button->state & State_Enabled);
            QRect rect = option->rect;

            QPixmap cache;
            QString pixmapName = uniqueName("panelbuttoncommand", option, rect.size());
            if (isDefault)
                pixmapName += QLatin1String("-") + QString::number(int(button->features), 16);

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, rect.width(), rect.height());
                QPainter buttonPainter(&cache);
                if (widget) {
                    buttonPainter.fillRect(pixmapRect, option->palette.brush(widget->backgroundRole()));
                } else {
                    buttonPainter.fillRect(pixmapRect, option->palette.background());
                }

                if (isEnabled) {
                    // gradient fill
                    QRect gradRect = pixmapRect.adjusted(2, 2, -2, -2);
                    if (down) {
                        qt_plastique_draw_gradient(&buttonPainter, gradRect,
                                                   option->palette.button().color().dark(111),
                                                   option->palette.button().color().dark(106));
                    } else {
                        if (hover) {
                            qt_plastique_draw_gradient(&buttonPainter, gradRect,
                                                       highlightedGradientStartColor,
                                                       highlightedGradientStopColor);
                        } else {
                            qt_plastique_draw_gradient(&buttonPainter, gradRect,
                                                       gradientStartColor,
                                                       gradientStopColor);
                        }
                    }
                }

                if (isDefault) {
                    buttonPainter.setPen(borderColor.dark(105));
                    buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.top() + 1, pixmapRect.right() - 3, pixmapRect.top() + 1);
                    buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.bottom() - 1, pixmapRect.right() - 3, pixmapRect.bottom() - 1);
                    buttonPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 3, pixmapRect.left() + 1, pixmapRect.bottom() - 3);
                    buttonPainter.drawLine(pixmapRect.right() - 1, pixmapRect.top() + 3, pixmapRect.right() - 1, pixmapRect.bottom() - 3);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.bottom() - 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.bottom() - 2);

                    QColor outlineColor = mergedColors(alphaCornerColor.dark(110), option->palette.background().color());
                    buttonPainter.setPen(outlineColor);
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top(), pixmapRect.right() - 2, pixmapRect.top());
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom(), pixmapRect.right() - 2, pixmapRect.bottom());
                    buttonPainter.drawLine(pixmapRect.left(), pixmapRect.top() + 2, pixmapRect.left(), pixmapRect.bottom() - 2);
                    buttonPainter.drawLine(pixmapRect.right(), pixmapRect.top() + 2, pixmapRect.right(), pixmapRect.bottom() - 2);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.bottom() - 1);

                    buttonPainter.setPen(mergedColors(outlineColor, option->palette.background().color()));
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.top());
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.bottom());
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.top());
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.bottom());
                    buttonPainter.drawPoint(pixmapRect.left(), pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.left(), pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.right(), pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.right(), pixmapRect.bottom() - 1);

                    buttonPainter.setPen(mergedColors(outlineColor, borderColor.dark(105)));
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.bottom() - 2);
                } else {
                    // outer border
                    buttonPainter.setPen(borderColor);
                    buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.top() + 1,
                                           pixmapRect.right() - 3, pixmapRect.top() + 1);
                    buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.bottom() - 1,
                                           pixmapRect.right() - 3, pixmapRect.bottom() - 1);
                    buttonPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 3,
                                           pixmapRect.left() + 1, pixmapRect.bottom() - 3);
                    buttonPainter.drawLine(pixmapRect.right() - 1, pixmapRect.top() + 3,
                                           pixmapRect.right() - 1, pixmapRect.bottom() - 3);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.bottom() - 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.bottom() - 2);

                    // "antialiased" corners
                    buttonPainter.setPen(alphaCornerColor);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.left() + 2, pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 2, pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.top() + 2);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.bottom() - 2);
                }

                // inner border, top and bottom line
                if (down) {
                    buttonPainter.setPen(option->palette.button().color().light(89));
                } else {
                    if (hover) {
                        buttonPainter.setPen(highlightedDarkInnerBorderColor);
                    } else {
                        buttonPainter.setPen(option->palette.button().color().light(103));
                    }
                }
                buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.top() + 2,
                                       pixmapRect.right() - 3, pixmapRect.top() + 2);

                if (down) {
                    buttonPainter.setPen(option->palette.button().color().light(96));
                } else {
                    if (hover) {
                        buttonPainter.setPen(highlightedDarkInnerBorderColor.dark(105));
                    } else {
                        buttonPainter.setPen(option->palette.button().color().light(91));
                    }
                }
                buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.bottom() - 2,
                                        pixmapRect.right() - 3, pixmapRect.bottom() - 2);

                QLinearGradient leftGrad(QPoint(pixmapRect.left() + 1, pixmapRect.top() + 2),
                                         QPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 2));
                QLinearGradient rightGrad(QPoint(pixmapRect.left() + 1, pixmapRect.top() + 2),
                                          QPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 2));

                // inner border, side lines w/gradient
                if (down) {
                    // left
                    leftGrad.setColorAt(0, option->palette.button().color().light(88));
                    leftGrad.setColorAt(1, option->palette.button().color().light(92));

                    // right
                    rightGrad.setColorAt(0, option->palette.button().color().light(92));
                    rightGrad.setColorAt(1, option->palette.button().color().light(96));
                } else {
                    // left
                    leftGrad.setColorAt(0, option->palette.button().color().light(102));
                    leftGrad.setColorAt(1, option->palette.button().color().light(99));

                    // right
                    rightGrad.setColorAt(0, option->palette.button().color().light(99));
                    rightGrad.setColorAt(1, option->palette.button().color().light(90));
                }

                buttonPainter.setPen(QPen(QBrush(leftGrad), 1));
                buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 3,
                                        pixmapRect.left() + 2, pixmapRect.bottom() - 3);
                buttonPainter.setPen(QPen(QBrush(rightGrad), 1));
                buttonPainter.drawLine(pixmapRect.right() - 2, pixmapRect.top() + 3,
                                       pixmapRect.right() - 2, pixmapRect.bottom() - 3);

                if (!down && hover) {
                    buttonPainter.setPen(highlightedLightInnerBorderColor);
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 3,
                                            pixmapRect.right() - 2, pixmapRect.top() + 3);
                    buttonPainter.setPen(highlightedLightInnerBorderColor.dark(105));
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom() - 3,
                                            pixmapRect.right() - 2, pixmapRect.bottom() - 3);

                }
                buttonPainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(rect.topLeft(), cache);
        }
        break;
    case PE_IndicatorCheckBox:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);
            QRect rect = option->rect;

            QPixmap cache;
            QString pixmapName = uniqueName("checkbox", option, option->rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, rect.width(), rect.height());
                QPainter checkBoxPainter(&cache);

                // border
                checkBoxPainter.setPen(borderColor);
                checkBoxPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top(),
                                         pixmapRect.right() - 1, pixmapRect.top());
                checkBoxPainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom(),
                                         pixmapRect.right() - 1, pixmapRect.bottom());
                checkBoxPainter.drawLine(pixmapRect.left(), pixmapRect.top() + 1,
                                         pixmapRect.left(), pixmapRect.bottom() - 1);
                checkBoxPainter.drawLine(pixmapRect.right(), pixmapRect.top() + 1,
                                         pixmapRect.right(), pixmapRect.bottom() - 1);

                // "antialiased" corners
                checkBoxPainter.setPen(alphaCornerColor);
                checkBoxPainter.drawPoint(pixmapRect.topLeft());
                checkBoxPainter.drawPoint(pixmapRect.topRight());
                checkBoxPainter.drawPoint(pixmapRect.bottomLeft());
                checkBoxPainter.drawPoint(pixmapRect.bottomRight());

                // fill background
                QRect adjustedRect = option->rect;
                QRect gradientRect(pixmapRect.left() + 1, pixmapRect.top() + 1,
                                   pixmapRect.right() - pixmapRect.left() - 1,
                                   pixmapRect.bottom() - pixmapRect.top() - 1);
                if (hover) {
                    qt_plastique_draw_gradient(&checkBoxPainter, gradientRect,
                                               highlightedBaseGradientStartColor,
                                               highlightedBaseGradientStopColor);
                } else {
                    qt_plastique_draw_gradient(&checkBoxPainter, gradientRect,
                                               baseGradientStartColor,
                                               baseGradientStopColor);
                }

                // draw highlighted border when hovering
                if (hover) {
                    checkBoxPainter.setPen(highlightedDarkInnerBorderColor);
                    checkBoxPainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom() - 1,
                                             pixmapRect.left() + 1, pixmapRect.top() + 1);
                    checkBoxPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 1,
                                             pixmapRect.right() - 2, pixmapRect.top() + 1);
                    checkBoxPainter.setPen(highlightedLightInnerBorderColor);
                    checkBoxPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom() - 2,
                                             pixmapRect.left() + 2, pixmapRect.top() + 2);
                    checkBoxPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 2,
                                             pixmapRect.right() - 3, pixmapRect.top() + 2);
                    checkBoxPainter.setPen(highlightedDarkInnerBorderColor.dark(110));
                    checkBoxPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom() - 1,
                                             pixmapRect.right() - 1, pixmapRect.bottom() - 1);
                    checkBoxPainter.drawLine(pixmapRect.right() - 1, pixmapRect.bottom() - 1,
                                             pixmapRect.right() - 1, pixmapRect.top() + 1);
                    checkBoxPainter.setPen(highlightedLightInnerBorderColor.dark(110));
                    checkBoxPainter.drawLine(pixmapRect.left() + 3, pixmapRect.bottom() - 2,
                                             pixmapRect.right() - 2, pixmapRect.bottom() - 2);
                    checkBoxPainter.drawLine(pixmapRect.right() - 2, pixmapRect.bottom() - 2,
                                             pixmapRect.right() - 2, pixmapRect.top() + 2);
                }

                // draw check mark when on
                if ((button->state & (State_On | State_Sunken | State_NoChange))) {
                    QImage image((button->state & (State_NoChange | State_Sunken)
                                  ? qt_plastique_check_sunken : qt_plastique_check));
                    if ((button->state & (State_Sunken | State_NoChange))) {
                        image.setColor(0, alphaLightTextColor.rgba());
                        image.setColor(1, alphaLightTextColor.light(130).rgba());
                        image.setColor(2, alphaLightTextColor.light(110).rgba());
                    } else {
                        image.setColor(0, option->palette.foreground().color().rgba());
                        image.setColor(1, alphaTextColor.rgba());
                    }
                    checkBoxPainter.drawImage(pixmapRect.x() + 2, pixmapRect.y() + 2, image);
                }
                checkBoxPainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(rect.topLeft(), cache);
        }
        break;
    case PE_IndicatorRadioButton:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);
            QRect rect = option->rect;
            QPixmap cache;
            QString pixmapName = uniqueName("radiobutton", option, rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, rect.width(), rect.height());
                QPainter radioButtonPainter(&cache);
                if (widget)
                    radioButtonPainter.fillRect(pixmapRect, option->palette.brush(widget->backgroundRole()));
                else
                    radioButtonPainter.fillRect(pixmapRect, option->palette.background());

                // fill
                QLinearGradient gradient(QPointF(pixmapRect.left(), pixmapRect.top()),
                                         QPointF(pixmapRect.right(), pixmapRect.bottom()));
                if (hover) {
                    gradient.setColorAt(0, highlightedBaseGradientStartColor);
                    gradient.setColorAt(1, highlightedBaseGradientStopColor);
                } else {
                    gradient.setColorAt(0, baseGradientStartColor);
                    gradient.setColorAt(1, baseGradientStopColor);
                }

                radioButtonPainter.setPen(QPen(Qt::NoPen));
                radioButtonPainter.setBrush(QBrush(gradient));
                radioButtonPainter.drawEllipse(pixmapRect.adjusted(0, 0, -1, 0));

                QImage image(qt_plastique_radioborder);
                image.setColor(0, borderColor.rgba());
                radioButtonPainter.drawImage(pixmapRect.topLeft(), image);

                radioButtonPainter.setPen(alphaCornerColor);
                image = QImage(qt_plastique_radio_outeralpha);
                image.setColor(0, alphaCornerColor.rgba());
                radioButtonPainter.drawImage(pixmapRect.topLeft(), image);

                QColor color;
                QRect adjustedRect = pixmapRect;
                if (hover) {
                    image = QImage(qt_plastique_radio_innerhover);
                    image.setColor(1, mergedColors(borderColor, highlightedDarkInnerBorderColor).rgba());
                    image.setColor(2, highlightedDarkInnerBorderColor.rgba());
                    image.setColor(3, highlightedLightInnerBorderColor.rgba());
                    adjustedRect = adjustedRect.adjusted(1, 1, 0, 0);
                } else {
                    image = QImage(qt_plastique_radio_inneralpha);
                    color = alphaInnerColorNoHover;
                    image.setColor(0, color.rgba());
                }
                radioButtonPainter.drawImage(adjustedRect.topLeft(), image);

                // draw check
                if (button->state & (State_On | State_Sunken)) {
                    image = QImage(qt_plastique_radio_check);
                    if (button->state & State_Sunken) {
                        image.setColor(1, mergedColors(button->palette.background().color(), alphaTextColor).rgba());
                        image.setColor(2, mergedColors(button->palette.background().color(), button->palette.foreground().color()).rgba());
                    } else {
                        image.setColor(1, alphaTextColor.rgba());
                        image.setColor(2, button->palette.foreground().color().rgba());
                    }
                    radioButtonPainter.drawImage(pixmapRect, image);
                }
                radioButtonPainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(rect.topLeft(), cache);
        }
        break;
#ifndef QT_NO_DOCKWIDGET
    case PE_IndicatorDockWidgetResizeHandle:
        if ((option->state & State_Enabled) && (option->state & State_MouseOver))
            painter->fillRect(option->rect, option->palette.base());
        if (option->state & State_Horizontal) {
            int width = option->rect.width() / 3;
            QRect rect(option->rect.center().x() - width / 2,
                       option->rect.top() + (option->rect.height() / 2) - 1, width, 3);
            qt_plastique_draw_handle(painter, option, rect, Qt::Vertical, widget);
        } else {
            int height = option->rect.height() / 3;
            QRect rect(option->rect.left() + (option->rect.width() / 2 - 1),
                       option->rect.center().y() - height / 2, 3, height);
            qt_plastique_draw_handle(painter, option, rect, Qt::Horizontal, widget);
        }
        break;
#endif // QT_NO_DOCKWIDGET
    case PE_IndicatorViewItemCheck: {
        QStyleOptionButton button;
        button.QStyleOption::operator=(*option);
        button.state &= ~State_MouseOver;
        drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
        break;
    }
    case PE_FrameWindow: {
        painter->save();
        bool active = (option->state & State_Active);
        int titleBarStop = option->rect.top() + pixelMetric(PM_TitleBarHeight, option, widget);

        QPalette palette = option->palette;
        if (!active)
            palette.setCurrentColorGroup(QPalette::Disabled);

        // Frame and rounded corners
        painter->setPen(mergedColors(palette.highlight().color(), Qt::black, 50));

        // bottom border line
        painter->drawLine(option->rect.left() + 1, option->rect.bottom(), option->rect.right() - 1, option->rect.bottom());

        // bottom left and right side border lines
        painter->drawLine(option->rect.left(), titleBarStop, option->rect.left(), option->rect.bottom() - 1);
        painter->drawLine(option->rect.right(), titleBarStop, option->rect.right(), option->rect.bottom() - 1);
        painter->drawPoint(option->rect.left() + 1, option->rect.bottom() - 1);
        painter->drawPoint(option->rect.right() - 1, option->rect.bottom() - 1);

#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3DockWindow")) {
            // also draw the frame on the title bar
            painter->drawLine(option->rect.left() + 1, option->rect.top(),
                              option->rect.right() - 1, option->rect.top());
            painter->drawLine(option->rect.left(), option->rect.top() + 1,
                              option->rect.left(), titleBarStop);
            painter->drawLine(option->rect.right(), option->rect.top() + 1,
                              option->rect.right(), titleBarStop);
        }
#endif

        // alpha corners
        painter->setPen(mergedColors(palette.highlight().color(), palette.background().color(), 55));
        painter->drawPoint(option->rect.left() + 2, option->rect.bottom() - 1);
        painter->drawPoint(option->rect.left() + 1, option->rect.bottom() - 2);
        painter->drawPoint(option->rect.right() - 2, option->rect.bottom() - 1);
        painter->drawPoint(option->rect.right() - 1, option->rect.bottom() - 2);

#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3DockWindow")) {
            // also draw the frame on the title bar
            painter->drawPoint(option->rect.topLeft());
            painter->drawPoint(option->rect.topRight());
        }
#endif

        // upper and lower left inner
        painter->setPen(active ? mergedColors(palette.highlight().color(), palette.background().color()) : palette.background().color().dark(120));
        painter->drawLine(option->rect.left() + 1, titleBarStop, option->rect.left() + 1, option->rect.bottom() - 2);

#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3DockWindow")) {
            // also draw the frame on the title bar
            painter->drawLine(option->rect.left() + 1, option->rect.top() + 1,
                              option->rect.left() + 1, titleBarStop);
            painter->drawLine(option->rect.right() - 1, option->rect.top() + 1,
                              option->rect.right() - 1, titleBarStop);
            painter->drawLine(option->rect.left() + 1, option->rect.top() + 1,
                              option->rect.right() - 1, option->rect.top() + 1);
        }
#endif

        painter->setPen(active ? mergedColors(palette.highlight().color(), palette.background().color(), 57) : palette.background().color().dark(130));
        painter->drawLine(option->rect.right() - 1, titleBarStop, option->rect.right() - 1, option->rect.bottom() - 2);
        painter->drawLine(option->rect.left() + 1, option->rect.bottom() - 1, option->rect.right() - 1, option->rect.bottom() - 1);

        painter->restore();
    }
        break;
    case PE_IndicatorBranch:
        {
            int mid_h = option->rect.x() + option->rect.width() / 2;
            int mid_v = option->rect.y() + option->rect.height() / 2;
            int bef_h = mid_h;
            int bef_v = mid_v;
            int aft_h = mid_h;
            int aft_v = mid_v;
            QBrush brush(option->palette.dark().color(), Qt::Dense4Pattern);
            if (option->state & State_Item) {
                if (option->direction == Qt::RightToLeft)
                    painter->fillRect(option->rect.left(), mid_v, bef_h - option->rect.left(), 1, brush);
                else
                    painter->fillRect(aft_h, mid_v, option->rect.right() - aft_h + 1, 1, brush);
            }
            if (option->state & State_Sibling)
                painter->fillRect(mid_h, aft_v, 1, option->rect.bottom() - aft_v + 1, brush);
            if (option->state & (State_Open | State_Children | State_Item | State_Sibling))
                painter->fillRect(mid_h, option->rect.y(), 1, bef_v - option->rect.y(), brush);

            if (option->state & State_Children) {
                painter->save();
                QPoint center = option->rect.center();
                // border
                QRect fullRect(center.x() - 4, center.y() - 4, 9, 9);
                painter->setPen(borderColor);
                painter->drawLine(fullRect.left() + 1, fullRect.top(),
                                fullRect.right() - 1, fullRect.top());
                painter->drawLine(fullRect.left() + 1, fullRect.bottom(),
                                fullRect.right() - 1, fullRect.bottom());
                painter->drawLine(fullRect.left(), fullRect.top() + 1,
                                fullRect.left(), fullRect.bottom() - 1);
                painter->drawLine(fullRect.right(), fullRect.top() + 1,
                                fullRect.right(), fullRect.bottom() - 1);
                // "antialiased" corners
                painter->setPen(alphaCornerColor);
                painter->drawPoint(fullRect.topLeft());
                painter->drawPoint(fullRect.topRight());
                painter->drawPoint(fullRect.bottomLeft());
                painter->drawPoint(fullRect.bottomRight());
                // fill
                QRect adjustedRect = fullRect;
                QRect gradientRect(adjustedRect.left() + 1, adjustedRect.top() + 1,
                                adjustedRect.right() - adjustedRect.left() - 1,
                                adjustedRect.bottom() - adjustedRect.top() - 1);
                qt_plastique_draw_gradient(painter, gradientRect, baseGradientStartColor, baseGradientStopColor);
                // draw "+" or "-"
                painter->setPen(alphaTextColor);
                painter->drawLine(center.x() - 2, center.y(), center.x() + 2, center.y());
                if (!(option->state & State_Open))
                    painter->drawLine(center.x(), center.y() - 2, center.x(), center.y() + 2);
                painter->restore();

            }
        }
        break;
    default:
        QWindowsStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

/*!
  \reimp
*/
void QPlastiqueStyle::drawControl(ControlElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const
{
    QColor borderColor = option->palette.background().color().dark(178);
    QColor alphaCornerColor;
    if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
    } else {
        alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    }
    QColor alphaTextColor = mergedColors(option->palette.background().color(), option->palette.text().color());

    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);

    QColor shadowGradientStartColor = option->palette.button().color().dark(115);
    QColor shadowGradientStopColor = option->palette.button().color().dark(120);

    QColor highlightedGradientStartColor = option->palette.button().color().light(101);
    QColor highlightedGradientStopColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 85);

    QColor lightShadowGradientStartColor = highlightedGradientStartColor.light(105);
    QColor lightShadowGradientStopColor = highlightedGradientStopColor.light(105);

    QColor highlightedDarkInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 35);
    QColor highlightedLightInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 58);

    QColor alphaInnerColor = mergedColors(highlightedDarkInnerBorderColor, option->palette.base().color());
    QColor lightShadow = lightShadowGradientStartColor;
    QColor shadow = shadowGradientStartColor;

    switch (element) {
#ifndef QT_NO_TABBAR
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

            if (tab->shape != QTabBar::RoundedNorth && tab->shape != QTabBar::RoundedWest &&
                tab->shape != QTabBar::RoundedSouth && tab->shape != QTabBar::RoundedEast) {
                QWindowsStyle::drawControl(element, option, painter, widget);
                break;
            }

            painter->save();

            // Set up some convenience variables
            bool disabled = !(tab->state & State_Enabled);
            bool onlyTab = tab->position == QStyleOptionTab::OnlyOneTab;
            bool selected = (tab->state & State_Selected) || onlyTab;
            bool mouseOver = (tab->state & State_MouseOver) && !selected && !disabled;
            bool previousSelected = tab->selectedPosition == QStyleOptionTab::PreviousIsSelected;
            bool nextSelected = tab->selectedPosition == QStyleOptionTab::NextIsSelected;
            bool leftCornerWidget = (tab->cornerWidgets & QStyleOptionTab::LeftCornerWidget);
            bool reverse = (tab->direction == Qt::RightToLeft);

            int lowerTop = selected ? 0 : 3; // to make the selected tab bigger than the rest
            QRect adjustedRect;
            bool atEnd = (tab->position == QStyleOptionTab::End) || onlyTab;
            bool atBeginning = ((tab->position == QStyleOptionTab::Beginning) || onlyTab)
                && !leftCornerWidget;
            bool reverseShadow = false;

            int borderThickness = pixelMetric(PM_TabBarBaseOverlap, tab, widget);
            int marginLeft = 0;
            if ((atBeginning && !selected) || (selected && leftCornerWidget && ((tab->position == QStyleOptionTab::Beginning) || onlyTab))) {
                marginLeft = 1;
            }

            // I've set the names based on the natural coordinate system. Vectors are used to rotate everything
            // if the orientation of the tab bare is different than north.
            {
                // Coordinates of corners of rectangle for transformation
                QPoint topLeft;
                QPoint topRight;
                QPoint bottomLeft;
                QPoint bottomRight;

                // Fill with normalized vectors in the direction of the coordinate system
                // (down and right should be complement of up and left, or it will look odd)
                QPoint vectorUp;
                QPoint vectorDown;
                QPoint vectorLeft;
                QPoint vectorRight;

                QColor baseColor1;
                QColor baseColor2;

                switch (tab->shape) {
                case QTabBar::RoundedNorth:
                    vectorUp = QPoint(0, -1);
                    vectorDown = QPoint(0, 1);

                    if (reverse) {
                        vectorLeft = QPoint(1, 0);
                        vectorRight = QPoint(-1, 0);
                        reverseShadow = true;
                    } else {
                        vectorLeft = QPoint(-1, 0);
                        vectorRight = QPoint(1, 0);
                    }

                    if (reverse) {
                        topLeft = tab->rect.topRight();
                        topRight = tab->rect.topLeft();
                        bottomLeft = tab->rect.bottomRight();
                        bottomRight = tab->rect.bottomLeft();
                    } else {
                        topLeft = tab->rect.topLeft();
                        topRight = tab->rect.topRight();
                        bottomLeft = tab->rect.bottomLeft();
                        bottomRight = tab->rect.bottomRight();
                    }


                    baseColor1 = borderColor;
                    baseColor2 = lightShadow;
                    break ;
                case QTabBar::RoundedWest:
                    vectorUp = QPoint(-1, 0);
                    vectorDown = QPoint(1, 0);
                    vectorLeft = QPoint(0, -1);
                    vectorRight = QPoint(0, 1);

                    topLeft = tab->rect.topLeft();
                    topRight = tab->rect.bottomLeft();
                    bottomLeft = tab->rect.topRight();
                    bottomRight = tab->rect.bottomRight();

                    baseColor1 = borderColor;
                    baseColor2 = lightShadow;
                    break ;
                case QTabBar::RoundedEast:
                    vectorUp = QPoint(1, 0);
                    vectorDown = QPoint(-1, 0);
                    vectorLeft = QPoint(0, -1);
                    vectorRight = QPoint(0, 1);

                    topLeft = tab->rect.topRight();
                    topRight = tab->rect.bottomRight();
                    bottomLeft = tab->rect.topLeft();
                    bottomRight = tab->rect.bottomLeft();

                    baseColor1 = borderColor;
                    baseColor2 = shadow;
                    break ;
                case QTabBar::RoundedSouth:
                    vectorUp = QPoint(0, 1);
                    vectorDown = QPoint(0, -1);

                    if (reverse) {
                        vectorLeft = QPoint(1, 0);
                        vectorRight = QPoint(-1, 0);
                        reverseShadow = true;

                        topLeft = tab->rect.bottomRight();
                        topRight = tab->rect.bottomLeft();
                        bottomLeft = tab->rect.topRight();
                        bottomRight = tab->rect.topLeft();
                    } else {
                        vectorLeft = QPoint(-1, 0);
                        vectorRight = QPoint(1, 0);

                        topLeft = tab->rect.bottomLeft();
                        topRight = tab->rect.bottomRight();
                        bottomLeft = tab->rect.topLeft();
                        bottomRight = tab->rect.topRight();
                    }

                    baseColor1 = borderColor;
                    baseColor2 = shadow;
                    break ;
                default:
                    break;
                }

                // Make the tab smaller when it's at the end, so that we are able to draw the corner
                if (atEnd) {
                    topRight += vectorLeft;
                    bottomRight += vectorLeft;
                }

                {
                    // Outer border
                    QLine topLine;
                    {
                        QPoint adjustTopLineLeft = (vectorRight * (marginLeft + (previousSelected ? 0 : 1))) +
                                                   (vectorDown * lowerTop);
                        QPoint adjustTopLineRight = (vectorDown * lowerTop);
                        if (atBeginning || selected)
                            adjustTopLineLeft += vectorRight;
                        if (atEnd || selected)
                            adjustTopLineRight += 2 * vectorLeft;

                        topLine = QLine(topLeft + adjustTopLineLeft, topRight + adjustTopLineRight);
                    }

                    QLine leftLine;
                    {
                        QPoint adjustLeftLineTop = (vectorRight * marginLeft) + (vectorDown * (lowerTop + 1));
                        QPoint adjustLeftLineBottom = (vectorRight * marginLeft) + (vectorUp * borderThickness);
                        if (atBeginning || selected)
                            adjustLeftLineTop += vectorDown; // Make place for rounded corner
                        if (atBeginning && selected)
                            adjustLeftLineBottom += borderThickness * vectorDown;
                        else if (selected)
                            adjustLeftLineBottom += vectorUp;

                        leftLine = QLine(topLeft + adjustLeftLineTop, bottomLeft + adjustLeftLineBottom);
                    }

                    QLine rightLine;
                    {
                        QPoint adjustRightLineTop = vectorDown * (2 + lowerTop);
                        QPoint adjustRightLineBottom = vectorUp * borderThickness;
                        if (selected)
                            adjustRightLineBottom += vectorUp;

                        rightLine = QLine(topRight + adjustRightLineTop, bottomRight + adjustRightLineBottom);
                    }

                    // Background
                    QPoint startPoint = topLine.p1() + vectorDown + vectorLeft;
                    if (mouseOver)
                        startPoint += vectorDown;
                    QPoint endPoint = rightLine.p2();

                    if (tab->state & State_Enabled) {
                        QRect fillRect = QRect(startPoint, endPoint).normalized();
                        if (fillRect.isValid()) {
                            if (selected) {
                                painter->fillRect(fillRect, painter->brush());
                            } else {
                                if (mouseOver) {
                                    qt_plastique_draw_gradient(painter, fillRect,
                                                               highlightedGradientStartColor,
                                                               highlightedGradientStopColor);
                                } else {
                                    qt_plastique_draw_gradient(painter, fillRect,
                                                               gradientStartColor,
                                                               gradientStopColor);
                                }
                            }
                        }
                    }

                    QPoint rightCornerDot = topRight + vectorLeft + (lowerTop + 1)*vectorDown;
                    QPoint leftCornerDot = topLeft + (marginLeft + 1)*vectorRight + (lowerTop + 1)*vectorDown;
                    QPoint bottomRightConnectToBase = rightLine.p2() + vectorRight + vectorDown;
                    QPoint bottomLeftConnectToBase = leftLine.p2() + vectorLeft + vectorDown;

                    painter->setPen(borderColor);
                    painter->drawLine(topLine);

                    if (mouseOver) {
                        QLine secondHoverLine = QLine(topLine.p1() + vectorDown * 2 + vectorLeft, topLine.p2() + vectorDown * 2 + vectorRight);
                        painter->setPen(highlightedLightInnerBorderColor);
                        painter->drawLine(secondHoverLine);
                    }

                    if (mouseOver)
                        painter->setPen(borderColor);
                    if (!previousSelected)
                        painter->drawLine(leftLine);
                    if (atEnd || selected) {
                        painter->drawLine(rightLine);
                        painter->drawPoint(rightCornerDot);
                    }
                    if (atBeginning || selected)
                        painter->drawPoint(leftCornerDot);
                    if (selected) {
                        painter->drawPoint(bottomRightConnectToBase);
                        painter->drawPoint(bottomLeftConnectToBase);
                    }

                    // Antialiasing
                    painter->setPen(alphaCornerColor);
                    if (atBeginning || selected)
                        painter->drawPoint(topLine.p1() + vectorLeft);
                    if (!previousSelected)
                        painter->drawPoint(leftLine.p1() + vectorUp);
                    if (atEnd || selected) {
                        painter->drawPoint(topLine.p2() + vectorRight);
                        painter->drawPoint(rightLine.p1() + vectorUp);
                    }

                    if (selected) {
                        painter->drawPoint(bottomRightConnectToBase + vectorLeft);
                        if (!atBeginning) {
                            painter->drawPoint(bottomLeftConnectToBase + vectorRight);

                            if (((tab->position == QStyleOptionTab::Beginning) || onlyTab) && leftCornerWidget) {
                                // A special case: When the first tab is selected and
                                // has a left corner widget, it needs to do more work
                                // to connect to the base
                                QPoint p1 = bottomLeftConnectToBase + vectorDown;

                                painter->drawPoint(p1);
                            }
                        }
                    }

                    // Inner border
                    QLine innerTopLine = QLine(topLine.p1() + vectorDown, topLine.p2() + vectorDown);
                    if (!selected) {
                        QLinearGradient topLineGradient(innerTopLine.p1(),innerTopLine.p2());
                        topLineGradient.setColorAt(0, lightShadowGradientStartColor);
                        topLineGradient.setColorAt(1, lightShadowGradientStopColor);
                        painter->setPen(QPen(mouseOver ? QBrush(highlightedDarkInnerBorderColor) : QBrush(topLineGradient), 1));
                    } else {
                        painter->setPen(lightShadow);
                    }
                    painter->drawLine(innerTopLine);

                    QLine innerLeftLine = QLine(leftLine.p1() + vectorRight + vectorDown, leftLine.p2() + vectorRight);
                    QLine innerRightLine = QLine(rightLine.p1() + vectorLeft + vectorDown, rightLine.p2() + vectorLeft);

                    if (selected) {
                        innerRightLine = QLine(innerRightLine.p1() + vectorUp, innerRightLine.p2());
                        innerLeftLine = QLine(innerLeftLine.p1() + vectorUp, innerLeftLine.p2());
                    }

                    if (selected || atBeginning) {
                        if (!selected) {
                            QLinearGradient leftLineGradient(innerLeftLine.p1(),innerLeftLine.p2());
                            leftLineGradient.setColorAt(0, lightShadowGradientStartColor);
                            leftLineGradient.setColorAt(1, lightShadowGradientStopColor);
                            painter->setPen(QPen(QBrush(leftLineGradient), 1));
                        }

                        // Assume the sun is on the same side in Right-To-Left layouts and draw the
                        // light shadow on the left side always (the right line is on the left side in
                        // reverse layouts for north and south)
                        if (reverseShadow)
                            painter->drawLine(innerRightLine);
                        else
                            painter->drawLine(innerLeftLine);
                    }


                    if (atEnd || selected) {
                        if (!selected) {
                            QLinearGradient rightLineGradient(innerRightLine.p1(),innerRightLine.p2());
                            rightLineGradient.setColorAt(0, shadowGradientStartColor);
                            rightLineGradient.setColorAt(1, shadowGradientStopColor);
                            painter->setPen(QPen(QBrush(rightLineGradient), 1));
                        } else {
                            painter->setPen(shadow);
                        }

                        if (reverseShadow)
                            painter->drawLine(innerLeftLine);
                        else
                            painter->drawLine(innerRightLine);
                    }


                    // Base
                    QLine baseLine = QLine(bottomLeft + marginLeft * 2 * vectorRight, bottomRight);
                    {

                        QPoint adjustedLeft;
                        QPoint adjustedRight;

                        if (atEnd && !selected) {
                            baseLine = QLine(baseLine.p1(), baseLine.p2() + vectorRight);
                        }

                        if (nextSelected) {
                            adjustedRight += vectorLeft;
                            baseLine = QLine(baseLine.p1(), baseLine.p2() + vectorLeft);
                        }
                        if (previousSelected) {
                            adjustedLeft += vectorRight;
                            baseLine = QLine(baseLine.p1() + vectorRight, baseLine.p2());
                        }
                        if (atBeginning)
                            adjustedLeft += vectorRight;

                        painter->setPen(baseColor2);
                        if (!selected)
                            painter->drawLine(baseLine);

                        if (atEnd && !selected)
                            painter->drawPoint(baseLine.p2() + vectorRight);

                        if (atBeginning && !selected)
                            adjustedLeft = vectorRight;
                        else
                            adjustedLeft = QPoint(0, 0);
                        painter->setPen(baseColor1);
                        if (!selected)
                            painter->drawLine(bottomLeft + vectorUp + adjustedLeft, baseLine.p2() + vectorUp);

                        QPoint endPoint = bottomRight + vectorUp;
                        if (atEnd && !selected)
                            painter->drawPoint(endPoint);

                        // For drawing a lower left "fake" corner on the base when the first tab is unselected
                        if (atBeginning && !selected) {
                            painter->drawPoint(baseLine.p1() + vectorLeft);
                        }

                        painter->setPen(alphaCornerColor);
                        if (nextSelected)
                            painter->drawPoint(endPoint);
                        else if (selected)
                            painter->drawPoint(endPoint + vectorRight);

                        // For drawing a lower left "fake" corner on the base when the first tab is unselected
                        if (atBeginning && !selected) {
                            painter->drawPoint(baseLine.p1() + 2 * vectorLeft);
                        }
                    }
                }
            }

            // Yay we're done

            painter->restore();
        }
        break;
#endif // QT_NO_TABBAR
#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarGroove:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QRect rect = bar->rect;
            QPen oldPen = painter->pen();

            // outline
            painter->setPen(borderColor);
            painter->drawLine(rect.left() + 2, rect.top(), rect.right() - 2, rect.top());
            painter->drawLine(rect.left() + 2, rect.bottom(), rect.right() - 2, rect.bottom());
            painter->drawLine(rect.left(), rect.top() + 2, rect.left(), rect.bottom() - 2);
            painter->drawLine(rect.right(), rect.top() + 2, rect.right(), rect.bottom() - 2);
            painter->drawPoint(rect.left() + 1, rect.top() + 1);
            painter->drawPoint(rect.right() - 1, rect.top() + 1);
            painter->drawPoint(rect.left() + 1, rect.bottom() - 1);
            painter->drawPoint(rect.right() - 1, rect.bottom() - 1);

            // alpha corners
            painter->setPen(alphaCornerColor);
            painter->drawPoint(rect.left(), rect.top() + 1);
            painter->drawPoint(rect.left() + 1, rect.top());
            painter->drawPoint(rect.right(), rect.top() + 1);
            painter->drawPoint(rect.right() - 1, rect.top());
            painter->drawPoint(rect.left(), rect.bottom() - 1);
            painter->drawPoint(rect.left() + 1, rect.bottom());
            painter->drawPoint(rect.right(), rect.bottom() - 1);
            painter->drawPoint(rect.right() - 1, rect.bottom());

            // inner outline, north-west
            painter->setPen(gradientStartColor.dark(105));
            painter->drawLine(rect.left() + 2, rect.top() + 1, rect.right() - 2, rect.top() + 1);
            painter->drawLine(rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom() - 2);

            // base of the groove
            painter->setPen(QPen());
            painter->fillRect(rect.adjusted(2, 2, -2, -1), QBrush(bar->palette.base().color()));
            painter->setPen(bar->palette.base().color());
            painter->drawLine(rect.right() - 1, rect.top() + 2, rect.right() - 1, rect.bottom() - 2);

            painter->setPen(oldPen);
        }
        break;
    case CE_ProgressBarLabel:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            // The busy indicator doesn't draw a label
            if (bar->minimum == 0 && bar->maximum == 0)
                return;

            painter->save();

            QRect rect = bar->rect;
            QRect leftRect;

            QFont font;
            font.setBold(true);
            painter->setFont(font);
            painter->setPen(bar->palette.text().color());

            bool vertical = false;
            bool inverted = false;
            bool bottomToTop = false;
            // Get extra style options if version 2
            if (const QStyleOptionProgressBarV2 *bar2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option)) {
                vertical = (bar2->orientation == Qt::Vertical);
                inverted = bar2->invertedAppearance;
                bottomToTop = bar2->bottomToTop;
            }

            if (vertical) {
                rect = QRect(rect.left(), rect.top(), rect.height(), rect.width()); // flip width and height
                QMatrix m;
                if (bottomToTop) {
                    m.translate(0.0, rect.width());
                    m.rotate(-90);
                } else {
                    m.translate(rect.height(), 0.0);
                    m.rotate(90);
                }
                painter->setMatrix(m);
            }

            int progressIndicatorPos = int(((bar->progress - bar->minimum) / double(bar->maximum - bar->minimum)) * rect.width());

            bool flip = (!vertical && (((bar->direction == Qt::RightToLeft) && !inverted)
                          || ((bar->direction == Qt::LeftToRight) && inverted))) || (vertical && ((!inverted && !bottomToTop) || (inverted && bottomToTop)));
            if (flip) {
                int indicatorPos = rect.width() - progressIndicatorPos;
                if (indicatorPos >= 0 && indicatorPos <= rect.width()) {
                    painter->setPen(bar->palette.base().color());
                    leftRect = QRect(rect.left(), rect.top(), indicatorPos, rect.height());
                } else if (indicatorPos > rect.width()) {
                    painter->setPen(bar->palette.text().color());
                } else {
                    painter->setPen(bar->palette.base().color());
                }
            } else {
                if (progressIndicatorPos >= 0 && progressIndicatorPos <= rect.width()) {
                    leftRect = QRect(rect.left(), rect.top(), progressIndicatorPos, rect.height());
                } else if (progressIndicatorPos > rect.width()) {
                    painter->setPen(bar->palette.base().color());
                } else {
                    painter->setPen(bar->palette.text().color());
                }
            }

            painter->drawText(rect, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            if (!leftRect.isNull()) {
                painter->setPen(flip ? bar->palette.text().color() : bar->palette.base().color());
                painter->setClipRect(leftRect, Qt::IntersectClip);
                painter->drawText(rect, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            }

            painter->restore();
        }
        break;
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {

            painter->save();
            QRect rect = bar->rect;
            bool vertical = false;
            bool inverted = false;
            bool indeterminate = (bar->minimum == 0 && bar->maximum == 0);
            if (!indeterminate && bar->progress == -1 ) {
                painter->restore();
                break;
            }

            // Get extra style options if version 2
            if (const QStyleOptionProgressBarV2 *bar2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option)) {
                vertical = (bar2->orientation == Qt::Vertical);
                inverted = bar2->invertedAppearance;
            }

            // If the orientation is vertical, we use a transform to rotate
            // the progress bar 90 degrees clockwise.  This way we can use the
            // same rendering code for both orientations.
            if (vertical) {
                rect = QRect(rect.left(), rect.top(), rect.height(), rect.width()); // flip width and height
                QMatrix m;
                m.translate(rect.height()-1, 0.0);
                m.rotate(90.0);
                painter->setMatrix(m);
            }

            int maxWidth = rect.width() - 4;
            int minWidth = 4;
            int progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
            int width = indeterminate ? maxWidth : qMax(int((((progress - bar->minimum))
                                                             / double(bar->maximum - bar->minimum)) * maxWidth), minWidth);
            bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;
            if (inverted)
                reverse = !reverse;

            QRect progressBar;
            if (!indeterminate) {
                if (!reverse) {
                    progressBar.setRect(rect.left() + 2, rect.top() + 2, width, rect.height() - 4);
                } else {
                    progressBar.setRect(rect.right() - 1 - width, rect.top() + 2, width, rect.height() - 4);
                }
            } else {
                int slideWidth = ((rect.width() - 4) * 2) / 3;
                int step = ((d->animateStep * slideWidth) / ProgressBarFps) % slideWidth;
                if ((((d->animateStep * slideWidth) / ProgressBarFps) % (2 * slideWidth)) >= slideWidth)
                    step = slideWidth - step;
                progressBar.setRect(rect.left() + 2 + step, rect.top() + 2,
                                    slideWidth / 2, rect.height() - 4);
            }

            // outline
            painter->setPen(highlightedDarkInnerBorderColor);
            if (!reverse) {
                if (width == minWidth) {
                    painter->drawPoint(progressBar.left() + 1, progressBar.top());
                    painter->drawPoint(progressBar.left() + 1, progressBar.bottom());
                } else {
                    if (indeterminate) {
                        painter->drawLine(progressBar.left() + 2, progressBar.top(),
                                          progressBar.right() - 2, progressBar.top());
                        painter->drawLine(progressBar.left() + 2, progressBar.bottom(),
                                          progressBar.right() - 2, progressBar.bottom());
                    } else {
                        painter->drawLine(progressBar.left() + 1, progressBar.top(),
                                          progressBar.right() - 2, progressBar.top());
                        painter->drawLine(progressBar.left() + 1, progressBar.bottom(),
                                          progressBar.right() - 2, progressBar.bottom());
                    }
                }

                if (indeterminate) {
                    painter->drawLine(progressBar.left(), progressBar.top() + 2,
                                      progressBar.left(), progressBar.bottom() - 2);
                } else {
                    painter->drawLine(progressBar.left(), progressBar.top() + 1,
                                      progressBar.left(), progressBar.bottom() - 1);
                }
                painter->drawLine(progressBar.right(), progressBar.top() + 2,
                                  progressBar.right(), progressBar.bottom() - 2);
            } else {
                if (width == minWidth) {
                    painter->drawPoint(progressBar.right() - 1, progressBar.top());
                    painter->drawPoint(progressBar.right() - 1, progressBar.bottom());
                } else {
                    if (indeterminate) {
                        painter->drawLine(progressBar.right() - 2, progressBar.top(),
                                          progressBar.left() + 2, progressBar.top());
                        painter->drawLine(progressBar.right() - 2, progressBar.bottom(),
                                          progressBar.left() + 2, progressBar.bottom());
                    } else {
                        painter->drawLine(progressBar.right() - 1, progressBar.top(),
                                          progressBar.left() + 2, progressBar.top());
                        painter->drawLine(progressBar.right() - 1, progressBar.bottom(),
                                          progressBar.left() + 2, progressBar.bottom());
                    }
                }
                if (indeterminate) {
                    painter->drawLine(progressBar.right(), progressBar.top() + 2,
                                      progressBar.right(), progressBar.bottom() - 2);
                } else {
                    painter->drawLine(progressBar.right(), progressBar.top() + 1,
                                      progressBar.right(), progressBar.bottom() - 1);
                }
                painter->drawLine(progressBar.left(), progressBar.top() + 2,
                                  progressBar.left(), progressBar.bottom() - 2);
            }

            // alpha corners
            painter->setPen(alphaInnerColor);
            if (!reverse) {
                if (indeterminate) {
                    painter->drawPoint(progressBar.left() + 1, progressBar.top());
                    painter->drawPoint(progressBar.left(), progressBar.top() + 1);
                    painter->drawPoint(progressBar.left() + 1, progressBar.bottom());
                    painter->drawPoint(progressBar.left(), progressBar.bottom() - 1);
                } else {
                    painter->drawPoint(progressBar.left(), progressBar.top());
                    painter->drawPoint(progressBar.left(), progressBar.bottom());
                }
                painter->drawPoint(progressBar.right() - 1, progressBar.top());
                painter->drawPoint(progressBar.right(), progressBar.top() + 1);
                painter->drawPoint(progressBar.right() - 1, progressBar.bottom());
                painter->drawPoint(progressBar.right(), progressBar.bottom() - 1);
            } else {
                if (indeterminate) {
                    painter->drawPoint(progressBar.right() - 1, progressBar.top());
                    painter->drawPoint(progressBar.right(), progressBar.top() + 1);
                    painter->drawPoint(progressBar.right() - 1, progressBar.bottom());
                    painter->drawPoint(progressBar.right(), progressBar.bottom() - 1);
                } else {
                    painter->drawPoint(progressBar.right(), progressBar.top());
                    painter->drawPoint(progressBar.right(), progressBar.bottom());
                }
                painter->drawPoint(progressBar.left() + 1, progressBar.top());
                painter->drawPoint(progressBar.left(), progressBar.top() + 1);
                painter->drawPoint(progressBar.left() + 1, progressBar.bottom());
                painter->drawPoint(progressBar.left(), progressBar.bottom() - 1);
            }

            // contents
            painter->setPen(QPen());

            QString progressBarName = uniqueName(QLatin1String("progressBarContents"),
                                                 option, rect.size());
            QPixmap cache;
            if (!UsePixmapCache || !QPixmapCache::find(progressBarName, cache)) {
                QSize size = rect.size();
                cache = QPixmap(QSize(size.width() - 6 + 30, size.height() - 6));
                cache.fill(Qt::white);
                QPainter cachePainter(&cache);
                QRect pixmapRect(0, 0, cache.width(), cache.height());

                int leftEdge = 0;
                bool flip = false;
                while (leftEdge < cache.width() + 1) {
                    QColor rectColor = option->palette.highlight().color();
                    QColor lineColor = option->palette.highlight().color();
                    if (flip) {
                        flip = false;
                        rectColor = rectColor.light(105);
                        lineColor = lineColor.light(105);
                    } else {
                        flip = true;
                    }

                    cachePainter.setPen(lineColor);
                    cachePainter.drawLine(pixmapRect.left() + leftEdge - 1, pixmapRect.top(),
                                          pixmapRect.left() + leftEdge + 9, pixmapRect.top());
                    cachePainter.drawLine(pixmapRect.left() + leftEdge - 1, pixmapRect.bottom(),
                                          pixmapRect.left() + leftEdge + 9, pixmapRect.bottom());
                    cachePainter.fillRect(QRect(pixmapRect.left() + leftEdge, pixmapRect.top(),
                                          10, pixmapRect.height()), rectColor);

                    leftEdge += 10;
                }

                if (UsePixmapCache)
                    QPixmapCache::insert(progressBarName, cache);
            }
            painter->setClipRect(progressBar.adjusted(1, 0, -1, -1));

            if (!indeterminate) {
                int step = (AnimateProgressBar || (indeterminate && AnimateBusyProgressBar)) ? (d->animateStep % 20) : 0;
                if (reverse)
                    painter->drawPixmap(progressBar.left() - 25 + step, progressBar.top() + 1, cache);
                else
                    painter->drawPixmap(progressBar.left() - 25 - step + width % 20, progressBar.top() + 1, cache);
            } else {
                painter->drawPixmap(progressBar.left(), progressBar.top() + 1, cache);
            }

            painter->restore();
        }
        break;
#endif // QT_NO_PROGRESSBAR
    case CE_HeaderSection:
        // Draws the header in tables.
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QPixmap cache;
            QString pixmapName = uniqueName("headersection", option, option->rect.size());
            pixmapName += QLatin1String("-") + QString::number(int(header->position));
            pixmapName += QLatin1String("-") + QString::number(int(header->orientation));

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(option->rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, option->rect.width(), option->rect.height());
                QPainter cachePainter(&cache);

                bool sunken = (header->state & State_Enabled) && (header->state & State_Sunken);

                QColor headerGradientStart = sunken ? option->palette.background().color().dark(114) : gradientStartColor;
                QColor headerGradientStop = sunken ? option->palette.background().color().dark(106) : gradientStopColor;

                QColor lightLine = sunken ? option->palette.background().color().dark(118) : gradientStartColor;
                QColor darkLine = sunken ? option->palette.background().color().dark(110) : gradientStopColor.dark(105);

                qt_plastique_draw_gradient(&cachePainter, pixmapRect,
                                           headerGradientStart, headerGradientStop);

                cachePainter.setPen(borderColor);
                cachePainter.drawRect(pixmapRect.adjusted(0, 0, -1, -1));
                cachePainter.setPen(alphaCornerColor);
                cachePainter.drawPoint(pixmapRect.topLeft());
                cachePainter.drawPoint(pixmapRect.topRight());
                cachePainter.drawPoint(pixmapRect.bottomLeft());
                cachePainter.drawPoint(pixmapRect.bottomRight());

                // inner lines
                cachePainter.setPen(lightLine);
                cachePainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 1,
                                      pixmapRect.right() - 2, pixmapRect.top() + 1);
                cachePainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 2,
                                      pixmapRect.left() + 1, pixmapRect.bottom() - 2);
                cachePainter.setPen(darkLine);
                cachePainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom() - 1,
                                      pixmapRect.right() - 2, pixmapRect.bottom() - 1);
                cachePainter.drawLine(pixmapRect.right() - 1, pixmapRect.bottom() - 2,
                                      pixmapRect.right() - 1, pixmapRect.top() + 2);
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(option->rect.topLeft(), cache);

        }
        break;
#ifndef QT_NO_MENU
    case CE_MenuItem:
        // Draws one item in a popup menu.
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            painter->save();

            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                painter->fillRect(menuItem->rect, option->palette.background().color().light(103));

                int w = 0;
                if (!menuItem->text.isEmpty()) {
                    painter->setFont(menuItem->font);
                    drawItemText(painter, menuItem->rect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter,
                                 menuItem->palette, menuItem->state & State_Enabled, menuItem->text,
                                 QPalette::Text);
                    w = menuItem->fontMetrics.width(menuItem->text) + 5;
                }

                painter->setPen(alphaCornerColor);
                bool reverse = menuItem->direction == Qt::RightToLeft;
                painter->drawLine(menuItem->rect.left() + 5 + (reverse ? 0 : w), menuItem->rect.center().y(),
                                  menuItem->rect.right() - 5 - (reverse ? w : 0), menuItem->rect.center().y());

                painter->restore();
                break;
            }

            bool selected = menuItem->state & State_Selected;
            bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
            bool checked = menuItem->checked;
            bool sunken = menuItem->state & State_Sunken;
            bool enabled = menuItem->state & State_Enabled;

            if (selected && enabled) {
                qt_plastique_draw_gradient(painter, menuItem->rect,
                                           option->palette.highlight().color().light(105),
                                           option->palette.highlight().color().dark(110));

                painter->setPen(option->palette.highlight().color().light(110));
                painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                painter->setPen(option->palette.highlight().color().dark(115));
                painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
            } else {
                painter->fillRect(option->rect, option->palette.background().color().light(103));
            }

            // Check
            QRect checkRect(option->rect.left() + 7, option->rect.center().y() - 6, 13, 13);
            checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
            if (checkable) {
                if (menuItem->checkType & QStyleOptionMenuItem::Exclusive && menuItem->icon.isNull()) {
                    // Radio button
                    QImage image(qt_plastique_radio);
                    image.setColor(0, borderColor.rgba());
                    image.setColor(1, alphaCornerColor.rgba());
                    image.setColor(2, menuItem->palette.base().color().rgba());
                    image.setColor(3, selected
                                   ? mergedColors(borderColor, option->palette.highlight().color()).rgba()
                                   : alphaCornerColor.rgba());
                    painter->drawImage(checkRect.topLeft(), image);

                    if (checked || (sunken && enabled)) {
                        image = QImage(qt_plastique_radio_check);
                        if (sunken && enabled) {
                            image.setColor(1, mergedColors(menuItem->palette.background().color(), alphaTextColor).rgba());
                            image.setColor(2, mergedColors(menuItem->palette.background().color(), menuItem->palette.foreground().color()).rgba());
                        } else {
                            image.setColor(1, alphaTextColor.rgba());
                            image.setColor(2, menuItem->palette.foreground().color().rgba());
                        }
                        painter->drawImage(checkRect, image);
                    }

                } else {
                    // Check box
                    if (menuItem->icon.isNull()) {
                        painter->setPen(borderColor);
                        painter->drawRect(checkRect.adjusted(0, 0, -1, -1));
                        painter->setPen(mergedColors(borderColor,
                                                     selected ? option->palette.highlight().color()
                                                     : option->palette.background().color()));
                        painter->drawPoint(checkRect.topLeft());
                        painter->drawPoint(checkRect.topRight());
                        painter->drawPoint(checkRect.bottomLeft());
                        painter->drawPoint(checkRect.bottomRight());
                        painter->fillRect(checkRect.adjusted(1, 1, -1, -1), option->palette.base().color());

                        if (checked || (sunken && enabled)) {
                            QImage image(qt_plastique_check);
                            if (sunken && enabled) {
                                image.setColor(0, mergedColors(menuItem->palette.background().color(), menuItem->palette.foreground().color()).rgba());
                                image.setColor(1, mergedColors(menuItem->palette.background().color(), alphaTextColor).rgba());
                            } else {
                                image.setColor(0, option->palette.text().color().rgba());
                                image.setColor(1, alphaTextColor.rgba());
                            }
                            painter->drawImage(QPoint(checkRect.center().x() - image.width() / 2,
                                                      checkRect.center().y() - image.height() / 2), image);
                        }
                    } else if (checked) {
                        int iconSize = qMax(menuItem->maxIconWidth, 20);
                        QRect sunkenRect(option->rect.left() + 1,
                                         option->rect.top() + (option->rect.height() - iconSize) / 2 + 1,
                                         iconSize, iconSize);
                        sunkenRect = visualRect(menuItem->direction, menuItem->rect, sunkenRect);

                        QStyleOption opt = *option;
                        opt.state |= State_Sunken;
                        opt.rect = sunkenRect;
                        qt_plastique_drawShadedPanel(painter, &opt, false, widget);
                    }
                }
            }

            // Text and icon, ripped from windows style
            bool dis = !(menuItem->state & State_Enabled);
            bool act = menuItem->state & State_Selected;
            const QStyleOption *opt = option;
            const QStyleOptionMenuItem *menuitem = menuItem;
            int checkcol = qMax(menuitem->maxIconWidth, 20);
            QPainter *p = painter;
            QRect vCheckRect = visualRect(opt->direction, menuitem->rect,
                                          QRect(menuitem->rect.x(), menuitem->rect.y(),
                                                checkcol, menuitem->rect.height()));
            if (!menuItem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checked)
                    pixmap = menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, QIcon::On);
                else
                    pixmap = menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();

                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vCheckRect.center());
                painter->setPen(menuItem->palette.text().color());
                if (checkable && checked)
                    painter->drawPixmap(QPoint(pmr.left() + 1, pmr.top() + 1), pixmap);
                else
                    painter->drawPixmap(pmr.topLeft(), pixmap);
            }

            if (selected) {
                painter->setPen(menuItem->palette.highlightedText().color());
            } else {
                painter->setPen(menuItem->palette.text().color());
            }
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }
            int xm = windowsItemFrame + checkcol + windowsItemHMargin;
            int xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect,
                                                     QRect(textRect.topRight(), menuitem->rect.bottomRight()));
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1,1,1,1), text_flags, s.mid(t + 1));
                        p->setPen(discol);
                    }
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                p->setFont(font);
                if (dis && !act) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1,1,1,1), text_flags, s.left(t));
                    p->setPen(discol);
                }
                p->drawText(vTextRect, text_flags, s.left(t));
                p->restore();
            }

            // Arrow
            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (menuItem->rect.height() - 4) / 2;
                PrimitiveElement arrow;
                arrow = (opt->direction == Qt::RightToLeft) ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                int xpos = menuItem->rect.left() + menuItem->rect.width() - 6 - 2 - dim;
                QRect  vSubMenuRect = visualRect(option->direction, menuItem->rect,
                                                 QRect(xpos, menuItem->rect.top() + menuItem->rect.height() / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuItem;
                newMI.rect = vSubMenuRect;
                newMI.state = !enabled ? State_None : State_Enabled;
                if (selected)
                    newMI.palette.setColor(QPalette::ButtonText,
                                           newMI.palette.highlightedText().color());
                drawPrimitive(arrow, &newMI, painter, widget);
            }


            painter->restore();
        }
        break;
#endif // QT_NO_MENU
#ifndef QT_NO_MENUBAR
    case CE_MenuBarItem:
        // Draws a menu bar item; File, Edit, Help etc..
        if ((option->state & State_Selected) && (option->state & State_Enabled)) {
            QPixmap cache;
            QString pixmapName = uniqueName("menubaritem", option, option->rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(option->rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, option->rect.width(), option->rect.height());
                QPainter cachePainter(&cache);

                QRect rect = pixmapRect;

                // gradient fill
                if (option->state & QStyle::State_Enabled) {
                    if ((option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On)) {
                        qt_plastique_draw_gradient(&cachePainter, rect.adjusted(1, 1, -1, -1),
                                                   option->palette.button().color().dark(114),
                                                   option->palette.button().color().dark(106));
                    } else {
                        qt_plastique_draw_gradient(&cachePainter, rect.adjusted(1, 1, -1, -1),
                                                   option->palette.background().color().light(105),
                                                   option->palette.background().color().dark(102));
                    }
                }

                // outer border and corners
                cachePainter.setPen(borderColor);
                cachePainter.drawRect(rect.adjusted(0, 0, -1, -1));
                cachePainter.setPen(alphaCornerColor);
                cachePainter.drawPoint(rect.topLeft());
                cachePainter.drawPoint(rect.topRight());
                cachePainter.drawPoint(rect.bottomLeft());
                cachePainter.drawPoint(rect.bottomRight());

                // inner border
                if ((option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On))
                    cachePainter.setPen(option->palette.button().color().dark(118));
                else
                    cachePainter.setPen(gradientStartColor);
                cachePainter.drawLine(rect.left() + 1, rect.top() + 1, rect.right() - 1, rect.top() + 1);
                cachePainter.drawLine(rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom() - 2);

                if ((option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On))
                    cachePainter.setPen(option->palette.button().color().dark(114));
                else
                    cachePainter.setPen(gradientStopColor.dark(102));
                cachePainter.drawLine(rect.left() + 1, rect.bottom() - 1, rect.right() - 1, rect.bottom() - 1);
                cachePainter.drawLine(rect.right() - 1, rect.top() + 1, rect.right() - 1, rect.bottom() - 2);
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(option->rect.topLeft(), cache);
        } else {
            painter->fillRect(option->rect, option->palette.background());
        }
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
#endif // QT_NO_MENUBAR
#ifndef QT_NO_TOOLBOX
    case CE_ToolBoxTab:
        if (const QStyleOptionToolBox *toolBox = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            painter->save();

            int width = toolBox->rect.width();
            int diag = toolBox->rect.height() - 2;

            // The essential points
            QPoint rightMost(toolBox->rect.right(), toolBox->rect.bottom() - 2);
            QPoint rightEdge(toolBox->rect.right() - width / 10, toolBox->rect.bottom() - 2);
            QPoint leftEdge(toolBox->rect.right() - width / 10 - diag, toolBox->rect.top());
            QPoint leftMost(toolBox->rect.left(), toolBox->rect.top());
            QPoint upOne(0, -1);
            QPoint downOne(0, 1);
            QPoint leftOne(-1, 0);
            QPoint rightOne(1, 0);

            // The upper path
            QPainterPath upperPath(rightMost + rightOne);
            upperPath.lineTo(rightEdge);
            upperPath.lineTo(leftEdge);
            upperPath.lineTo(leftMost + leftOne);
            upperPath.lineTo(toolBox->rect.topLeft() + leftOne + upOne);
            upperPath.lineTo(toolBox->rect.topRight() + rightOne + upOne);
            upperPath.lineTo(rightMost + rightOne);

            // The lower path
            QPainterPath lowerPath(rightMost + rightOne);
            lowerPath.lineTo(rightEdge);
            lowerPath.lineTo(leftEdge);
            lowerPath.lineTo(leftMost + leftOne);
            lowerPath.lineTo(toolBox->rect.bottomLeft() + leftOne + downOne);
            lowerPath.lineTo(toolBox->rect.bottomRight() + rightOne + downOne);
            lowerPath.lineTo(rightMost);

            //painter->fillRect(option->rect, toolBox->palette.base());

            // Fill the tab
            // if (toolBox->selectedPosition == QStyleOptionToolBox::PreviousIsSelected) {
            //    painter->fillPath(upperPath, toolBox->palette.base());
            //    painter->fillPath(lowerPath, toolBox->palette.background());
            // } else {
            //    painter->fillPath(upperPath, toolBox->palette.background());
            //    painter->fillPath(lowerPath, toolBox->palette.base());
            // }

            // Draw the outline
            painter->setPen(borderColor);
            painter->drawLine(rightMost, rightEdge);
            painter->drawLine(rightEdge + leftOne, leftEdge);
            painter->drawLine(leftEdge + leftOne, leftMost);
            painter->setPen(toolBox->palette.base().color());
            painter->drawLine(rightMost + downOne, rightEdge + downOne);
            painter->drawLine(rightEdge + leftOne + downOne, leftEdge + downOne);
            painter->drawLine(leftEdge + leftOne + downOne, leftMost + downOne);

            painter->restore();
        }
        break;
#endif // QT_NO_TOOLBOX
#ifndef QT_NO_SPLITTER
    case CE_Splitter:
        if ((option->state & State_Enabled) && (option->state & State_MouseOver))
            painter->fillRect(option->rect, option->palette.base());
        if (option->state & State_Horizontal) {
            int height = option->rect.height() / 3;
            QRect rect(option->rect.left() + (option->rect.width() / 2 - 1),
                       option->rect.center().y() - height / 2, 3, height);
            qt_plastique_draw_handle(painter, option, rect, Qt::Horizontal, widget);
        } else {
            int width = option->rect.width() / 3;
            QRect rect(option->rect.center().x() - width / 2,
                       option->rect.top() + (option->rect.height() / 2) - 1, width, 3);
            qt_plastique_draw_handle(painter, option, rect, Qt::Vertical, widget);
        }
        break;
#endif // QT_NO_SPLITTER
#ifndef QT_NO_DOCKWIDGET
    case CE_DockWidgetTitle:
        if (const QStyleOptionDockWidget *dockWidget = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            painter->save();

            // Find text width and title rect
            int textWidth = option->fontMetrics.width(dockWidget->title);
            int margin = 4;
            QRect titleRect = visualRect(dockWidget->direction, dockWidget->rect,
                                         dockWidget->rect.adjusted(margin, 0, -margin * 2 - 26, 0));

            // Chop and insert ellide into title if text is too wide
            QString title = elliditide(dockWidget->title, dockWidget->fontMetrics, titleRect, &textWidth);

            // Draw the toolbar handle pattern to the left and right of the text
            QImage handle(qt_toolbarhandle);
            handle.setColor(1, alphaCornerColor.rgba());
            handle.setColor(2, mergedColors(alphaCornerColor, option->palette.base().color()).rgba());
            handle.setColor(3, option->palette.base().color().rgba());

            if (title.isEmpty()) {
                // Joint handle if there's no title
                QRect r;
#ifdef QT3_SUPPORT
                // Q3DockWindow doesn't need space for buttons
                if (widget && widget->inherits("Q3DockWindowTitleBar")) {
                    r = dockWidget->rect;
                } else
#endif
                r.setRect(titleRect.left(), titleRect.top(), titleRect.width(), titleRect.bottom());
                int nchunks = (r.width() / handle.width()) - 1;
                int indent = (r.width() - (nchunks * handle.width())) / 2;
                for (int i = 0; i < nchunks; ++i) {
                    painter->drawImage(QPoint(r.left() + indent + i * handle.width(),
                                              r.center().y() - handle.height() / 2),
                                       handle);
                }
            } else {
                // Handle pattern to the left of the title
                QRect leftSide(titleRect.left(), titleRect.top(),
                               titleRect.width() / 2 - textWidth / 2 - margin, titleRect.bottom());
                int nchunks = leftSide.width() / handle.width();
                int indent = (leftSide.width() - (nchunks * handle.width())) / 2;
                for (int i = 0; i < nchunks; ++i) {
                    painter->drawImage(QPoint(leftSide.left() + indent + i * handle.width(),
                                              leftSide.center().y() - handle.height() / 2),
                                       handle);
                }

                // Handle pattern to the right of the title
                QRect rightSide = titleRect.adjusted(titleRect.width() / 2 + textWidth / 2 + margin, 0, 0, 0);
                nchunks = rightSide.width() / handle.width();
                indent = (rightSide.width() - (nchunks * handle.width())) / 2;
                for (int j = 0; j < nchunks; ++j) {
                    painter->drawImage(QPoint(rightSide.left() + indent + j * handle.width(),
                                              rightSide.center().y() - handle.height() / 2),
                                       handle);
                }
            }

            // Draw the text centered
            QFont font = painter->font();
            font.setPointSize(font.pointSize() - 1);
            painter->setFont(font);
            painter->setPen(dockWidget->palette.text().color());
            painter->drawText(titleRect, title, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));

            painter->restore();
        }
        break;
#endif // QT_NO_DOCKWIDGET
#ifndef QT_NO_TOOLBAR
    case CE_ToolBar:
        if (const QStyleOptionToolBar *toolBar = qstyleoption_cast<const QStyleOptionToolBar *>(option)) {
            // Draws the light line above and the dark line below menu bars and
            // tool bars.
            QPen oldPen = painter->pen();
            if (toolBar->toolBarArea == Qt::TopToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                    || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The end and onlyone top toolbar lines draw a double
                    // line at the bottom to blend with the central
                    // widget.
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.left(), option->rect.bottom() - 1,
                                      option->rect.right(), option->rect.bottom() - 1);
                } else {
                    // All others draw a single dark line at the bottom.
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                // All top toolbar lines draw a light line at the top.
                painter->setPen(option->palette.background().color().light(104));
                painter->drawLine(option->rect.topLeft(), option->rect.topRight());
            } else if (toolBar->toolBarArea == Qt::BottomToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                    || toolBar->positionOfLine == QStyleOptionToolBar::Middle) {
                    // The end and middle bottom toolbar lines draw a dark
                    // line at the bottom.
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::Beginning
                    || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The beginning and onlyone toolbar lines draw a
                    // double line at the bottom to blend with the
                    // statusbar.
                    // ### The styleoption could contain whether the
                    // mainwindow has a menubar and a statusbar, and
                    // possibly dockwidgets.
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.left(), option->rect.bottom() - 1,
                                      option->rect.right(), option->rect.bottom() - 1);
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.left(), option->rect.top() + 1,
                                      option->rect.right(), option->rect.top() + 1);

                } else {
                    // All other bottom toolbars draw a light line at the top.
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                }
            }
            if (toolBar->toolBarArea == Qt::LeftToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::Middle
                    || toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // The middle and left end toolbar lines draw a light
                    // line to the left.
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // All other left toolbar lines draw a dark line to the right
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.right() - 1, option->rect.top(),
                                      option->rect.right() - 1, option->rect.bottom());
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                } else {
                    // All other left toolbar lines draw a dark line to the right
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                }
            } else if (toolBar->toolBarArea == Qt::RightToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::Middle
                    || toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // Right middle and end toolbar lines draw the dark right line
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                    || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The right end and single toolbar draws the dark
                    // line on its left edge
                    painter->setPen(alphaCornerColor);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                    // And a light line next to it
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.left() + 1, option->rect.top(),
                                      option->rect.left() + 1, option->rect.bottom());
                } else {
                    // Other right toolbars draw a light line on its left edge
                    painter->setPen(option->palette.background().color().light(104));
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                }
            }
            painter->setPen(oldPen);
        }
        break;
#endif // QT_NO_TOOLBAR
#ifndef QT_NO_SCROLLBAR
    case CE_ScrollBarAddLine:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {

            bool horizontal = scrollBar->orientation == Qt::Horizontal;
            bool reverse = scrollBar->direction == Qt::RightToLeft;
            bool sunken = scrollBar->state & State_Sunken;

            QString addLinePixmapName = uniqueName("scrollbar_addline", option, option->rect.size());
            QPixmap cache;
            if (!UsePixmapCache || !QPixmapCache::find(addLinePixmapName, cache)) {
                cache = QPixmap(option->rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, cache.width(), cache.height());
                QPainter addLinePainter(&cache);
                addLinePainter.fillRect(pixmapRect, option->palette.background());

                if (option->state & State_Enabled) {
                    // Gradient
                    QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top() + 2,
                                             pixmapRect.center().x(), pixmapRect.bottom() - 2);
                    if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                        gradient.setColorAt(0, gradientStopColor);
                        gradient.setColorAt(1, gradientStopColor);
                    } else {
                        gradient.setColorAt(0, gradientStartColor.light(105));
                        gradient.setColorAt(1, gradientStopColor);
                    }
                    addLinePainter.fillRect(pixmapRect.left() + 2, pixmapRect.top() + 2,
                                            pixmapRect.right() - 3, pixmapRect.bottom() - 3,
                                            gradient);
                }

                // Details
                QImage addButton;
                if (horizontal) {
                    addButton = QImage(reverse ? qt_scrollbar_button_left : qt_scrollbar_button_right);
                } else {
                    addButton = QImage(qt_scrollbar_button_down);
                }
                addButton.setColor(1, alphaCornerColor.rgba());
                addButton.setColor(2, borderColor.rgba());
                if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                    addButton.setColor(3, gradientStopColor.rgba());
                    addButton.setColor(4, gradientStopColor.rgba());
                } else {
                    addButton.setColor(3, gradientStartColor.light(105).rgba());
                    addButton.setColor(4, gradientStopColor.rgba());
                }
                addButton.setColor(5, scrollBar->palette.text().color().rgba());
                addLinePainter.drawImage(pixmapRect, addButton);

                // Arrow
                if (horizontal) {
                    QImage arrow(reverse ? qt_scrollbar_button_arrow_left : qt_scrollbar_button_arrow_right);
                    arrow.setColor(1, scrollBar->palette.foreground().color().rgba());

                    if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                        addLinePainter.drawImage(QPoint(pixmapRect.left() + 7, pixmapRect.top() + 5), arrow);
                    } else {
                        addLinePainter.drawImage(QPoint(pixmapRect.left() + 6, pixmapRect.top() + 4), arrow);
                    }
                } else {
                    QImage arrow(qt_scrollbar_button_arrow_down);
                    arrow.setColor(1, scrollBar->palette.foreground().color().rgba());

                    if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                        addLinePainter.drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 7), arrow);
                    } else {
                        addLinePainter.drawImage(QPoint(pixmapRect.left() + 4, pixmapRect.top() + 6), arrow);
                    }
                }
                addLinePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(addLinePixmapName, cache);
            }
            painter->drawPixmap(option->rect.topLeft(), cache);
        }
        break;
    case CE_ScrollBarSubPage:
    case CE_ScrollBarAddPage:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            bool sunken = scrollBar->state & State_Sunken;
            bool horizontal = scrollBar->orientation == Qt::Horizontal;

            QString groovePixmapName = uniqueName("scrollbar_groove", option, option->rect.size());
            if (sunken)
                groovePixmapName += "-sunken";

            QPixmap cache;
            if (!UsePixmapCache || !QPixmapCache::find(groovePixmapName, cache)) {
                cache = QPixmap(option->rect.size());
                cache.fill(option->palette.background().color());
                QPainter groovePainter(&cache);
                QRect pixmapRect = QRect(0, 0, option->rect.width(), option->rect.height());
                QColor color = scrollBar->palette.base().color().dark(sunken ? 125 : 100);
                groovePainter.fillRect(pixmapRect, QBrush(color, Qt::Dense4Pattern));

                QColor edgeColor = scrollBar->palette.base().color().dark(125);
                if (horizontal) {
                    groovePainter.setBrushOrigin(1, 0);
                    groovePainter.fillRect(QRect(pixmapRect.topLeft(), QSize(pixmapRect.width(), 1)),
                                           QBrush(edgeColor, Qt::Dense4Pattern));
                    groovePainter.fillRect(QRect(pixmapRect.bottomLeft(), QSize(pixmapRect.width(), 1)),
                                           QBrush(edgeColor, Qt::Dense4Pattern));
                } else {
                    groovePainter.setBrushOrigin(0, 1);
                    groovePainter.fillRect(QRect(pixmapRect.topLeft(), QSize(1, pixmapRect.height())),
                                           QBrush(edgeColor, Qt::Dense4Pattern));
                    groovePainter.fillRect(QRect(pixmapRect.topRight(), QSize(1, pixmapRect.height())),
                                           QBrush(edgeColor, Qt::Dense4Pattern));
                }

                groovePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(groovePixmapName, cache);
            }
            painter->drawPixmap(option->rect.topLeft(), cache);
        }
        break;
    case CE_ScrollBarSubLine:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect scrollBarSubLine = scrollBar->rect;

            bool horizontal = scrollBar->orientation == Qt::Horizontal;
            bool isEnabled = scrollBar->state & State_Enabled;
            bool reverse = scrollBar->direction == Qt::RightToLeft;
            bool sunken = scrollBar->state & State_Sunken;

            // The SubLine (up/left) buttons
            int scrollBarExtent = pixelMetric(PM_ScrollBarExtent, option, widget);
            QRect button1;
            QRect button2;
            if (horizontal) {
                button1.setRect(scrollBarSubLine.left(), scrollBarSubLine.top(), 16, scrollBarExtent);
                button2.setRect(scrollBarSubLine.right() - 15, scrollBarSubLine.top(), 16, scrollBarExtent);
            } else {
                button1.setRect(scrollBarSubLine.left(), scrollBarSubLine.top(), scrollBarExtent, 16);
                button2.setRect(scrollBarSubLine.left(), scrollBarSubLine.bottom() - 15, scrollBarExtent, 16);
            }

            QString subLinePixmapName = uniqueName("scrollbar_subline", option, button1.size());
            QPixmap cache;
            if (!UsePixmapCache || !QPixmapCache::find(subLinePixmapName, cache)) {
                cache = QPixmap(button1.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, cache.width(), cache.height());
                QPainter subLinePainter(&cache);
                subLinePainter.fillRect(pixmapRect, option->palette.background());

                if (isEnabled) {
                    // Gradients
                    if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                        qt_plastique_draw_gradient(&subLinePainter,
                                                   QRect(pixmapRect.left() + 2, pixmapRect.top() + 2,
                                                         pixmapRect.right() - 3, pixmapRect.bottom() - 3),
                                                   gradientStopColor,
                                                   gradientStopColor);
                    } else {
                        qt_plastique_draw_gradient(&subLinePainter,
                                                   QRect(pixmapRect.left() + 2, pixmapRect.top() + 2,
                                                         pixmapRect.right() - 3, pixmapRect.bottom() - 3),
                                                   gradientStartColor.light(105),
                                                   gradientStopColor);
                    }
                }

                // Details
                QImage subButton;
                if (horizontal) {
                    subButton = QImage(reverse ? qt_scrollbar_button_right : qt_scrollbar_button_left);
                } else {
                    subButton = QImage(qt_scrollbar_button_up);
                }
                subButton.setColor(1, alphaCornerColor.rgba());
                subButton.setColor(2, borderColor.rgba());
                if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                    subButton.setColor(3, gradientStopColor.rgba());
                    subButton.setColor(4, gradientStopColor.rgba());
                } else {
                    subButton.setColor(3, gradientStartColor.light(105).rgba());
                    subButton.setColor(4, gradientStopColor.rgba());
                }
                subButton.setColor(5, scrollBar->palette.text().color().rgba());
                subLinePainter.drawImage(pixmapRect, subButton);

                // Arrows
                if (horizontal) {
                    QImage arrow(reverse ? qt_scrollbar_button_arrow_right : qt_scrollbar_button_arrow_left);
                    arrow.setColor(1, scrollBar->palette.foreground().color().rgba());

                    if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                        subLinePainter.drawImage(QPoint(pixmapRect.left() + 6, pixmapRect.top() + 5), arrow);
                    } else {
                        subLinePainter.drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 4), arrow);
                    }
                } else {
                    QImage arrow(qt_scrollbar_button_arrow_up);
                    arrow.setColor(1, scrollBar->palette.foreground().color().rgba());

                    if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                        subLinePainter.drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 7), arrow);
                    } else {
                        subLinePainter.drawImage(QPoint(pixmapRect.left() + 4, pixmapRect.top() + 6), arrow);
                    }
                }
                subLinePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(subLinePixmapName, cache);
            }
            painter->drawPixmap(button1.topLeft(), cache);
            painter->drawPixmap(button2.topLeft(), cache);
        }
        break;
    case CE_ScrollBarSlider:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            bool horizontal = scrollBar->orientation == Qt::Horizontal;
            bool isEnabled = scrollBar->state & State_Enabled;

            // The slider
            if (option->rect.isValid()) {
                QString sliderPixmapName = uniqueName("scrollbar_slider", option, option->rect.size());
                if (horizontal)
                    sliderPixmapName += QLatin1String("-horizontal");

                QPixmap cache;
                if (!UsePixmapCache || !QPixmapCache::find(sliderPixmapName, cache)) {
                    cache = QPixmap(option->rect.size());
                    cache.fill(Qt::white);
                    QRect pixmapRect(0, 0, cache.width(), cache.height());
                    QPainter sliderPainter(&cache);
                    bool sunken = (scrollBar->state & State_Sunken);

                    if (isEnabled) {
                        QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                                 pixmapRect.center().x(), pixmapRect.bottom());
                        if (sunken) {
                            gradient.setColorAt(0, gradientStartColor.light(110));
                            gradient.setColorAt(1, gradientStopColor.light(105));
                        } else {
                            gradient.setColorAt(0, gradientStartColor.light(105));
                            gradient.setColorAt(1, gradientStopColor);
                        }
                        sliderPainter.fillRect(pixmapRect.adjusted(2, 2, -2, -2), gradient);
                    } else {
                        sliderPainter.fillRect(pixmapRect.adjusted(2, 2, -2, -2), option->palette.background());
                    }

                    sliderPainter.setPen(borderColor);
                    sliderPainter.drawRect(pixmapRect.adjusted(0, 0, -1, -1));
                    sliderPainter.setPen(alphaCornerColor);
                    sliderPainter.drawPoint(pixmapRect.left(), pixmapRect.top());
                    sliderPainter.drawPoint(pixmapRect.left(), pixmapRect.bottom());
                    sliderPainter.drawPoint(pixmapRect.right(), pixmapRect.top());
                    sliderPainter.drawPoint(pixmapRect.right(), pixmapRect.bottom());

                    sliderPainter.setPen(sunken ? gradientStartColor.light(110) : gradientStartColor.light(105));
                    sliderPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 1,
                                           pixmapRect.right() - 1, pixmapRect.top() + 1);
                    sliderPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 2,
                                           pixmapRect.left() + 1, pixmapRect.bottom() - 2);

                    sliderPainter.setPen(sunken ? gradientStopColor.light(105) : gradientStopColor);
                    sliderPainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom() - 1,
                                           pixmapRect.right() - 1, pixmapRect.bottom() - 1);
                    sliderPainter.drawLine(pixmapRect.right() - 1, pixmapRect.top() + 2,
                                           pixmapRect.right() - 1, pixmapRect.bottom() - 1);

                    int sliderMinLength = pixelMetric(PM_ScrollBarSliderMin, scrollBar, widget);
                    if ((horizontal && scrollBar->rect.width() > sliderMinLength)
                        || (!horizontal && scrollBar->rect.height() > sliderMinLength)) {
                        QImage pattern(horizontal ? qt_scrollbar_slider_pattern_horizontal
                                       : qt_scrollbar_slider_pattern_vertical);
                        pattern.setColor(1, alphaCornerColor.rgba());
                        pattern.setColor(2, (sunken ? gradientStartColor.light(110) : gradientStartColor.light(105)).rgba());

                        if (horizontal) {
                            sliderPainter.drawImage(pixmapRect.center().x() - pattern.width() / 2 + 1,
                                                    pixmapRect.center().y() - 4,
                                                    pattern);
                        } else {
                            sliderPainter.drawImage(pixmapRect.center().x() - 4,
                                                    pixmapRect.center().y() - pattern.height() / 2 + 1,
                                                    pattern);
                        }
                    }
                    sliderPainter.end();
                    // insert the slider into the cache
                    if (UsePixmapCache)
                        QPixmapCache::insert(sliderPixmapName, cache);
                }
                painter->drawPixmap(option->rect.topLeft(), cache);
            }
        }
        break;
#endif
    default:
        QWindowsStyle::drawControl(element, option, painter, widget);
        break;
    }
}

/*!
  \reimp
*/
void QPlastiqueStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                         QPainter *painter, const QWidget *widget) const
{
    QColor borderColor = option->palette.background().color().dark(178);
    QColor alphaCornerColor;
   if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
    } else {
        alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    }
    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);
    QColor highlightedGradientStartColor = option->palette.button().color().light(101);
    QColor highlightedGradientStopColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 85);
    QColor highlightedDarkInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 35);
    QColor highlightedLightInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 58);

    switch (control) {
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            QRect ticks = subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);
            QRect grooveRegion = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            bool horizontal = slider->orientation == Qt::Horizontal;
            bool ticksAbove = slider->tickPosition & QSlider::TicksAbove;
            bool ticksBelow = slider->tickPosition & QSlider::TicksBelow;

            QRect groove;
            //The clickable region is 5 px wider than the visible groove for improved usability
            if (grooveRegion.isValid())
                groove = horizontal ? grooveRegion.adjusted(0, 5, 0, -5) : grooveRegion.adjusted(5, 0, -5, 0);

            QPixmap cache;

            if ((option->subControls & SC_SliderGroove) && groove.isValid()) {
                QString groovePixmapName = uniqueName("slider_groove", option, groove.size());
                if (!UsePixmapCache || !QPixmapCache::find(groovePixmapName, cache)) {
                    cache = QPixmap(groove.size());
                    cache.fill(Qt::white);
                    QRect pixmapRect(0, 0, groove.width(), groove.height());
                    QPainter groovePainter(&cache);
                    groovePainter.fillRect(pixmapRect, option->palette.background());

                    // draw groove
                    if (horizontal) {
                        groovePainter.setPen(borderColor);
                        groovePainter.drawLine(pixmapRect.left() + 1, pixmapRect.top(),
                                               pixmapRect.right() - 1, pixmapRect.top());
                        groovePainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom(),
                                               pixmapRect.right() - 1, pixmapRect.bottom());
                        groovePainter.drawLine(pixmapRect.left(), pixmapRect.top() + 1,
                                               pixmapRect.left(), pixmapRect.bottom() - 1);
                        groovePainter.drawLine(pixmapRect.right(), pixmapRect.top() + 1,
                                               pixmapRect.right(), pixmapRect.bottom() - 1);
                        groovePainter.setPen(alphaCornerColor);
                        groovePainter.drawPoint(pixmapRect.left(), pixmapRect.top());
                        groovePainter.drawPoint(pixmapRect.left(), pixmapRect.bottom());
                        groovePainter.drawPoint(pixmapRect.right(), pixmapRect.top());
                        groovePainter.drawPoint(pixmapRect.right(), pixmapRect.bottom());
                    } else {
                        groovePainter.setPen(borderColor);
                        groovePainter.drawLine(pixmapRect.left() + 1, pixmapRect.top(),
                                               pixmapRect.right() - 1, pixmapRect.top());
                        groovePainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom(),
                                               pixmapRect.right() - 1, pixmapRect.bottom());
                        groovePainter.drawLine(pixmapRect.left(), pixmapRect.top() + 1,
                                               pixmapRect.left(), pixmapRect.bottom() - 1);
                        groovePainter.drawLine(pixmapRect.right(), pixmapRect.top() + 1,
                                               pixmapRect.right(), pixmapRect.bottom() - 1);
                        groovePainter.setPen(alphaCornerColor);
                        groovePainter.drawPoint(pixmapRect.left(), pixmapRect.top());
                        groovePainter.drawPoint(pixmapRect.right(), pixmapRect.top());
                        groovePainter.drawPoint(pixmapRect.left(), pixmapRect.bottom());
                        groovePainter.drawPoint(pixmapRect.right(), pixmapRect.bottom());
                    }
                    groovePainter.end();
                    if (UsePixmapCache)
                        QPixmapCache::insert(groovePixmapName, cache);
                }
                painter->drawPixmap(groove.topLeft(), cache);
            }

            if ((option->subControls & SC_SliderHandle) && handle.isValid()) {
                QString handlePixmapName = uniqueName("slider_handle", option, handle.size());
                if (ticksAbove && !ticksBelow)
                    handlePixmapName += QLatin1String("-flipped");
                if ((option->activeSubControls & SC_SliderHandle) && (option->state & State_Sunken))
                    handlePixmapName += QLatin1String("-sunken");

                if (!UsePixmapCache || !QPixmapCache::find(handlePixmapName, cache)) {
                    cache = QPixmap(handle.size());
                    cache.fill(Qt::white);
                    QRect pixmapRect(0, 0, handle.width(), handle.height());
                    QPainter handlePainter(&cache);
                    handlePainter.fillRect(pixmapRect, option->palette.background());

                    // draw handle
                    if (horizontal) {
                        QPainterPath path;
                        if (ticksAbove && !ticksBelow) {
                            path.moveTo(QPoint(pixmapRect.right(), pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.bottom() - 10));
                            path.lineTo(QPoint(pixmapRect.right() - 5, pixmapRect.bottom() - 14));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 10));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.bottom()));
                        } else {
                            path.moveTo(QPoint(pixmapRect.right(), pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.top() + 10));
                            path.lineTo(QPoint(pixmapRect.right() - 5, pixmapRect.top() + 14));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.top() + 10));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.top() + 1));
                        }
                        if (slider->state & State_Enabled) {
                            QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                                     pixmapRect.center().x(), pixmapRect.bottom());
                            if ((option->activeSubControls & SC_SliderHandle) && (option->state & State_Sunken)) {
                                gradient.setColorAt(0, gradientStartColor.light(110));
                                gradient.setColorAt(1, gradientStopColor.light(110));
                            } else {
                                gradient.setColorAt(0, gradientStartColor);
                                gradient.setColorAt(1, gradientStopColor);
                            }
                            handlePainter.fillPath(path, gradient);
                        } else {
                            handlePainter.fillPath(path, slider->palette.background());
                        }
                    } else {
                        QPainterPath path;
                        if (ticksAbove && !ticksBelow) {
                            path.moveTo(QPoint(pixmapRect.right(), pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.right() - 10, pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.right() - 14, pixmapRect.top() + 5));
                            path.lineTo(QPoint(pixmapRect.right() - 10, pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.top() + 1));
                        } else {
                            path.moveTo(QPoint(pixmapRect.left() + 1, pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.left() + 10, pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.left() + 14, pixmapRect.top() + 5));
                            path.lineTo(QPoint(pixmapRect.left() + 10, pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.top() + 1));
                        }
                        if (slider->state & State_Enabled) {
                            QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                                     pixmapRect.center().x(), pixmapRect.bottom());
                            gradient.setColorAt(0, gradientStartColor);
                            gradient.setColorAt(1, gradientStopColor);
                            handlePainter.fillPath(path, gradient);
                        } else {
                            handlePainter.fillPath(path, slider->palette.background());
                        }
                    }

                    QImage image;
                    if (horizontal) {
                        image = QImage((ticksAbove && !ticksBelow) ? qt_plastique_slider_horizontalhandle_up : qt_plastique_slider_horizontalhandle);
                    } else {
                        image = QImage((ticksAbove && !ticksBelow) ? qt_plastique_slider_verticalhandle_left : qt_plastique_slider_verticalhandle);
                    }

                    image.setColor(1, borderColor.rgba());
                    image.setColor(2, gradientStartColor.rgba());
                    image.setColor(3, alphaCornerColor.rgba());
                    handlePainter.drawImage(pixmapRect, image);
                    handlePainter.end();
                    if (UsePixmapCache)
                        QPixmapCache::insert(handlePixmapName, cache);
                }

                painter->drawPixmap(handle.topLeft(), cache);

                if (slider->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*slider);
                    fropt.rect = subElementRect(SE_SliderFocusRect, slider, widget);
                    drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
            }

            if (option->subControls & SC_SliderTickmarks) {
                QPen oldPen = painter->pen();
                painter->setPen(borderColor);
                int tickSize = pixelMetric(PM_SliderTickmarkOffset, option, widget);
                int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
                int interval = slider->tickInterval;
                if (interval <= 0) {
                    interval = slider->singleStep;
                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                        available)
                        - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          0, available) < 3)
                        interval = slider->pageStep;
                }
                if (interval <= 0)
                    interval = 1;

                int sliderLength = slider->maximum - slider->minimum + 1;
                int nticks = sliderLength / interval; // add one to get the end tickmark
                if (sliderLength % interval > 0)
                    nticks++; // round up the number of tick marks

                int v = slider->minimum;
                int len = pixelMetric(PM_SliderLength, slider, widget);
                while (v <= slider->maximum) {
                    int pos = sliderPositionFromValue(slider->minimum, slider->maximum,
                                                      v, (horizontal
                                                          ? slider->rect.width()
                                                          : slider->rect.height()) - len,
                                                      slider->upsideDown) + len / 2;

                    int extra = 2 - ((v == slider->minimum || v == slider->maximum) ? 1 : 0);

                    if (horizontal) {
                        if (ticksAbove) {
                            painter->drawLine(pos, slider->rect.top() + extra,
                                pos, slider->rect.top() + tickSize);
                        }
                        if (ticksBelow) {
                            painter->drawLine(pos, slider->rect.bottom() - extra,
                                              pos, slider->rect.bottom() - tickSize);
                        }
                    } else {
                        if (ticksAbove) {
                            painter->drawLine(slider->rect.left() + extra, pos,
                                              slider->rect.left() + tickSize, pos);
                        }
                        if (ticksBelow) {
                            painter->drawLine(slider->rect.right() - extra, pos,
                                              slider->rect.right() - tickSize, pos);
                        }
                    }

                    v += interval;
                }
                painter->setPen(oldPen);
            }
        }
        break;
#endif // QT_NO_SLIDER
#ifndef QT_NO_SPINBOX
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            QPixmap cache;
            QString pixmapName = uniqueName("spinbox", spinBox, spinBox->rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(spinBox->rect.size());
                cache.fill(Qt::white);
                QRect pixmapRect(0, 0, spinBox->rect.width(), spinBox->rect.height());
                QPainter cachePainter(&cache);

                bool isEnabled = (spinBox->state & State_Enabled);
                bool focus = isEnabled && (spinBox->state & State_HasFocus);
                bool hover = isEnabled && (spinBox->state & State_MouseOver);
                bool sunken = (spinBox->state & State_Sunken);
                bool upIsActive = (spinBox->activeSubControls == SC_SpinBoxUp);
                bool downIsActive = (spinBox->activeSubControls == SC_SpinBoxDown);

                QRect rect = pixmapRect;

                // Draw a line edit
                QStyleOptionFrame lineEdit;
                lineEdit.QStyleOption::operator=(*spinBox);
                lineEdit.rect = pixmapRect;
                lineEdit.state = spinBox->state;
                lineEdit.state |= State_Sunken;
                drawPrimitive(PE_FrameLineEdit, &lineEdit, &cachePainter, widget);

                QStyleOptionSpinBox spinBoxCopy = *spinBox;
                spinBoxCopy.rect = pixmapRect;
                QRect upRect = subControlRect(CC_SpinBox, &spinBoxCopy, SC_SpinBoxUp, widget);
                QRect downRect = subControlRect(CC_SpinBox, &spinBoxCopy, SC_SpinBoxDown, widget);

                if (isEnabled) {
                    // gradients
                    if (upIsActive && sunken) {
                        cachePainter.fillRect(upRect.adjusted(1, 1, -1, 0), gradientStopColor);
                    } else {
                        if (focus) {
                            qt_plastique_draw_gradient(&cachePainter, upRect.adjusted(2, 2, -2, -2),
                                                       highlightedGradientStartColor,
                                                       highlightedGradientStopColor);
                        } else {
                            qt_plastique_draw_gradient(&cachePainter, upRect.adjusted(2, 2, -2, -2),
                                                       gradientStartColor,
                                                       gradientStopColor);
                        }
                    }

                    if (downIsActive && sunken) {
                        cachePainter.fillRect(downRect.adjusted(1, 0, -1, -1), gradientStopColor);
                    } else {
                        if (focus) {
                            qt_plastique_draw_gradient(&cachePainter, downRect.adjusted(2, 1, -2, -2),
                                                       highlightedGradientStartColor,
                                                       highlightedGradientStopColor);
                        } else {
                            qt_plastique_draw_gradient(&cachePainter, downRect.adjusted(2, 1, -2, -2),
                                                       gradientStartColor,
                                                       gradientStopColor);
                        }
                    }
                } else {
                    cachePainter.fillRect(upRect.adjusted(1, 1, -1, 0), option->palette.background());
                    cachePainter.fillRect(downRect.adjusted(1, 0, -1, -1), option->palette.background());
                }

                // outline the up/down buttons
                cachePainter.setPen(borderColor);
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                    cachePainter.drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                    cachePainter.drawLine(upRect.right(), upRect.bottom(), upRect.left(), upRect.bottom());
                    cachePainter.drawLine(upRect.right(), upRect.top(), upRect.left() + 2, upRect.top());
                    cachePainter.drawLine(upRect.right(), upRect.bottom(), upRect.left() + 2, upRect.bottom());
                    cachePainter.drawLine(upRect.left(), upRect.top() + 2, upRect.left(), upRect.bottom() - 1);
                    cachePainter.drawLine(downRect.left(), downRect.top(), downRect.left(), downRect.bottom() - 2);
                    cachePainter.drawLine(downRect.right(), downRect.bottom(), downRect.left() + 2, downRect.bottom());
                    cachePainter.drawPoint(upRect.left() + 1, upRect.top() + 1);
                    cachePainter.drawPoint(downRect.left() - 1, downRect.bottom() - 1);
                    cachePainter.setPen(alphaCornerColor);
                    cachePainter.drawPoint(upRect.left() + 1, upRect.top());
                    cachePainter.drawPoint(upRect.left(), upRect.top() + 1);
                    cachePainter.drawPoint(downRect.left() + 1, downRect.bottom());
                    cachePainter.drawPoint(downRect.left(), downRect.bottom() - 1);
                } else {
                    cachePainter.drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                    cachePainter.drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                    cachePainter.drawLine(upRect.left(), upRect.bottom(), upRect.right(), upRect.bottom());
                    cachePainter.drawLine(upRect.left(), upRect.top(), upRect.right() - 2, upRect.top());
                    cachePainter.drawLine(upRect.left(), upRect.bottom(), upRect.right() - 2, upRect.bottom());
                    cachePainter.drawLine(upRect.right(), upRect.top() + 2, upRect.right(), upRect.bottom() - 1);
                    cachePainter.drawLine(downRect.right(), downRect.top(), downRect.right(), downRect.bottom() - 2);
                    cachePainter.drawLine(downRect.left(), downRect.bottom(), downRect.right() - 2, downRect.bottom());
                    cachePainter.drawPoint(upRect.right() - 1, upRect.top() + 1);
                    cachePainter.drawPoint(downRect.right() - 1, downRect.bottom() - 1);
                    cachePainter.setPen(alphaCornerColor);
                    cachePainter.drawPoint(upRect.right() - 1, upRect.top());
                    cachePainter.drawPoint(upRect.right(), upRect.top() + 1);
                    cachePainter.drawPoint(downRect.right() - 1, downRect.bottom());
                    cachePainter.drawPoint(downRect.right(), downRect.bottom() - 1);
                }

                // draw the line to the left of the buttons for shading against the
                // base of the line edit
                if (focus) {
                    cachePainter.setPen(option->palette.highlight().color().light(101));
                } else {
                    cachePainter.setPen(option->palette.button().color().light(101));
                }
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(upRect.right() + 1, upRect.top() + 2,
                                          upRect.right() + 1, downRect.bottom() - 2);
                } else {
                    cachePainter.drawLine(upRect.left() - 1, upRect.top() + 2,
                                          upRect.left() - 1, downRect.bottom() - 2);
                }

                // Button bevels
                if (hover && upIsActive && !sunken) {
                    cachePainter.setPen(highlightedDarkInnerBorderColor);
                } else {
                    if (sunken && upIsActive) {
                        cachePainter.setPen(option->palette.button().color().light(89));
                    } else {
                        cachePainter.setPen(gradientStartColor.light(105));
                    }
                }
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(upRect.right() - 1, upRect.top() + 1,
                                          upRect.left() + 2, upRect.top() + 1);
                    cachePainter.drawLine(upRect.left() + 1, upRect.top() + 2,
                                          upRect.left() + 1, upRect.bottom() - 1);
                } else {
                    cachePainter.drawLine(upRect.left() + 1, upRect.top() + 1,
                                          upRect.right() - 2, upRect.top() + 1);
                    cachePainter.drawLine(upRect.left() + 1, upRect.top() + 2,
                                          upRect.left() + 1, upRect.bottom() - 2);
                }

                if (hover && upIsActive && !sunken) {
                    cachePainter.setPen(highlightedDarkInnerBorderColor.dark(105));
                } else {
                    if (sunken && upIsActive) {
                        cachePainter.setPen(option->palette.button().color().light(96));
                    } else {
                        cachePainter.setPen(gradientStopColor.dark(105));
                    }
                }
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(upRect.right() - 1, upRect.bottom() - 1,
                                          upRect.left() + 2, upRect.bottom() - 1);
                    cachePainter.drawLine(upRect.right() - 1, upRect.top() + 2,
                                          upRect.right() - 1, upRect.bottom() - 2);
                } else {
                    cachePainter.drawLine(upRect.left() + 1, upRect.bottom() - 1,
                                          upRect.right() - 2, upRect.bottom() - 1);
                    cachePainter.drawLine(upRect.right() - 1, upRect.top() + 2,
                                          upRect.right() - 1, upRect.bottom() - 1);
                }

                if (hover && downIsActive && !sunken) {
                    cachePainter.setPen(highlightedDarkInnerBorderColor);
                } else {
                    if (sunken && downIsActive) {
                        cachePainter.setPen(option->palette.button().color().light(89));
                    } else {
                        cachePainter.setPen(gradientStartColor.light(105));
                    }
                }
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(downRect.right() - 1, downRect.top() + 1,
                                          downRect.left() + 1, downRect.top() + 1);
                    cachePainter.drawLine(downRect.left() + 1, downRect.top() + 2,
                                          downRect.left() + 1, downRect.bottom() - 2);
                } else {
                    cachePainter.drawLine(downRect.left() + 1, downRect.top() + 1,
                                          downRect.right() - 1, downRect.top() + 1);
                    cachePainter.drawLine(downRect.left() + 1, downRect.top() + 2,
                                          downRect.left() + 1, downRect.bottom() - 1);
                }

                if (hover && downIsActive && !sunken) {
                    cachePainter.setPen(highlightedDarkInnerBorderColor.dark(105));
                } else {
                    if (sunken && downIsActive) {
                        cachePainter.setPen(option->palette.button().color().light(96));
                    } else {
                        cachePainter.setPen(gradientStopColor.dark(105));
                    }
                }
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(downRect.right() - 1, downRect.bottom() - 1,
                                          downRect.left() + 2, downRect.bottom() - 1);
                    cachePainter.drawLine(downRect.right() - 1, downRect.top() + 2,
                                          downRect.right() - 1, downRect.bottom() - 1);
                } else {
                    cachePainter.drawLine(downRect.left() + 1, downRect.bottom() - 1,
                                          downRect.right() - 2, downRect.bottom() - 1);
                    cachePainter.drawLine(downRect.right() - 1, downRect.top() + 2,
                                          downRect.right() - 1, downRect.bottom() - 2);
                }

                if (hover) {
                    if (upIsActive && !sunken) {
                        cachePainter.setPen(highlightedLightInnerBorderColor);
                        if (spinBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(upRect.right() - 2, upRect.top() + 2,
                                                  upRect.left() + 2, upRect.top() + 2);
                            cachePainter.drawLine(upRect.right() - 2, upRect.top() + 3,
                                                  upRect.right() - 2, upRect.bottom() - 3);
                        } else {
                            cachePainter.drawLine(upRect.left() + 2, upRect.top() + 2,
                                                  upRect.right() - 2, upRect.top() + 2);
                            cachePainter.drawLine(upRect.left() + 2, upRect.top() + 3,
                                                  upRect.left() + 2, upRect.bottom() - 3);
                        }
                        cachePainter.setPen(highlightedLightInnerBorderColor.dark(105));
                        if (spinBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(upRect.right() - 2, upRect.bottom() - 2,
                                                  upRect.left() + 2, upRect.bottom() - 2);
                            cachePainter.drawLine(upRect.left() + 2, upRect.top() + 3,
                                                  upRect.left() + 2, upRect.bottom() - 3);
                        } else {
                            cachePainter.drawLine(upRect.left() + 2, upRect.bottom() - 2,
                                                  upRect.right() - 2, upRect.bottom() - 2);
                            cachePainter.drawLine(upRect.right() - 2, upRect.top() + 3,
                                                  upRect.right() - 2, upRect.bottom() - 3);
                        }

                    }
                    if (downIsActive && !sunken) {
                        cachePainter.setPen(highlightedLightInnerBorderColor);
                        if (spinBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downRect.right() - 2, downRect.top() + 2,
                                                  downRect.left() + 2, downRect.top() + 2);
                            cachePainter.drawLine(downRect.right() - 2, downRect.top() + 3,
                                                  downRect.right() - 2, downRect.bottom() - 3);
                        } else {
                            cachePainter.drawLine(downRect.left() + 2, downRect.top() + 2,
                                                  downRect.right() - 2, downRect.top() + 2);
                            cachePainter.drawLine(downRect.left() + 2, downRect.top() + 3,
                                                  downRect.left() + 2, downRect.bottom() - 3);
                        }

                        cachePainter.setPen(highlightedLightInnerBorderColor.dark(105));
                        if (spinBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downRect.right() - 2, downRect.bottom() - 2,
                                                  downRect.left() + 2, downRect.bottom() - 2);
                            cachePainter.drawLine(downRect.left() + 2, downRect.top() + 3,
                                                  downRect.left() + 2, downRect.bottom() - 3);
                        } else {
                            cachePainter.drawLine(downRect.left() + 2, downRect.bottom() - 2,
                                                  downRect.right() - 2, downRect.bottom() - 2);
                            cachePainter.drawLine(downRect.right() - 2, downRect.top() + 3,
                                                  downRect.right() - 2, downRect.bottom() - 3);
                        }
                    }
                }

                if (spinBox->buttonSymbols == QAbstractSpinBox::PlusMinus) {
                    int centerX = upRect.center().x();
                    int centerY = upRect.center().y();
                    cachePainter.setPen(spinBox->palette.foreground().color());

                    // plus/minus
                    if (spinBox->activeSubControls == SC_SpinBoxUp && sunken) {
                        cachePainter.drawLine(1 + centerX - 2, 1 + centerY, 1 + centerX + 2, 1 + centerY);
                        cachePainter.drawLine(1 + centerX, 1 + centerY - 2, 1 + centerX, 1 + centerY + 2);
                    } else {
                        cachePainter.drawLine(centerX - 2, centerY, centerX + 2, centerY);
                        cachePainter.drawLine(centerX, centerY - 2, centerX, centerY + 2);
                    }

                    centerX = downRect.center().x();
                    centerY = downRect.center().y();
                    if (spinBox->activeSubControls == SC_SpinBoxDown && sunken) {
                        cachePainter.drawLine(1 + centerX - 2, 1 + centerY, 1 + centerX + 2, 1 + centerY);
                    } else {
                        cachePainter.drawLine(centerX - 2, centerY, centerX + 2, centerY);
                    }
                } else {
                    // arrows
                    QImage upArrow(qt_scrollbar_button_arrow_up);
                    upArrow.setColor(1, spinBox->palette.foreground().color().rgba());
                    if (spinBox->activeSubControls == SC_SpinBoxUp && sunken) {
                        cachePainter.drawImage(1 + upRect.center().x() - upArrow.width() / 2,
                                               1 + upRect.center().y() - upArrow.height() / 2,
                                               upArrow);
                    } else {
                        cachePainter.drawImage(upRect.center().x() - upArrow.width() / 2,
                                               upRect.center().y() - upArrow.height() / 2,
                                               upArrow);
                    }
                    QImage downArrow(qt_scrollbar_button_arrow_down);
                    downArrow.setColor(1, spinBox->palette.foreground().color().rgba());
                    if (spinBox->activeSubControls == SC_SpinBoxDown && sunken) {
                        cachePainter.drawImage(1 + downRect.center().x() - downArrow.width() / 2,
                                               2 + downRect.center().y() - downArrow.height() / 2,
                                               downArrow);
                    } else {
                        cachePainter.drawImage(downRect.center().x() - downArrow.width() / 2,
                                               1 + downRect.center().y() - downArrow.height() / 2,
                                               downArrow);
                    }
                }
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(spinBox->rect.topLeft(), cache);
        }
        break;
#endif // QT_NO_SPINBOX
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            bool sunken = (comboBox->state & (State_Sunken | State_On));
            bool isEnabled = (comboBox->state & State_Enabled);
            bool focus = isEnabled && (comboBox->state & State_HasFocus);
            bool hover = isEnabled && (comboBox->state & State_MouseOver) && !sunken
                         && (comboBox->activeSubControls == SC_ComboBoxArrow);

            QPixmap cache;
            QString pixmapName = uniqueName("combobox", option, comboBox->rect.size());
            if (sunken)
                pixmapName += "-sunken";
            if (comboBox->editable)
                pixmapName += "-editable";
            if (isEnabled)
                pixmapName += "-enabled";

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(comboBox->rect.size());
                cache.fill(Qt::white);
                QPainter cachePainter(&cache);
                QRect pixmapRect(0, 0, comboBox->rect.width(), comboBox->rect.height());
                cachePainter.fillRect(pixmapRect, painter->brush());

                QStyleOptionComboBox comboBoxCopy = *comboBox;
                comboBoxCopy.rect = pixmapRect;

                QRect rect = pixmapRect;
                QRect downArrowRect = subControlRect(CC_ComboBox, &comboBoxCopy,
                                                     SC_ComboBoxArrow, widget);

                // Draw a push button
                if (comboBox->editable) {
                    // Draw a line edit
                    QStyleOptionFrame lineEdit;
                    lineEdit.rect = pixmapRect;
                    lineEdit.state = comboBox->state;
                    lineEdit.state |= State_Sunken;
                    drawPrimitive(PE_FrameLineEdit, &lineEdit, &cachePainter, widget);

                    // Top, bottom, left and right borderColor lines
                    cachePainter.setPen(borderColor);
                    if (comboBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.top(),
                                              downArrowRect.left() + 1, downArrowRect.top());
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.bottom(),
                                              downArrowRect.left() + 1, downArrowRect.bottom());
                        cachePainter.drawLine(rect.left(), rect.top() + 2,
                                              rect.left(), rect.bottom() - 2);
                        cachePainter.drawPoint(rect.left() + 1, rect.top() + 1);
                        cachePainter.drawPoint(rect.left() + 1, rect.bottom() - 1);
                        cachePainter.drawLine(downArrowRect.right() - 1, downArrowRect.top(),
                                              downArrowRect.right() - 1, downArrowRect.bottom());
                        cachePainter.setPen(alphaCornerColor);
                        cachePainter.drawPoint(rect.left() + 1, rect.top());
                        cachePainter.drawPoint(rect.left(), rect.top() + 1);
                        cachePainter.drawPoint(rect.left() + 1, rect.bottom());
                        cachePainter.drawPoint(rect.left(), rect.bottom() - 1);
                    } else {
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.top(),
                                              downArrowRect.right() - 1, downArrowRect.top());
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.bottom(),
                                              downArrowRect.right() - 1, downArrowRect.bottom());
                        cachePainter.drawLine(rect.right(), rect.top() + 2,
                                              rect.right(), rect.bottom() - 2);
                        cachePainter.drawPoint(rect.right() - 1, rect.top() + 1);
                        cachePainter.drawPoint(rect.right() - 1, rect.bottom() - 1);
                        cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.top(),
                                              downArrowRect.left() + 1, downArrowRect.bottom());
                        cachePainter.setPen(alphaCornerColor);
                        cachePainter.drawPoint(rect.right() - 1, rect.top());
                        cachePainter.drawPoint(rect.right(), rect.top() + 1);
                        cachePainter.drawPoint(rect.right() - 1, rect.bottom());
                        cachePainter.drawPoint(rect.right(), rect.bottom() - 1);
                    }

                    // Bevel
                    if (hover) {
                        cachePainter.setPen(highlightedDarkInnerBorderColor);
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() - 2, downArrowRect.top() + 1,
                                                  downArrowRect.left() + 1, downArrowRect.top() + 1);
                            cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 2,
                                                  downArrowRect.left(), downArrowRect.bottom() - 2);
                        } else {
                            cachePainter.drawLine(downArrowRect.left() + 2, downArrowRect.top() + 1,
                                                  downArrowRect.right() - 1, downArrowRect.top() + 1);
                            cachePainter.drawLine(downArrowRect.left() + 2, downArrowRect.top() + 2,
                                                  downArrowRect.left() + 2, downArrowRect.bottom() - 2);
                        }
                        cachePainter.setPen(highlightedDarkInnerBorderColor.dark(105));
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() - 2, downArrowRect.top() + 2,
                                                  downArrowRect.right() - 2, downArrowRect.bottom() - 1);
                            cachePainter.drawLine(downArrowRect.right() - 2, downArrowRect.bottom() - 1,
                                                  downArrowRect.left() + 1, downArrowRect.bottom() - 1);
                        } else {
                            cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 2,
                                                  downArrowRect.right(), downArrowRect.bottom() - 2);
                            cachePainter.drawLine(downArrowRect.left() + 2, downArrowRect.bottom() - 1,
                                                  downArrowRect.right() - 1, downArrowRect.bottom() - 1);
                        }

                        cachePainter.setPen(highlightedLightInnerBorderColor);
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() - 3, downArrowRect.top() + 2,
                                                  downArrowRect.left() + 2, downArrowRect.top() + 2);
                            cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.top() + 2,
                                                  downArrowRect.left() + 1, downArrowRect.bottom() - 2);
                        } else {
                            cachePainter.drawLine(downArrowRect.left() + 3, downArrowRect.top() + 2,
                                                  downArrowRect.right() - 2, downArrowRect.top() + 2);
                            cachePainter.drawLine(downArrowRect.left() + 3, downArrowRect.top() + 3,
                                                  downArrowRect.left() + 3, downArrowRect.bottom() - 2);
                        }
                        cachePainter.setPen(highlightedLightInnerBorderColor.dark(105));
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() - 3, downArrowRect.top() + 3,
                                                  downArrowRect.right() - 3, downArrowRect.bottom() - 2);
                            cachePainter.drawLine(downArrowRect.right() - 3, downArrowRect.bottom() - 2,
                                                  downArrowRect.left() + 2, downArrowRect.bottom() - 2);
                        } else {
                            cachePainter.drawLine(downArrowRect.left() + 3, downArrowRect.bottom() - 2,
                                                  downArrowRect.right() - 1, downArrowRect.bottom() - 2);
                            cachePainter.drawLine(downArrowRect.right() - 1, downArrowRect.top() + 2,
                                                  downArrowRect.right() - 1, downArrowRect.bottom() - 3);
                        }
                    } else {
                        if (sunken) {
                            cachePainter.setPen(option->palette.button().color().light(89));
                        } else {
                            cachePainter.setPen(gradientStartColor.light(105));
                        }
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() - 2, downArrowRect.top() + 1,
                                                  downArrowRect.left() + 1, downArrowRect.top() + 1);
                            cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 2,
                                                  downArrowRect.left(), downArrowRect.bottom() - 2);
                        } else {
                            cachePainter.drawLine(downArrowRect.left() + 2, downArrowRect.top() + 1,
                                                  downArrowRect.right() - 1, downArrowRect.top() + 1);
                            cachePainter.drawLine(downArrowRect.left() + 2, downArrowRect.top() + 2,
                                                  downArrowRect.left() + 2, downArrowRect.bottom() - 2);
                        }

                        if (sunken) {
                            cachePainter.setPen(option->palette.button().color().light(96));
                        } else {
                            cachePainter.setPen(gradientStopColor.dark(105));
                        }
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() - 2, downArrowRect.bottom() - 1,
                                                  downArrowRect.left() + 1, downArrowRect.bottom() - 1);
                            cachePainter.drawLine(downArrowRect.right() - 2, downArrowRect.top() + 2,
                                                  downArrowRect.right() - 2, downArrowRect.bottom() - 2);
                        } else {
                            cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 2,
                                                  downArrowRect.right(), downArrowRect.bottom() - 2);
                            cachePainter.drawLine(downArrowRect.left() + 2, downArrowRect.bottom() - 1,
                                                  downArrowRect.right() - 1, downArrowRect.bottom() - 1);
                        }
                    }

                    // The line to the left of the button, shades against the base
                    // color of the line edit.
                    if (focus) {
                        cachePainter.setPen(option->palette.highlight().color().light(101));
                    } else {
                        cachePainter.setPen(option->palette.button().color().light(101));
                    }
                    if (comboBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 2,
                                              downArrowRect.right(), downArrowRect.bottom() - 2);
                    } else {
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 2,
                                              downArrowRect.left(), downArrowRect.bottom() - 2);
                    }

                    QRect downArrowGradientRect;
                    if (comboBox->direction == Qt::RightToLeft) {
                        downArrowGradientRect = downArrowRect.adjusted(1, 2, -3, -2);
                        if (hover)
                            downArrowGradientRect = downArrowGradientRect.adjusted(1, 1, -1, -1);
                    } else {
                        downArrowGradientRect = downArrowRect.adjusted(3, 2, -1, -2);
                        if (hover)
                            downArrowGradientRect = downArrowGradientRect.adjusted(1, 1, -1, -1);
                    }

                    // The button fill
                    if (isEnabled) {
                        if (sunken) {
                            cachePainter.fillRect(downArrowGradientRect, gradientStopColor);
                        } else {
                            if (focus || hover) {
                                qt_plastique_draw_gradient(&cachePainter, downArrowGradientRect,
                                                           highlightedGradientStartColor,
                                                           highlightedGradientStopColor);
                            } else {
                                qt_plastique_draw_gradient(&cachePainter, downArrowGradientRect,
                                                           gradientStartColor,
                                                           gradientStopColor);
                            }
                        }
                    } else {
                        cachePainter.fillRect(downArrowGradientRect, option->palette.background());
                    }
                } else {
                    QStyleOptionButton buttonOption;
                    buttonOption.QStyleOption::operator=(*comboBox);
                    buttonOption.rect = rect.adjusted(-1, -1, 1, 1);
                    buttonOption.state = comboBox->state & (State_Enabled | State_MouseOver);
                    if (sunken) {
                        buttonOption.state |= State_Sunken;
                        buttonOption.state &= ~State_MouseOver;
                    }
                    drawPrimitive(PE_PanelButtonCommand, &buttonOption, &cachePainter, widget);

                    cachePainter.setPen(borderColor);
                    if (!sunken) {
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.topRight(), downArrowRect.bottomRight());
                        } else {
                            cachePainter.drawLine(downArrowRect.topLeft(), downArrowRect.bottomLeft());
                        }
                    } else {
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(downArrowRect.right() + 1, downArrowRect.top(),
                                                  downArrowRect.right() + 1, downArrowRect.bottom());
                        } else {
                            cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.top(),
                                                  downArrowRect.left() + 1, downArrowRect.bottom());
                        }
                    }
                }

                // Draw the little arrow
                QImage downArrow(qt_scrollbar_button_arrow_down);
                downArrow.setColor(1, comboBox->palette.foreground().color().rgba());
                if (sunken)
                    downArrowRect = downArrowRect.adjusted(1, 1, 1, 1);

                if (comboBox->direction == Qt::RightToLeft) {
                    cachePainter.drawImage(downArrowRect.center().x() - downArrow.width() / 2 - 1,
                                           downArrowRect.center().y() - downArrow.height() / 2 + 1, downArrow);
                } else {
                    cachePainter.drawImage(downArrowRect.center().x() - downArrow.width() / 2 + 1,
                                           downArrowRect.center().y() - downArrow.height() / 2 + 1, downArrow);
                }

                // Draw the focus rect
                if ((focus && (option->state & State_KeyboardFocusChange)) && !comboBox->editable) {
                    QStyleOptionFocusRect focus;
                    focus.rect = subControlRect(CC_ComboBox, &comboBoxCopy, SC_ComboBoxEditField, widget)
                                 .adjusted(0, 0, option->direction == Qt::RightToLeft ? 1 : -1, 0);
                    drawPrimitive(PE_FrameFocusRect, &focus, &cachePainter, widget);
                }
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(comboBox->rect.topLeft(), cache);
        }
        break;
#endif // QT_NO_COMBOBOX
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
#ifdef QT3_SUPPORT
            if (widget && widget->inherits("Q3DockWindowTitleBar")) {
                // Redirect Q3DockWindow to CE_DockWidgetTitle
                QStyleOptionDockWidget dockOption;
                dockOption.QStyleOption::operator=(*titleBar);
                // Fixup palette distortion
                dockOption.palette = QPalette();
                drawControl(CE_DockWidgetTitle, &dockOption, painter, widget);
                break;
            }
#endif
            painter->save();
            bool active = (titleBar->titleBarState & State_Active);
            QRect fullRect = titleBar->rect;

            // ### use palette colors instead
            QColor titleBarGradientStart(active ? 0x3b508a : 0x6e6e6e);
            QColor titleBarGradientStop(active ? 0x5d6e9e : 0x818181);
            QColor titleBarFrameBorder(0x393939);
            QColor titleBarAlphaCorner(active ? 0x4b5e7f : 0x6a6a6a);
            QColor titleBarInnerTopLine(active ? 0x8e98ba : 0xa4a4a4);
            QColor titleBarInnerInnerTopLine(active ? 0x57699b : 0x808080);
            QColor leftCorner(active ? 0x6f7ea8 : 0x8e8e8e);
            QColor rightCorner(active ? 0x44537d : 0x676767);
            QColor textColor(active ? 0x282e40 : 0x282e40);
            QColor textAlphaColor(active ? 0x3f4862 : 0x3f4862);

            // Fill titlebar gradient
            qt_plastique_draw_gradient(painter, option->rect.adjusted(1, 1, -1, 0),
                                       titleBarGradientStart,
                                       titleBarGradientStop);

            // Frame and rounded corners
            painter->setPen(titleBarFrameBorder);

            // top border line
            painter->drawLine(fullRect.left() + 2, fullRect.top(), fullRect.right() - 2, fullRect.top());
            painter->drawLine(fullRect.left(), fullRect.top() + 2, fullRect.left(), fullRect.bottom());
            painter->drawLine(fullRect.right(), fullRect.top() + 2, fullRect.right(), fullRect.bottom());
            painter->drawPoint(fullRect.left() + 1, fullRect.top() + 1);
            painter->drawPoint(fullRect.right() - 1, fullRect.top() + 1);

            // alpha corners
            painter->setPen(titleBarAlphaCorner);
            painter->drawPoint(fullRect.left() + 2, fullRect.top() + 1);
            painter->drawPoint(fullRect.left() + 1, fullRect.top() + 2);
            painter->drawPoint(fullRect.right() - 2, fullRect.top() + 1);
            painter->drawPoint(fullRect.right() - 1, fullRect.top() + 2);

            // inner top line
            painter->setPen(titleBarInnerTopLine);
            painter->drawLine(fullRect.left() + 3, fullRect.top() + 1, fullRect.right() - 3, fullRect.top() + 1);

            // inner inner top line
            painter->setPen(titleBarInnerInnerTopLine);
            painter->drawLine(fullRect.left() + 2, fullRect.top() + 2, fullRect.right() - 2, fullRect.top() + 2);

            // left and right inner
            painter->setPen(leftCorner);
            painter->drawLine(fullRect.left() + 1, fullRect.top() + 3, fullRect.left() + 1, fullRect.bottom());
            painter->setPen(rightCorner);
            painter->drawLine(fullRect.right() - 1, fullRect.top() + 3, fullRect.right() - 1, fullRect.bottom());

            if (titleBar->titleBarState & Qt::WindowMinimized) {
                painter->setPen(titleBarFrameBorder);
                painter->drawLine(fullRect.left() + 2, fullRect.bottom(), fullRect.right() - 2, fullRect.bottom());
                painter->drawPoint(fullRect.left() + 1, fullRect.bottom() - 1);
                painter->drawPoint(fullRect.right() - 1, fullRect.bottom() - 1);
                painter->setPen(rightCorner);
                painter->drawLine(fullRect.left() + 2, fullRect.bottom() - 1, fullRect.right() - 2, fullRect.bottom() - 1);
                painter->setPen(titleBarAlphaCorner);
                painter->drawPoint(fullRect.left() + 1, fullRect.bottom() - 2);
                painter->drawPoint(fullRect.left() + 2, fullRect.bottom() - 1);
                painter->drawPoint(fullRect.right() - 1, fullRect.bottom() - 2);
                painter->drawPoint(fullRect.right() - 2, fullRect.bottom() - 1);
            }

            // draw title
            QRect textRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarLabel, widget);

            QFont font = painter->font();
            font.setBold(true);
            painter->setFont(font);
            painter->setPen(titleBar->palette.text().color());

            // Attempt to align left if there is not enough room for the title
            // text. Otherwise, align center. QWorkspace does elliding for us,
            // and it doesn't know about the bold title, so we need to work
            // around some of the width mismatches.
            bool tooWide = (QFontMetrics(font).width(titleBar->text) > textRect.width());
            QTextOption option((tooWide ? Qt::AlignLeft : Qt::AlignHCenter) | Qt::AlignVCenter);
            option.setWrapMode(QTextOption::NoWrap);

            painter->drawText(textRect.adjusted(1, 1, 1, 1), titleBar->text, option);
            painter->setPen(titleBar->palette.highlightedText().color());
            painter->drawText(textRect, titleBar->text, option);

            // min button
            if ((titleBar->subControls & SC_TitleBarMinButton) && (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint)) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarMinButton) && (titleBar->state & State_MouseOver);
                QRect minButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarMinButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, minButtonRect, hover);

                int xoffset = minButtonRect.width() / 3;
                int yoffset = minButtonRect.height() / 3;

                QRect minButtonIconRect(minButtonRect.left() + xoffset, minButtonRect.top() + yoffset,
                                        minButtonRect.width() - xoffset * 2, minButtonRect.height() - yoffset * 2);

                painter->setPen(textColor);
                painter->drawLine(minButtonIconRect.center().x() - 2, minButtonIconRect.center().y() + 3,
                                  minButtonIconRect.center().x() + 3, minButtonIconRect.center().y() + 3);
                painter->drawLine(minButtonIconRect.center().x() - 2, minButtonIconRect.center().y() + 4,
                                  minButtonIconRect.center().x() + 3, minButtonIconRect.center().y() + 4);
                painter->setPen(textAlphaColor);
                painter->drawLine(minButtonIconRect.center().x() - 3, minButtonIconRect.center().y() + 3,
                                  minButtonIconRect.center().x() - 3, minButtonIconRect.center().y() + 4);
                painter->drawLine(minButtonIconRect.center().x() + 4, minButtonIconRect.center().y() + 3,
                                  minButtonIconRect.center().x() + 4, minButtonIconRect.center().y() + 4);
            }

            // max button
            if ((titleBar->subControls & SC_TitleBarMaxButton) && (titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint)) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarMaxButton) && (titleBar->state & State_MouseOver);
                QRect maxButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarMaxButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, maxButtonRect, hover);

                int xoffset = maxButtonRect.width() / 3;
                int yoffset = maxButtonRect.height() / 3;

                QRect maxButtonIconRect(maxButtonRect.left() + xoffset, maxButtonRect.top() + yoffset,
                                        maxButtonRect.width() - xoffset * 2, maxButtonRect.height() - yoffset * 2);

                painter->setPen(textColor);
                painter->drawRect(maxButtonIconRect.adjusted(0, 0, -1, -1));
                painter->drawLine(maxButtonIconRect.left() + 1, maxButtonIconRect.top() + 1,
                                  maxButtonIconRect.right() - 1, maxButtonIconRect.top() + 1);
                painter->setPen(textAlphaColor);
                painter->drawPoint(maxButtonIconRect.topLeft());
                painter->drawPoint(maxButtonIconRect.topRight());
                painter->drawPoint(maxButtonIconRect.bottomLeft());
                painter->drawPoint(maxButtonIconRect.bottomRight());
            }

            // close button
            if (titleBar->subControls & SC_TitleBarCloseButton) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarCloseButton) && (titleBar->state & State_MouseOver);
                QRect closeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarCloseButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, closeButtonRect, hover);

                int xoffset = closeButtonRect.width() / 3;
                int yoffset = closeButtonRect.height() / 3;

                QRect closeIconRect(closeButtonRect.left() + xoffset, closeButtonRect.top() + yoffset,
                                    closeButtonRect.width() - xoffset * 2, closeButtonRect.height() - yoffset * 2);

                painter->setPen(textAlphaColor);
                painter->drawLine(closeIconRect.left() + 1, closeIconRect.top(),
                                  closeIconRect.right(), closeIconRect.bottom() - 1);
                painter->drawLine(closeIconRect.left(), closeIconRect.top() + 1,
                                  closeIconRect.right() - 1, closeIconRect.bottom());
                painter->drawLine(closeIconRect.right() - 1, closeIconRect.top(),
                                  closeIconRect.left(), closeIconRect.bottom() - 1);
                painter->drawLine(closeIconRect.right(), closeIconRect.top() + 1,
                                  closeIconRect.left() + 1, closeIconRect.bottom());
                painter->drawPoint(closeIconRect.topLeft());
                painter->drawPoint(closeIconRect.topRight());
                painter->drawPoint(closeIconRect.bottomLeft());
                painter->drawPoint(closeIconRect.bottomRight());

                painter->setPen(textColor);
                painter->drawLine(closeIconRect.left() + 1, closeIconRect.top() + 1,
                                  closeIconRect.right() - 1, closeIconRect.bottom() - 1);
                painter->drawLine(closeIconRect.left() + 1, closeIconRect.bottom() - 1,
                                  closeIconRect.right() - 1, closeIconRect.top() + 1);
            }

            // normalize button
            if ((titleBar->subControls & SC_TitleBarNormalButton) &&
                (((titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint) &&
                 (titleBar->titleBarState & Qt::WindowMinimized)) ||
                 ((titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint) &&
                  (titleBar->titleBarState & Qt::WindowMaximized)))) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarNormalButton) && (titleBar->state & State_MouseOver);
                QRect normalButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarNormalButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, normalButtonRect, hover);
                int xoffset = int(normalButtonRect.width() / 3.5);
                int yoffset = int(normalButtonRect.height() / 3.5);

                QRect normalButtonIconRect(normalButtonRect.left() + xoffset, normalButtonRect.top() + yoffset,
                                           normalButtonRect.width() - xoffset * 2, normalButtonRect.height() - yoffset * 2);

                QRect frontWindowRect = normalButtonIconRect.adjusted(0, 3, -3, 0);
                painter->setPen(textColor);
                painter->drawRect(frontWindowRect.adjusted(0, 0, -1, -1));
                painter->drawLine(frontWindowRect.left() + 1, frontWindowRect.top() + 1,
                                  frontWindowRect.right() - 1, frontWindowRect.top() + 1);
                painter->setPen(textAlphaColor);
                painter->drawPoint(frontWindowRect.topLeft());
                painter->drawPoint(frontWindowRect.topRight());
                painter->drawPoint(frontWindowRect.bottomLeft());
                painter->drawPoint(frontWindowRect.bottomRight());

                QRect backWindowRect = normalButtonIconRect.adjusted(3, 0, 0, -3);
                QRegion clipRegion = backWindowRect;
                clipRegion -= frontWindowRect;
                painter->save();
                painter->setClipRegion(clipRegion);
                painter->setPen(textColor);
                painter->drawRect(backWindowRect.adjusted(0, 0, -1, -1));
                painter->drawLine(backWindowRect.left() + 1, backWindowRect.top() + 1,
                                  backWindowRect.right() - 1, backWindowRect.top() + 1);
                painter->setPen(textAlphaColor);
                painter->drawPoint(backWindowRect.topLeft());
                painter->drawPoint(backWindowRect.topRight());
                painter->drawPoint(backWindowRect.bottomLeft());
                painter->drawPoint(backWindowRect.bottomRight());
                painter->restore();
            }

            // context help button
            if (titleBar->subControls & SC_TitleBarContextHelpButton
                && (titleBar->titleBarFlags & Qt::WindowContextHelpButtonHint)) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_MouseOver);
                QRect contextHelpButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarContextHelpButton, widget);

                qt_plastique_draw_mdibutton(painter, titleBar, contextHelpButtonRect, hover);

                QColor blend;
                // ### Use palette colors
                if (active) {
                    blend = mergedColors(QColor(hover ? 0x7d8bb1 : 0x55689a),
                                         QColor(hover ? 0x939ebe : 0x7381ab));
                } else {
                    blend = mergedColors(QColor(hover ? 0x9e9e9e : 0x818181),
                                         QColor(hover ? 0xababab : 0x929292));
                }
                QImage image(qt_titlebar_context_help);
                image.setColor(4, textColor.rgba());
                image.setColor(3, mergedColors(blend, textColor, 30).rgba());
                image.setColor(2, mergedColors(blend, textColor, 70).rgba());
                image.setColor(1, mergedColors(blend, textColor, 90).rgba());

                painter->drawImage(contextHelpButtonRect, image);
            }

            // shade button
            if (titleBar->subControls & SC_TitleBarShadeButton) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarShadeButton) && (titleBar->state & State_MouseOver);
                QRect shadeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarShadeButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, shadeButtonRect, hover);

                int xoffset = shadeButtonRect.width() / 3;
                int yoffset = shadeButtonRect.height() / 3;

                QRect shadeButtonIconRect(shadeButtonRect.left() + xoffset, shadeButtonRect.top() + yoffset,
                                          shadeButtonRect.width() - xoffset * 2, shadeButtonRect.height() - yoffset * 2);

                QPainterPath path(shadeButtonIconRect.bottomLeft());
                path.lineTo(shadeButtonIconRect.center().x(), shadeButtonIconRect.bottom() - shadeButtonIconRect.height() / 2);
                path.lineTo(shadeButtonIconRect.bottomRight());
                path.lineTo(shadeButtonIconRect.bottomLeft());

                painter->setPen(textAlphaColor);
                painter->setBrush(textColor);
                painter->drawPath(path);
            }

            // unshade button
            if (titleBar->subControls & SC_TitleBarUnshadeButton) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_MouseOver);
                QRect unshadeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarUnshadeButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, unshadeButtonRect, hover);

                int xoffset = unshadeButtonRect.width() / 3;
                int yoffset = unshadeButtonRect.height() / 3;

                QRect unshadeButtonIconRect(unshadeButtonRect.left() + xoffset, unshadeButtonRect.top() + yoffset,
                                          unshadeButtonRect.width() - xoffset * 2, unshadeButtonRect.height() - yoffset * 2);

                int midY = unshadeButtonIconRect.bottom() - unshadeButtonIconRect.height() / 2;
                QPainterPath path(QPoint(unshadeButtonIconRect.left(), midY));
                path.lineTo(unshadeButtonIconRect.right(), midY);
                path.lineTo(unshadeButtonIconRect.center().x(), unshadeButtonIconRect.bottom());
                path.lineTo(unshadeButtonIconRect.left(), midY);

                painter->setPen(textAlphaColor);
                painter->setBrush(textColor);
                painter->drawPath(path);
            }

            // from qwindowsstyle.cpp
            if ((titleBar->subControls & SC_TitleBarSysMenu) && (titleBar->titleBarFlags & Qt::WindowSystemMenuHint)) {

                bool hover = (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_MouseOver);
                QRect iconRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarSysMenu, widget);
                if (hover)
                    qt_plastique_draw_mdibutton(painter, titleBar, iconRect, hover);

                if (!titleBar->icon.isNull()) {
                    titleBar->icon.paint(painter, iconRect);
                } else {
                    QStyleOption tool(0);
                    tool.palette = titleBar->palette;
                    QPixmap pm = standardPixmap(SP_TitleBarMenuButton, &tool, widget);
                    tool.rect = iconRect;
                    painter->save();
                    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pm);
                    painter->restore();
                }
            }
            painter->restore();
        }
        break;
    default:
        QWindowsStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

/*!
  \reimp
*/
QSize QPlastiqueStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                        const QSize &size, const QWidget *widget) const
{
    QSize newSize = QWindowsStyle::sizeFromContents(type, option, size, widget);

    switch (type) {
    case CT_PushButton:
        newSize.rwidth() += 10;
        newSize += QSize(2, 2); //ensure room for default rect
        break;
    case CT_RadioButton:
        ++newSize.rheight();
        ++newSize.rwidth();
        break;
#ifndef QT_NO_SLIDER
    case CT_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int tickSize = pixelMetric(PM_SliderTickmarkOffset, option, widget);
            if (slider->tickPosition & QSlider::TicksBelow) {
                if (slider->orientation == Qt::Horizontal)
                    newSize.rheight() += tickSize;
                else
                    newSize.rwidth() += tickSize;
            }
            if (slider->tickPosition & QSlider::TicksAbove) {
                if (slider->orientation == Qt::Horizontal)
                    newSize.rheight() += tickSize;
                else
                    newSize.rwidth() += tickSize;
            }
        }
        break;
#endif // QT_NO_SLIDER
#ifndef QT_NO_SCROLLBAR
    case CT_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int scrollBarExtent = pixelMetric(PM_ScrollBarExtent, option, widget);
            int scrollBarSliderMinimum = pixelMetric(PM_ScrollBarSliderMin, option, widget);
            if (scrollBar->orientation == Qt::Horizontal) {
                newSize = QSize(16 * 3 + scrollBarSliderMinimum, scrollBarExtent);
            } else {
                newSize = QSize(scrollBarExtent, 16 * 3 + scrollBarSliderMinimum);
            }
        }
        break;
#endif // QT_NO_SCROLLBAR
#ifndef QT_NO_SPINBOX
    case CT_SpinBox:
        // Make sure the size is odd
        newSize.setHeight(sizeFromContents(CT_LineEdit, option, size, widget).height());
        newSize.rheight() -= (1 - newSize.rheight() & 1);
        break;
#endif
#ifndef QT_NO_TOOLBUTTON
    case CT_ToolButton:
        newSize.rheight() += 3;
        newSize.rwidth() += 3;
        break;
#endif
#ifndef QT_NO_COMBOBOX
    case CT_ComboBox:
        ++newSize.rheight();
        break;
#endif
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator)
                newSize.setHeight(menuItem->text.isEmpty() ? 2 : menuItem->fontMetrics.lineSpacing());
        }
        break;
    case CT_MenuBarItem:
        newSize.setHeight(newSize.height() + 2);
        break;
    default:
        break;
    }

    return newSize;
}

/*!
  \reimp
*/
QRect QPlastiqueStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect;
    switch (element) {
    case SE_RadioButtonIndicator:
        rect = visualRect(option->direction, option->rect,
                          QWindowsStyle::subElementRect(element, option, widget)).adjusted(0, 0, 1, 1);
        break;
#ifndef QT_NO_PROGRESSBAR
    case SE_ProgressBarLabel:
    case SE_ProgressBarContents:
    case SE_ProgressBarGroove:
        return option->rect;
#endif // QT_NO_PROGRESSBAR
    default:
        rect = visualRect(option->direction, option->rect,
                          QWindowsStyle::subElementRect(element, option, widget));
        break;
    }

    return visualRect(option->direction, option->rect, rect);
}

/*!
  \reimp
*/
QRect QPlastiqueStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                      SubControl subControl, const QWidget *widget) const
{
    QRect rect = QWindowsStyle::subControlRect(control, option, subControl, widget);

    switch (control) {
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int tickSize = pixelMetric(PM_SliderTickmarkOffset, option, widget);

            switch (subControl) {
            case SC_SliderHandle:
                if (slider->orientation == Qt::Horizontal) {
                    rect.setWidth(11);
                    rect.setHeight(15);
                    int centerY = slider->rect.center().y() - rect.height() / 2;
                    if (slider->tickPosition & QSlider::TicksAbove)
                        centerY += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        centerY -= tickSize;
                    rect.moveTop(centerY);
                } else {
                    rect.setWidth(15);
                    rect.setHeight(11);
                    int centerX = slider->rect.center().x() - rect.width() / 2;
                    if (slider->tickPosition & QSlider::TicksAbove)
                        centerX += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        centerX -= tickSize;
                    rect.moveLeft(centerX);
                }
                break;
            case SC_SliderGroove: {
                QPoint grooveCenter = slider->rect.center();
                if (slider->orientation == Qt::Horizontal) {
                    rect.setHeight(14);
                    --grooveCenter.ry();
                    if (slider->tickPosition & QSlider::TicksAbove)
                        grooveCenter.ry() += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        grooveCenter.ry() -= tickSize;
                } else {
                    rect.setWidth(14);
                    --grooveCenter.rx();
                    if (slider->tickPosition & QSlider::TicksAbove)
                        grooveCenter.rx() += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        grooveCenter.rx() -= tickSize;
                }
                rect.moveCenter(grooveCenter);
                break;
            }
            default:
                break;
            }
        }
        break;
#endif // QT_NO_SLIDER
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int scrollBarExtent = pixelMetric(PM_ScrollBarExtent, scrollBar, widget);
            int sliderMaxLength = ((scrollBar->orientation == Qt::Horizontal) ?
                                   scrollBar->rect.width() : scrollBar->rect.height()) - (16 * 3);
            int sliderMinLength = pixelMetric(PM_ScrollBarSliderMin, scrollBar, widget);
            int sliderLength;

            // calculate slider length
            if (scrollBar->maximum != scrollBar->minimum) {
                uint valueRange = scrollBar->maximum - scrollBar->minimum;
                sliderLength = (scrollBar->pageStep * sliderMaxLength) / (valueRange + scrollBar->pageStep);

                if (sliderLength < sliderMinLength || valueRange > INT_MAX / 2)
                    sliderLength = sliderMinLength;
                if (sliderLength > sliderMaxLength)
                    sliderLength = sliderMaxLength;
            } else {
                sliderLength = sliderMaxLength;
            }

            int sliderStart = 16 + sliderPositionFromValue(scrollBar->minimum,
                                                           scrollBar->maximum,
                                                           scrollBar->sliderPosition,
                                                           sliderMaxLength - sliderLength,
                                                           scrollBar->upsideDown);

            QRect scrollBarRect = scrollBar->rect;

            switch (subControl) {
            case SC_ScrollBarSubLine: // top/left button
                if (scrollBar->orientation == Qt::Horizontal) {
                    rect.setRect(scrollBarRect.left(), scrollBarRect.top(), scrollBarRect.width() - 16, scrollBarExtent);
                } else {
                    rect.setRect(scrollBarRect.left(), scrollBarRect.top(), scrollBarExtent, scrollBarRect.height() - 16);
                }
                break;
            case SC_ScrollBarAddLine: // bottom/right button
                if (scrollBar->orientation == Qt::Horizontal) {
                    rect.setRect(scrollBarRect.right() - 15, scrollBarRect.top(), 16, scrollBarExtent);
                } else {
                    rect.setRect(scrollBarRect.left(), scrollBarRect.bottom() - 15, scrollBarExtent, 16);
                }
                break;
            case SC_ScrollBarSubPage:
                if (scrollBar->orientation == Qt::Horizontal) {
                    rect.setRect(scrollBarRect.left() + 16, scrollBarRect.top(),
                                 sliderStart - (scrollBarRect.left() + 16), scrollBarExtent);
                } else {
                    rect.setRect(scrollBarRect.left(), scrollBarRect.top() + 16,
                                 scrollBarExtent, sliderStart - (scrollBarRect.left() + 16));
                }
                break;
            case SC_ScrollBarAddPage:
                if (scrollBar->orientation == Qt::Horizontal)
                    rect.setRect(sliderStart + sliderLength, 0,
                                 sliderMaxLength - sliderStart - sliderLength + 16, scrollBarExtent);
                else
                    rect.setRect(0, sliderStart + sliderLength,
                                 scrollBarExtent, sliderMaxLength - sliderStart - sliderLength + 16);
                break;
            case SC_ScrollBarGroove:
                if (scrollBar->orientation == Qt::Horizontal) {
                    rect = scrollBarRect.adjusted(16, 0, -32, 0);
                } else {
                    rect = scrollBarRect.adjusted(0, 16, 0, -32);
                }
                break;
            case SC_ScrollBarSlider:
                if (scrollBar->orientation == Qt::Horizontal) {
                    rect.setRect(sliderStart, 0, sliderLength, scrollBarExtent);
                } else {
                    rect.setRect(0, sliderStart, scrollBarExtent, sliderLength);
                }
                break;
            default:
                break;
            }
            rect = visualRect(scrollBar->direction, scrollBarRect, rect);
        }
        break;
#endif // QT_NO_SCROLLBAR
#ifndef QT_NO_SPINBOX
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            int center = spinBox->rect.height() / 2;
            switch (subControl) {
            case SC_SpinBoxUp:
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
                rect.setRect(spinBox->rect.right() - 16, spinBox->rect.top(), 17, center + 1);
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
                break;
            case SC_SpinBoxDown:
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
                rect.setRect(spinBox->rect.right() - 16, center, 17, spinBox->rect.height() - center);
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
                break;
            case SC_SpinBoxEditField: {
                int frameWidth = pixelMetric(PM_DefaultFrameWidth);
                rect = spinBox->rect.adjusted(frameWidth, frameWidth, -frameWidth - 16, -frameWidth);
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
            }
            default:
                break;
            }
        }
        break;
#endif // QT_NO_SPINBOX
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        switch (subControl) {
        case SC_ComboBoxArrow:
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(rect.right() - 15, rect.top() - 2,
                         17, rect.height() + 4);
            rect = visualRect(option->direction, option->rect, rect);
            break;
        case SC_ComboBoxEditField: {
            int frameWidth = pixelMetric(PM_DefaultFrameWidth);
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(option->rect.left() + frameWidth, option->rect.top() + frameWidth,
                         option->rect.width() - 16 - 2 * frameWidth,
                         option->rect.height() - 2 * frameWidth);
            if (const QStyleOptionComboBox *box = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
                if (!box->editable) {
                    rect.setLeft(rect.left() + 2);
                    rect.setRight(rect.right() - 2);
                    if (box->state & (State_Sunken | State_On))
                        rect.translate(1, 1);
                }
            }
            rect = visualRect(option->direction, option->rect, rect);
            break;
        }
        default:
            break;
        }
        break;
#endif // QT_NO_COMBOBOX
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            SubControl sc = subControl;
            QRect &ret = rect;
            const int indent = 3;
            const int controlTopMargin = 4;
            const int controlBottomMargin = 3;
            const int controlWidthMargin = 1;
            const int controlHeight = tb->rect.height() - controlTopMargin - controlBottomMargin;
            const int delta = controlHeight + controlWidthMargin;
            int offset = 0;

            bool isMinimized = tb->titleBarState & Qt::WindowMinimized;
            bool isMaximized = tb->titleBarState & Qt::WindowMaximized;

            switch (sc) {
            case SC_TitleBarLabel:
                if (tb->titleBarFlags & (Qt::WindowTitleHint | Qt::WindowSystemMenuHint)) {
                    ret = tb->rect;
                    if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                        ret.adjust(delta, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowShadeButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    ret.adjust(indent, 0, -indent, 0);
                }
                break;
            case SC_TitleBarContextHelpButton:
                if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                    offset += delta;
            case SC_TitleBarMinButton:
                if (!isMinimized && (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarMinButton)
                    break;
            case SC_TitleBarNormalButton:
                if (isMinimized && (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
                    offset += delta;
                else if (isMaximized && (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarNormalButton)
                    break;
            case SC_TitleBarMaxButton:
                if (!isMaximized && (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarMaxButton)
                    break;
            case SC_TitleBarShadeButton:
                if (!isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarShadeButton)
                    break;
            case SC_TitleBarUnshadeButton:
                if (isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarUnshadeButton)
                    break;
            case SC_TitleBarCloseButton:
                if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                    offset += delta;
                else if (sc == SC_TitleBarCloseButton)
                    break;
                ret.setRect(tb->rect.right() - indent - offset, tb->rect.top() + controlTopMargin,
                            controlHeight, controlHeight);
                break;
            case SC_TitleBarSysMenu:
                if (tb->titleBarFlags & Qt::WindowSystemMenuHint) {
                    ret.setRect(tb->rect.left() + controlWidthMargin + indent, tb->rect.top() + controlTopMargin,
                                controlHeight, controlHeight);
                }
                break;
            default:
                break;
            }
            ret = visualRect(tb->direction, tb->rect, ret);
        }
        break;
    default:
        break;
    }

    return rect;
}

/*!
  \reimp
*/
int QPlastiqueStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                               QStyleHintReturn *returnData) const
{
    int ret = 0;
    switch (hint) {
    case SH_WindowFrame_Mask:
        ret = 1;
        if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData)) {
            mask->region = option->rect;
            mask->region -= QRect(option->rect.left(), option->rect.top(), 2, 1);
            mask->region -= QRect(option->rect.right() - 1, option->rect.top(), 2, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 1, 1, 1);
            mask->region -= QRect(option->rect.right(), option->rect.top() + 1, 1, 1);

            const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
            if (titleBar && (titleBar->titleBarState & Qt::WindowMinimized)) {
                mask->region -= QRect(option->rect.left(), option->rect.bottom(), 2, 1);
                mask->region -= QRect(option->rect.right() - 1, option->rect.bottom(), 2, 1);
                mask->region -= QRect(option->rect.left(), option->rect.bottom() - 1, 1, 1);
                mask->region -= QRect(option->rect.right(), option->rect.bottom() - 1, 1, 1);
            } else {
                mask->region -= QRect(option->rect.bottomLeft(), QSize(1, 1));
                mask->region -= QRect(option->rect.bottomRight(), QSize(1, 1));
            }
        }
        break;
    case SH_TitleBar_NoBorder:
        ret = 1;
        break;
    case SH_ItemView_ShowDecorationSelected:
        ret = true;
        break;
    case SH_ToolBox_SelectedPageTitleBold:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
        ret = true;
        break;
    case SH_MainWindow_SpaceBelowMenuBar:
        ret = 0;
        break;
    case SH_DialogButtonLayoutPolicy:
        ret = QDialogButtonBox::KdeLayout;
        break;
    default:
        ret = QWindowsStyle::styleHint(hint, option, widget, returnData);
        break;
    }
    return ret;
}

/*!
  \reimp
*/
QStyle::SubControl QPlastiqueStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                          const QPoint &pos, const QWidget *widget) const
{
    SubControl ret = SC_None;
    switch (control) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect slider = subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);
            if (slider.contains(pos)) {
                ret = SC_ScrollBarSlider;
                break;
            }

            QRect scrollBarAddLine = subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
            if (scrollBarAddLine.contains(pos)) {
                ret = SC_ScrollBarAddLine;
                break;
            }

            QRect scrollBarSubPage = subControlRect(control, scrollBar, SC_ScrollBarSubPage, widget);
            if (scrollBarSubPage.contains(pos)) {
                ret = SC_ScrollBarSubPage;
                break;
            }

            QRect scrollBarAddPage = subControlRect(control, scrollBar, SC_ScrollBarAddPage, widget);
            if (scrollBarAddPage.contains(pos)) {
                ret = SC_ScrollBarAddPage;
                break;
            }

            QRect scrollBarSubLine = subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
            if (scrollBarSubLine.contains(pos)) {
                ret = SC_ScrollBarSubLine;
                break;
            }
        }
        break;
#endif // QT_NO_SCROLLBAR
    default:
        break;
    }

    return ret != SC_None ? ret : QWindowsStyle::hitTestComplexControl(control, option, pos, widget);
}

/*!
  \reimp
*/
int QPlastiqueStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    int ret = -1;
    switch (metric) {
    case PM_ToolBarIconSize:
        ret = 24;
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 1;
        break;
    case PM_ButtonDefaultIndicator:
        ret = 0;
        break;
#ifndef QT_NO_SLIDER
    case PM_SliderThickness:
        ret = 15;
        break;
    case PM_SliderLength:
    case PM_SliderControlThickness:
        ret = 11;
        break;
    case PM_SliderTickmarkOffset:
        ret = 5;
        break;
    case PM_SliderSpaceAvailable:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int size = 15;
            if (slider->tickPosition & QSlider::TicksBelow)
                ++size;
            if (slider->tickPosition & QSlider::TicksAbove)
                ++size;
            ret = size;
            break;
        }
#endif // QT_NO_SLIDER
    case PM_ScrollBarExtent:
        ret = 16;
        break;
    case PM_ScrollBarSliderMin:
        ret = 26;
        break;
    case PM_ProgressBarChunkWidth:
        ret = 1;
        break;
    case PM_MenuBarItemSpacing:
        ret = 3;
        break;
    case PM_MenuBarVMargin:
        ret = 2;
        break;
    case PM_MenuBarHMargin:
        ret = 0;
        break;
    case PM_MenuBarPanelWidth:
        ret = 1;
        break;
    case PM_ToolBarHandleExtent:
        ret = 9;
        break;
    case PM_ToolBarSeparatorExtent:
        ret = 2;
        break;
    case PM_ToolBarItemSpacing:
        ret = 1;
        break;
    case PM_ToolBarItemMargin:
        ret = 1;
        break;
    case PM_ToolBarFrameWidth:
        ret = 2;
        break;
    case PM_SplitterWidth:
        ret = 6;
        break;
    case PM_DockWidgetSeparatorExtent:
        ret = 6;
        break;
    case PM_DockWidgetHandleExtent:
        ret = 20;
        break;
    case PM_DefaultFrameWidth:
#ifndef QT_NO_MENU
        if (qobject_cast<const QMenu *>(widget)) {
            ret = 1;
            break;
        }
#endif
        ret = 2;
        break;
#ifndef QT_NO_TABBAR
    case PM_TabBarTabVSpace:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (!tab->icon.isNull()) {
                ret = 15;
                break;
            }
        }
        break;
#endif // QT_NO_TABBAR
    case PM_MDIFrameWidth:
        ret = 4;
        break;
    case PM_TitleBarHeight:
#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3DockWindowTitleBar")) {
            // Q3DockWindow has smaller titlebars than QDockWidget
            ret = qMax(widget->fontMetrics().lineSpacing(), 16);
        } else
#endif
            ret = qMax(widget ? widget->fontMetrics().lineSpacing() : option->fontMetrics.lineSpacing(), 30);
        break;
    case PM_MaximumDragDistance:
        return -1;
    case PM_DockWidgetTitleMargin:
        return 0;
    default:
        break;
    }

    return ret != -1 ? ret : QWindowsStyle::pixelMetric(metric, option, widget);
}

/*!
  \reimp
*/
QPalette QPlastiqueStyle::standardPalette() const
{
    QPalette palette;

    palette.setBrush(QPalette::Disabled, QPalette::Foreground, QColor(QRgb(0xff808080)));
    palette.setBrush(QPalette::Disabled, QPalette::Button, QColor(QRgb(0xffdddfe4)));
    palette.setBrush(QPalette::Disabled, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Disabled, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Disabled, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(QRgb(0xff808080)));
    palette.setBrush(QPalette::Disabled, QPalette::Base, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, palette.color(QPalette::Disabled, QPalette::Base).dark(110));
    palette.setBrush(QPalette::Disabled, QPalette::Background, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(QRgb(0xff567594)));
    palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Link, QColor(QRgb(0xff0000ee)));
    palette.setBrush(QPalette::Disabled, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    palette.setBrush(QPalette::Active, QPalette::Foreground, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Button, QColor(QRgb(0xffdddfe4)));
    palette.setBrush(QPalette::Active, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Active, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Active, QPalette::Text, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::ButtonText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::AlternateBase, palette.color(QPalette::Active, QPalette::Base).dark(110));
    palette.setBrush(QPalette::Active, QPalette::Background, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Active, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Highlight, QColor(QRgb(0xff678db2)));
    palette.setBrush(QPalette::Active, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Link, QColor(QRgb(0xff0000ee)));
    palette.setBrush(QPalette::Active, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    palette.setBrush(QPalette::Inactive, QPalette::Foreground, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Button, QColor(QRgb(0xffdddfe4)));
    palette.setBrush(QPalette::Inactive, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Inactive, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Inactive, QPalette::Text, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::ButtonText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, palette.color(QPalette::Inactive, QPalette::Base).dark(110));
    palette.setBrush(QPalette::Inactive, QPalette::Background, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Inactive, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, QColor(QRgb(0xff678db2)));
    palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Link, QColor(QRgb(0xff0000ee)));
    palette.setBrush(QPalette::Inactive, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    return palette;
}

/*!
  \reimp
*/
void QPlastiqueStyle::polish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget)
#ifndef QT_NO_COMBOBOX
        || qobject_cast<QComboBox *>(widget)
#endif
#ifndef QT_NO_SPINBOX
        || qobject_cast<QAbstractSpinBox *>(widget)
#endif
        || qobject_cast<QCheckBox *>(widget)
#ifndef QT_NO_GROUPBOX
        || qobject_cast<QGroupBox *>(widget)
#endif
        || qobject_cast<QRadioButton *>(widget)
#ifndef QT_NO_SPLITTER
        || qobject_cast<QSplitterHandle *>(widget)
#endif
#ifndef QT_NO_TABBAR
        || qobject_cast<QTabBar *>(widget)
#endif
        ) {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (widget->inherits("QWorkspaceTitleBar")
        || widget->inherits("QDockSeparator")
        || widget->inherits("QDockWidgetSeparator")
        || widget->inherits("Q3DockWindowResizeHandle")) {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (false // to simplify the #ifdefs
#ifndef QT_NO_MENUBAR
        || qobject_cast<QMenuBar *>(widget)
#endif
#ifdef QT3_SUPPORT
        || widget->inherits("Q3ToolBar")
#endif
#ifndef QT_NO_TOOLBAR
        || qobject_cast<QToolBar *>(widget)
        || (widget && qobject_cast<QToolBar *>(widget->parent()))
#endif
        ) {
        widget->setBackgroundRole(QPalette::Background);
    }

#ifndef QT_NO_PROGRESSBAR
    if (AnimateBusyProgressBar && qobject_cast<QProgressBar *>(widget))
        widget->installEventFilter(this);
#endif

#if defined QPlastique_MaskButtons
    if (qobject_cast<QPushButton *>(widget) || qobject_cast<QToolButton *>(widget))
        widget->installEventFilter(this);
#endif
}

/*!
  \reimp
*/
void QPlastiqueStyle::unpolish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget)
#ifndef QT_NO_COMBOBOX
        || qobject_cast<QComboBox *>(widget)
#endif
#ifndef QT_NO_SPINBOX
        || qobject_cast<QAbstractSpinBox *>(widget)
#endif
        || qobject_cast<QCheckBox *>(widget)
#ifndef QT_NO_GROUPBOX
        || qobject_cast<QGroupBox *>(widget)
#endif
#ifndef QT_NO_SPLITTER
        || qobject_cast<QSplitterHandle *>(widget)
#endif
#ifndef QT_NO_TABBAR
        || qobject_cast<QTabBar *>(widget)
#endif
        || qobject_cast<QRadioButton *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    if (widget->inherits("QWorkspaceTitleBar")
        || widget->inherits("QDockSeparator")
        || widget->inherits("QDockWidgetSeparator")
        || widget->inherits("Q3DockWindowResizeHandle")) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    if (false // to simplify the #ifdefs
#ifndef QT_NO_MENUBAR
        || qobject_cast<QMenuBar *>(widget)
#endif
#ifndef QT_NO_TOOLBOX
        || qobject_cast<QToolBox *>(widget)
#endif
#ifdef QT3_SUPPORT
        || widget->inherits("Q3ToolBar")
#endif
#ifndef QT_NO_TOOLBAR
        || qobject_cast<QToolBar *>(widget)
        || (widget && qobject_cast<QToolBar *>(widget->parent()))
#endif
        ) {
        widget->setBackgroundRole(QPalette::Button);
    }

#ifndef QT_NO_PROGRESSBAR
    if (AnimateBusyProgressBar && qobject_cast<QProgressBar *>(widget))
        widget->removeEventFilter(this);
#endif

#if defined QPlastique_MaskButtons
    if (qobject_cast<QPushButton *>(widget) || qobject_cast<QToolButton *>(widget))
        widget->removeEventFilter(this);
#endif
}

/*!
  \reimp
*/
void QPlastiqueStyle::polish(QApplication *app)
{
    QWindowsStyle::polish(app);
}

/*!
  \reimp
*/
void QPlastiqueStyle::polish(QPalette &pal)
{
    QWindowsStyle::polish(pal);
    pal.setBrush(QPalette::AlternateBase, pal.base().color().dark(110));
}

/*!
  \reimp
*/
void QPlastiqueStyle::unpolish(QApplication *app)
{
    QWindowsStyle::unpolish(app);
}

/*!
    \reimp
*/
bool QPlastiqueStyle::eventFilter(QObject *watched, QEvent *event)
{
#ifndef QT_NO_PROGRESSBAR
    switch (event->type()) {
    case QEvent::Show:
        if (QProgressBar *bar = qobject_cast<QProgressBar *>(watched)) {
            d->bars << bar;
            if (d->bars.size() == 1) {
                Q_ASSERT(ProgressBarFps > 0);
                d->timer.start();
                d->progressBarAnimateTimer = startTimer(1000 / ProgressBarFps);
            }
        }
        break;
    case QEvent::Destroy:
        d->bars.removeAll(reinterpret_cast<QProgressBar *>(watched));
        break;
    case QEvent::Hide:
        if (QProgressBar *bar = qobject_cast<QProgressBar *>(watched)) {
            d->bars.removeAll(bar);
            if (d->bars.isEmpty()) {
                killTimer(d->progressBarAnimateTimer);
                d->progressBarAnimateTimer = 0;
            }
        }
#if defined QPlastique_MaskButtons
    case QEvent::Resize:
        if (qobject_cast<QPushButton *>(watched) || qobject_cast<QToolButton *>(watched)) {
            QWidget *widget = qobject_cast<QWidget *>(watched);
            QRect rect = widget->rect();
            QRegion region(rect);
            region -= QRect(rect.left(), rect.top(), 2, 1);
            region -= QRect(rect.left(), rect.top() + 1, 1, 1);
            region -= QRect(rect.left(), rect.bottom(), 2, 1);
            region -= QRect(rect.left(), rect.bottom() - 1, 1, 1);
            region -= QRect(rect.right() - 1, rect.top(), 2, 1);
            region -= QRect(rect.right(), rect.top() + 1, 1, 1);
            region -= QRect(rect.right() - 1, rect.bottom(), 2, 1);
            region -= QRect(rect.right(), rect.bottom() - 1, 1, 1);
            widget->setMask(region);
        }
        break;
#endif
    default:
        break;
    }
#endif // QT_NO_PROGRESSBAR

    return QWindowsStyle::eventFilter(watched, event);
}

/*!
    \reimp
*/
void QPlastiqueStyle::timerEvent(QTimerEvent *event)
{
#ifndef QT_NO_PROGRESSBAR
    if (event->timerId() == d->progressBarAnimateTimer) {
        Q_ASSERT(ProgressBarFps > 0);
        d->animateStep = d->timer.elapsed() / (1000 / ProgressBarFps);
        foreach (QProgressBar *bar, d->bars) {
            if (AnimateProgressBar || (bar->minimum() == 0 && bar->maximum() == 0))
                bar->update();
        }
    }
#endif // QT_NO_PROGRESSBAR
    event->ignore();
}

#endif // QT_NO_STYLE_PLASTIQUE
