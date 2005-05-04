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

static const bool UsePixmapCache = true;

#include "qplastiquestyle.h"

#include <qdebug.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qabstractitemview.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qstyleoption.h>
#include <qtextedit.h>
#include <qtoolbox.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include <limits.h>

// from windows style
static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsSepHeight        =  2; // separator item height
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  2; // menu item ver text margin
static const int windowsArrowHMargin	 =  6; // arrow horizontal margin
static const int windowsTabSpacing	 = 12; // space between text and tab
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
    "13 13 3 1",
    "X c #828282",
    "o c None",
    ". c None",
    "...oXXXXXo...",
    "..XXo...oXX..",
    ".Xo.......oX.",
    "oX.........Xo",
    "Xo.........oX",
    "X...........X",
    "X...........X",
    "X...........X",
    "Xo.........oX",
    "oX.........Xo",
    ".Xo.......oX.",
    "..XXo...oXX..",
    "...oXXXXXo..."};

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
    " 	c None",
    ".	c #567CB6",
    "+	c #7AA1DB",
    "@	c #ABC3E8",
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
    " 	c None",
    "+	c #979797",
    "@	c #C9C9C9",
    "$	c #C1C1C1",
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
    " 	c None",
    "+	c #979797",
    "@	c #C9C9C9",
    "$	c #C1C1C1",
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
    " 	c None",
    "+	c #979797",
    "@	c #C9C9C9",
    "$	c #C1C1C1",
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
    " 	c None",
    "+	c #979797",
    "@	c #C9C9C9",
    "$	c #C1C1C1",
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
    " 	c None",
    "*	c #BFBFBF",
    "   *",
    "  **",
    " ***",
    "****",
    " ***",
    "  **",
    "   *"};

static const char * const qt_scrollbar_button_arrow_right[] = {
    "4 7 2 1",
    " 	c None",
    "*	c #BFBFBF",
    "*   ",
    "**  ",
    "*** ",
    "****",
    "*** ",
    "**  ",
    "*   "};

static const char * const qt_scrollbar_button_arrow_up[] = {
    "7 4 2 1",
    " 	c None",
    "*	c #BFBFBF",
    "   *   ",
    "  ***  ",
    " ***** ",
    "*******"};

static const char * const qt_scrollbar_button_arrow_down[] = {
    "7 4 2 1",
    " 	c None",
    "*	c #BFBFBF",
    "*******",
    " ***** ",
    "  ***  ",
    "   *   "};

static const char * const qt_scrollbar_button_left[] = {
    "16 16 6 1",
    " 	c None",
    ".	c #BFBFBF",
    "+	c #979797",
    "#	c #FAFAFA",
    "<	c #FAFAFA",
    "*	c #FAFAFA",
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
    " 	c None",
    ".	c #BFBFBF",
    "+	c #979797",
    "#	c #FAFAFA",
    "<	c #FAFAFA",
    "*	c #FAFAFA",
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
    " 	c None",
    ".	c #BFBFBF",
    "+	c #979797",
    "#	c #FAFAFA",
    "<	c #FAFAFA",
    "*	c #FAFAFA",
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
    " 	c None",
    ".	c #BFBFBF",
    "+	c #979797",
    "#	c #FAFAFA",
    "<	c #FAFAFA",
    "*	c #FAFAFA",
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

static const char * const qt_scrollbar_slider_pattern[] = {
    "10 10 3 1",
    " 	c None",
    ".	c #BFBFBF",
    "+	c #979797",
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

static void qt_plastique_draw_gradient(QPainter *painter, const QRect &rect, QColor gradientStart, QColor gradientStop)
{
    QString gradientName;
    gradientName.sprintf("%dx%d-%x-%x", rect.width(), rect.height(), gradientStart.rgba(), gradientStop.rgba());
    QPixmap cache;
    if (!UsePixmapCache || !QPixmapCache::find(gradientName, cache)) {
        cache = QPixmap(rect.size());
        QPainter cachePainter(&cache);
        QRect pixmapRect(0, 0, rect.width(), rect.height());
        int x = pixmapRect.center().x();
        QLinearGradient gradient(x, pixmapRect.top(), x, pixmapRect.bottom());
        gradient.setColorAt(0, gradientStart);
        gradient.setColorAt(1, gradientStop);
        cachePainter.fillRect(pixmapRect, gradient);
        if (UsePixmapCache)
            QPixmapCache::insert(gradientName, cache);
    }
    painter->drawPixmap(rect, cache);
}

static void qt_plastique_drawFrame(QPainter *painter, const QStyleOption *option)
{
    QRect rect = option->rect;
    QPen oldPen = painter->pen();

    QColor borderColor = option->palette.background().color().dark(178);
    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);
    QColor alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);

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

static void qt_plastique_drawShadedPanel(QPainter *painter, const QStyleOption *option, bool base = false)
{
    QRect rect = option->rect;
    QPen oldPen = painter->pen();

    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);

    // gradient fill
    if (option->state & QStyle::State_Enabled) {
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

    qt_plastique_drawFrame(painter, option);

    painter->setPen(oldPen);
}

static void qt_plastique_draw_mdibutton(QPainter *painter, const QStyleOptionTitleBar *option, const QRect &tmp, bool hover)
{
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

static void qt_plastique_draw_handle(QPainter *painter, const QStyleOption *option,
                                     const QRect &rect, Qt::Orientation orientation)
{
    QColor borderColor = option->palette.background().color().dark(178);
    QColor alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);

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

class QPlastiqueStylePrivate
{
public:
    QPlastiqueStylePrivate(QPlastiqueStyle *qq);
    virtual ~QPlastiqueStylePrivate();

    QPlastiqueStyle *q;
};

/*!
  \internal
 */
QPlastiqueStylePrivate::QPlastiqueStylePrivate(QPlastiqueStyle *qq)
{
    q = qq;
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

    The Plastique style provides a default look and feel for widgets on
    X11 that closely resembles the Plastik style introduced in KDE 3.2.
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

    QColor alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    QColor alphaInnerColor = mergedColors(highlightedLightInnerBorderColor, gradientStartColor);

    QColor alphaInnerColorNoHover = mergedColors(borderColor, gradientStartColor);
    QColor alphaTextColor = mergedColors(option->palette.background().color(), option->palette.text().color());

    QColor alphaLightTextColor = mergedColors(option->palette.background().color().light(250), option->palette.text().color().light(250));

    QColor lightShadow = option->palette.button().color().light(105);

    QColor shadowGradientStartColor = option->palette.button().color().dark(115);
    QColor shadow = shadowGradientStartColor;

    switch (element) {
    case PE_FrameDefaultButton:
        // Draws the frame around a default button (drawn in
        // PE_PanelButtonCommand).
        break;
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
    case PE_FrameLineEdit:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            int lw = 1; int mlw = 1;

            // For line edits, we need to fill the background in the corners before drawing
            // the frame since we have rounded corners
            painter->fillRect(QRect(option->rect.left(), option->rect.top(),option->rect.width(),lw+mlw),
                              frame->palette.background());
            painter->fillRect(QRect(option->rect.left(), option->rect.bottom() - lw - mlw + 1,option->rect.width(),lw+mlw),
                              frame->palette.background());
        }

        // fall throoooough

    case PE_FrameGroupBox:
    case PE_Frame:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            painter->save();

            bool focus = (element == PE_FrameLineEdit || qobject_cast<const QTextEdit *>(widget))
                             && (frame->state & State_Enabled) && (frame->state & State_HasFocus);
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
            QColor alphaLineEnds = mergedColors(frame->palette.background().color(), color);
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
                    color = focus ? option->palette.highlight().color().light(101) : option->palette.button().color().light(101);
                } else {
                    color = focus ? option->palette.highlight().color().dark(130) : option->palette.button().color().dark(130);
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
                    color = focus ? option->palette.highlight().color().dark(130) : option->palette.button().color().dark(130);
                } else {
                    color = focus ? option->palette.highlight().color().light(101) : option->palette.button().color().light(101);
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
    case PE_FrameMenu: {
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
    case PE_PanelMenuBar:
    case PE_PanelToolBar: {
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
        break;
    }
    case PE_PanelButtonTool:
        // Draws a tool button (f.ex., in QToolBar and QTabBar)
        if ((option->state & State_Enabled) || !(option->state & State_AutoRaise))
            qt_plastique_drawShadedPanel(painter, option, true);
        break;
    case PE_IndicatorToolBarHandle: {
        QPixmap cache;
        QString pixmapName = uniqueName("toolbarhandle", option, option->rect.size());
        if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
            cache = QPixmap(option->rect.size());
            QPainter cachePainter(&cache);
            if (widget)
                cachePainter.fillRect(option->rect, option->palette.brush(widget->backgroundRole()));
            else
                cachePainter.fillRect(option->rect, option->palette.background());
            
            QImage handle(qt_toolbarhandle);
            handle.setColor(1, alphaCornerColor.rgba());
            handle.setColor(2, mergedColors(alphaCornerColor, option->palette.base().color()).rgba());
            handle.setColor(3, option->palette.base().color().rgba());

            if (option->state & State_Horizontal) {
                int nchunks = option->rect.height() / handle.height();
                int indent = (option->rect.height() - (nchunks * handle.height())) / 2;
                for (int i = 0; i < nchunks; ++i)
                    cachePainter.drawImage(QPoint(option->rect.left() + 3, option->rect.top() + indent + i * handle.height()),
                                           handle);
            } else {
                int nchunks = option->rect.width() / handle.width();
                int indent = (option->rect.width() - (nchunks * handle.width())) / 2;
                for (int i = 0; i < nchunks; ++i)
                    cachePainter.drawImage(QPoint(option->rect.left() + indent + i * handle.width(), option->rect.top() + 3),
                                           handle);
            }
            if (UsePixmapCache)
                QPixmapCache::insert(pixmapName, cache);
        }
        painter->drawPixmap(option->rect.topLeft(), cache);
        break;
    }
    case PE_IndicatorToolBarSeparator: {
        painter->save();
        painter->setPen(alphaCornerColor);
        painter->drawLine(option->rect.left(), option->rect.top() + 1, option->rect.left(), option->rect.bottom() - 2);
        painter->setPen(option->palette.base().color());
        painter->drawLine(option->rect.right(), option->rect.top() + 1, option->rect.right(), option->rect.bottom() - 2);
        painter->restore();
        break;
    }
    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const QPushButton *pushButton = qobject_cast<const QPushButton *>(widget);
            bool hoverable = (pushButton || qobject_cast<const QComboBox *>(widget));
            bool down = (button->state & State_Sunken) || (button->state & State_On);
            bool hover = hoverable && (button->state & State_Enabled) && (button->state & State_MouseOver);
            bool isDefault = (button->features & QStyleOptionButton::DefaultButton);
            bool isEnabled = (button->state & State_Enabled);
            QRect rect = option->rect;

            QPixmap cache;
            QString pixmapName = uniqueName("panelbuttoncommand", option, rect.size());
            if (isDefault)
                pixmapName += QLatin1String("-") + QString::number(int(button->features), 16);

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(rect.size());
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
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top(),
                                           pixmapRect.right() - 2, pixmapRect.top());
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom(),
                                           pixmapRect.right() - 2, pixmapRect.bottom());
                    buttonPainter.drawLine(pixmapRect.left(), pixmapRect.top() + 2,
                                           pixmapRect.left(), pixmapRect.bottom() - 2);
                    buttonPainter.drawLine(pixmapRect.right(), pixmapRect.top() + 2,
                                           pixmapRect.right(), pixmapRect.bottom() - 2);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.bottom() - 1);

                    // "antialiased" corners
                    buttonPainter.setPen(alphaCornerColor);
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.top());
                    buttonPainter.drawPoint(pixmapRect.left() + 1, pixmapRect.bottom());
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.top());
                    buttonPainter.drawPoint(pixmapRect.right() - 1, pixmapRect.bottom());
                    buttonPainter.drawPoint(pixmapRect.left(), pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.left(), pixmapRect.bottom() - 1);
                    buttonPainter.drawPoint(pixmapRect.right(), pixmapRect.top() + 1);
                    buttonPainter.drawPoint(pixmapRect.right(), pixmapRect.bottom() - 1);
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
                if (isDefault) {
                    buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.top() + 2,
                                           pixmapRect.right() - 3, pixmapRect.top() + 2);
                } else {
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 1,
                                           pixmapRect.right() - 2, pixmapRect.top() + 1);
                }

                if (down) {
                    buttonPainter.setPen(option->palette.button().color().light(96));
                } else {
                    if (hover) {
                        buttonPainter.setPen(highlightedDarkInnerBorderColor.dark(105));
                    } else {
                        buttonPainter.setPen(option->palette.button().color().light(91));
                    }
                }
                if (isDefault) {
                    buttonPainter.drawLine(pixmapRect.left() + 3, pixmapRect.bottom() - 2,
                                           pixmapRect.right() - 3, pixmapRect.bottom() - 2);
                } else {
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom() - 1,
                                           pixmapRect.right() - 2, pixmapRect.bottom() - 1);
                }

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

                if (isDefault) {
                    buttonPainter.setPen(QPen(QBrush(leftGrad), 1));
                    buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 3,
                                           pixmapRect.left() + 2, pixmapRect.bottom() - 3);
                    buttonPainter.setPen(QPen(QBrush(rightGrad), 1));
                    buttonPainter.drawLine(pixmapRect.right() - 2, pixmapRect.top() + 3,
                                           pixmapRect.right() - 2, pixmapRect.bottom() - 3);
                } else {
                    buttonPainter.setPen(QPen(QBrush(leftGrad), 1));
                    buttonPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 2,
                                           pixmapRect.left() + 1, pixmapRect.bottom() - 2);
                    buttonPainter.setPen(QPen(QBrush(rightGrad), 1));
                    buttonPainter.drawLine(pixmapRect.right() - 1, pixmapRect.top() + 2,
                                           pixmapRect.right() - 1, pixmapRect.bottom() - 2);
                }

                if (!down && hover) {
                    if (isDefault) {
                        buttonPainter.setPen(highlightedLightInnerBorderColor);
                        buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.top() + 2,
                                               pixmapRect.right() - 2, pixmapRect.top() + 2);
                        buttonPainter.setPen(highlightedLightInnerBorderColor.dark(105));
                        buttonPainter.drawLine(pixmapRect.left() + 2, pixmapRect.bottom() - 2,
                                               pixmapRect.right() - 2, pixmapRect.bottom() - 2);
                    } else {
                        buttonPainter.setPen(highlightedLightInnerBorderColor);
                        buttonPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 2,
                                               pixmapRect.right() - 1, pixmapRect.top() + 2);
                        buttonPainter.setPen(highlightedLightInnerBorderColor.dark(105));
                        buttonPainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom() - 2,
                                               pixmapRect.right() - 1, pixmapRect.bottom() - 2);
                    }
                }
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
                if ((button->state & (State_On | State_NoChange))) {
                    QImage image((button->state & (State_NoChange | State_Sunken)
                                  ? qt_plastique_check_sunken : qt_plastique_check));
                    if ((button->state & (State_Sunken | State_NoChange))) {
                        image.setColor(0, alphaLightTextColor.rgba());
                        image.setColor(1, alphaLightTextColor.light(130).rgba());
                        image.setColor(2, alphaLightTextColor.light(110).rgba());
                    } else {
                        image.setColor(0, option->palette.text().color().rgba());
                        image.setColor(1, alphaTextColor.rgba());
                    }
                    checkBoxPainter.drawImage(pixmapRect.x() + 2, pixmapRect.y() + 2, image);
                }
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
                if (button->state & State_On) {
                    image = QImage(qt_plastique_radio_check);
                    if (button->state & State_Sunken) {
                        image.setColor(1, mergedColors(button->palette.background().color(), alphaTextColor).rgba());
                        image.setColor(2, mergedColors(button->palette.background().color(), button->palette.text().color()).rgba());
                    } else {
                        image.setColor(1, alphaTextColor.rgba());
                        image.setColor(2, button->palette.text().color().rgba());
                    }
                    radioButtonPainter.drawImage(pixmapRect, image);
                }
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(rect.topLeft(), cache);
        }
        break;
    case PE_IndicatorDockWidgetResizeHandle:
        if (option->state & State_Horizontal) {
            int width = option->rect.width() / 3;
            QRect rect(option->rect.center().x() - width / 2,
                       option->rect.left() + (option->rect.width() / 2) - 1, width, 3);
            qt_plastique_draw_handle(painter, option, rect, Qt::Vertical);
        } else {
            int height = option->rect.height() / 3;
            QRect rect(option->rect.left() + (option->rect.width() / 2 - 1),
                       option->rect.center().y() - height / 2, 3, height);
            qt_plastique_draw_handle(painter, option, rect, Qt::Horizontal);
        }
        break;
    case PE_IndicatorViewItemCheck: {
        QStyleOptionButton button;
        button.state = option->state & ~State_MouseOver;
        button.rect = option->rect;
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

        // alpha corners
        painter->setPen(mergedColors(palette.highlight().color(), palette.background().color(), 55));
        painter->drawPoint(option->rect.left() + 2, option->rect.bottom() - 1);
        painter->drawPoint(option->rect.left() + 1, option->rect.bottom() - 2);
        painter->drawPoint(option->rect.right() - 2, option->rect.bottom() - 1);
        painter->drawPoint(option->rect.right() - 1, option->rect.bottom() - 2);

        // upper and lower left inner
        painter->setPen(active ? mergedColors(palette.highlight().color(), palette.background().color()) : palette.background().color().dark(120));
        painter->drawLine(option->rect.left() + 1, titleBarStop, option->rect.left() + 1, option->rect.bottom() - 2);
        painter->setPen(active ? mergedColors(palette.highlight().color(), palette.background().color(), 57) : palette.background().color().dark(130));
        painter->drawLine(option->rect.right() - 1, titleBarStop, option->rect.right() - 1, option->rect.bottom() - 2);
        painter->drawLine(option->rect.left() + 1, option->rect.bottom() - 1, option->rect.right() - 1, option->rect.bottom() - 1);

        painter->restore();
    }
        break;
    case PE_IndicatorBranch:
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
    QColor alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
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
            bool atBeginning = ((tab->position == QStyleOptionTab::Beginning) || onlyTab) && !leftCornerWidget;
            bool reverseShadow = false;

            int borderThickness = pixelMetric(PM_TabBarBaseOverlap, tab, widget);
            int marginLeft = 0;
            if ((atBeginning && !selected) || (selected && leftCornerWidget && (tab->position == QStyleOptionTab::Beginning) || onlyTab)) {
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
            painter->save();

            int grooveWidth = subElementRect(SE_ProgressBarGroove, option, widget).width();
            QRect rect = subElementRect(SE_ProgressBarLabel, option, widget);
            QRect leftRect;

            QFont font = widget->font();
            font.setBold(true);
            painter->setFont(font);
            painter->setPen(bar->palette.text().color());

            int progressIndicatorPos = int(((bar->progress - bar->minimum) / double(bar->maximum - bar->minimum)) * grooveWidth);
            if (bar->direction == Qt::RightToLeft) {
                int indicatorPos = grooveWidth - progressIndicatorPos;
                if (indicatorPos >= rect.left() && indicatorPos <= rect.right()) {
                    int leftWidth = indicatorPos - rect.left();
                    painter->setPen(bar->palette.base().color());
                    leftRect = QRect(rect.left(), rect.top(), leftWidth, rect.height());
                } else if (indicatorPos > rect.right()) {
                    painter->setPen(bar->palette.text().color());
                } else {
                    painter->setPen(bar->palette.base().color());
                }
            } else {
                if (progressIndicatorPos >= rect.left() && progressIndicatorPos <= rect.right()) {
                    int leftWidth = progressIndicatorPos - rect.left();
                    leftRect = QRect(rect.left(), rect.top(), leftWidth, rect.height());
                } else if (progressIndicatorPos > rect.right()) {
                    painter->setPen(bar->palette.base().color());
                } else {
                    painter->setPen(bar->palette.text().color());
                }
            }

            painter->drawText(rect, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            if (!leftRect.isNull()) {
                painter->setPen(bar->direction == Qt::RightToLeft ? bar->palette.text().color() : bar->palette.base().color());
                painter->setClipRect(leftRect, Qt::IntersectClip);
                painter->drawText(rect, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            }
            painter->restore();
        }
        break;
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QPen oldPen = painter->pen();

            int maxWidth = bar->rect.width() - 4;
            int minWidth = 4;
            int progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
            int width = qMax(int((((progress - bar->minimum)) / double(bar->maximum - bar->minimum)) * maxWidth), minWidth);
            bool reverse = bar->direction == Qt::RightToLeft;

            QRect progressBar;
            if (!reverse) {
                progressBar.setRect(bar->rect.left() + 2, bar->rect.top() + 2, width, bar->rect.height() - 4);
            } else {
                progressBar.setRect(bar->rect.right() - 1 - width, bar->rect.top() + 2, width, bar->rect.height() - 4);
            }

            // outline
            painter->setPen(highlightedDarkInnerBorderColor);
            if (!reverse) {
                if (width == minWidth) {
                    painter->drawPoint(progressBar.left() + 1, progressBar.top());
                    painter->drawPoint(progressBar.left() + 1, progressBar.bottom());
                } else {
                    painter->drawLine(progressBar.left() + 1, progressBar.top(), progressBar.right() - 2, progressBar.top());
                    painter->drawLine(progressBar.left() + 1, progressBar.bottom(), progressBar.right() - 2, progressBar.bottom());
                }
                painter->drawLine(progressBar.left(), progressBar.top() + 1, progressBar.left(), progressBar.bottom() - 1);
                painter->drawLine(progressBar.right(), progressBar.top() + 2, progressBar.right(), progressBar.bottom() - 2);
            } else {
                if (width == minWidth) {
                    painter->drawPoint(progressBar.right() - 1, progressBar.top());
                    painter->drawPoint(progressBar.right() - 1, progressBar.bottom());
                } else {
                    painter->drawLine(progressBar.right() - 1, progressBar.top(), progressBar.left() + 2, progressBar.top());
                    painter->drawLine(progressBar.right() - 1, progressBar.bottom(), progressBar.left() + 2, progressBar.bottom());
                }
                painter->drawLine(progressBar.right(), progressBar.top() + 1, progressBar.right(), progressBar.bottom() - 1);
                painter->drawLine(progressBar.left(), progressBar.top() + 2, progressBar.left(), progressBar.bottom() - 2);
            }

            // alpha corners
            painter->setPen(alphaInnerColor);
            if (!reverse) {
                painter->drawPoint(progressBar.left(), progressBar.top());
                painter->drawPoint(progressBar.left(), progressBar.bottom());
                painter->drawPoint(progressBar.right() - 1, progressBar.top());
                painter->drawPoint(progressBar.right(), progressBar.top() + 1);
                painter->drawPoint(progressBar.right() - 1, progressBar.bottom());
                painter->drawPoint(progressBar.right(), progressBar.bottom() - 1);
            } else {
                painter->drawPoint(progressBar.right(), progressBar.top());
                painter->drawPoint(progressBar.right(), progressBar.bottom());
                painter->drawPoint(progressBar.left() + 1, progressBar.top());
                painter->drawPoint(progressBar.left(), progressBar.top() + 1);
                painter->drawPoint(progressBar.left() + 1, progressBar.bottom());
                painter->drawPoint(progressBar.left(), progressBar.bottom() - 1);
            }

            // contents
            painter->setPen(QPen());

            int rectStart;
            if (!reverse)
                rectStart = progressBar.left() + 1 - 9 + (progressBar.width() % 10);
            else
                rectStart = progressBar.right() + 9 - (progressBar.width() % 10);
            int lineStart;
            if (!reverse)
                lineStart = progressBar.left() + 1 - 10 + (progressBar.width() % 10);
            else
                lineStart = progressBar.right() + 10 - (progressBar.width() % 10);

            int rectFlip = (progressBar.width() % 20) < 10;
            int lineFlip;
            if (!reverse)
                lineFlip = ((progressBar.width() - 1) % 20) < 10;
            else
                lineFlip = ((progressBar.width() + 1) % 20) < 10;
            QColor rectColor = option->palette.highlight().color();
            QColor lineColor = option->palette.highlight().color();
            if (rectFlip)
                rectColor = rectColor.light(105);
            if (lineFlip)
                lineColor = lineColor.light(105);

            while (reverse ? (rectStart > progressBar.left() + 2) : (rectStart < progressBar.right() - 2)) {
                int rectWidth = 10;
                int lineWidth = 10;

                if (!reverse) {
                    if (rectStart + rectWidth > progressBar.right())
                        rectWidth = (progressBar.right() - rectStart);
                    if (lineStart + lineWidth > progressBar.right() - 1)
                        lineWidth = (progressBar.right() - 1 - lineStart);
                } else {
                    if (rectStart - rectWidth < progressBar.left())
                        rectWidth = rectStart - progressBar.left();
                    if (lineStart - lineWidth < progressBar.left() + 1)
                        lineWidth = lineStart - (progressBar.left() + 1);
                }

                if (!reverse) {
                    int adjustedRectStart = qMax(rectStart, progressBar.left() + 1);
                    QRect section(adjustedRectStart, progressBar.top() + 2,
                                  (rectWidth - (adjustedRectStart - rectStart)), progressBar.height() - 4);
                    if (section.width() == 1) {
                        painter->setPen(rectColor);
                        painter->drawLine(section.left(), section.top(), section.left(), section.bottom());
                    } else {
                        painter->fillRect(section, rectColor);
                        painter->setPen(rectColor.dark(102));
                        painter->drawLine(section.right(), section.top(), section.right(), section.bottom());
                    }
                } else {
                    int adjustedRectStart = qMin(rectStart, progressBar.right());
                    rectWidth -= (rectStart - adjustedRectStart);
                    QRect section(adjustedRectStart - rectWidth, progressBar.top() + 2,
                                  rectWidth, progressBar.height() - 4);
                    if (section.width() == 1) {
                        painter->setPen(rectColor);
                        painter->drawLine(section.right(), section.top(), section.right(), section.bottom());
                    } else {
                        painter->fillRect(section, rectColor);
                        painter->setPen(rectColor.dark(102));
                        painter->drawLine(section.right(), section.top(), section.right(), section.bottom());
                    }
                }

                painter->setPen(lineColor);
                if (!reverse) {
                    int adjustedLineStart = qMax(lineStart, progressBar.left() + 1);
                    painter->drawLine(adjustedLineStart, progressBar.top() + 1,
                                      adjustedLineStart + (lineWidth - (adjustedLineStart - lineStart)), progressBar.top() + 1);
                    painter->drawLine(adjustedLineStart, progressBar.bottom() - 1,
                                      adjustedLineStart + (lineWidth - (adjustedLineStart - lineStart)), progressBar.bottom() - 1);
                } else {
                    int adjustedLineStart = qMin(lineStart, progressBar.right() - 1);
                    painter->drawLine(adjustedLineStart, progressBar.top() + 1,
                                      adjustedLineStart - (lineWidth - (lineStart - adjustedLineStart)), progressBar.top() + 1);
                    painter->drawLine(adjustedLineStart, progressBar.bottom() - 1,
                                      adjustedLineStart - (lineWidth - (lineStart - adjustedLineStart)), progressBar.bottom() - 1);
                }

                rectStart += reverse ? -10 : 10;
                rectColor = option->palette.highlight().color();
                rectFlip ^= 1;
                if (rectFlip)
                    rectColor = rectColor.light(105);

                lineStart += reverse ? -10 : 10;
                lineColor = option->palette.highlight().color();
                lineFlip ^= 1;
                if (lineFlip)
                    lineColor = lineColor.light(105);
            }

            painter->setPen(oldPen);
        }
        break;
    case CE_HeaderSection:
        // Draws the header in tables.
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QPixmap cache;
            QString pixmapName = uniqueName("headersection", option, option->rect.size());
            pixmapName += QLatin1String("-") + QString::number(int(header->position));
            pixmapName += QLatin1String("-") + QString::number(int(header->orientation));

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(option->rect.size());
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
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(option->rect.topLeft(), cache);

        }
        break;
    case CE_MenuItem:
        // Draws one item in a popup menu.
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            painter->save();

            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                painter->setPen(alphaCornerColor);
                painter->drawLine(menuItem->rect.left() + 5, menuItem->rect.top(),
                                  menuItem->rect.right() - 5, menuItem->rect.top());

                painter->restore();
                break;
            }

            bool selected = menuItem->state & State_Selected;
            if (selected) {
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

            bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
            bool checked = menuItem->checked;
            bool enabled = menuItem->state & State_Enabled;

            // Check
            QRect checkRect(option->rect.left() + 7, option->rect.center().y() - 6, 13, 13);
            checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
            if (checkable && checked) {
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

                    QImage image(qt_plastique_check);
                    image.setColor(0, option->palette.text().color().rgba());
                    image.setColor(1, alphaTextColor.rgba());
                    painter->drawImage(QPoint(checkRect.center().x() - image.width() / 2,
                                              checkRect.center().y() - image.height() / 2), image);
                } else {
                    int iconSize = pixelMetric(PM_SmallIconSize);
                    QRect sunkenRect(option->rect.left() + iconSize / 2,
                                     option->rect.center().y() - iconSize / 2, iconSize, iconSize);
                    sunkenRect = visualRect(menuItem->direction, menuItem->rect, sunkenRect);

                    QStyleOption opt = *option;
                    opt.state |= State_Sunken;
                    opt.rect = sunkenRect;
                    qt_plastique_drawShadedPanel(painter, &opt, false);
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
                arrow = QApplication::isRightToLeft() ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
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
    case CE_MenuBarItem:
        // Draws a menu bar item; File, Edit, Help etc..
        if ((option->state & State_Selected) && (option->state & State_Enabled)) {
            QPixmap cache;
            QString pixmapName = uniqueName("menubaritem", option, option->rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(option->rect.size());
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

                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(option->rect.topLeft(), cache);
        } else {
            painter->fillRect(option->rect, option->palette.background());
        }
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
    case CE_MenuBarEmptyArea:
        // Draws the area in a menu bar that is not populated by menu items.
        painter->fillRect(option->rect, option->palette.background());
        break;
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
    case CE_Splitter:
        if (option->state & State_Horizontal) {
            int height = option->rect.height() / 3;
            QRect rect(option->rect.left() + (option->rect.width() / 2 - 1),
                       option->rect.center().y() - height / 2, 3, height);
            qt_plastique_draw_handle(painter, option, rect, Qt::Horizontal);
        } else {
            int width = option->rect.width() / 3;
            QRect rect(option->rect.center().x() - width / 2,
                       option->rect.top() + (option->rect.height() / 2) - 1, width, 3);
            qt_plastique_draw_handle(painter, option, rect, Qt::Vertical);
        }
        break;
    case CE_DockWidgetTitle:
        if (const QStyleOptionDockWidget *dockWidget = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            painter->save();

            // Find text width and title rect
            int textWidth = option->fontMetrics.width(dockWidget->title);
            int margin = 2;
            QRect titleRect = visualRect(dockWidget->direction, dockWidget->rect,
                                         dockWidget->rect.adjusted(margin, 0, -margin * 2 - 26, 0));

            // Chop and insert ellide into title if text is too wide
            QString title = elliditide(dockWidget->title, dockWidget->fontMetrics, titleRect, &textWidth);

            // Draw the toolbar handle pattern to the left and right of the text
            QImage handle(qt_toolbarhandle);
            handle.setColor(1, alphaCornerColor.rgba());
            handle.setColor(2, mergedColors(alphaCornerColor, option->palette.base().color()).rgba());
            handle.setColor(3, option->palette.base().color().rgba());

            // Handle pattern to the left of the title
            QRect leftSide(titleRect.left(), titleRect.top(), titleRect.width() / 2 - textWidth / 2 - margin, titleRect.bottom());
            int nchunks = leftSide.width() / handle.width();
            int indent = (leftSide.width() - (nchunks * handle.width())) / 2;
            for (int i = 0; i < nchunks; ++i) {
                painter->drawImage(QPoint(leftSide.left() + indent + i * handle.width(), leftSide.top() + 3),
                                   handle);
            }

            // Handle pattern to the right of the title
            QRect rightSide = titleRect.adjusted(titleRect.width() / 2 + textWidth / 2 + margin, 0, 0, 0);
            nchunks = rightSide.width() / handle.width();
            indent = (rightSide.width() - (nchunks * handle.width())) / 2;
            for (int j = 0; j < nchunks; ++j) {
                painter->drawImage(QPoint(rightSide.left() + indent + j * handle.width(), rightSide.top() + 3),
                                   handle);
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
    QColor alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    QColor gradientStartColor = option->palette.button().color().light(104);
    QColor gradientStopColor = option->palette.button().color().dark(105);
    QColor highlightedGradientStartColor = option->palette.button().color().light(101);
    QColor highlightedGradientStopColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 85);
    QColor highlightedDarkInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 35);
    QColor highlightedLightInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 58);

    switch (control) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect groove = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            QRect ticks = subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);
            bool horizontal = slider->orientation == Qt::Horizontal;
            bool ticksAbove = slider->tickPosition & QSlider::TicksAbove;
            bool ticksBelow = slider->tickPosition & QSlider::TicksBelow;

            QPixmap cache;

            if ((option->subControls & SC_SliderGroove) && groove.isValid()) {
                QString groovePixmapName = uniqueName("slider_groove", option, groove.size());
                if (!UsePixmapCache || !QPixmapCache::find(groovePixmapName, cache)) {
                    cache = QPixmap(groove.size());
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
                    if (UsePixmapCache)
                        QPixmapCache::insert(groovePixmapName, cache);
                }
                painter->drawPixmap(groove.topLeft(), cache);
            }

            if ((option->subControls & SC_SliderHandle) && handle.isValid()) {
                QString handlePixmapName = uniqueName("slider_handle", option, handle.size());
                if (ticksAbove && !ticksBelow)
                    handlePixmapName += QLatin1String("-flipped");

                if (!UsePixmapCache || !QPixmapCache::find(handlePixmapName, cache)) {
                    cache = QPixmap(handle.size());
                    QRect pixmapRect(0, 0, handle.width(), handle.height());
                    QPainter handlePainter(&cache);
                    handlePainter.fillRect(pixmapRect, option->palette.background());

                    // draw handle
                    if (horizontal) {
                        QPainterPath path;
                        if (ticksAbove && !ticksBelow) {
                            path.moveTo(QPoint(pixmapRect.right(), pixmapRect.bottom() - 1));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.bottom() - 10));
                            path.lineTo(QPoint(pixmapRect.right() - 5, pixmapRect.bottom() - 14));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 10));
                            path.lineTo(QPoint(pixmapRect.left() + 1, pixmapRect.bottom() - 1));
                            path.lineTo(QPoint(pixmapRect.right(), pixmapRect.bottom() - 1));
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
                            gradient.setColorAt(0, gradientStartColor);
                            gradient.setColorAt(1, gradientStopColor);
                            handlePainter.fillPath(path, gradient);
                        } else {
                            handlePainter.fillPath(path, slider->palette.background());
                        }
                    } else {
                        QPainterPath path;
                        if (ticksAbove && !ticksBelow) {
                            path.moveTo(QPoint(pixmapRect.right() - 1, pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.right() - 10, pixmapRect.top() + 1));
                            path.lineTo(QPoint(pixmapRect.right() - 14, pixmapRect.top() + 5));
                            path.lineTo(QPoint(pixmapRect.right() - 10, pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.right() - 1, pixmapRect.bottom()));
                            path.lineTo(QPoint(pixmapRect.right() - 1, pixmapRect.top() + 1));
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
                    if (UsePixmapCache)
                        QPixmapCache::insert(handlePixmapName, cache);
                }

                painter->drawPixmap(handle.topLeft(), cache);
            }

            if (option->subControls & SC_SliderTickmarks) {
                QPen oldPen = painter->pen();
                painter->setPen(borderColor);
                int nticks = 15;
                int tickSize = pixelMetric(PM_SliderTickmarkOffset, option, widget);

                // draw ticks
                if (horizontal) {
                    int startPos = slider->rect.left() + 5;
                    int endPos = slider->rect.right() - 5;
                    for (int i = 0; i < nticks; ++i) {
                        if (ticksAbove) {
                            int linePos = startPos + ((endPos - startPos) * i) / (nticks - 1);
                            painter->drawLine(linePos, slider->rect.top() + 2 - ((i == 0 || i == nticks-1) ? 1 : 0),
                                              linePos, slider->rect.top() + 2 + tickSize - 2);
                        }
                        if (ticksBelow) {
                            int linePos = startPos + ((endPos - startPos) * i) / (nticks - 1);
                            painter->drawLine(linePos, slider->rect.bottom() - 2 + ((i == 0 || i == nticks-1) ? 1 : 0),
                                              linePos, slider->rect.bottom() - 2 - tickSize + 2);
                        }
                    }
                } else {
                    int startPos = slider->rect.top() + 5;
                    int endPos = slider->rect.bottom() - 5;
                    for (int i = 0; i < nticks; ++i) {
                        if (ticksAbove) {
                            int linePos = startPos + ((endPos - startPos) * i) / (nticks - 1);
                            painter->drawLine(slider->rect.left() + 2 - ((i == 0 || i == nticks - 1) ? 1 : 0), linePos,
                                              slider->rect.left() + 2 + tickSize - 2, linePos);
                        }
                        if (ticksBelow) {
                            int linePos = startPos + ((endPos - startPos) * i) / (nticks - 1);
                            painter->drawLine(slider->rect.right() - 2 + ((i == 0 || i == nticks - 1) ? 1 : 0), linePos,
                                              slider->rect.right() - 2 - tickSize + 2, linePos);
                        }
                    }
                }
                painter->setPen(oldPen);
            }
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect rect = scrollBar->rect;
            QRect scrollBarSubLine = subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
            QRect scrollBarAddLine = subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
            QRect scrollBarSlider = subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);

            bool isEnabled = scrollBar->state & State_Enabled;
            bool reverse = scrollBar->direction == Qt::RightToLeft;
            bool horizontal = scrollBar->orientation == Qt::Horizontal;
            bool sunken = scrollBar->state & State_Sunken;

            QPixmap cache;

            // The groove
            if (scrollBar->subControls & SC_ScrollBarGroove) {
                QString groovePixmapName = uniqueName("scrollbar_groove", option, rect.size());
                if (!UsePixmapCache || !QPixmapCache::find(groovePixmapName, cache)) {
                    QRect grooveRect = rect;
                    cache = QPixmap(grooveRect.size());
                    QPainter groovePainter(&cache);

                    QRect pixmapRect = QRect(0, 0, grooveRect.width(), grooveRect.height());
                    groovePainter.fillRect(pixmapRect, option->palette.background());
                    groovePainter.fillRect(pixmapRect, QBrush(option->palette.base().color(), Qt::Dense4Pattern));

                    if (horizontal) {
                        groovePainter.setBrushOrigin(QPoint(pixmapRect.left(), pixmapRect.top()));
                        pixmapRect.setHeight(1);
                        pixmapRect.moveTop(rect.top());
                    } else {
                        groovePainter.setBrushOrigin(QPoint(pixmapRect.left(), pixmapRect.top()));
                        pixmapRect.setWidth(1);
                        pixmapRect.moveLeft(rect.left());
                    }
                    groovePainter.fillRect(pixmapRect,
                                           QBrush(scrollBar->palette.base().color().dark(115), Qt::Dense4Pattern));
                    if (UsePixmapCache)
                        QPixmapCache::insert(groovePixmapName, cache);
                }
                painter->drawPixmap(QPoint(rect.left(), rect.top()), cache);
            }

            // The SubLine (up/left) buttons
            if (scrollBar->subControls & SC_ScrollBarSubLine) {
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

                if (!UsePixmapCache || !QPixmapCache::find(subLinePixmapName, cache)) {
                    cache = QPixmap(button1.size());
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
                        arrow.setColor(1, scrollBar->palette.text().color().rgba());

                        if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                            subLinePainter.drawImage(QPoint(pixmapRect.left() + 6, pixmapRect.top() + 5), arrow);
                        } else {
                            subLinePainter.drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 4), arrow);
                        }
                    } else {
                        QImage arrow(qt_scrollbar_button_arrow_up);
                        arrow.setColor(1, scrollBar->palette.text().color().rgba());

                        if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                            subLinePainter.drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 7), arrow);
                        } else {
                            subLinePainter.drawImage(QPoint(pixmapRect.left() + 4, pixmapRect.top() + 6), arrow);
                        }
                    }
                    if (UsePixmapCache)
                        QPixmapCache::insert(subLinePixmapName, cache);
                }
                painter->drawPixmap(button1.topLeft(), cache);
                painter->drawPixmap(button2.topLeft(), cache);
            }

            // The AddLine (down/right) button
            if (scrollBar->subControls & SC_ScrollBarAddLine) {
                QString addLinePixmapName = uniqueName("scrollbar_addline", option, QSize(16, 16));

                if (!UsePixmapCache || !QPixmapCache::find(addLinePixmapName, cache)) {
                    cache = QPixmap(scrollBarAddLine.size());
                    QRect pixmapRect(0, 0, cache.width(), cache.height());
                    QPainter addLinePainter(&cache);
                    addLinePainter.fillRect(pixmapRect, option->palette.background());

                    if (isEnabled) {
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
                        arrow.setColor(1, scrollBar->palette.text().color().rgba());

                        if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                            addLinePainter.drawImage(QPoint(pixmapRect.left() + 7, pixmapRect.top() + 5), arrow);
                        } else {
                            addLinePainter.drawImage(QPoint(pixmapRect.left() + 6, pixmapRect.top() + 4), arrow);
                        }
                    } else {
                        QImage arrow(qt_scrollbar_button_arrow_down);
                        arrow.setColor(1, scrollBar->palette.text().color().rgba());

                        if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                            addLinePainter.drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 7), arrow);
                        } else {
                            addLinePainter.drawImage(QPoint(pixmapRect.left() + 4, pixmapRect.top() + 6), arrow);
                        }
                    }
                    if (UsePixmapCache)
                        QPixmapCache::insert(addLinePixmapName, cache);
                }
                painter->drawPixmap(scrollBarAddLine.topLeft(), cache);
            }

            // The slider
            if ((scrollBar->subControls & SC_ScrollBarSlider) && scrollBarSlider.isValid()) {
                QString sliderPixmapName = uniqueName("scrollbar_slider", option, scrollBarSlider.size());
                if (!UsePixmapCache || !QPixmapCache::find(sliderPixmapName, cache)) {
                    cache = QPixmap(scrollBarSlider.size());
                    QRect pixmapRect(0, 0, cache.width(), cache.height());
                    QPainter sliderPainter(&cache);

                    if (isEnabled) {
                        QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                                 pixmapRect.center().x(), pixmapRect.bottom());
                        gradient.setColorAt(0, gradientStartColor.light(105));
                        gradient.setColorAt(1, gradientStopColor);
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

                    sliderPainter.setPen(gradientStartColor.light(105));
                    sliderPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 1,
                                           pixmapRect.right() - 1, pixmapRect.top() + 1);
                    sliderPainter.drawLine(pixmapRect.left() + 1, pixmapRect.top() + 2,
                                           pixmapRect.left() + 1, pixmapRect.bottom() - 2);

                    sliderPainter.setPen(gradientStopColor);
                    sliderPainter.drawLine(pixmapRect.left() + 1, pixmapRect.bottom() - 1,
                                           pixmapRect.right() - 1, pixmapRect.bottom() - 1);
                    sliderPainter.drawLine(pixmapRect.right() - 1, pixmapRect.top() + 2,
                                           pixmapRect.right() - 1, pixmapRect.bottom() - 1);

                    int sliderMinLength = pixelMetric(PM_ScrollBarSliderMin, scrollBar, widget);
                    if ((horizontal && scrollBar->rect.width() > (16 * 3 + sliderMinLength))
                        || (!horizontal && scrollBar->rect.height() > (16 * 3 + sliderMinLength))) {
                        QImage pattern(qt_scrollbar_slider_pattern);
                        pattern.setColor(1, alphaCornerColor.rgba());
                        pattern.setColor(2, gradientStartColor.light(105).rgba());

                        if (horizontal) {
                            sliderPainter.drawImage(pixmapRect.center().x() - pattern.width() / 2 + 1,
                                                    pixmapRect.center().y() - 4,
                                                    pattern);
                        } else {
                            sliderPainter.drawImage(pixmapRect.center().x() - 4,
                                                    pixmapRect.center().y() - pattern.width() / 2 + 1,
                                                    pattern);
                        }
                    }

                    // insert the slider into the cache
                    if (UsePixmapCache)
                        QPixmapCache::insert(sliderPixmapName, cache);
                }
                painter->drawPixmap(scrollBarSlider.topLeft(), cache);
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            QPixmap cache;
            QString pixmapName = uniqueName("spinbox", spinBox, spinBox->rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(spinBox->rect.size());
                QRect pixmapRect(0, 0, spinBox->rect.width(), spinBox->rect.height());
                QPainter cachePainter(&cache);

                bool isEnabled = (spinBox->state & State_Enabled);
                bool focus = isEnabled && (spinBox->state & State_HasFocus);

                QRect rect = pixmapRect;

                cachePainter.fillRect(rect, option->palette.base());

                // Draw a line edit
                QStyleOptionFrame lineEdit;
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
                    if (spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken)) {
                        cachePainter.fillRect(upRect.adjusted(1, 1, -1, 0), gradientStopColor);
                    } else {
                        if (focus) {
                            qt_plastique_draw_gradient(&cachePainter, upRect.adjusted(1, 1, -1, -1),
                                                       highlightedGradientStartColor,
                                                       highlightedGradientStopColor);
                        } else {
                            qt_plastique_draw_gradient(&cachePainter, upRect.adjusted(1, 1, -1, -1),
                                                       gradientStartColor,
                                                       gradientStopColor);
                        }
                    }

                    if (spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken)) {
                        cachePainter.fillRect(downRect.adjusted(1, 0, -1, -1), gradientStopColor);
                    } else {
                        if (focus) {
                            qt_plastique_draw_gradient(&cachePainter, downRect.adjusted(1, 0, -1, -1),
                                                       highlightedGradientStartColor,
                                                       highlightedGradientStopColor);
                        } else {
                            qt_plastique_draw_gradient(&cachePainter, downRect.adjusted(1, 0, -1, -1),
                                                       gradientStartColor,
                                                       gradientStopColor);
                        }
                    }
                }

                // outline the up/down buttons
                if (!focus) {
                    cachePainter.setPen(borderColor);
                    if (spinBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                        cachePainter.drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                        cachePainter.drawLine(upRect.right(), upRect.bottom(), upRect.left(), upRect.bottom());
                    } else {
                        cachePainter.drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                        cachePainter.drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                        cachePainter.drawLine(upRect.left(), upRect.bottom(), upRect.right(), upRect.bottom());
                    }
                } else {
                    cachePainter.setPen(option->palette.highlight().color().dark(130));
                    if (spinBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(upRect.right(), upRect.top() + 1, upRect.right(), downRect.bottom() - 2);
                        cachePainter.drawLine(upRect.right() - 1, upRect.top() + 1, upRect.left() + 2, upRect.top() + 1);
                        cachePainter.drawLine(upRect.right() - 1, upRect.top() + 1, upRect.left() + 2, upRect.top() + 1);
                        cachePainter.drawLine(upRect.right() - 1, upRect.bottom(), upRect.left() + 2, upRect.bottom());

                        cachePainter.setPen(highlightedDarkInnerBorderColor);
                        cachePainter.drawLine(upRect.right() + 1, upRect.top() + 2, upRect.right() + 1, downRect.bottom() - 2);

                        cachePainter.setPen(option->palette.highlight().color().light(101));
                        cachePainter.drawLine(upRect.left() + 1, upRect.top() + 2, upRect.left() + 1, downRect.bottom() - 2);
                        cachePainter.drawLine(downRect.right() - 1, downRect.bottom() - 1, downRect.left() + 2, downRect.bottom() - 1);
                    } else {
                        cachePainter.drawLine(upRect.left(), upRect.top() + 1, upRect.left(), downRect.bottom() - 2);
                        cachePainter.drawLine(upRect.left() + 1, upRect.top() + 1, upRect.right() - 2, upRect.top() + 1);
                        cachePainter.drawLine(upRect.left() + 1, upRect.top() + 1, upRect.right() - 2, upRect.top() + 1);
                        cachePainter.drawLine(upRect.left() + 1, upRect.bottom(), upRect.right() - 2, upRect.bottom());

                        cachePainter.setPen(highlightedDarkInnerBorderColor);
                        cachePainter.drawLine(upRect.left() - 1, upRect.top() + 2, upRect.left() - 1, downRect.bottom() - 2);

                        cachePainter.setPen(option->palette.highlight().color().light(101));
                        cachePainter.drawLine(upRect.right() - 1, upRect.top() + 2, upRect.right() - 1, downRect.bottom() - 2);
                        cachePainter.drawLine(downRect.left() + 1, downRect.bottom() - 1, downRect.right() - 2, downRect.bottom() - 1);
                    }
                }

                // finish alpha corners
                cachePainter.setPen(focus ? highlightedDarkInnerBorderColor : borderColor);

                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawPoint(rect.left() + 1, rect.top() + 1);
                    cachePainter.drawPoint(rect.left() + 1, rect.bottom() - 1);
                } else {
                    cachePainter.drawPoint(rect.right() - 1, rect.top() + 1);
                    cachePainter.drawPoint(rect.right() - 1, rect.bottom() - 1);
                }

                if (spinBox->buttonSymbols == QAbstractSpinBox::PlusMinus) {
                    int centerX = upRect.center().x();
                    int centerY = upRect.center().y();
                    cachePainter.setPen(spinBox->palette.text().color());

                    // plus/minus
                    if (spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken)) {
                        cachePainter.drawLine(1 + centerX - 2, 1 + centerY, 1 + centerX + 2, 1 + centerY);
                        cachePainter.drawLine(1 + centerX, 1 + centerY - 2, 1 + centerX, 1 + centerY + 2);
                    } else {
                        cachePainter.drawLine(centerX - 2, centerY, centerX + 2, centerY);
                        cachePainter.drawLine(centerX, centerY - 2, centerX, centerY + 2);
                    }

                    centerX = downRect.center().x();
                    centerY = downRect.center().y();
                    if (spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken)) {
                        cachePainter.drawLine(1 + centerX - 2, 1 + centerY, 1 + centerX + 2, 1 + centerY);
                    } else {
                        cachePainter.drawLine(centerX - 2, centerY, centerX + 2, centerY);
                    }
                } else {
                    // arrows
                    QImage upArrow(qt_scrollbar_button_arrow_up);
                    upArrow.setColor(1, spinBox->palette.text().color().rgba());
                    if (spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken)) {
                        cachePainter.drawImage(1 + upRect.center().x() - upArrow.width() / 2,
                                               2 + upRect.center().y() - upArrow.height() / 2,
                                               upArrow);
                    } else {
                        cachePainter.drawImage(upRect.center().x() - upArrow.width() / 2,
                                               1+ upRect.center().y() - upArrow.height() / 2,
                                               upArrow);
                    }
                    QImage downArrow(qt_scrollbar_button_arrow_down);
                    downArrow.setColor(1, spinBox->palette.text().color().rgba());
                    if (spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken)) {
                        cachePainter.drawImage(1 + downRect.center().x() - downArrow.width() / 2,
                                               1 + downRect.center().y() - downArrow.height() / 2,
                                               downArrow);
                    } else {
                        cachePainter.drawImage(downRect.center().x() - downArrow.width() / 2,
                                               downRect.center().y() - downArrow.height() / 2,
                                               downArrow);
                    }
                }
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(spinBox->rect.topLeft(), cache);
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {

            const QComboBox *box = qobject_cast<const QComboBox *>(widget);
            bool sunken = (box && box->view()->isVisible());
            bool isEnabled = (comboBox->state & State_Enabled);
            bool focus = isEnabled && (comboBox->state & State_HasFocus);

            QPixmap cache;
            QString pixmapName = uniqueName("combobox", option, comboBox->rect.size());
            if (sunken)
                pixmapName += "-sunken";
            if (comboBox->editable)
                pixmapName += "-editable";

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(comboBox->rect.size());
                QPainter cachePainter(&cache);
                QRect pixmapRect(0, 0, comboBox->rect.width(), comboBox->rect.height());
                cachePainter.fillRect(pixmapRect, painter->brush());

                QStyleOptionComboBox comboBoxCopy = *comboBox;
                comboBoxCopy.rect = pixmapRect;                
                
                QRect rect = pixmapRect;
                QRect downArrowRect = subControlRect(CC_ComboBox, &comboBoxCopy, SC_ComboBoxArrow, widget);
                if (comboBox->direction == Qt::RightToLeft)
                    downArrowRect.setRect(downArrowRect.left() + 1, downArrowRect.top(), 16, downArrowRect.height());
                else
                    downArrowRect.setRect(downArrowRect.right() - 16, downArrowRect.top(), 16, downArrowRect.height());

                // Draw a push button
                if (comboBox->editable) {
                    // Draw a line edit
                    QStyleOptionFrame lineEdit;
                    lineEdit.rect = pixmapRect;
                    lineEdit.state = comboBox->state;
                    lineEdit.state |= State_Sunken;
                    drawPrimitive(PE_FrameLineEdit, &lineEdit, &cachePainter, widget);

                    // Add the button
                    cachePainter.setPen(sunken ? gradientStopColor.dark(110) : gradientStartColor);
                    cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.top() + 1,
                                          downArrowRect.right() - 1, downArrowRect.top() + 1);
                    cachePainter.setPen(sunken ? gradientStopColor.light(105)
                                        : option->palette.button().color().light(91));
                    cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.bottom() - 1,
                                          downArrowRect.right() - 1, downArrowRect.bottom() - 1);

                    if (isEnabled) {
                        QRect downArrowGradientRect = downArrowRect.adjusted(1, 2, -1, -2);
                        if (sunken) {
                            cachePainter.fillRect(downArrowGradientRect, gradientStopColor);
                        } else {
                            if (focus) {
                                qt_plastique_draw_gradient(&cachePainter, downArrowGradientRect,
                                                           highlightedGradientStartColor,
                                                           highlightedGradientStopColor);
                            } else {
                                qt_plastique_draw_gradient(&cachePainter, downArrowGradientRect,
                                                           gradientStartColor,
                                                           gradientStopColor);
                            }
                        }
                    }

                    cachePainter.setPen(sunken ? gradientStopColor.light(105)
                                        : option->palette.button().color().light(91));
                    if (comboBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 2,
                                              downArrowRect.left(), downArrowRect.bottom() - 2);
                    } else {
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 2,
                                              downArrowRect.right(), downArrowRect.bottom() - 2);
                    }
                    
                } else {
                    QStyleOptionButton buttonOption;
                    buttonOption.rect = rect;
                    buttonOption.state = comboBox->state & (State_Enabled | State_MouseOver);
                    if (sunken) {
                        buttonOption.state |= State_Sunken;
                        buttonOption.state &= ~State_MouseOver;
                    }
                    drawPrimitive(PE_PanelButtonCommand, &buttonOption, &cachePainter, widget);
                }

                // In editable mode, outline the button if we have input focus
                if (focus && comboBox->editable) {
                    cachePainter.setPen(option->palette.highlight().color().dark(130));
                    if (comboBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 1,
                                              downArrowRect.right(), downArrowRect.bottom() - 1);
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 2,
                                              downArrowRect.left(), downArrowRect.bottom() - 2);
                    } else {
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 1,
                                              downArrowRect.left(), downArrowRect.bottom() - 1);
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 2,
                                              downArrowRect.right(), downArrowRect.bottom() - 2);
                    }
                    cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.top() + 1,
                                          downArrowRect.right() - 1, downArrowRect.top() + 1);
                    cachePainter.setPen(highlightedDarkInnerBorderColor);
                    if (comboBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(downArrowRect.right() + 1, downArrowRect.top() + 2,
                                              downArrowRect.right() + 1, downArrowRect.bottom() - 2);
                    } else {
                        cachePainter.drawLine(downArrowRect.left() - 1, downArrowRect.top() + 2,
                                              downArrowRect.left() - 1, downArrowRect.bottom() - 2);
                    }
                    cachePainter.setPen(option->palette.highlight().color().light(101));
                    cachePainter.drawLine(downArrowRect.left() + 1, downArrowRect.bottom() - 1,
                                          downArrowRect.right() - 1, downArrowRect.bottom() - 1);
                } else {
                    cachePainter.setPen(borderColor);
                    if (comboBox->direction == Qt::RightToLeft) {
                        cachePainter.drawLine(downArrowRect.right(), downArrowRect.top() + 1,
                                              downArrowRect.right(), downArrowRect.bottom() - 1);
                    } else {
                        cachePainter.drawLine(downArrowRect.left(), downArrowRect.top() + 1,
                                              downArrowRect.left(), downArrowRect.bottom() - 1);
                    }
                }

                // Draw the little arrow
                QImage downArrow(qt_scrollbar_button_arrow_down);
                downArrow.setColor(1, comboBox->palette.text().color().rgba());
                if (sunken)
                    downArrowRect = downArrowRect.adjusted(1, 1, 1, 1);
                cachePainter.drawImage(downArrowRect.center().x() - downArrow.width() / 2,
                                       downArrowRect.center().y() - downArrow.height() / 2, downArrow);

                // Draw the focus rect
                if ((focus && (option->state & State_KeyboardFocusChange)) && !comboBox->editable) {
                    QStyleOptionFocusRect focus;
                    focus.rect = subControlRect(CC_ComboBox, &comboBoxCopy, SC_ComboBoxEditField, widget)
                                 .adjusted(0, 0, option->direction == Qt::RightToLeft ? 1 : -1, 0);
                    drawPrimitive(PE_FrameFocusRect, &focus, &cachePainter, widget);
                }
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(comboBox->rect.topLeft(), cache);

        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
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
            painter->drawText(textRect.adjusted(1, 1, 1, 1), titleBar->text, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
            painter->setPen(titleBar->palette.highlightedText().color());
            painter->drawText(textRect, titleBar->text, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));

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

            if ((titleBar->subControls & SC_TitleBarNormalButton) && (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint)) {
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

            if (titleBar->subControls & SC_TitleBarContextHelpButton) {
                // ### add context help icon
                bool hover = (titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_MouseOver);
		QRect contextHelpButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarContextHelpButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, contextHelpButtonRect, hover);
            }

            if (titleBar->subControls & SC_TitleBarShadeButton) {
                // ### add shade icon
                bool hover = (titleBar->activeSubControls & SC_TitleBarShadeButton) && (titleBar->state & State_MouseOver);
		QRect shadeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarShadeButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, shadeButtonRect, hover);
            }

            if (titleBar->subControls & SC_TitleBarUnshadeButton) {
                // ### add unshade icon
                bool hover = (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_MouseOver);
		QRect unshadeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarUnshadeButton, widget);
                qt_plastique_draw_mdibutton(painter, titleBar, unshadeButtonRect, hover);
            }

	    // from qwindowsstyle.cpp
            if ((titleBar->subControls & SC_TitleBarSysMenu) && (titleBar->titleBarFlags & Qt::WindowSystemMenuHint)) {

                bool hover = (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_MouseOver);
		QRect iconRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarSysMenu, widget);
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
        break;
    case CT_RadioButton:
        ++newSize.rheight();
        ++newSize.rwidth();
        break;
    case CT_LineEdit:
        newSize.rheight() += 4;
        break;
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
    case CT_SpinBox:
        newSize.rheight() -= 2;
        break;
    case CT_ToolButton:
        newSize.rwidth() += 3;
        newSize.rheight() += 3;
        break;
    case CT_ComboBox:
        ++newSize.rheight();
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
    QRect rect = visualRect(option->direction, option->rect,
                            QWindowsStyle::subElementRect(element, option, widget));

    switch (element) {
    case SE_RadioButtonIndicator:
        rect = rect.adjusted(0, 0, 1, 1);
        break;
    case SE_ProgressBarGroove:
        rect = widget->rect();
        break;
    case SE_ProgressBarContents:
        rect = widget->rect();
        break;
    case SE_ProgressBarLabel:
        rect.moveLeft(widget->rect().center().x() - rect.width() / 2);
        break;
    default:
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
                    rect.setHeight(4);
                    --grooveCenter.ry();
                    if (slider->tickPosition & QSlider::TicksAbove)
                        grooveCenter.ry() += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        grooveCenter.ry() -= tickSize;
                } else {
                    rect.setWidth(4);
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
                rect.setRect(spinBox->rect.right() - 16, center + 1, 17, spinBox->rect.height() - (center + 1));
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
    case CC_ComboBox:
        switch (subControl) {
        case SC_ComboBoxArrow:
            rect = option->rect;
            break;
        case SC_ComboBoxEditField: {
            int frameWidth = pixelMetric(PM_DefaultFrameWidth);
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(option->rect.left() + frameWidth, option->rect.top() + frameWidth,
                         option->rect.width() - 16 - 2 * frameWidth + 1,
                         option->rect.height() - 2 * frameWidth);
            rect = visualRect(option->direction, option->rect, rect);
            break;
        }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {

            const bool isToolTitle = false; // widget->testWFlags(Qt::WA_WState_Tool)
            const int height = titleBar->rect.height() - 2;
            const int width = titleBar->rect.width() - 1;
            const int controlTop = 4; //widget->testWFlags(Qt::WA_WState_Tool) ? 4 : 6;
            const int controlHeight = height - controlTop - 1;

            const bool sysmenuHint  = (titleBar->titleBarFlags & Qt::WindowSystemMenuHint) != 0;
            const bool minimizeHint = (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint) != 0;
            const bool maximizeHint = (titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint) != 0;
            const bool contextHint = (titleBar->titleBarFlags & Qt::WindowContextHelpButtonHint) != 0;
            const bool shadeHint = (titleBar->titleBarFlags & Qt::WindowShadeButtonHint) != 0;

            switch (subControl) {
            case SC_TitleBarLabel: {
                QRect ir(0, 0, width, height);
                if (isToolTitle) {
                    if (sysmenuHint)
                        ir.adjust(0, 0, -controlHeight-6, 0);
                    if (minimizeHint || maximizeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                } else {
                    if (sysmenuHint)
                        ir.adjust(controlHeight+6, 0, -controlHeight-6, 0);
                    if (minimizeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (maximizeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (contextHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (shadeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                }
                return ir;
            }
            case SC_TitleBarCloseButton:
                return QRect(width - (controlHeight + 1) - controlTop,
                            controlTop, controlHeight, controlHeight);
            case SC_TitleBarShadeButton:
                if (!(titleBar->titleBarFlags & Qt::WindowShadeButtonHint))
                    return QRect();
                return QRect(width - ((controlHeight + 1) * 2) - controlTop,
                            controlTop, controlHeight, controlHeight);
            case SC_TitleBarMaxButton:
            case SC_TitleBarUnshadeButton:
                return QRect(width - ((controlHeight + 1) * 2) - controlTop,
                            controlTop, controlHeight, controlHeight);
            case SC_TitleBarMinButton: {
                if (!(titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint))
                    return QRect();
                int offset = controlHeight + 1;
                if (!maximizeHint)
                    offset *= 2;
                else
                    offset *= 3;
                return QRect(width - offset - controlTop,
                            controlTop, controlHeight, controlHeight);
            }
            case SC_TitleBarNormalButton: {
                int offset = controlHeight + 1;
                if (!maximizeHint)
                    offset *= 2;
                else
                    offset *= 3;
                return QRect(width - offset - controlTop,
                            controlTop, controlHeight, controlHeight);
            }
            case SC_TitleBarSysMenu: {
                QSize iconSize = titleBar->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).size();
                if (titleBar->icon.isNull())
                    iconSize = QSize(controlHeight, controlHeight);
                int hPad = (controlHeight - iconSize.height())/2;
                int vPad = (controlHeight - iconSize.width())/2;
                QRect ret = QRect(6 + vPad, controlTop + hPad, iconSize.width(), controlHeight - hPad);
                return ret;
            }
            default:
                break;
            }
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
        ret = true;
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
    switch (control) {
    case CC_ComboBox:
        return SC_ComboBoxArrow;
   case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect slider = subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);
            if (slider.contains(pos))
                return SC_ScrollBarSlider;

            QRect scrollBarAddLine = subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
            if (scrollBarAddLine.contains(pos))
                return SC_ScrollBarAddLine;

            QRect scrollBarSubPage = subControlRect(control, scrollBar, SC_ScrollBarSubPage, widget);
            if (scrollBarSubPage.contains(pos))
                return SC_ScrollBarSubPage;

            QRect scrollBarAddPage = subControlRect(control, scrollBar, SC_ScrollBarAddPage, widget);
            if (scrollBarAddPage.contains(pos))
                return SC_ScrollBarAddPage;

            QRect scrollBarSubLine = subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
            if (scrollBarSubLine.contains(pos))
                return SC_ScrollBarSubLine;
        }
        break;
    default:
        break;
    }

    return QWindowsStyle::hitTestComplexControl(control, option, pos, widget);
}

/*!
  \reimp
*/
int QPlastiqueStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        return 1;
    case PM_ButtonDefaultIndicator:
        return 0;
    case PM_SliderThickness:
        return 15;
    case PM_SliderLength:
    case PM_SliderControlThickness:
        return 11;
    case PM_SliderTickmarkOffset:
        return 5;
    case PM_SliderSpaceAvailable:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int size = 15;
            if (slider->tickPosition & QSlider::TicksBelow)
                ++size;
            if (slider->tickPosition & QSlider::TicksAbove)
                ++size;
            return size;
        }
    case PM_ScrollBarExtent:
        return 16;
    case PM_ScrollBarSliderMin:
        return 22;
    case PM_ProgressBarChunkWidth:
        return 1;
    case PM_MenuBarItemSpacing:
        return 3;
    case PM_MenuBarVMargin:
        return 3;
    case PM_MenuBarHMargin:
        return 0;
    case PM_MenuBarPanelWidth:
        return 1;
    case PM_ToolBarHandleExtent:
        return 9;
    case PM_ToolBarSeparatorExtent:
        return 2;
    case PM_ToolBarFrameWidth:
        return 2;
    case PM_SplitterWidth:
        return 6;
    case PM_DockWidgetSeparatorExtent:
        return 6;
    case PM_DockWidgetHandleExtent:
        return 20;
    case PM_DefaultFrameWidth:
        if (qobject_cast<const QMenu *>(widget))
            return 1;
        return 2;
    case PM_TabBarTabVSpace:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (!tab->icon.isNull()) return 15;
        }
        break;
    case PM_MDIFrameWidth:
        return 4;
    case PM_TitleBarHeight:
        return qMax(widget ? widget->fontMetrics().lineSpacing() : 0, 30);
    case PM_DefaultChildMargin:
        return 0;
        break;
    default:
        break;
    }

    return QWindowsStyle::pixelMetric(metric, option, widget);
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
        || qobject_cast<QComboBox *>(widget)
        || qobject_cast<QCheckBox *>(widget)
        || qobject_cast<QRadioButton *>(widget)
        || qobject_cast<QTabBar *>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (widget->inherits("QWorkspaceTitleBar"))
        widget->setAttribute(Qt::WA_Hover);

    QToolBox *box = qobject_cast<QToolBox *>(widget);
    if (box)
        box->setBackgroundRole(QPalette::Background);
}

/*!
  \reimp
*/
void QPlastiqueStyle::unpolish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget)
        || qobject_cast<QComboBox *>(widget)
        || qobject_cast<QCheckBox *>(widget)
        || qobject_cast<QRadioButton *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    if (widget->inherits("QWorkspaceTitleBar"))
        widget->setAttribute(Qt::WA_Hover, false);
    else if (qobject_cast<QTabBar *>(widget))
        widget->setAttribute(Qt::WA_Hover, false);

    QToolBox *box = qobject_cast<QToolBox *>(widget);
    if (box)
        box->setBackgroundRole(QPalette::Button);
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

void QPlastiqueStyle::unpolish(QApplication *app)
{
    QWindowsStyle::unpolish(app);
}
