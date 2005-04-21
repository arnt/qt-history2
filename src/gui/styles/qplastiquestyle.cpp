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

#include <qdebug.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qabstractitemview.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstyleoption.h>

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


static const char * const qt_toolbarhandle_horizontal[] = {
    "6 27 4 1",
    "       c None",
    ".      c #C5C5C5",
    "+      c #EEEEEE",
    "@      c #FAFAFA",
    "..    ",
    ".+@   ",
    " @@   ",
    "   .. ",
    "   .+@",
    "    @@",
    "..    ",
    ".+@   ",
    " @@   ",
    "   .. ",
    "   .+@",
    "    @@",
    "..    ",
    ".+@   ",
    " @@   ",
    "   .. ",
    "   .+@",
    "    @@",
    "..    ",
    ".+@   ",
    " @@   ",
    "   .. ",
    "   .+@",
    "    @@",
    "..    ",
    ".+@   ",
    "+@@   "};

static QColor qt_plastique_mergedColors(const QColor &colorA, const QColor &colorB)
{
    QColor tmp = colorA;
    tmp.setRed((tmp.red() + colorB.red()) / 2);
    tmp.setGreen((tmp.green() + colorB.green()) / 2);
    tmp.setBlue((tmp.blue() + colorB.blue()) / 2);
    return tmp;
}

static void qt_plastique_drawShadedPanel(QPainter *painter, const QStyleOption *option, bool base = false)
{
    QRect rect = option->rect;
    QPen oldPen = painter->pen();
    
    QColor borderColor = option->palette.button().color().dark(162);
    QColor gradientStartColor = option->palette.button().color().dark(90);
    QColor gradientStopColor = option->palette.button().color().dark(108);

    QColor alphaCornerColor = option->palette.background().color();
    alphaCornerColor.setRed((alphaCornerColor.red() + borderColor.red()) / 2);
    alphaCornerColor.setGreen((alphaCornerColor.green() + borderColor.green()) / 2);
    alphaCornerColor.setBlue((alphaCornerColor.blue() + borderColor.blue()) / 2);

    // outline / border
    painter->setPen(borderColor);
    painter->drawLine(rect.left() + 1, rect.top(), rect.right() - 1, rect.top());
    painter->drawLine(rect.left() + 1, rect.bottom(), rect.right() - 1, rect.bottom());
    painter->drawLine(rect.left(), rect.top() + 1, rect.left(), rect.bottom() - 1);
    painter->drawLine(rect.right(), rect.top() + 1, rect.right(), rect.bottom() - 1);

    // gradient fill
    QLinearGradient gradient(rect.center().x(), rect.top() + 1, rect.center().x(), rect.bottom() - 2);
    if (option->state & QStyle::State_Sunken) {
        gradient.setColorAt(0, option->palette.button().color().dark(111));
        gradient.setColorAt(1, option->palette.button().color().dark(106));
    } else {
        gradient.setColorAt(0, base ? option->palette.background().color().light(105) : gradientStartColor);
        gradient.setColorAt(1, base ? option->palette.background().color().dark(102) : gradientStopColor);
    }
    painter->fillRect(rect.adjusted(1, 1, -2, -2), gradient);

    // inner border
    painter->setPen(gradientStopColor.dark(102));
    painter->drawLine(rect.left() + 1, rect.bottom() - 1, rect.right() - 1, rect.bottom() - 1);
    painter->drawLine(rect.right() - 1, rect.top() + 2, rect.right() - 1, rect.bottom() - 2);
    painter->setPen(gradientStartColor);
    painter->drawLine(rect.left() + 1, rect.top() + 1, rect.left() + 1, option->rect.bottom() - 2);

    painter->setPen(alphaCornerColor);
    painter->drawPoint(rect.topLeft());
    painter->drawPoint(rect.topRight());
    painter->drawPoint(rect.bottomLeft());
    painter->drawPoint(rect.bottomRight());
    
    painter->setPen(oldPen);
}

QPlastiqueStyle::QPlastiqueStyle()
    : QWindowsStyle()
{
}

void QPlastiqueStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                    QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);

    QColor borderColor = option->palette.button().color().dark(162);

    QColor gradientStartColor = option->palette.button().color().dark(90);
    QColor gradientStopColor = option->palette.button().color().dark(108);
    QColor highlightedGradientStartColor = option->palette.highlight().color().light(200);
    QColor highlightedGradientStopColor = option->palette.highlight().color().light(190);
    QColor highlightedDarkInnerBorderColor = option->palette.highlight().color().light(143);
    QColor highlightedLightInnerBorderColor = option->palette.highlight().color().light(163);

    QColor alphaCornerColor = option->palette.background().color();
    alphaCornerColor.setRed((alphaCornerColor.red() + borderColor.red()) / 2);
    alphaCornerColor.setGreen((alphaCornerColor.green() + borderColor.green()) / 2);
    alphaCornerColor.setBlue((alphaCornerColor.blue() + borderColor.blue()) / 2);

    QColor alphaInnerColor = highlightedLightInnerBorderColor;
    alphaInnerColor.setRed((alphaInnerColor.red() + gradientStartColor.red()) / 2);
    alphaInnerColor.setGreen((alphaInnerColor.green() + gradientStartColor.green()) / 2);
    alphaInnerColor.setBlue((alphaInnerColor.blue() + gradientStartColor.blue()) / 2);  

    QColor alphaInnerColorNoHover = borderColor;
    alphaInnerColorNoHover.setRed((alphaInnerColorNoHover.red() + gradientStartColor.red()) / 2);
    alphaInnerColorNoHover.setGreen((alphaInnerColorNoHover.green() + gradientStartColor.green()) / 2);
    alphaInnerColorNoHover.setBlue((alphaInnerColorNoHover.blue() + gradientStartColor.blue()) / 2);

    QColor alphaTextColor = option->palette.background().color();
    alphaTextColor.setRed((alphaTextColor.red() + option->palette.text().color().red()) / 2);
    alphaTextColor.setGreen((alphaTextColor.green() + option->palette.text().color().green()) / 2);
    alphaTextColor.setBlue((alphaTextColor.blue() + option->palette.text().color().blue()) / 2);

    QColor alphaLightTextColor = option->palette.background().color().light(250);
    alphaLightTextColor.setRed((alphaLightTextColor.red() + option->palette.text().color().light(250).red()) / 2);
    alphaLightTextColor.setGreen((alphaLightTextColor.green() + option->palette.text().color().light(250).green()) / 2);
    alphaLightTextColor.setBlue((alphaLightTextColor.blue() + option->palette.text().color().light(250).blue()) / 2);
    
    switch (element) {
    case PE_FrameDefaultButton:
        break;
    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            painter->save();
            QRegion region(twf->rect);
            region -= QRect(0, 0, twf->tabBarSize.width(), twf->tabBarSize.height());
            painter->setClipRegion(region);

            // Outer border
            QLine leftLine = QLine(twf->rect.topLeft(), twf->rect.bottomLeft() - QPoint(0, 2));
            QLine rightLine = QLine(twf->rect.topRight() + QPoint(0, 2), twf->rect.bottomRight() - QPoint(0, 2));
            QLine bottomLine = QLine(twf->rect.bottomLeft() + QPoint(2, 0), twf->rect.bottomRight() - QPoint(2, 0));
            QLine topLine = QLine(twf->rect.topLeft(), twf->rect.topRight() - QPoint(2, 0));
            
            painter->setPen(borderColor);
            painter->drawLine(topLine);

            painter->setPen(twf->palette.button().color().light());
            painter->drawLine(bottomLine);

            // Inner border
            QLine innerLeftLine = QLine(leftLine.p1() + QPoint(1, 0), leftLine.p2() + QPoint(1, 0));
            QLine innerRightLine = QLine(rightLine.p1() - QPoint(1, 0), rightLine.p2() - QPoint(1, 0));
            QLine innerBottomLine = QLine(bottomLine.p1() - QPoint(0, 1), bottomLine.p2() - QPoint(0, 1));
            QLine innerTopLine = QLine(twf->rect.topLeft() + QPoint(0, 1), twf->rect.topRight() + QPoint(-2, 1));

            // Rounded Corner 
            QPoint leftOuterCorner = QPoint(innerLeftLine.p2() + QPoint(0, 1));
            QPoint leftInnerCorner1 = QPoint(leftLine.p2() + QPoint(0, 1));
            QPoint leftInnerCorner2 = QPoint(bottomLine.p1() - QPoint(1, 0));
            QPoint rightOuterCorner = QPoint(innerRightLine.p2() + QPoint(0, 1));
            QPoint rightInnerCorner1 = QPoint(rightLine.p2() + QPoint(0, 1));
            QPoint rightInnerCorner2 = QPoint(bottomLine.p2() + QPoint(1, 0));
            QPoint rightTopOuterCorner = QPoint(innerRightLine.p1() - QPoint(0, 1));
            QPoint rightTopInnerCorner1 = QPoint(rightLine.p1() - QPoint(0, 1));
            QPoint rightTopInnerCorner2 = QPoint(topLine.p2() + QPoint(1, 0));

            painter->setPen(borderColor);
            painter->drawLine(leftLine);
            painter->drawLine(rightLine);
            painter->drawLine(bottomLine);
            painter->drawPoint(leftOuterCorner);
            painter->drawPoint(rightOuterCorner);
            painter->drawPoint(rightTopOuterCorner);

            painter->setPen(twf->palette.button().color().light());
            painter->drawLine(innerLeftLine);
            painter->drawLine(innerTopLine);

            painter->setPen(alphaCornerColor);
            painter->drawLine(innerRightLine);
            painter->drawLine(innerBottomLine);
            painter->drawPoint(leftInnerCorner1);
            painter->drawPoint(leftInnerCorner2);
            painter->drawPoint(rightInnerCorner1);
            painter->drawPoint(rightInnerCorner2);
            painter->drawPoint(rightTopInnerCorner1);
            painter->drawPoint(rightTopInnerCorner2);
            
            painter->restore();
        }
        break ;
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            painter->save();

            QRegion region(tbb->rect);
            region -= tbb->tabBarRect;
            painter->setClipRegion(region);

            QLine topLine = QLine(tbb->rect.bottomLeft() - QPoint(0, 1), tbb->rect.bottomRight() - QPoint(0, 1));
            QLine bottomLine = QLine(tbb->rect.bottomLeft(), tbb->rect.bottomRight());

            painter->setPen(borderColor);
            painter->drawLine(topLine);

            painter->setPen(tbb->palette.button().color().light());
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

            bool focus = element == PE_FrameLineEdit && (frame->state & State_Enabled) && (frame->state & State_HasFocus);
            bool groupbox = element == PE_FrameGroupBox;
            
            int lw = 1; int mlw = 1;

            // Don't show frames in tiny rects
            if (lw+mlw > frame->rect.width() || lw+mlw > frame->rect.height()) break ;

            // Outer border, left side and top side
            QColor color = focus ? highlightedDarkInnerBorderColor : borderColor.dark(112);
            painter->fillRect(QRect(frame->rect.left() + lw + mlw,frame->rect.top(),
                                    frame->rect.width() - lw*2 - mlw*2,lw),color); // top line
            painter->fillRect(QRect(frame->rect.left(), frame->rect.top() + lw + mlw, 
                                    lw, frame->rect.height() - lw*2 - mlw*2),color); // left line

            // Line ends
            QColor alphaLineEnds = qt_plastique_mergedColors(frame->palette.background().color(),
                                                             color);
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
                                        
            }
                                                   
            painter->restore();
        }
        break ;
    case PE_FrameMenu: {
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
        painter->save();
        painter->setPen(alphaCornerColor);
        painter->drawLine(option->rect.left() + 2, option->rect.bottom(),
                          option->rect.right() - 2, option->rect.bottom());
        painter->setPen(option->palette.background().color().light(104));
        painter->drawLine(option->rect.left() + 2, option->rect.top(),
                          option->rect.right() - 2, option->rect.top());
        painter->restore();
        break;
    }
    case PE_FrameButtonTool:
    case PE_PanelButtonTool:
        if ((option->state & State_Enabled) && (option->state & State_MouseOver))
            qt_plastique_drawShadedPanel(painter, option, true);
        else
            painter->fillRect(option->rect, option->palette.background());
        break;
    case PE_IndicatorToolBarHandle: {
        painter->save();

        QImage handle(qt_toolbarhandle_horizontal);
        handle.setColor(1, alphaCornerColor.rgba());
        handle.setColor(2, qt_plastique_mergedColors(alphaCornerColor, option->palette.base().color()).rgba());
        handle.setColor(3, option->palette.base().color().rgba());
        painter->drawImage(QPoint(option->rect.left() + 3, option->rect.top()), handle);
        
        painter->restore();
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
            painter->save();

            bool down = (button->state & State_Sunken) || (button->state & State_On);
            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);
            const QPushButton *pushButton = qobject_cast<const QPushButton *>(widget);
            bool isDefault = pushButton && pushButton->isDefault();
            
            // gradient fill            
            QLinearGradient gradient(QPointF(option->rect.center().x(), option->rect.top()),
                                     QPointF(option->rect.center().x(), option->rect.bottom()));
            if (down) {
                gradient.setColorAt(0, option->palette.button().color().dark(111));
                gradient.setColorAt(1, option->palette.button().color().dark(106));
            } else {
                if (hover) {
                    gradient.setColorAt(0, highlightedGradientStartColor);
                    gradient.setColorAt(1, highlightedGradientStopColor);
                } else {
                    gradient.setColorAt(0, gradientStartColor);
                    gradient.setColorAt(1, gradientStopColor);
                }
            }
            painter->fillRect(option->rect.adjusted(2, 2, -2, -2), QBrush(gradient));
            QPen oldPen = painter->pen();
            QRect rect = option->rect;

            if (isDefault) {               
                painter->setPen(borderColor.dark(105));
                painter->drawLine(rect.left() + 3, rect.top() + 1, rect.right() - 3, rect.top() + 1);
                painter->drawLine(rect.left() + 3, rect.bottom() - 1, rect.right() - 3, rect.bottom() - 1);
                painter->drawLine(rect.left() + 1, rect.top() + 3, rect.left() + 1, rect.bottom() - 3);
                painter->drawLine(rect.right() - 1, rect.top() + 3, rect.right() - 1, rect.bottom() - 3);
                painter->drawPoint(rect.left() + 2, rect.top() + 2);
                painter->drawPoint(rect.right() - 2, rect.top() + 2);
                painter->drawPoint(rect.left() + 2, rect.bottom() - 2);
                painter->drawPoint(rect.right() - 2, rect.bottom() - 2);

                QColor outlineColor = qt_plastique_mergedColors(alphaCornerColor.dark(110), option->palette.background().color());
                painter->setPen(outlineColor);
                painter->drawLine(rect.left() + 2, rect.top(), rect.right() - 2, rect.top());
                painter->drawLine(rect.left() + 2, rect.bottom(), rect.right() - 2, rect.bottom());
                painter->drawLine(rect.left(), rect.top() + 2, rect.left(), rect.bottom() - 2);
                painter->drawLine(rect.right(), rect.top() + 2, rect.right(), rect.bottom() - 2);
                painter->drawPoint(rect.left() + 1, rect.top() + 1);
                painter->drawPoint(rect.right() - 1, rect.top() + 1);
                painter->drawPoint(rect.left() + 1, rect.bottom() - 1);
                painter->drawPoint(rect.right() - 1, rect.bottom() - 1);

                painter->setPen(qt_plastique_mergedColors(outlineColor, option->palette.background().color()));
                painter->drawPoint(rect.left() + 1, rect.top());
                painter->drawPoint(rect.left() + 1, rect.bottom());
                painter->drawPoint(rect.right() - 1, rect.top());
                painter->drawPoint(rect.right() - 1, rect.bottom());
                painter->drawPoint(rect.left(), rect.top() + 1);
                painter->drawPoint(rect.left(), rect.bottom() - 1);
                painter->drawPoint(rect.right(), rect.top() + 1);
                painter->drawPoint(rect.right(), rect.bottom() - 1);
                
                painter->setPen(qt_plastique_mergedColors(outlineColor, borderColor.dark(105)));
                painter->drawPoint(rect.left() + 2, rect.top() + 1);
                painter->drawPoint(rect.right() - 2, rect.top() + 1);
                painter->drawPoint(rect.left() + 2, rect.bottom() - 1);
                painter->drawPoint(rect.right() - 2, rect.bottom() - 1);
                painter->drawPoint(rect.left() + 1, rect.top() + 2);
                painter->drawPoint(rect.right() - 1, rect.top() + 2);
                painter->drawPoint(rect.left() + 1, rect.bottom() - 2);
                painter->drawPoint(rect.right() - 1, rect.bottom() - 2);                
            } else {
                // outer border
                painter->setPen(borderColor);
                painter->drawLine(rect.left() + 2, rect.top(),
                                  rect.right() - 2, rect.top());
                painter->drawLine(rect.left() + 2, rect.bottom(),
                                  rect.right() - 2, rect.bottom());
                painter->drawLine(rect.left(), rect.top() + 2,
                                  rect.left(), rect.bottom() - 2);
                painter->drawLine(rect.right(), rect.top() + 2,
                                  rect.right(), rect.bottom() - 2);
                painter->drawPoint(rect.left() + 1, rect.top() + 1);
                painter->drawPoint(rect.right() - 1, rect.top() + 1);
                painter->drawPoint(rect.left() + 1, rect.bottom() - 1);
                painter->drawPoint(rect.right() - 1, rect.bottom() - 1);
           
                // "antialiased" corners
                painter->setPen(alphaCornerColor);
                painter->drawPoint(rect.left() + 1, rect.top());
                painter->drawPoint(rect.left() + 1, rect.bottom());
                painter->drawPoint(rect.right() - 1, rect.top());
                painter->drawPoint(rect.right() - 1, rect.bottom());
                painter->drawPoint(rect.left(), rect.top() + 1);
                painter->drawPoint(rect.left(), rect.bottom() - 1);
                painter->drawPoint(rect.right(), rect.top() + 1);
                painter->drawPoint(rect.right(), rect.bottom() - 1);
            }
            
            // inner border, top and bottom line
            if (down) {
                painter->setPen(option->palette.button().color().light(89));
            } else {
                if (hover) {
                    painter->setPen(highlightedDarkInnerBorderColor);
                } else {
                    painter->setPen(option->palette.button().color().light(103));
                }
            }
            if (isDefault) {
                painter->drawLine(rect.left() + 3, rect.top() + 2,
                                  rect.right() - 3, rect.top() + 2);
            } else {
                painter->drawLine(rect.left() + 2, rect.top() + 1,
                                  rect.right() - 2, rect.top() + 1);
            }

            if (down) {
                painter->setPen(option->palette.button().color().light(96));
            } else {
                if (hover) {
                    painter->setPen(option->palette.highlight().color().light(138));
                } else {
                    painter->setPen(option->palette.button().color().light(91));
                }
            }
            if (isDefault) {
                painter->drawLine(rect.left() + 3, rect.bottom() - 2,
                                  rect.right() - 3, rect.bottom() - 2);
            } else {
                painter->drawLine(rect.left() + 2, rect.bottom() - 1,
                                  rect.right() - 2, rect.bottom() - 1);
            }
            
            QLinearGradient leftGrad(QPoint(rect.left() + 1, rect.top() + 2),
                                     QPoint(rect.left() + 1, rect.bottom() - 2));
            QLinearGradient rightGrad(QPoint(rect.left() + 1, rect.top() + 2),
                                      QPoint(rect.left() + 1, rect.bottom() - 2));

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
                painter->setPen(QPen(QBrush(leftGrad), 1));
                painter->drawLine(rect.left() + 2, rect.top() + 3,
                                  rect.left() + 2, rect.bottom() - 3);
                painter->setPen(QPen(QBrush(rightGrad), 1));
                painter->drawLine(rect.right() - 2, rect.top() + 3,
                                  rect.right() - 2, rect.bottom() - 3);
            } else {           
                painter->setPen(QPen(QBrush(leftGrad), 1));
                painter->drawLine(rect.left() + 1, rect.top() + 2,
                                  rect.left() + 1, rect.bottom() - 2);
                painter->setPen(QPen(QBrush(rightGrad), 1));
                painter->drawLine(rect.right() - 1, rect.top() + 2,
                                  rect.right() - 1, rect.bottom() - 2);
            }
            
            if (!down && hover) {
                if (isDefault) {
                    painter->setPen(highlightedLightInnerBorderColor);
                    painter->drawLine(rect.left() + 2, rect.top() + 2,
                                      rect.right() - 2, rect.top() + 2);
                    painter->setPen(option->palette.highlight().color().light(160));
                    painter->drawLine(rect.left() + 2, rect.bottom() - 2,
                                      rect.right() - 2, rect.bottom() - 2);
                } else {
                    painter->setPen(highlightedLightInnerBorderColor);
                    painter->drawLine(rect.left() + 1, rect.top() + 2,
                                      rect.right() - 1, rect.top() + 2);
                    painter->setPen(option->palette.highlight().color().light(160));
                    painter->drawLine(rect.left() + 1, rect.bottom() - 2,
                                      rect.right() - 1, rect.bottom() - 2);
                }
            }

            painter->restore();
        }
        break; 
    case PE_IndicatorCheckBox:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            painter->save();

            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);
            
            // border
            QRect fullRect = option->rect;
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
           
            // fill if it's a real checkbox.
            if (qobject_cast<const QCheckBox *>(widget)) {
                QRect adjustedRect = option->rect.adjusted(0, 0, -1, -1);
                QLinearGradient gradient(QPointF(adjustedRect.left() + 1, adjustedRect.top() + 1),
                                         QPointF(adjustedRect.right() - 1, adjustedRect.bottom() - 1));
                if (hover) {
                    gradient.setColorAt(0, highlightedGradientStartColor);
                    gradient.setColorAt(1, highlightedGradientStopColor);
                } else {
                    gradient.setColorAt(0, gradientStartColor);
                    gradient.setColorAt(1, gradientStopColor);
                }
                painter->fillRect(adjustedRect.left() + 1, adjustedRect.top() + 1,
                                  adjustedRect.right() - adjustedRect.left() - 1,
                                  adjustedRect.bottom() - adjustedRect.top() - 1, gradient);
            }
            
            // draw highlighted border when hovering
            if (hover) {
                painter->setPen(highlightedDarkInnerBorderColor);
                painter->drawLine(fullRect.left() + 1, fullRect.bottom() - 1,
                                  fullRect.left() + 1, fullRect.top() + 1);
                painter->drawLine(fullRect.left() + 1, fullRect.top() + 1,
                                  fullRect.right() - 2, fullRect.top() + 1);
                painter->setPen(highlightedLightInnerBorderColor);
                painter->drawLine(fullRect.left() + 2, fullRect.bottom() - 2,
                                  fullRect.left() + 2, fullRect.top() + 2);
                painter->drawLine(fullRect.left() + 2, fullRect.top() + 2,
                                  fullRect.right() - 3, fullRect.top() + 2);
                painter->setPen(highlightedDarkInnerBorderColor.dark(110));
                painter->drawLine(fullRect.left() + 2, fullRect.bottom() - 1,
                                  fullRect.right() - 1, fullRect.bottom() - 1);
                painter->drawLine(fullRect.right() - 1, fullRect.bottom() - 1,
                                  fullRect.right() - 1, fullRect.top() + 1);
                painter->setPen(highlightedLightInnerBorderColor.dark(110));
                painter->drawLine(fullRect.left() + 3, fullRect.bottom() - 2,
                                  fullRect.right() - 2, fullRect.bottom() - 2);
                painter->drawLine(fullRect.right() - 2, fullRect.bottom() - 2,
                                  fullRect.right() - 2, fullRect.top() + 2);
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
                painter->drawImage(fullRect.x() + 2, fullRect.y() + 2, image);
            }
            
            painter->restore();
        }
        break;
    case PE_IndicatorRadioButton:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            painter->save();

            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);

            // fill
            QLinearGradient gradient(QPointF(button->rect.left(), button->rect.top()),
                                     QPointF(button->rect.right(), button->rect.bottom()));

            if (hover) {
                gradient.setColorAt(0, highlightedGradientStartColor);
                gradient.setColorAt(1, highlightedGradientStopColor);
            } else {
                gradient.setColorAt(0, gradientStartColor);
                gradient.setColorAt(1, gradientStopColor);
            }
           
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(QBrush(gradient));
            painter->drawEllipse(button->rect.adjusted(0, 0, -1, 0));

            QImage image(qt_plastique_radioborder); 
            image.setColor(0, borderColor.rgba());
            painter->drawImage(button->rect.topLeft(), image);

            painter->setPen(alphaCornerColor);
            image = QImage(qt_plastique_radio_outeralpha);
            image.setColor(0, alphaCornerColor.rgba());
            painter->drawImage(button->rect.topLeft(), image);

            QColor color;
            QRect adjustedRect = button->rect;
            if (hover) {
                image = QImage(qt_plastique_radio_innerhover);
                image.setColor(1, qt_plastique_mergedColors(borderColor, highlightedDarkInnerBorderColor).rgba());
                image.setColor(2, highlightedDarkInnerBorderColor.rgba());
                image.setColor(3, highlightedLightInnerBorderColor.rgba());
                adjustedRect = adjustedRect.adjusted(1, 1, 0, 0);
            } else {
                image = QImage(qt_plastique_radio_inneralpha);
                color = alphaInnerColorNoHover;
                image.setColor(0, color.rgba());
            }
            painter->drawImage(adjustedRect.topLeft(), image);

            // draw check
            if (button->state & State_On) {
                image = QImage(qt_plastique_radio_check);
                if (button->state & State_Sunken) {
                    image.setColor(1, qt_plastique_mergedColors(button->palette.background().color(),
                                                                alphaTextColor).rgba());
                    image.setColor(2, qt_plastique_mergedColors(button->palette.background().color(),
                                                                button->palette.text().color()).rgba());
                } else {
                    image.setColor(1, alphaTextColor.rgba());
                    image.setColor(2, button->palette.text().color().rgba());
                }
                painter->drawImage(button->rect, image);
            }

            
            painter->restore();
        }
        break;
    case PE_IndicatorDockWidgetResizeHandle:
        drawControl(CE_Splitter, option, painter, widget);
        break;
    case PE_IndicatorViewItemCheck: {
        QStyleOptionButton button;
        button.state = option->state & ~State_MouseOver;
        button.rect = option->rect;
        drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
        break;
    }
    default:
        QWindowsStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void QPlastiqueStyle::drawControl(ControlElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const
{
    QColor borderColor = option->palette.button().color().dark(162);

    QColor alphaCornerColor = option->palette.background().color();
    alphaCornerColor.setRed((alphaCornerColor.red() + borderColor.red()) / 2);
    alphaCornerColor.setGreen((alphaCornerColor.green() + borderColor.green()) / 2);
    alphaCornerColor.setBlue((alphaCornerColor.blue() + borderColor.blue()) / 2);

    QColor gradientStartColor = option->palette.button().color().dark(90);
    QColor gradientStopColor = option->palette.button().color().dark(108);

    QColor highlightedDarkInnerBorderColor = qt_plastique_mergedColors(option->palette.highlight().color(), borderColor);
    QColor alphaInnerColor = qt_plastique_mergedColors(highlightedDarkInnerBorderColor, option->palette.base().color());
    
    switch (element) {
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

            // Temporary: Fall back to Windows style if the orientation is not supported
            if (tab->shape != QTabBar::RoundedNorth) {
                QWindowsStyle::drawControl(element, option, painter, widget);
                break;
            }

            painter->save();

            bool onlyTab = tab->position == QStyleOptionTab::OnlyOneTab;
            bool selected = (tab->state & State_Selected) || onlyTab;
            bool previousSelected = tab->selectedPosition == QStyleOptionTab::PreviousIsSelected;
            bool nextSelected = tab->selectedPosition == QStyleOptionTab::NextIsSelected;
            

            int lowerTop = selected ? 0 : 3; // to make the selected tab bigger than the rest
            QRect adjustedRect = tab->rect.adjusted(previousSelected ? 0 : 1, 1 + lowerTop, -1, -1);
            bool atEnd = (tab->position == QStyleOptionTab::End) || onlyTab;
            bool atBeginning = (tab->position == QStyleOptionTab::Beginning) || onlyTab;
            
            int borderThickness = pixelMetric(PM_TabBarBaseOverlap, tab, widget);

            int marginLeft = 0;
            if (atBeginning && !selected) {
                adjustedRect = adjustedRect.adjusted(1, 0, 0, 0);
                marginLeft = 1;
            }

            // Create the default top border line
            QLine topLine = QLine(tab->rect.topLeft() + QPoint(marginLeft + (previousSelected ? 0 : 1), lowerTop), 
                                  tab->rect.topRight() + QPoint(0, lowerTop));               

            // If we are the final tab or selected tab, we must make the top line shorter to round off the corner
            if (atEnd || selected)
                topLine = QLine(topLine.p1(), topLine.p2() - QPoint(2, 0));

            // Likewise for the first tab (and selected)
            if (atBeginning || selected)
                topLine = QLine(topLine.p1() + QPoint(1, 0), topLine.p2());

            // The default left border line
            QLine leftLine = QLine(tab->rect.topLeft() + QPoint(marginLeft, lowerTop), 
                                   tab->rect.bottomLeft() - QPoint(-1*marginLeft, borderThickness));

            // If we are the first tab or selected tab, we draw a rounded corner, and hence the left line must 
            // be shorter than usual
            if (atBeginning && selected)
                leftLine = QLine(leftLine.p1() + QPoint(0,2), leftLine.p2() + QPoint(0, borderThickness)); 
            else if (atBeginning || selected)  
                leftLine = QLine(leftLine.p1() + QPoint(0,2), leftLine.p2()); 
            
            // Default right border line
            QLine rightLine = QLine(tab->rect.topRight() + QPoint(0,2 + lowerTop), 
                                    tab->rect.bottomRight() - QPoint(0,borderThickness));
            
            // For rounding corners on selected tabs and tabs at the end and beginning
            QPoint rightConnectDot = tab->rect.topRight() + QPoint(-1,1 + lowerTop);
            QPoint leftConnectDot = tab->rect.topLeft() + QPoint(marginLeft + 1,1 + lowerTop);

            // A single dot which connects the frame of the tab to the base 
            QPoint bottomRightConnectToBase = rightLine.p2() + QPoint(1, 0);
            QPoint bottomLeftConnectToBase = leftLine.p2() - QPoint(1, 0);

            // Draw the outer border
            painter->setPen(borderColor);

            // The left line ends up behind the selected tab if previous is selected
            if (!previousSelected)
                painter->drawLine(leftLine); 

            // Always draw the top line
            painter->drawLine(topLine);

            // Only if we are the final tab or the selected tab do we draw a right line
            // We also draw the rounded corner on the right side
            if (atEnd || selected) {         
                painter->drawLine(rightLine); 
                painter->drawPoint(rightConnectDot);                
            } 

            // Selected and first tabs get a rounded corner on the left side
            if (atBeginning || selected) 
                painter->drawPoint(leftConnectDot);                         

            // The selected tab gets connected to the base
            if (selected) {
                painter->drawPoint(bottomRightConnectToBase);
                painter->drawPoint(bottomLeftConnectToBase);
            }

            // More for the rounded corners. We've now moved to the inner shade for the border 
            // (some anti-aliasing effect)
            QPoint topLeftCorner = tab->rect.topLeft() + QPoint(marginLeft, lowerTop);
            if (atBeginning || selected) 
                topLeftCorner += QPoint(1,0);
            QPoint topRightCorner = tab->rect.topRight() - QPoint(1, -1*lowerTop);
            QPoint topRightCorner2 = tab->rect.topRight() + QPoint(0, 1 + lowerTop);
            QPoint topLeftCorner2 = tab->rect.topLeft() + QPoint(marginLeft, 1 + lowerTop);
                       
            painter->setPen(alphaCornerColor);

            // Hide the top left corner if the previous tab is selected
            // Otherwise: Draw it. This is drawn for middle tabs as well.
            if (!previousSelected)
                painter->drawPoint(topLeftCorner);

            // Draw a rounded corner for the end tab
            if (atEnd || selected) {
                painter->drawPoint(topRightCorner);
                painter->drawPoint(topRightCorner2);                
            }

            // Round off the left corner
            if (atBeginning || selected) {
                painter->drawPoint(topLeftCorner2);                
            }

            // Connect the inner border to the base 
            if (selected) {
                painter->drawPoint(bottomRightConnectToBase - QPoint(1, 0));
                if (!atBeginning) 
                    painter->drawPoint(bottomLeftConnectToBase + QPoint(1, 0));

            }
    
            // Default inner top line
            QLine innerTopLine = QLine(adjustedRect.topLeft(), adjustedRect.topRight() + QPoint(1, 0));

            // Modify for special tabs
            if (atEnd || selected)
                innerTopLine = QLine(innerTopLine.p1(), innerTopLine.p2() - QPoint(2,0));
            if (atBeginning || selected) 
                innerTopLine = QLine(innerTopLine.p1() + QPoint(1,0), innerTopLine.p2());                                
            
            // And inner vertical lines
            QLine innerRightLine = QLine(adjustedRect.topRight() + QPoint(0, 1), 
                                         adjustedRect.bottomRight() - QPoint(0, 1));
            QLine innerLeftLine = QLine(adjustedRect.topLeft() + QPoint(0, 1),
                                        adjustedRect.bottomLeft() - QPoint(0, 2));

            if (selected && atBeginning)
                innerLeftLine = QLine(innerLeftLine.p1(), innerLeftLine.p2() + QPoint(0, 1 + borderThickness));
            
            if (selected)
                innerRightLine = QLine(innerRightLine.p1(), innerRightLine.p2() - QPoint(0, 1));

            // Draw the inner border, top
            painter->setPen(tab->palette.button().color().light());
            painter->drawLine(innerTopLine);
            if (selected)
                painter->drawLine(innerLeftLine);
            if (atEnd || selected) {
                // Draw the inner border, right
                QLinearGradient rightLineGradient(innerRightLine.p1(),innerRightLine.p2());
                rightLineGradient.setColorAt(0, tab->palette.button().color().dark(115));
                rightLineGradient.setColorAt(1, tab->palette.button().color().dark(120));

                painter->setPen(QPen(QBrush(rightLineGradient), 1));
                painter->drawLine(innerRightLine);
            }

            // Fill the rect with a gradient
            QRect fillRect = adjustedRect.adjusted(0, 1, 1, -1);
            if (atEnd)
                fillRect = fillRect.adjusted(0, 0, -1*borderThickness, 0);
            if (selected)
                fillRect = fillRect.adjusted(1, 0, -1*borderThickness, 0);

            QLinearGradient fillGradient(fillRect.topLeft(), fillRect.bottomLeft());            
            fillGradient.setColorAt(0, tab->palette.button().color().dark(106));
            fillGradient.setColorAt(1, tab->palette.button().color().dark(120));                                            
            if (selected)
                painter->fillRect(fillRect, tab->palette.background()); 
            else
                painter->fillRect(fillRect, fillGradient);

            // Draw the base (for this to work with QTabWidget we can't use the ordinary primitive)
            QLine baseTopLine = QLine(tab->rect.bottomLeft() - QPoint(0, 1), tab->rect.bottomRight() - QPoint(0, 1));
            QLine baseBottomLine = QLine(tab->rect.bottomLeft(), tab->rect.bottomRight());

            if (nextSelected) {
                baseTopLine = QLine(baseTopLine.p1(), baseTopLine.p2() - QPoint(1, 0));
                baseBottomLine = QLine(baseBottomLine.p1(), baseBottomLine.p2() - QPoint(1, 0));
            }

            if (previousSelected) {
                baseTopLine = QLine(baseTopLine.p1() + QPoint(1, 0), baseTopLine.p2());
                baseBottomLine = QLine(baseBottomLine.p1() + QPoint(1, 0), baseBottomLine.p2());
            }

            if (atBeginning) {
                baseTopLine = QLine(baseTopLine.p1() + QPoint(1, 0), baseTopLine.p2());
                baseBottomLine = QLine(baseBottomLine.p1() + QPoint(2, 0), baseBottomLine.p2());
            }

            painter->setPen(borderColor);
            if (!selected) 
                painter->drawLine(baseTopLine);

            painter->setPen(tab->palette.button().color().light());
            if (!selected)
                painter->drawLine(baseBottomLine);               

            painter->setPen(alphaCornerColor);
            if (nextSelected) {
                QPoint endPoint = baseTopLine.p2() + QPoint(1, 0);
                painter->drawPoint(endPoint);
            }

            if (previousSelected) {
                QPoint endPoint = baseTopLine.p1() - QPoint(1, 0);
                painter->drawPoint(endPoint);
            }

            // Draw the top left corner of the tab bar base 
            if (atBeginning && !selected) {
                QPoint specialCorner1 = QPoint(baseBottomLine.p1() - QPoint(1, 0));
                QPoint specialCorner2 = QPoint(baseBottomLine.p1() - QPoint(2, 0));

                painter->setPen(borderColor);
                painter->drawPoint(specialCorner1);

                painter->setPen(alphaCornerColor);
                painter->drawPoint(specialCorner2);
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
            
            if (bar->direction == Qt::RightToLeft) {
                int progressIndicatorPos = grooveWidth - (((bar->progress - bar->minimum) * grooveWidth) / (bar->maximum - bar->minimum));
                if (progressIndicatorPos >= rect.left() && progressIndicatorPos <= rect.right()) {
                    int leftWidth = progressIndicatorPos - rect.left();
                    painter->setPen(bar->palette.base().color());
                    leftRect = QRect(rect.left(), rect.top(), leftWidth, rect.height());
                } else if (progressIndicatorPos > rect.right()) {
                    painter->setPen(bar->palette.text().color());
                } else {
                    painter->setPen(bar->palette.base().color());
                }

            } else {
                int progressIndicatorPos = ((bar->progress - bar->minimum) * grooveWidth) / (bar->maximum - bar->minimum);
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
            int width = qMax(((progress - bar->minimum) * maxWidth) / (bar->maximum - bar->minimum), minWidth);
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
        qt_plastique_drawShadedPanel(painter, option);
        break;
    case CE_MenuBarItem:
        if (option->state & State_Selected)
            qt_plastique_drawShadedPanel(painter, option, true);
        else
            painter->fillRect(option->rect, option->palette.background());
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
    case CE_MenuBarEmptyArea:
        painter->fillRect(option->rect, option->palette.background());
        break;
    case CE_ToolBoxTab:
        if (option->state & State_Selected)
            qt_plastique_drawShadedPanel(painter, option, true);
        else
            painter->fillRect(option->rect, option->palette.background());
        break;
    case CE_Splitter: {
        painter->save();
        painter->setPen(borderColor);
        QRect rect = option->rect;
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->setPen(alphaCornerColor);
        painter->drawPoint(rect.topLeft());
        painter->drawPoint(rect.topRight());
        painter->drawPoint(rect.bottomLeft());
        painter->drawPoint(rect.bottomRight());
        painter->restore();
    }
        break;
    default:
        QWindowsStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void QPlastiqueStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                         QPainter *painter, const QWidget *widget) const
{
    QColor borderColor = option->palette.button().color().dark(162);
    QColor alphaCornerColor = qt_plastique_mergedColors(borderColor, option->palette.background().color());
    QColor gradientStartColor = option->palette.button().color().dark(90);
    QColor gradientStopColor = option->palette.button().color().dark(108);
    
    switch (control) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect groove = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            QRect ticks = subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);
            bool horizontal = slider->orientation == Qt::Horizontal;
            bool ticksAbove = slider->tickPosition & QSlider::TicksAbove;
            bool ticksBelow = slider->tickPosition & QSlider::TicksBelow;

            painter->save();

            if ((option->subControls & SC_SliderGroove) && groove.isValid()) {
                // draw groove
                if (horizontal) {
                    painter->setPen(borderColor);
                    painter->drawLine(groove.left() + 1, groove.top(),
                                      groove.right() - 1, groove.top());
                    painter->drawLine(groove.left() + 1, groove.bottom(),
                                      groove.right() - 1, groove.bottom());
                    painter->drawLine(groove.left(), groove.top() + 1,
                                      groove.left(), groove.bottom() - 1);
                    painter->drawLine(groove.right(), groove.top() + 1,
                                      groove.right(), groove.bottom() - 1);
                    painter->setPen(alphaCornerColor);
                    painter->drawPoint(groove.left(), groove.top());
                    painter->drawPoint(groove.left(), groove.bottom());
                    painter->drawPoint(groove.right(), groove.top());
                    painter->drawPoint(groove.right(), groove.bottom());
                } else {
                    painter->setPen(borderColor);
                    painter->drawLine(groove.left() + 1, groove.top(),
                                      groove.right() - 1, groove.top());
                    painter->drawLine(groove.left() + 1, groove.bottom(),
                                      groove.right() - 1, groove.bottom());
                    painter->drawLine(groove.left(), groove.top() + 1,
                                      groove.left(), groove.bottom() - 1);
                    painter->drawLine(groove.right(), groove.top() + 1,
                                      groove.right(), groove.bottom() - 1);
                    painter->setPen(alphaCornerColor);
                    painter->drawPoint(groove.left(), groove.top());
                    painter->drawPoint(groove.right(), groove.top());
                    painter->drawPoint(groove.left(), groove.bottom());
                    painter->drawPoint(groove.right(), groove.bottom());
                }
            }

            if ((option->subControls & SC_SliderHandle) && handle.isValid()) {
                // draw handle
                if (horizontal) {
                    QPainterPath path;
                    if (ticksAbove && !ticksBelow) {
                        path.moveTo(QPoint(handle.right(), handle.bottom() - 1));
                        path.lineTo(QPoint(handle.right(), handle.bottom() - 10));
                        path.lineTo(QPoint(handle.right() - 5, handle.bottom() - 14));
                        path.lineTo(QPoint(handle.left() + 1,  handle.bottom() - 10));
                        path.lineTo(QPoint(handle.left() + 1, handle.bottom() - 1));
                        path.lineTo(QPoint(handle.right(), handle.bottom() - 1));
                    } else {
                        path.moveTo(QPoint(handle.right(), handle.top() + 1));
                        path.lineTo(QPoint(handle.right(), handle.top() + 10));
                        path.lineTo(QPoint(handle.right() - 5, handle.top() + 14));
                        path.lineTo(QPoint(handle.left() + 1,  handle.top() + 10));
                        path.lineTo(QPoint(handle.left() + 1, handle.top() + 1));
                        path.lineTo(QPoint(handle.right(), handle.top() + 1));
                    }
                    QLinearGradient gradient(handle.center().x(), handle.top(),
                                             handle.center().x(), handle.bottom());
                    gradient.setColorAt(0, gradientStartColor);
                    gradient.setColorAt(1, gradientStopColor);
                    painter->fillPath(path, gradient);
                } else {
                    QPainterPath path;
                    if (ticksAbove && !ticksBelow) {
                        path.moveTo(QPoint(handle.right() - 1, handle.top() + 1));
                        path.lineTo(QPoint(handle.right() - 10, handle.top() + 1));
                        path.lineTo(QPoint(handle.right() - 14, handle.top() + 5));
                        path.lineTo(QPoint(handle.right() - 10, handle.bottom()));
                        path.lineTo(QPoint(handle.right() - 1, handle.bottom()));
                        path.lineTo(QPoint(handle.right() - 1, handle.top() + 1));
                    } else {
                        path.moveTo(QPoint(handle.left() + 1, handle.top() + 1));
                        path.lineTo(QPoint(handle.left() + 10, handle.top() + 1));
                        path.lineTo(QPoint(handle.left() + 14, handle.top() + 5));
                        path.lineTo(QPoint(handle.left() + 10, handle.bottom()));
                        path.lineTo(QPoint(handle.left() + 1, handle.bottom()));
                        path.lineTo(QPoint(handle.left() + 1, handle.top() + 1));
                    }
                    QLinearGradient gradient(handle.center().x(), handle.top(),
                                             handle.center().x(), handle.bottom());
                    gradient.setColorAt(0, gradientStartColor);                    
                    gradient.setColorAt(1, gradientStopColor);
                    painter->fillPath(path, gradient);
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
                painter->drawImage(handle, image);
            }

            if (option->subControls & SC_SliderTickmarks) {
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
            }
            painter->restore();
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            painter->save();            

            QRect rect = scrollBar->rect;
            QRect scrollBarSubLine = subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
            QRect scrollBarAddLine = subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
            QRect scrollBarSlider = subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);
            
            bool reverse = scrollBar->direction == Qt::RightToLeft;
            bool horizontal = scrollBar->orientation == Qt::Horizontal;

            // The groove
            QRect grooveRect;
            if (horizontal) {
                grooveRect = QRect(rect.left() + 1, rect.top() + 1, rect.width() - 2, rect.height() - 1);
            } else {
                grooveRect = QRect(rect.left() + 1, rect.top() + 1, rect.width() - 1, rect.height() - 2);
            }
            painter->fillRect(grooveRect, QBrush(option->palette.base().color(), Qt::Dense4Pattern));
            if (horizontal) {
                painter->setBrushOrigin(QPoint(grooveRect.left() + 1, grooveRect.top()));
                grooveRect.setHeight(1);
                grooveRect.moveTop(rect.top());
            } else {
                painter->setBrushOrigin(QPoint(grooveRect.left(), grooveRect.top() + 1));
                grooveRect.setWidth(1);
                grooveRect.moveLeft(rect.left());
            }
            painter->fillRect(grooveRect,
                              QBrush(scrollBar->palette.base().color().dark(115), Qt::Dense4Pattern));
            
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

                // Gradients
                QLinearGradient gradient1(button1.center().x(), button1.top() + 2,
                                          button1.center().x(), button1.bottom() - 2);
                QLinearGradient gradient2(button2.center().x(), button2.top() + 2,
                                          button2.center().x(), button2.bottom() - 2);
                if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && (scrollBar->state & State_Sunken)) {
                    gradient1.setColorAt(0, gradientStopColor);
                    gradient1.setColorAt(1, gradientStopColor);
                    gradient2.setColorAt(0, gradientStopColor);
                    gradient2.setColorAt(1, gradientStopColor);
                } else {
                    gradient1.setColorAt(0, gradientStartColor.dark(106));
                    gradient1.setColorAt(1, gradientStopColor);
                    gradient2.setColorAt(0, gradientStartColor.dark(106));
                    gradient2.setColorAt(1, gradientStopColor);
                }
                painter->fillRect(button1.left() + 2, button1.top() + 2,
                                  button1.right() - 3, button1.bottom() - 3, gradient1);
                painter->fillRect(button2.left() + 2, button2.top() + 2,
                                  button2.right() - 3, button2.bottom() - 3, gradient2);

                // Details
                QImage subButton;
                if (horizontal) {
                    subButton = QImage(reverse ? qt_scrollbar_button_right : qt_scrollbar_button_left);
                } else {
                    subButton = QImage(qt_scrollbar_button_up);
                }
                subButton.setColor(1, alphaCornerColor.rgba());
                subButton.setColor(2, borderColor.rgba());
                if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && (scrollBar->state & State_Sunken)) {
                    subButton.setColor(3, gradientStopColor.rgba());
                    subButton.setColor(4, gradientStopColor.dark(104).rgba());
                } else {
                    subButton.setColor(3, gradientStartColor.dark(105).rgba());
                    subButton.setColor(4, gradientStopColor.dark(104).rgba());
                }
                subButton.setColor(5, scrollBar->palette.text().color().rgba());
                painter->drawImage(button1, subButton);
                painter->drawImage(button2, subButton);

                // Arrows
                if (horizontal) {
                    QImage arrow(reverse ? qt_scrollbar_button_arrow_right : qt_scrollbar_button_arrow_left);
                    arrow.setColor(1, scrollBar->palette.text().color().rgba());
                    
                    if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && (scrollBar->state & State_Sunken)) {
                        painter->drawImage(QPoint(button1.left() + 6, button1.top() + 5), arrow);
                        painter->drawImage(QPoint(button2.left() + 6, button2.top() + 5), arrow);
                    } else {
                        painter->drawImage(QPoint(button1.left() + 5, button1.top() + 4), arrow);
                        painter->drawImage(QPoint(button2.left() + 5, button2.top() + 4), arrow);
                    }
                } else {
                    QImage arrow(qt_scrollbar_button_arrow_up);
                    arrow.setColor(1, scrollBar->palette.text().color().rgba());

                    if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && (scrollBar->state & State_Sunken)) {
                        painter->drawImage(QPoint(button1.left() + 5, button1.top() + 7), arrow);
                        painter->drawImage(QPoint(button2.left() + 5, button2.top() + 7), arrow);
                    } else {
                        painter->drawImage(QPoint(button1.left() + 4, button1.top() + 6), arrow);
                        painter->drawImage(QPoint(button2.left() + 4, button2.top() + 6), arrow);
                    }
                }
            }

            // The AddLine (down/right) button
            if (scrollBar->subControls & SC_ScrollBarAddLine) {
                // Gradient
                QLinearGradient gradient(scrollBarAddLine.center().x(), scrollBarAddLine.top() + 2,
                                         scrollBarAddLine.center().x(), scrollBarAddLine.bottom() - 2);
                if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && (scrollBar->state & State_Sunken)) {
                    gradient.setColorAt(0, gradientStopColor);
                    gradient.setColorAt(1, gradientStopColor);
                } else {                
                    gradient.setColorAt(0, gradientStartColor.dark(106));
                    gradient.setColorAt(1, gradientStopColor);
                }
                painter->fillRect(scrollBarAddLine.left() + 2, scrollBarAddLine.top() + 2,
                                  scrollBarAddLine.right() - 3, scrollBarAddLine.bottom() - 3,
                                  gradient);

                // Details
                QImage addButton;
                if (horizontal) {
                    addButton = QImage(reverse ? qt_scrollbar_button_left : qt_scrollbar_button_right);
                } else {
                    addButton = QImage(qt_scrollbar_button_down);
                }
                addButton.setColor(1, alphaCornerColor.rgba());
                addButton.setColor(2, borderColor.rgba());
                if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && (scrollBar->state & State_Sunken)) {
                    addButton.setColor(3, gradientStopColor.rgba());
                    addButton.setColor(4, gradientStopColor.dark(104).rgba());
                } else {
                    addButton.setColor(3, gradientStartColor.dark(105).rgba());
                    addButton.setColor(4, gradientStopColor.dark(104).rgba());
                }
                addButton.setColor(5, scrollBar->palette.text().color().rgba());                
                painter->drawImage(scrollBarAddLine, addButton);

                // Arrow
                if (horizontal) {
                    QImage arrow(reverse ? qt_scrollbar_button_arrow_left : qt_scrollbar_button_arrow_right);
                    arrow.setColor(1, scrollBar->palette.text().color().rgba());
                    
                    if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && (scrollBar->state & State_Sunken)) {
                        painter->drawImage(QPoint(scrollBarAddLine.left() + 7, scrollBarAddLine.top() + 5), arrow);
                    } else {
                        painter->drawImage(QPoint(scrollBarAddLine.left() + 6, scrollBarAddLine.top() + 4), arrow);
                    }
                } else {
                    QImage arrow(qt_scrollbar_button_arrow_down);
                    arrow.setColor(1, scrollBar->palette.text().color().rgba());

                    if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && (scrollBar->state & State_Sunken)) {
                        painter->drawImage(QPoint(scrollBarAddLine.left() + 5, scrollBarAddLine.top() + 7), arrow);
                    } else {
                        painter->drawImage(QPoint(scrollBarAddLine.left() + 4, scrollBarAddLine.top() + 6), arrow);
                    }
                }
            }

            // The slider
            if (scrollBar->subControls & SC_ScrollBarSlider) {
                QLinearGradient gradient(scrollBarSlider.center().x(), scrollBarSlider.top(),
                                         scrollBarSlider.center().x(), scrollBarSlider.bottom());
                gradient.setColorAt(0, gradientStartColor);
                gradient.setColorAt(1, gradientStopColor);
                painter->fillRect(scrollBarSlider.adjusted(2, 2, -2, -2), gradient);
                
                painter->setPen(borderColor);
                painter->drawRect(scrollBarSlider.adjusted(0, 0, -1, -1));
                painter->setPen(alphaCornerColor);
                painter->drawPoint(scrollBarSlider.left(), scrollBarSlider.top());
                painter->drawPoint(scrollBarSlider.left(), scrollBarSlider.bottom());
                painter->drawPoint(scrollBarSlider.right(), scrollBarSlider.top());
                painter->drawPoint(scrollBarSlider.right(), scrollBarSlider.bottom());

                painter->setPen(gradientStartColor.dark(105));
                painter->drawLine(scrollBarSlider.left() + 1, scrollBarSlider.top() + 1,
                                  scrollBarSlider.right() - 1, scrollBarSlider.top() + 1);
                painter->drawLine(scrollBarSlider.left() + 1, scrollBarSlider.top() + 2,
                                  scrollBarSlider.left() + 1, scrollBarSlider.bottom() - 2);
                                  
                painter->setPen(gradientStopColor.dark(105));
                painter->drawLine(scrollBarSlider.left() + 1, scrollBarSlider.bottom() - 1,
                                  scrollBarSlider.right() - 1, scrollBarSlider.bottom() - 1);
                painter->drawLine(scrollBarSlider.right() - 1, scrollBarSlider.top() + 2,
                                  scrollBarSlider.right() - 1, scrollBarSlider.bottom() - 1);
                
                QImage pattern(qt_scrollbar_slider_pattern);
                pattern.setColor(1, alphaCornerColor.rgba());
                pattern.setColor(2, gradientStartColor.rgba());
                
                if (horizontal) {
                    painter->drawImage(scrollBarSlider.center().x() - pattern.width() / 2 + 1,
                                       scrollBarSlider.center().y() - 4,
                                       pattern);
                } else {
                    painter->drawImage(scrollBarSlider.center().x() - 4,
                                       scrollBarSlider.center().y() - pattern.width() / 2 + 1,
                                       pattern);
                }
            }

            painter->restore();
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            painter->save();

            QRect rect = spinBox->rect;

            // Border
            painter->setPen(borderColor);
            painter->drawLine(rect.left() + 2, rect.top(), rect.right() - 2, rect.top());
            painter->drawLine(rect.left() + 2, rect.bottom(), rect.right() - 2, rect.bottom());
            painter->drawLine(rect.left(), rect.top() + 2, rect.left(), rect.bottom() - 2);
            painter->drawLine(rect.right(), rect.top() + 2, rect.right(), rect.bottom() - 2);
            if (spinBox->direction == Qt::RightToLeft) {
                painter->drawPoint(rect.right() - 1, rect.top() + 1);
                painter->drawPoint(rect.right() - 1, rect.bottom() - 1);
            } else {
                painter->drawPoint(rect.left() + 1, rect.top() + 1);
                painter->drawPoint(rect.left() + 1, rect.bottom() - 1);
            }
            painter->setPen(alphaCornerColor);
            painter->drawPoint(rect.left() + 1, rect.top());
            painter->drawPoint(rect.left() + 1, rect.bottom());
            painter->drawPoint(rect.right() - 1, rect.top());
            painter->drawPoint(rect.right() - 1, rect.bottom());
            painter->drawPoint(rect.left(), rect.top() + 1);
            painter->drawPoint(rect.left(), rect.bottom() - 1);
            painter->drawPoint(rect.right(), rect.top() + 1);
            painter->drawPoint(rect.right(), rect.bottom() - 1);
            painter->setPen(qt_plastique_mergedColors(borderColor, option->palette.base().color()));
            painter->drawLine(rect.left() + 2, rect.top() + 1, rect.right() - 2, rect.top() + 1);
            painter->drawLine(rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom() - 2);

            QRect upRect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
            QRect downRect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);

            // outline the up/down buttons
            painter->setPen(borderColor);
            if (spinBox->direction == Qt::RightToLeft) {
                painter->drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                painter->drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                painter->drawLine(upRect.right(), upRect.bottom(), upRect.left(), upRect.bottom());
            } else {
                painter->drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                painter->drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                painter->drawLine(upRect.left(), upRect.bottom(), upRect.right(), upRect.bottom());
            }

            // gradients
            QLinearGradient upGradient(upRect.center().x(), upRect.top(),
                                       upRect.center().x(), upRect.bottom());
            if (spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken)) {
                upGradient.setColorAt(0, gradientStopColor.rgba());
                upGradient.setColorAt(1, gradientStopColor.rgba());
            } else {
                upGradient.setColorAt(0, gradientStartColor.rgba());
                upGradient.setColorAt(1, gradientStopColor.rgba());
            }
            painter->fillRect(upRect.adjusted(1, 1, -1, -1), upGradient);
            
            QLinearGradient downGradient(downRect.center().x(), downRect.top(),
                                         downRect.center().x(), downRect.bottom());
            if (spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken)) {
                downGradient.setColorAt(0, gradientStopColor.rgba());
                downGradient.setColorAt(1, gradientStopColor.rgba());
            } else {
                downGradient.setColorAt(0, gradientStartColor.rgba());
                downGradient.setColorAt(1, gradientStopColor.rgba());
            }
            painter->fillRect(downRect.adjusted(1, 1, -1, -1), downGradient);
            
            // finish alpha corners
            if (spinBox->direction == Qt::RightToLeft) {
                painter->drawPoint(rect.left() + 1, rect.top() + 1);
                painter->drawPoint(rect.left() + 1, rect.bottom() - 1);
            } else {
                painter->drawPoint(rect.right() - 1, rect.top() + 1);
                painter->drawPoint(rect.right() - 1, rect.bottom() - 1);
            }

            if (spinBox->buttonSymbols == QAbstractSpinBox::PlusMinus) {
                int centerX = upRect.center().x();
                int centerY = upRect.center().y();
                painter->setPen(spinBox->palette.text().color());

                // plus/minus
                if (spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken)) {
                    painter->drawLine(1 + centerX - 2, 1 + centerY, 1 + centerX + 2, 1 + centerY);
                    painter->drawLine(1 + centerX, 1 + centerY - 2, 1 + centerX, 1 + centerY + 2);
                } else {
                    painter->drawLine(centerX - 2, centerY, centerX + 2, centerY);
                    painter->drawLine(centerX, centerY - 2, centerX, centerY + 2);
                }

                centerX = downRect.center().x();
                centerY = downRect.center().y();
                if (spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken)) {
                    painter->drawLine(1 + centerX - 2, 1 + centerY, 1 + centerX + 2, 1 + centerY);
                } else {
                    painter->drawLine(centerX - 2, centerY, centerX + 2, centerY);
                }
            } else {
                // arrows
                QImage upArrow(qt_scrollbar_button_arrow_up);
                upArrow.setColor(1, spinBox->palette.text().color().rgba());
                if (spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken)) {
                    painter->drawImage(1 + upRect.center().x() - upArrow.width() / 2,
                                       1 + upRect.center().y() - upArrow.height() / 2,
                                       upArrow);
                } else {
                    painter->drawImage(upRect.center().x() - upArrow.width() / 2,
                                       upRect.center().y() - upArrow.height() / 2,
                                       upArrow);
                }
                QImage downArrow(qt_scrollbar_button_arrow_down);
                downArrow.setColor(1, spinBox->palette.text().color().rgba());
                if (spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken)) {
                    painter->drawImage(1 + downRect.center().x() - downArrow.width() / 2,
                                       1 + downRect.center().y() - downArrow.height() / 2,
                                       downArrow);
                } else {
                    painter->drawImage(downRect.center().x() - downArrow.width() / 2,
                                       downRect.center().y() - downArrow.height() / 2,
                                       downArrow);
                }
            }
            
            painter->restore();
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            painter->save();
           
            QRect downArrowRect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
            QRect rect = comboBox->rect;

            // Draw a push button
            QStyleOptionButton buttonOption;
            buttonOption.rect = rect;

            if (comboBox->state & State_Enabled)
                buttonOption.state |= State_Enabled;
            if (comboBox->state & State_MouseOver)
                buttonOption.state |= State_MouseOver;
            if (qobject_cast<const QComboBox *>(widget)->view()->isVisible())
                buttonOption.state |= State_Sunken;
            
            buttonOption.state = comboBox->state;
            drawPrimitive(PE_PanelButtonCommand, &buttonOption, painter, widget);

            // Draw the little arrow
            painter->setPen(borderColor);
            QImage downArrow(qt_scrollbar_button_arrow_down);
            downArrow.setColor(1, comboBox->palette.text().color().rgba());

            if (comboBox->direction == Qt::RightToLeft) {
                painter->drawLine(downArrowRect.topRight(), downArrowRect.bottomRight());
            } else {
                painter->drawLine(downArrowRect.topLeft(), downArrowRect.bottomLeft());
            }
            
            painter->drawImage(downArrowRect.center().x() - downArrow.width() / 2,
                               downArrowRect.center().y() - downArrow.height() / 2, downArrow);

            if (widget->hasFocus()) {
                QStyleOptionFocusRect focus;
                focus.rect = subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget);
                drawPrimitive(PE_FrameFocusRect, &focus, painter, widget);
            }
            
            painter->restore();
        }
        break;
        /*
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            painter->save();
            QRect rect = titleBar->rect;
            QLinearGradient gradient(rect.center().x(), rect.top(), rect.center().x(), rect.bottom());
            gradient.setColorAt(0, qt_plastique_mergedColors(titleBar->palette.highlight().color(), Qt::black));
            gradient.setColorAt(1, qt_plastique_mergedColors(titleBar->palette.highlight().color(), Qt::black).light(110));
            painter->fillRect(rect.adjusted(0, 0, -1, -1), gradient);

            painter->setPen(titleBar->palette.base().color());
            painter->drawText(subControlRect(CC_TitleBar, option, SC_TitleBarLabel, widget), titleBar->text);
            painter->restore();
        }
        break;
        */
    default:
        QWindowsStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

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
        newSize.rheight() -= 3;
        break;
    case CT_ToolButton:
        newSize.rwidth() += 3;
        newSize.rheight() += 3;
        break;
    default:
        break;
    }

    return newSize;
}

QRect QPlastiqueStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect = QWindowsStyle::subElementRect(element, option, widget);

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
    
    return rect;
}

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
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {

            switch (subControl) {
            case SC_TitleBarLabel:
                rect = QRect(widget->rect().center().x() - widget->fontMetrics().width(titleBar->text)/2,
                             widget->rect().top(),
                             widget->fontMetrics().width(titleBar->text),
                             widget->rect().height());
                break;
            default:
                break;
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            int center = spinBox->rect.height() / 2;
            switch (subControl) {
            case SC_SpinBoxUp:
                rect.setRect(spinBox->rect.right() - 16, spinBox->rect.top(), 17, center);
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
                break;
            case SC_SpinBoxDown:
                rect.setRect(spinBox->rect.right() - 16, center, 17, spinBox->rect.bottom() - center + 1);
                rect = visualRect(spinBox->direction, spinBox->rect, rect);
                break;
            default:
                break;
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            if (subControl == SC_ComboBoxArrow) {
                rect.setRect(comboBox->rect.right() - 17, comboBox->rect.top(),
                             17, comboBox->rect.height());
                rect = visualRect(comboBox->direction, comboBox->rect, rect);
            }
        }
        break;
    default:
        break;
    }

    return rect;
}

QStyle::SubControl QPlastiqueStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                          const QPoint &pos, const QWidget *widget) const
{
    switch (control) {
    case CC_ComboBox:
        return SC_ComboBoxListBoxPopup;
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
        return 4;
    case PM_DockWidgetSeparatorExtent:
        return 4;
    case PM_DockWidgetHandleExtent:
        return 20;
    default:
        break;
    }

    return QWindowsStyle::pixelMetric(metric, option, widget);
}

QPalette QPlastiqueStyle::standardPalette()
{
    QPalette palette;
    palette.setBrush(QPalette::Disabled, QPalette::Foreground, QColor(QRgb(0xff808080)));
    palette.setBrush(QPalette::Disabled, QPalette::Button, QColor(QRgb(0xfff4f4f4)));
    palette.setBrush(QPalette::Disabled, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Disabled, QPalette::Mid, QColor(QRgb(0xffc6c6c6)));
    palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(QRgb(0xffc6c6c6)));
    palette.setBrush(QPalette::Disabled, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(QRgb(0xff808080)));
    palette.setBrush(QPalette::Disabled, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Background, QColor(QRgb(0xffeeeeee)));
    palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(QRgb(0xff3966aa)));
    palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Link, QColor(QRgb(0xff535378)));
    palette.setBrush(QPalette::Disabled, QPalette::LinkVisited, QColor(QRgb(0xff004000)));
    palette.setBrush(QPalette::Active, QPalette::Foreground, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Button, QColor(QRgb(0xfff4f4f4)));
    palette.setBrush(QPalette::Active, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Active, QPalette::Mid, QColor(QRgb(0xffc6c6c6)));
    palette.setBrush(QPalette::Active, QPalette::Text, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::ButtonText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Background, QColor(QRgb(0xffeeeeee)));
    palette.setBrush(QPalette::Active, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Highlight, QColor(QRgb(0xff447bcd)));
    palette.setBrush(QPalette::Active, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Link, QColor(QRgb(0xff535378)));
    palette.setBrush(QPalette::Active, QPalette::LinkVisited, QColor(QRgb(0xff004000)));
    palette.setBrush(QPalette::Inactive, QPalette::Foreground, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Button, QColor(QRgb(0xfff4f4f4)));
    palette.setBrush(QPalette::Inactive, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Inactive, QPalette::Mid, QColor(QRgb(0xffc6c6c6)));
    palette.setBrush(QPalette::Inactive, QPalette::Text, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::ButtonText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Background, QColor(QRgb(0xffeeeeee)));
    palette.setBrush(QPalette::Inactive, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, QColor(QRgb(0xff447bcd)));
    palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Link, QColor(QRgb(0xff535378)));
    palette.setBrush(QPalette::Inactive, QPalette::LinkVisited, QColor(QRgb(0xff004000)));
    return palette;
}    

void QPlastiqueStyle::polish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget))
        widget->setAttribute(Qt::WA_Hover);
    if (qobject_cast<QComboBox *>(widget))
        widget->setAttribute(Qt::WA_Hover);
    else if (qobject_cast<QCheckBox *>(widget))
        widget->setAttribute(Qt::WA_Hover);
    else if (qobject_cast<QRadioButton *>(widget))
        widget->setAttribute(Qt::WA_Hover);
}
