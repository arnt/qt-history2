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

#if !defined(QT_NO_STYLE_CLEANLOOKS) || defined(QT_PLUGIN)

#include "qcleanlooksstyle.h"
#include "qwindowsstyle_p.h"
#include <QComboBox>

#include <qpainter.h>
#include <qstyleoption.h>
#include <qdebug.h>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QFont>
#include <QGroupBox>
#include <QPainterPath>
#include <QPushButton>
#include <QPixmapCache>
#include <QProgressBar>
#include <QScrollBar>
#include <QSpinBox>
#include <QSlider>
#include <QSplitter>

static const bool UsePixmapCache = true;

enum Direction {
    TopDown,
    FromLeft,
    BottomUp,
    FromRight
};

// from windows style
static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsSepHeight        =  6; // separator item height
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  8; // menu item ver text margin
static const int windowsArrowHMargin     =  6; // arrow horizontal margin
static const int windowsTabSpacing       = 12; // space between text and tab
static const int windowsCheckMarkHMargin =  2; // horiz. margins of check mark
static const int windowsRightBorder      = 15; // right border on windows
static const int windowsCheckMarkWidth   = 12; // checkmarks width on windows

/* XPM */
static const char * const dock_widget_close_xpm[] = {
    "15 15 7 1",
    " 	c None",
    ".	c #D5CFCB",
    "+	c #8F8B88",
    "@	c #6C6A67",
    "#	c #ABA6A3",
    "$	c #B5B0AC",
    "%	c #A4A09D",
    "               ",
    "               ",
    "  .+@@@@@@@+.  ",
    "  +#       #+  ",
    "  @ $@   @$ @  ",
    "  @ @@@ @@@ @  ",
    "  @  @@@@@  @  ",
    "  @   @@@   @  ",
    "  @  @@@@@  @  ",
    "  @ @@@ @@@ @  ",
    "  @ $@   @$ @  ",
    "  +%       #+  ",
    "  .+@@@@@@@+.  ",
    "               ",
    "               "};

static const char * const qt_cleanlooks_arrow_xpm[] = {
    "11 7 2 1",
    " 	c None",
    ".	c #000000",
    "           ",
    "  .     .  ",
    " ...   ... c",
    "  .......  ",
    "   .....   ",
    "    ...    ",
    "     .     "};

static const char * const dock_widget_restore_xpm[] = {
    "15 15 7 1",
    " 	c None",
    ".	c #D5CFCB",
    "+	c #8F8B88",
    "@	c #6C6A67",
    "#	c #ABA6A3",
    "$	c #B5B0AC",
    "%	c #A4A09D",
    "               ",
    "               ",
    "  .+@@@@@@@+.  ",
    "  +#       #+  ",
    "  @   #@@@# @  ",
    "  @   @   @ @  ",
    "  @ #@@@# @ @  ",
    "  @ @   @ @ @  ",
    "  @ @   @@@ @  ",
    "  @ @   @   @  ",
    "  @ #@@@#   @  ",
    "  +%       #+  ",
    "  .+@@@@@@@+.  ",
    "               ",
    "               "};

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

static const char * const qt_cleanlooks_radiobutton[] = {
    "13 13 9 1",
    " 	c None",
    ".	c #ABA094",
    "+	c #B7ADA0",
    "@	c #C4BBB2",
    "#	c #DDD4CD",
    "$	c #E7E1E0",
    "%	c #F4EFED",
    "&	c #FFFAF9",
    "*	c #FCFEFB",
    "   #@...@#   ",
    "  @+@#$$#+@  ",
    " @+$%%***&@@ ",
    "#+$%**&&**&+#",
    "@@$&&******#@",
    ".#**********.",
    ".$&******&*&.",
    ".$*&******&*.",
    "+#********&#@",
    "#+*********+#",
    " @@*******@@ ",
    "  @+#%*%#+@  ",
    "   #@...+#   "};

static const char * const qt_cleanlooks_radiobutton_checked[] = {
    "13 13 20 1",
    " 	c None",
    ".	c #A8ABAE",
    "+	c #596066",
    "@	c #283138",
    "#	c #A9ACAF",
    "$	c #A6A9AB",
    "%	c #6B7378",
    "&	c #8C9296",
    "*	c #A2A6AA",
    "=	c #61696F",
    "-	c #596065",
    ";	c #93989C",
    ">	c #777E83",
    ",	c #60686E",
    "'	c #252D33",
    ")	c #535B62",
    "!	c #21292E",
    "~	c #242B31",
    "{	c #1F262B",
    "]	c #41484E",
    "             ",
    "             ",
    "             ",
    "    .+@+#    ",
    "   $%&*&=#   ",
    "   -&;>,'+   ",
    "   @*>,)!@   ",
    "   +&,)~{+   ",
    "   #='!{]#   ",
    "    #+@+#    ",
    "             ",
    "             ",
    "             "};


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

static const char * const qt_spinbox_button_arrow_down[] = {
    "5 3 2 1",
    "   c None",
    "*  c #BFBFBF",
    "*****",
    " *** ",
    "  *  "};

static const char * const qt_spinbox_button_arrow_up[] = {
    "5 3 2 1",
    "   c None",
    "*  c #BFBFBF",
    "  *  ",
    " *** ",
    "*****"};

static const char * const qt_scrollbar_button_left[] = {
    "16 16 6 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "#  c #FAFAFA",
    "<  c #FAFAFA",
    "*  c #FAFAFA",
    " .++++++++++++++",
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
    " .++++++++++++++"};

static const char * const qt_scrollbar_button_right[] = {
    "16 16 6 1",
    "   c None",
    ".  c #BFBFBF",
    "+  c #979797",
    "#  c #FAFAFA",
    "<  c #FAFAFA",
    "*  c #FAFAFA",
    "++++++++++++++. ",
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
    "++++++++++++++. "};

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
    "++++++++++++++++"};

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

static const char * const qt_cleanlooks_menuitem_checkbox_checked[] = {
    "8 7 6 1",
    " 	g None",
    ".	g #959595",
    "+	g #676767",
    "@	g #454545",
    "#	g #1D1D1D",
    "0	g #101010",
    "      ..",
    "     .+ ",
    "    .+  ",
    "0  .@   ",
    "@#++.   ",
    "  @#    ",
    "   .    "};

static const char * const qt_cleanlooks_checkbox_checked[] = {
    "13 13 8 1",
    " 	c None",
    "%	c #ABB0B2",
    ".	c #373E44",
    "+	c #444B52",
    "@	c #575E65",
    "#	c #727981",
    "$	c #999EA0",
    "&	c #BCC1C3",
    "             ",
    "             ",
    "         %%  ",
    "        %#   ",
    "       %+    ",
    "  %.% &.%    ",
    "  &..&@+     ",
    "   %...$     ",
    "    %..      ",
    "     +%      ",
    "     &       ",
    "             ",
    "             "};

class QCleanLooksStylePrivate : public QWindowsStylePrivate
{
    Q_DECLARE_PUBLIC(QCleanLooksStyle)
public:
    QCleanLooksStylePrivate()
        : QWindowsStylePrivate()
    {  }

~QCleanLooksStylePrivate()
    { }
};

static void qt_cleanlooks_draw_gradient(QPainter *painter, const QRect &rect, const QColor &gradientStart,
                                        const QColor &gradientStop, Direction direction = TopDown)
{
        int x = rect.center().x();
        int y = rect.center().y();
        QLinearGradient *gradient;
        switch(direction) {
            case FromLeft:
                gradient = new QLinearGradient(rect.left(), y, rect.right(), y);
                break;
            case FromRight:
                gradient = new QLinearGradient(rect.right(), y, rect.left(), y);
                break;
            case BottomUp:
                gradient = new QLinearGradient(x, rect.bottom(), x, rect.top());
            case TopDown:
            default:
                gradient = new QLinearGradient(x, rect.top(), x, rect.bottom());
                break;
        }
        gradient->setColorAt(0, gradientStart);
        gradient->setColorAt(1, gradientStop);
        painter->fillRect(rect, *gradient);
        delete gradient;
}

static QString uniqueName(const QString &key, const QStyleOption *option, const QSize &size)
{
    QString tmp;
    const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
    tmp.sprintf("%s-%d-%d-%d-%dx%d", key.toLatin1().constData(), uint(option->state),
                complexOption ? uint(complexOption->activeSubControls) : uint(0),
                option->palette.serialNumber(), size.width(), size.height());
    if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
        tmp.append("-" + QString::number(spinBox->buttonSymbols) + "-" + QString::number(spinBox->stepEnabled));
    }
    return tmp;
}

static void qt_cleanlooks_draw_mdibutton(QPainter *painter, const QStyleOptionTitleBar *option, const QRect &tmp)
{

    bool active = (option->titleBarState & QStyle::State_Active);
    QColor titleBarHighlight(active ? option->palette.highlight().color().light(120): option->palette.background().color().light(120));

    QColor mdiButtonGradientStartColor;
    QColor mdiButtonGradientStopColor;

    mdiButtonGradientStartColor = QColor(active ? option->palette.highlight().color().light(115): option->palette.background().color().light(115));
    mdiButtonGradientStopColor = QColor(active ? option->palette.highlight().color().dark(110) : option->palette.background().color());

    QLinearGradient gradient(tmp.center().x(), tmp.top(), tmp.center().x(), tmp.bottom());
    gradient.setColorAt(0, mdiButtonGradientStartColor);
    gradient.setColorAt(1, mdiButtonGradientStopColor);

    QColor mdiButtonBorderColor(active ? option->palette.highlight().color().dark(180): option->palette.dark().color().dark(120));

    painter->setPen(QPen(mdiButtonBorderColor, 1));
    painter->drawLine(tmp.left() + 2, tmp.top(), tmp.right() - 2, tmp.top());
    painter->drawLine(tmp.left() + 2, tmp.bottom(), tmp.right() - 2, tmp.bottom());
    painter->drawLine(tmp.left(), tmp.top() + 2, tmp.left(), tmp.bottom() - 2);
    painter->drawLine(tmp.right(), tmp.top() + 2, tmp.right(), tmp.bottom() - 2);
    painter->drawPoint(tmp.left() + 1, tmp.top() + 1);
    painter->drawPoint(tmp.right() - 1, tmp.top() + 1);
    painter->drawPoint(tmp.left() + 1, tmp.bottom() - 1);
    painter->drawPoint(tmp.right() - 1, tmp.bottom() - 1);

    painter->setPen(titleBarHighlight);
    painter->drawLine(tmp.left() + 2, tmp.top() + 1, tmp.right() - 2, tmp.top() + 1);
    painter->setPen(QPen(gradient, 1));
    painter->drawLine(tmp.left() + 1, tmp.top() + 2, tmp.left() + 1, tmp.bottom() - 2);
    painter->drawLine(tmp.right() + 1, tmp.top() + 2, tmp.right() + 1, tmp.bottom() - 2);
    painter->drawPoint(tmp.right() , tmp.top() + 1);

    painter->drawLine(tmp.left() + 2, tmp.bottom() + 1, tmp.right() - 2, tmp.bottom() + 1);
    painter->drawPoint(tmp.left() + 1, tmp.bottom());
    painter->drawPoint(tmp.right() - 1, tmp.bottom());
    painter->drawPoint(tmp.right() , tmp.bottom() - 1);
}

/*!
    \class QCleanLooksStyle
    \brief The QCleanLooksStyle class provides a widget style similar to the
    ClearLooks style available in GNOME.

    The CleanLooks style provides a look and feel for widgets
    that closely resembles the ClearLooks style, introduced by Richard
    Stellingwerff and Daniel Borgmann.

    \sa QWindowsXPStyle, QMacStyle, QWindowsStyle, QCDEStyle, QMotifStyle, QPlastiqueStyle.
*/

/*!
    Constructs a QCleanLooksStyle object.
*/
QCleanLooksStyle::QCleanLooksStyle()
{
    setObjectName("CleanLooks");
}

/*!
    Destroys the QCleanLooksStyle object.
*/
QCleanLooksStyle::~QCleanLooksStyle()
{
}

/*!
    Draws the \a text in rectangle \a rect using \a painter and
    palette \a pal.

    Text is drawn using the painter's pen. If an explicit \a textRole
    is specified, then the text is drawn using the color specified in
    \a pal for the specified role.  The \a enabled bool indicates
    whether or not the item is enabled; when reimplementing this bool
    should influence how the item is drawn.

    The text is aligned and wrapped according to \a alignment.

    \sa Qt::Alignment
*/
void QCleanLooksStyle::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                                    bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;

    painter->save();
    QPen savedPen;
    if (textRole != QPalette::NoRole) {
        painter->setPen(pal.color(textRole));
    }
    if (!enabled) {
        QPen pen = painter->pen();
        painter->setPen(pen);
    }
    painter->drawText(rect, alignment, text);
    painter->restore();
}

static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

/*!
    \reimp
*/
void QCleanLooksStyle::drawPrimitive(PrimitiveElement elem,
                        const QStyleOption *option,
                        QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);
    QRect rect = option->rect;
    int state = option->state;
    QColor outlineColor = option->palette.dark().color();
    QColor buttonShadow = option->palette.button().color().dark(110);
    QColor buttonShadowAlpha = option->palette.background().color().dark(105);
    QColor grooveColor = mergedColors(option->palette.dark().color(), option->palette.button().color(),60);
    QColor gripShadow = grooveColor.dark(110);
    QColor shadow = option->palette.background().color().dark(120);

    switch(elem) {
    case PE_IndicatorButtonDropDown:
        drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
        break;
    case PE_IndicatorToolBarSeparator:
        {
            QRect rect = option->rect;
            const int margin = 6;
            if (option->state & State_Horizontal) {
                const int offset = rect.width()/2;
                painter->setPen(QPen(option->palette.background().color().dark(110)));
                painter->drawLine(rect.bottomLeft().x() + offset,
                            rect.bottomLeft().y() - margin,
                            rect.topLeft().x() + offset,
                            rect.topLeft().y() + margin);
                painter->setPen(QPen(option->palette.background().color().light(110)));
                painter->drawLine(rect.bottomLeft().x() + offset + 1,
                            rect.bottomLeft().y() - margin,
                            rect.topLeft().x() + offset + 1,
                            rect.topLeft().y() + margin);
            } else { //Draw vertical separator
                const int offset = rect.height()/2;
                painter->setPen(QPen(option->palette.background().color().dark(110)));
                painter->drawLine(rect.topLeft().x() + margin ,
                            rect.topLeft().y() + offset,
                            rect.topRight().x() - margin,
                            rect.topRight().y() + offset);
                painter->setPen(QPen(option->palette.background().color().light(110)));
                painter->drawLine(rect.topLeft().x() + margin ,
                            rect.topLeft().y() + offset + 1,
                            rect.topRight().x() - margin,
                            rect.topRight().y() + offset + 1);
            }
        }
        break;
    case PE_Frame:
        painter->save();
        painter->setPen(shadow);
        painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
        painter->restore();
        break;
    case PE_FrameMenu:
        painter->save();
        {
            painter->setPen(QPen(option->palette.shadow().color().light(120), 1));
            painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
            QColor frameLight = option->palette.background().color().light(160);
            QColor frameShadow = option->palette.background().color().dark(110);

            //paint beveleffect
            QRect frame = option->rect.adjusted(1, 1, -1, -1);
            painter->setPen(frameLight);
            painter->drawLine(frame.topLeft(), frame.bottomLeft());
            painter->drawLine(frame.topLeft(), frame.topRight());

            painter->setPen(frameShadow);
            painter->drawLine(frame.topRight(), frame.bottomRight());
            painter->drawLine(frame.bottomLeft(), frame.bottomRight());
        }
        painter->restore();
        break;
    case PE_FrameDockWidget:

        painter->save();
        {
            QColor softshadow = option->palette.background().color().dark(120);

            QRect rect= option->rect;
            painter->setPen(softshadow);
            painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
            painter->setPen(QPen(option->palette.light(), 0));
            painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1), QPoint(rect.left() + 1, rect.bottom() - 1));
            painter->setPen(QPen(option->palette.background().color().dark(120), 0));
            painter->drawLine(QPoint(rect.left() + 1, rect.bottom() - 1), QPoint(rect.right() - 2, rect.bottom() - 1));
            painter->drawLine(QPoint(rect.right() - 1, rect.top() + 1), QPoint(rect.right() - 1, rect.bottom() - 1));

        }
        painter->restore();
        break;
    case PE_PanelButtonTool:
        painter->save();
        if ((option->state & State_Enabled) || !(option->state & State_AutoRaise)) {
            QRect rect = option->rect;
            QPen oldPen = painter->pen();

            QColor gradientStartColor = option->palette.button().color().light(104);
            QColor gradientStopColor = option->palette.button().color().dark(105);

            if (widget && widget->inherits("QDockWidgetTitleButton")) {
                   if (option->state & State_MouseOver)
                       drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            } else {
                drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            }
        }
        painter->restore();
        break;
    case PE_IndicatorDockWidgetResizeHandle:
        {
            QStyleOption dockWidgetHandle = *option;
            bool horizontal = option->state & State_Horizontal;
            if (horizontal)
                dockWidgetHandle.state &= ~State_Horizontal;
            else
                dockWidgetHandle.state |= State_Horizontal;
            drawControl(CE_Splitter, &dockWidgetHandle, painter, widget);
        }
        break;
    case PE_FrameWindow:
        painter->save();
        {
            QRect rect= option->rect;
            painter->setPen(QPen(option->palette.shadow().color().dark(140), 0));
            painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
            painter->setPen(QPen(option->palette.light(), 0));
            painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1),
                              QPoint(rect.left() + 1, rect.bottom() - 1));
            painter->setPen(QPen(option->palette.background().color().dark(120), 0));
            painter->drawLine(QPoint(rect.left() + 1, rect.bottom() - 1),
                              QPoint(rect.right() - 2, rect.bottom() - 1));
            painter->drawLine(QPoint(rect.right() - 1, rect.top() + 1),
                              QPoint(rect.right() - 1, rect.bottom() - 1));
        }
        painter->restore();
        break;
#ifndef QT_NO_LINEDIT
    case PE_FrameLineEdit:
        // fall through
#endif // QT_NO_LINEEDIT
#ifdef QT3_SUPPORT
        if (widget && widget->inherits("Q3ToolBar")) {
            drawPrimitive(PE_Q3Separator, option, painter, widget);
            break;
        }
#endif
        {
            if (option->state & State_Enabled) {
                painter->setPen(QPen(option->palette.background(), 0));
                painter->drawRect(rect.adjusted(0, 0, 0, 0));
                painter->drawRect(rect.adjusted(1, 1, -1, -1));
            } else {
                painter->fillRect(rect, option->palette.background());
            }
            QRect r = rect.adjusted(0, 1, 0, -1);
            painter->setPen(buttonShadowAlpha);
            painter->drawLine(QPoint(r.left() + 2, r.top() - 1), QPoint(r.right() - 2, r.top() - 1));
            painter->drawPoint(r.right() - 1, r.top());
            painter->drawPoint(r.right(), r.top() + 1);
            painter->drawPoint(r.right() - 1, r.bottom());
            painter->drawPoint(r.right(), r.bottom() - 1);
            painter->drawPoint(r.left() + 1, r.top() );
            painter->drawPoint(r.left(), r.top() + 1);
            painter->drawPoint(r.left() + 1, r.bottom() );
            painter->drawPoint(r.left(), r.bottom() - 1);

            painter->setPen(QPen(option->palette.background().color(), 1));
            painter->drawLine(QPoint(r.left() + 2, r.top() + 1), QPoint(r.right() - 2, r.top() + 1));
            painter->setPen(QPen(option->palette.dark().color(), 1));
            painter->drawLine(QPoint(r.left(), r.top() + 2), QPoint(r.left(), r.bottom() - 2));
            painter->drawLine(QPoint(r.right(), r.top() + 2), QPoint(r.right(), r.bottom() - 2));
            painter->drawLine(QPoint(r.left() + 2, r.bottom()), QPoint(r.right() - 2, r.bottom()));
            painter->drawPoint(QPoint(r.right() - 1, r.bottom() - 1));
            painter->drawPoint(QPoint(r.right() - 1, r.top() + 1));
            painter->drawPoint(QPoint(r.left() + 1, r.bottom() - 1));
            painter->drawPoint(QPoint(r.left() + 1, r.top() + 1));
            painter->drawLine(QPoint(r.left() + 2, r.top()), QPoint(r.right() - 2, r.top()));

            painter->setPen(QPen(option->palette.light().color()));
            painter->drawLine(option->rect.bottomLeft() + QPoint(2, 0),
                              option->rect.bottomRight() - QPoint(2, 0));
        }
        break;
    case PE_IndicatorCheckBox:
        painter->save();
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            QRect checkRect;
            checkRect.setX(rect.left() );
            checkRect.setY(rect.top() );
            checkRect.setWidth(rect.width() - 1);
            checkRect.setHeight(rect.height() - 1);
            if (state & State_Sunken)
                painter->setBrush(option->palette.dark().color().light(120));
            else
                painter->setBrush(option->palette.base());
            painter->setPen(QPen(option->palette.dark(), 0));
            painter->drawRect(checkRect);
            if (checkbox->state & (State_On | State_Sunken  | State_NoChange)) {
                QImage image(qt_cleanlooks_checkbox_checked);
                painter->drawImage(rect, image);
                if (checkbox->state & State_NoChange) {
                    QColor bgc = option->palette.background();
                    bgc.setAlpha(127);
                    painter->fillRect(checkRect.adjusted(1, 1, -1, -1), bgc);
                }
            }
        }
        painter->restore();
        break;
    case PE_IndicatorRadioButton:
        painter->save();
        {
            QRect checkRect = rect.adjusted(0, 0, 0, 0);
            if (state & (State_On )) {
                painter->drawImage(rect, QImage(qt_cleanlooks_radiobutton));
                painter->drawImage(checkRect, QImage(qt_cleanlooks_radiobutton_checked));
            }
            else if (state & State_Sunken) {
                painter->drawImage(rect, QImage(qt_cleanlooks_radiobutton));
                QColor bgc = buttonShadow;
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setBrush(bgc);
                painter->setPen(Qt::NoPen);
                painter->drawEllipse(rect.adjusted(1, 1, -1, -1));                }
            else {
                painter->drawImage(rect, QImage(qt_cleanlooks_radiobutton));
            }
        }
        painter->restore();
    break;
    case PE_IndicatorToolBarHandle:
        painter->save();
        if (option->state & State_Horizontal) {
            for (int i = rect.height()/5; i <= 4*(rect.height()/5) ; ++i) {
                int y = rect.topLeft().y() + i + 1;
                int x1 = rect.topLeft().x() + 3;
                int x2 = rect.topRight().x() - 2;

                if (i % 2 == 0)
                    painter->setPen(QPen(option->palette.light(), 0));
                else
                    painter->setPen(QPen(gripShadow, 0));
                painter->drawLine(x1, y, x2, y);
            }
        }
        else { //vertical toolbar
            for (int i = rect.width()/5; i <= 4*(rect.width()/5) ; ++i) {
                int x = rect.topLeft().x() + i + 1;
                int y1 = rect.topLeft().y() + 3;
                int y2 = rect.topLeft().y() + 5;

                if (i % 2 == 0)
                    painter->setPen(QPen(option->palette.light(), 0));
                else
                    painter->setPen(QPen(gripShadow, 0));
                painter->drawLine(x, y1, x, y2);
            }
        }
        painter->restore();
        break;
    case PE_FrameDefaultButton:
        case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *focusFrame = qstyleoption_cast<const QStyleOptionFocusRect *>(option)) {
            if (!(focusFrame->state & State_KeyboardFocusChange))
                return;
            QRect rect = focusFrame->rect;
            painter->save();
            painter->setBackgroundMode(Qt::TransparentMode);
            painter->setBrush(QBrush(focusFrame->palette.shadow().color().dark(110), Qt::Dense4Pattern));
            painter->setBrushOrigin(rect.topLeft());
            painter->setPen(Qt::NoPen);
            painter->drawRect(rect.left(), rect.top(), rect.width(), 1);    // Top
            painter->drawRect(rect.left(), rect.bottom(), rect.width(), 1); // Bottom
            painter->drawRect(rect.left(), rect.top(), 1, rect.height());   // Left
            painter->drawRect(rect.right(), rect.top(), 1, rect.height());  // Right
            painter->restore();
        }
        break;
    case PE_PanelButtonCommand:
        painter->save();
        {
            bool down = (option->state & State_Sunken) || (option->state & State_On);

            bool isDefault = false;
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton*>(option))
                isDefault = (button->features & QStyleOptionButton::DefaultButton) &&(button->state & State_Enabled);
            bool isEnabled = (option->state & State_Enabled);

            QRect rect = option->rect;

            QColor highlightedGradientStartColor = option->palette.button().color().light(107);
            QColor highlightedGradientStopColor = buttonShadow.light(107);
            QColor gradientStartColor = option->palette.button().color().light(108);
            QColor gradientStopColor = mergedColors(option->palette.button().color().dark(108), option->palette.dark().color().light(140), 70);

            rect.adjust(0, 1, 0, -1);
            QRect gradRect = rect.adjusted(1, 1, -1, -1);
            if (isEnabled) {
                // gradient fill
                QRect innerBorder = rect.adjusted(1, 1, -1, 0);

                if (down) {
		            painter->fillRect(gradRect, gradientStopColor.dark(110));
                    painter->setPen(gradientStopColor.dark(125));
                    painter->drawLine(innerBorder.topLeft(), innerBorder.topRight());
                    painter->drawLine(innerBorder.topLeft(), innerBorder.bottomLeft());
                } else {
                    if (option->state & State_MouseOver ) {
                        qt_cleanlooks_draw_gradient(painter, gradRect,
                                                    highlightedGradientStartColor,
                                                    highlightedGradientStopColor);
                    } else {
                        qt_cleanlooks_draw_gradient(painter, gradRect,
                                                    gradientStartColor,
                                                    gradientStopColor);
                    }
                    painter->setPen(Qt::white);
                    painter->drawLine(innerBorder.topLeft(), innerBorder.topRight());
                    painter->drawLine(innerBorder.topLeft(), innerBorder.bottomLeft());
                    painter->setPen(buttonShadow);
                    painter->drawLine(innerBorder.bottomLeft(), innerBorder.bottomRight());
                    painter->drawLine(innerBorder.topRight(), innerBorder.bottomRight());
                }
            } else {
                QColor gradientStartColor = option->palette.button().color();
                QColor gradientStopColor = buttonShadow;
                painter->fillRect(gradRect, option->palette.background());
                qt_cleanlooks_draw_gradient(painter, gradRect,
                                                gradientStartColor,
                                                gradientStopColor);
            }

            bool hasFocus = option->state & State_HasFocus;

            QRect r = rect.adjusted(0, 0, 0, 0);

            if (isDefault)
                painter->setPen(QPen(Qt::black, 1));
            else
                painter->setPen(QPen(option->palette.dark().color().dark(110), 1));

            painter->drawLine(QPoint(r.left(), r.top() + 2),
                              QPoint(r.left(), r.bottom() - 2));
            painter->drawLine(QPoint(r.right(), r.top() + 2),
                              QPoint(r.right(), r.bottom() - 2));
            painter->drawLine(QPoint(r.left() + 2, r.bottom()),
                              QPoint(r.right() - 2, r.bottom()));
            painter->drawPoint(QPoint(r.right() - 1, r.bottom() - 1));
            painter->drawPoint(QPoint(r.right() - 1, r.top() + 1));
            painter->drawPoint(QPoint(r.left() + 1, r.bottom() - 1));
            painter->drawPoint(QPoint(r.left() + 1, r.top() + 1));

            if (!isDefault && !hasFocus && isEnabled)
                painter->setPen(QPen(option->palette.shadow(), 1));

            painter->drawLine(QPoint(r.left() + 2, r.top()),
                              QPoint(r.right() - 2, r.top()));
            painter->setPen(option->palette.light().color());
            painter->drawLine(QPoint(r.left() + 2, r.bottom() + 1),
                              QPoint(r.right() - 2, r.bottom() + 1));
            painter->setPen(buttonShadowAlpha.dark(130));
            painter->drawPoint(QPoint(r.right(), r.top() + 1));
            painter->drawPoint(QPoint(r.right() - 1, r.top() ));
            painter->setPen(buttonShadowAlpha);
            painter->drawLine(QPoint(r.left() + 2, r.top() - 1),
                              QPoint(r.right() - 2, r.top() - 1));
            painter->drawPoint(QPoint(r.left() + 1, r.top()));
            painter->drawPoint(QPoint(r.left(), r.top() + 1));

            if (isDefault) {
                r.adjust(-1, -1, 1, 1);
                painter->setPen(buttonShadow);
                painter->drawLine(r.topLeft() + QPoint(3, 0), r.topRight() - QPoint(3, 0));
                painter->drawLine(r.bottomLeft() + QPoint(3, 0), r.bottomRight() - QPoint(3, 0));
                painter->drawLine(r.topLeft() + QPoint(0, 3), r.bottomLeft() - QPoint(0, 3));
                painter->drawLine(r.topRight() + QPoint(0, 3), r.bottomRight() - QPoint(0, 3));

                painter->drawLine(r.topRight() + QPoint(0, 3), r.topRight() - QPoint(3, 0));
                painter->drawLine(r.bottomRight() - QPoint(0, 3), r.bottomRight() - QPoint(3, 0));

                painter->drawLine(r.topLeft() + QPoint(0, 3), r.topLeft() + QPoint(3, 0));
                painter->drawLine(r.bottomLeft() - QPoint(0, 3), r.bottomLeft() + QPoint(3, 0));
            }
        }
        painter->restore();
        break;
#ifndef QT_NO_TABBAR
        case PE_FrameTabWidget:
            painter->save();
        {
            QColor tabFrameColor = mergedColors(option->palette.background().color(),
                                                option->palette.dark().color().light(125), 80);

            painter->fillRect(option->rect, tabFrameColor);
        }
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QColor borderColor = option->palette.dark().color();
            QColor alphaCornerColor = mergedColors(borderColor, option->palette.background().color());
            QColor innerShadow = mergedColors(borderColor, option->palette.base().color());

            int borderThickness = pixelMetric(PM_TabBarBaseOverlap, twf, widget);
            bool reverse = (twf->direction == Qt::RightToLeft);
            QRect tabBarRect;

            switch (twf->shape) {
            case QTabBar::RoundedNorth:
                if (reverse) {
                    tabBarRect = QRect(twf->rect.right() - twf->leftCornerWidgetSize.width()
                                       - twf->tabBarSize.width() + 1,
                                       twf->rect.top(),
                                       twf->tabBarSize.width(), borderThickness);
                } else {
                    tabBarRect = QRect(twf->rect.left() + twf->leftCornerWidgetSize.width(),
                                       twf->rect.top(),
                                       twf->tabBarSize.width(), borderThickness);
                }
                break ;
            case QTabBar::RoundedWest:
                tabBarRect = QRect(twf->rect.left(),
                                   twf->rect.top() + twf->leftCornerWidgetSize.height(),
                                   borderThickness,
                                   twf->tabBarSize.height());
                tabBarRect = tabBarRect; //adjust
                break ;
            case QTabBar::RoundedEast:
                tabBarRect = QRect(twf->rect.right() - borderThickness + 1,
                                   twf->rect.top()  + twf->leftCornerWidgetSize.height(),
                                   0,
                                   twf->tabBarSize.height());
                break ;
            case QTabBar::RoundedSouth:
                if (reverse) {
                    tabBarRect = QRect(twf->rect.right() - twf->leftCornerWidgetSize.width() - twf->tabBarSize.width() + 1,
                                       twf->rect.bottom() + 1,
                                       twf->tabBarSize.width(),
                                       borderThickness);
                } else {
                    tabBarRect = QRect(twf->rect.left() + twf->leftCornerWidgetSize.width(),
                                       twf->rect.bottom() + 1,
                                       twf->tabBarSize.width(),
                                       borderThickness);
                }
                break;
            default:
                break;
            }

            QRegion region(twf->rect);
            region -= tabBarRect;
            painter->setClipRegion(region);

            // Outer border
            QLine leftLine = QLine(twf->rect.topLeft() + QPoint(0, 2), twf->rect.bottomLeft() - QPoint(0, 2));
            QLine rightLine = QLine(twf->rect.topRight(), twf->rect.bottomRight() - QPoint(0, 2));
            QLine bottomLine = QLine(twf->rect.bottomLeft() + QPoint(2, 0), twf->rect.bottomRight() - QPoint(2, 0));
            QLine topLine = QLine(twf->rect.topLeft(), twf->rect.topRight());

            painter->setPen(borderColor);
            painter->drawLine(topLine);

            // Inner border
            QLine innerLeftLine = QLine(leftLine.p1() + QPoint(1, 0), leftLine.p2() + QPoint(1, 0));
            QLine innerRightLine = QLine(rightLine.p1() - QPoint(1, -1), rightLine.p2() - QPoint(1, 0));
            QLine innerBottomLine = QLine(bottomLine.p1() - QPoint(0, 1), bottomLine.p2() - QPoint(0, 1));
            QLine innerTopLine = QLine(topLine.p1() + QPoint(0, 1), topLine.p2() + QPoint(-1, 1));

            // Rounded Corner
            QPoint leftBottomOuterCorner = QPoint(innerLeftLine.p2() + QPoint(0, 1));
            QPoint leftBottomInnerCorner1 = QPoint(leftLine.p2() + QPoint(0, 1));
            QPoint leftBottomInnerCorner2 = QPoint(bottomLine.p1() - QPoint(1, 0));
            QPoint rightBottomOuterCorner = QPoint(innerRightLine.p2() + QPoint(0, 1));
            QPoint rightBottomInnerCorner1 = QPoint(rightLine.p2() + QPoint(0, 1));
            QPoint rightBottomInnerCorner2 = QPoint(bottomLine.p2() + QPoint(1, 0));
            QPoint leftTopOuterCorner = QPoint(innerLeftLine.p1() - QPoint(0, 1));
            QPoint leftTopInnerCorner1 = QPoint(leftLine.p1() - QPoint(0, 1));
            QPoint leftTopInnerCorner2 = QPoint(topLine.p1() - QPoint(1, 0));

            painter->setPen(borderColor);
            painter->drawLine(leftLine);
            painter->drawLine(rightLine);
            painter->drawLine(bottomLine);
            painter->drawPoint(leftBottomOuterCorner);
            painter->drawPoint(rightBottomOuterCorner);
            painter->drawPoint(leftTopOuterCorner);

            painter->setPen(option->palette.light().color());
            painter->drawLine(innerLeftLine);
            painter->drawLine(innerTopLine);

            painter->setPen(option->palette.dark().color().light(130));
            painter->drawLine(innerRightLine);
            painter->drawLine(innerBottomLine);

            painter->setPen(alphaCornerColor);
            painter->drawPoint(leftBottomInnerCorner1);
            painter->drawPoint(leftBottomInnerCorner2);
            painter->drawPoint(rightBottomInnerCorner1);
            painter->drawPoint(rightBottomInnerCorner2);
            painter->drawPoint(leftTopInnerCorner1);
            painter->drawPoint(leftTopInnerCorner2);
        }
    painter->restore();
    break ;
#endif // QT_NO_TABBAR
    default:
        QWindowsStyle::drawPrimitive(elem, option, painter, widget);
        break;
    }
}

/*!
  \reimp
*/
void QCleanLooksStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                                   const QWidget *widget) const
{
    QRect rect = option->rect;
    QColor shadow = mergedColors(option->palette.background().color().dark(120),
                                 option->palette.dark().color().light(130), 60);

    switch(element) {
     case CE_RadioButton: //fall through
     case CE_CheckBox:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            bool hover = (btn->state & State_MouseOver && btn->state & State_Enabled);
            if (hover)
                painter->fillRect(rect, btn->palette.background().color().light(104));       
            QStyleOptionButton copy = *btn;
            copy.rect.adjust(2, 0, -2, 0);
            QWindowsStyle::drawControl(element, &copy, painter, widget);
        }
        break;
    case CE_Splitter:
        painter->save();
        {
            //hover appearence
            QBrush fillColor = option->palette.background().color();
            if (option->state & State_MouseOver && option->state & State_Enabled)
                fillColor = fillColor.color().light(115);

            painter->fillRect(option->rect, fillColor);

            QColor grooveColor = mergedColors(option->palette.dark().color(), option->palette.button().color(),40);
            QColor gripShadow = grooveColor.dark(110);
            QPalette palette = option->palette;
            bool vertical = !(option->state & State_Horizontal);
            QRect scrollBarSlider = option->rect;
            int gripMargin = 4;
            //draw grips
            if (vertical) {
                for( int i = -20; i< 20 ; i += 2) {
                    painter->setPen(QPen(gripShadow, 1));
                    painter->drawLine(
                        QPoint(scrollBarSlider.center().x() + i ,
                               scrollBarSlider.top() + gripMargin),
                        QPoint(scrollBarSlider.center().x() + i,
                               scrollBarSlider.bottom() - gripMargin));
                    painter->setPen(QPen(palette.light(), 1));
                    painter->drawLine(
                        QPoint(scrollBarSlider.center().x() + i + 1,
                               scrollBarSlider.top() + gripMargin  ),
                        QPoint(scrollBarSlider.center().x() + i + 1,
                               scrollBarSlider.bottom() - gripMargin));
                }
            } else {
                for (int i = -20; i < 20 ; i += 2) {
                    painter->setPen(QPen(gripShadow, 1));
                    painter->drawLine(
                        QPoint(scrollBarSlider.left() + gripMargin ,
                               scrollBarSlider.center().y()+ i),
                        QPoint(scrollBarSlider.right() - gripMargin,
                               scrollBarSlider.center().y()+ i));
                    painter->setPen(QPen(palette.light(), 1));
                    painter->drawLine(
                        QPoint(scrollBarSlider.left() + gripMargin,
                               scrollBarSlider.center().y() + 1 + i),
                        QPoint(scrollBarSlider.right() - gripMargin,
                               scrollBarSlider.center().y() + 1 + i));

                }
            }
        }
        painter->restore();
        break;
#ifndef QT_NO_SIZEGRIP
    case CE_SizeGrip:
        painter->save();
        {
            int x, y, w, h;
            option->rect.getRect(&x, &y, &w, &h);
            int sw = qMin(h, w);
            if (h > w)
                painter->translate(0, h - w);
            else
                painter->translate(w - h, 0);
            
            int sx = x;
            int sy = y;
            int s = 4;
            QColor dark = option->palette.dark().color().light(110);
            if (option->direction == Qt::RightToLeft) {
                sx = x + sw;
                for (int i = 0; i < 4; ++i) {
                    painter->setPen(QPen(option->palette.light().color(), 1));
                    painter->drawLine(x, sy - 1 , sx + 1, sw);
                    painter->setPen(QPen(dark, 1));
                    painter->drawLine(x, sy, sx, sw);
                    sx -= s;
                    sy += s;
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    painter->setPen(QPen(option->palette.light().color(), 1));
                    painter->drawLine(sx - 1, sw, sw, sy - 1);
                    painter->setPen(QPen(dark, 1));
                    painter->drawLine(sx, sw, sw, sy);
                    sx += s;
                    sy += s;
                }
            }
        }
        painter->restore();
        break;
#endif // QT_NO_SIZEGRIP
#ifndef QT_NO_TOOLBAR
    case CE_ToolBar:
        painter->save();
        if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(option)) {
            QRect rect = option->rect;

            bool paintLeftBorder = true;
            bool paintRightBorder = true;
            bool paintBottomBorder = true;

            switch (toolbar->toolBarArea) {
            case Qt::BottomToolBarArea:
                switch(toolbar->positionOfLine) {
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintBottomBorder = false;
                default:
                    break;
                }
            case Qt::TopToolBarArea:
                switch (toolbar->positionWithinLine) {
                case QStyleOptionToolBar::Beginning:
                    paintLeftBorder = false;
                    break;
                case QStyleOptionToolBar::End:
                    paintRightBorder = false;
                    break;
                case QStyleOptionToolBar::OnlyOne:
                    paintRightBorder = false;
                    paintLeftBorder = false;
                default:
                    break;
                }
                if (QApplication::layoutDirection() == Qt::RightToLeft) { //reverse layout changes the order of Beginning/end
                    bool tmp = paintLeftBorder;
                    paintRightBorder=paintLeftBorder;
                    paintLeftBorder=tmp;
                }
                break;
            case Qt::RightToolBarArea:
                switch (toolbar->positionOfLine) {
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintRightBorder = false;
                    break;
                default:
                    break;
                }
                break;
            case Qt::LeftToolBarArea:
                switch (toolbar->positionOfLine) {
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintLeftBorder = false;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }

            QColor light = option->palette.background().color().light(110);

            //draw top border
            painter->setPen(QPen(light));
            painter->drawLine(rect.topLeft().x(),
                        rect.topLeft().y(),
                        rect.topRight().x(),
                        rect.topRight().y());

            if (paintLeftBorder) {
                painter->setPen(QPen(light));
                painter->drawLine(rect.topLeft().x(),
                            rect.topLeft().y(),
                            rect.bottomLeft().x(),
                            rect.bottomLeft().y());
            }

            if (paintRightBorder) {
                painter->setPen(QPen(shadow));
                painter->drawLine(rect.topRight().x(),
                            rect.topRight().y(),
                            rect.bottomRight().x(),
                            rect.bottomRight().y());
            }

            if (paintBottomBorder) {
                painter->setPen(QPen(shadow));
                painter->drawLine(rect.bottomLeft().x(),
                            rect.bottomLeft().y(),
                            rect.bottomRight().x(),
                            rect.bottomRight().y());
            }
        }
        painter->restore();
        break;
#endif // QT_NO_TOOLBAR
#ifndef QT_NO_DOCKWIDGET
    case CE_DockWidgetTitle:
        painter->save();
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            QRect r = dwOpt->rect.adjusted(0, 0, -1, 0);
            painter->setPen(dwOpt->palette.color(QPalette::Light));
            painter->setPen(option->palette.light().color());
            painter->drawRect(r.adjusted(1, 1, 1, 1));
            painter->setPen(shadow);
            painter->drawRect(r);

            if (!dwOpt->title.isEmpty()) {
                const int indent = painter->fontMetrics().descent();
                drawItemText(painter, r.adjusted(indent + 1, 0, - indent - 1, -1),
                            Qt::AlignLeft | Qt::AlignVCenter, dwOpt->palette,
                            dwOpt->state & State_Enabled, dwOpt->title,
                            QPalette::Foreground);
            }
        }
        painter->restore();
        break;
#endif // QT_NO_DOCKWIDGET
    case CE_HeaderSection:
        painter->save();
        // Draws the header in tables.
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QPixmap cache;
            QString pixmapName = uniqueName("headersection", option, option->rect.size());
            pixmapName += QLatin1String("-") + QString::number(int(header->position));
            pixmapName += QLatin1String("-") + QString::number(int(header->orientation));
            QRect r = option->rect;

            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(r.size());
                cache.fill(Qt::transparent);
                QRect pixmapRect(0, 0, r.width(), r.height());
                QPainter cachePainter(&cache);
                QColor gradientStartColor = option->palette.button().color();
                QColor gradientStopColor = option->palette.button().color().dark(105);

                if (header->orientation == Qt::Vertical) {
                    cachePainter.setPen(QPen(option->palette.dark().color()));
                    cachePainter.drawLine(pixmapRect.topRight(), pixmapRect.bottomRight());
                    if (header->position != QStyleOptionHeader::End) {
                        cachePainter.setPen(QPen(shadow));
                        cachePainter.drawLine(pixmapRect.bottomLeft() + QPoint(3, -1), pixmapRect.bottomRight() + QPoint(-3, -1));                       cachePainter.setPen(QPen(option->palette.light().color()));
                        cachePainter.drawLine(pixmapRect.bottomLeft() + QPoint(3, 0), pixmapRect.bottomRight() + QPoint(-3, 0));                     }
                } else {
                    cachePainter.setPen(QPen(option->palette.dark().color()));
                    cachePainter.drawLine(pixmapRect.bottomLeft(), pixmapRect.bottomRight());
                    if (header->position != QStyleOptionHeader::End) {
                        cachePainter.setPen(QPen(shadow));
                        cachePainter.drawLine(pixmapRect.topRight() + QPoint(-1, 3), pixmapRect.bottomRight() + QPoint(-1, -3));                         cachePainter.setPen(QPen(option->palette.light().color()));
                        cachePainter.drawLine(pixmapRect.topRight() + QPoint(0, 3), pixmapRect.bottomRight() + QPoint(0, -3));                       }
                }
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(r.topLeft(), cache);
        }
        painter->restore();
        break;
    case CE_ProgressBarGroove:
        painter->save();
        {
            painter->fillRect(rect, option->palette.base());
            QColor borderColor = option->palette.dark().color();
            painter->setPen(QPen(borderColor, 0));
            painter->drawLine(QPoint(rect.left() + 1, rect.top()), QPoint(rect.right() - 1, rect.top()));
            painter->drawLine(QPoint(rect.left() + 1, rect.bottom()), QPoint(rect.right() - 1, rect.bottom()));
            painter->drawLine(QPoint(rect.left(), rect.top() + 1), QPoint(rect.left(), rect.bottom() - 1));
            painter->drawLine(QPoint(rect.right(), rect.top() + 1), QPoint(rect.right(), rect.bottom() - 1));
            QColor alphaCorner = mergedColors(borderColor, option->palette.background().color());
            QColor innerShadow = mergedColors(borderColor, option->palette.base().color());

            //corner smoothing
            painter->setPen(alphaCorner);
            painter->drawPoint(rect.topRight());
            painter->drawPoint(rect.topLeft());
            painter->drawPoint(rect.bottomRight());
            painter->drawPoint(rect.bottomLeft());

            //inner shadow
            painter->setPen(innerShadow);
            painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1),
                              QPoint(rect.right() - 1, rect.top() + 1));
            painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1),
                              QPoint(rect.left() + 1, rect.bottom() + 1));

        }
        painter->restore();
        break;
    case CE_ProgressBarContents:
        painter->save();
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QRect rect = bar->rect;
            bool vertical = false;
            bool inverted = false;
            bool indeterminate = (bar->minimum == 0 && bar->maximum == 0);

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
                m.translate(rect.height()-1, -1.0);
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
                    progressBar.setRect(rect.left() + 1, rect.top() + 1, width + 1, rect.height() - 3);
                } else {
                    progressBar.setRect(rect.right() - 1 - width, rect.top() + 1, width + 1, rect.height() - 3);
                }
            } else {
                int step = 0;
                int slideWidth = rect.width() / 2;
                progressBar.setRect(rect.left() + 1 + step, rect.top() + 1,
                                    slideWidth / 2, rect.height() - 3);

            }
            painter->setPen(QPen(option->palette.highlight().color().dark(140), 0));

            QColor highlightedGradientStartColor = option->palette.highlight().color().light(100);
            QColor highlightedGradientStopColor = option->palette.highlight().color().light(130);

            QLinearGradient gradient(rect.topLeft(), QPoint(rect.bottomLeft().x(),
                                                            rect.bottomLeft().y()*2));

            gradient.setColorAt(0, highlightedGradientStartColor);
            gradient.setColorAt(1, highlightedGradientStopColor);

            painter->setBrush(gradient);
            painter->drawRect(progressBar);

            painter->setPen(QPen(option->palette.highlight().color().light(120), 0));
            painter->drawLine(QPoint(progressBar.left() + 1, progressBar.top() + 1),
                              QPoint(progressBar.right(), progressBar.top() + 1));
            painter->drawLine(QPoint(progressBar.left() + 1, progressBar.top() + 1),
                              QPoint(progressBar.left() + 1, progressBar.bottom() - 1));

            painter->setPen(QPen(highlightedGradientStartColor, 7.0));//QPen(option->palette.highlight(), 3));

            painter->save();
            painter->setClipRect(progressBar.adjusted(2, 2, -1, -1));
            for (int x = rect.left() - 32; x< rect.right() ; x+=18) {
                painter->drawLine(x, progressBar.bottom() + 1, x + 23, progressBar.top() - 2);
            }
            painter->restore();

        }
        painter->restore();
        break;
    case CE_MenuBarItem:
        painter->save();
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect.adjusted(0, 3, 0, -1);
            QColor highlightOutline = option->palette.highlight().color().dark(130);
            QLinearGradient gradient(rect.topLeft(), QPoint(rect.bottomLeft().x(), rect.bottomLeft().y()*2));
            gradient.setColorAt(0, option->palette.button().color());
            gradient.setColorAt(1, option->palette.button().color().dark(110));
            painter->fillRect(rect, gradient);

            QCommonStyle::drawControl(element, &item, painter, widget);

            bool act = mbi->state & State_Selected && mbi->state & State_Sunken;
            bool dis = !(mbi->state & State_Enabled);

            QRect r = option->rect;
            if (act) {
                qt_cleanlooks_draw_gradient(painter, r.adjusted(1, 1, -1, -1), option->palette.highlight().color(),
                                            highlightOutline);
                painter->setPen(QPen(highlightOutline, 0));
                painter->drawLine(QPoint(r.left(), r.top() + 1), QPoint(r.left(), r.bottom()));
                painter->drawLine(QPoint(r.right(), r.top() + 1), QPoint(r.right(), r.bottom()));
                painter->drawLine(QPoint(r.left() + 1, r.bottom()), QPoint(r.right() - 1, r.bottom()));
                painter->drawLine(QPoint(r.left() + 1, r.top()), QPoint(r.right() - 1, r.top()));

                //draw text
                QPalette::ColorRole textRole = dis ? QPalette::Text:
                                               act ? QPalette::HighlightedText : QPalette::ButtonText;
                uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                drawItemText(painter, item.rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
            }

        }
        painter->restore();
        break;
    case CE_MenuItem:
        painter->save();
        // Draws one item in a popup menu.
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            QColor highlightOutline = option->palette.highlight().color().dark(130);
            QColor menuBackground = option->palette.background().color().light(104);
            QColor borderColor = option->palette.background().color().dark(160);
            QColor alphaCornerColor;

            if (widget) {
                // ### backgroundrole/foregroundrole should be part of the style option
                alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
            } else {
                alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
            }
            QColor alphaTextColor = mergedColors(option->palette.background().color(), option->palette.text().color());
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                painter->fillRect(menuItem->rect, menuBackground);
                int w = 0;
                if (!menuItem->text.isEmpty()) {
                    painter->setFont(menuItem->font);
                    drawItemText(painter, menuItem->rect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter,
                                 menuItem->palette, menuItem->state & State_Enabled, menuItem->text,
                                 QPalette::Text);
                    w = menuItem->fontMetrics.width(menuItem->text) + 5;
                }
                painter->setPen(shadow.light(106));
                bool reverse = menuItem->direction == Qt::RightToLeft;
                painter->drawLine(menuItem->rect.left() + 5 + (reverse ? 0 : w), menuItem->rect.center().y(),
                                  menuItem->rect.right() - 5 - (reverse ? w : 0), menuItem->rect.center().y());
                painter->restore();
                break;
            }
            bool selected = menuItem->state & State_Selected && menuItem->state & State_Enabled;
            if (selected) {
                QRect r = option->rect.adjusted(1, 0, -2, -1);
                qt_cleanlooks_draw_gradient(painter, r, option->palette.highlight().color(), highlightOutline);
                r = r.adjusted(-1, 0, 1, 0);
                painter->setPen(QPen(highlightOutline, 0));
                painter->drawLine(QPoint(r.left(), r.top() + 1), QPoint(r.left(), r.bottom() - 1));
                painter->drawLine(QPoint(r.right(), r.top() + 1), QPoint(r.right(), r.bottom() - 1));
                painter->drawLine(QPoint(r.left() + 1, r.bottom()), QPoint(r.right() - 1, r.bottom()));
                painter->drawLine(QPoint(r.left() + 1, r.top()), QPoint(r.right() - 1, r.top()));
            } else {
                painter->fillRect(option->rect, menuBackground);
            }

            bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
            bool checked = menuItem->checked;
            bool sunken = menuItem->state & State_Sunken;
            bool enabled = menuItem->state & State_Enabled;

            // Check
            QRect checkRect(option->rect.left() + 7, option->rect.center().y() - 6, 13, 13);
            checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
            if (checkable) {
                if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
                    // Radio button
                    if (checked || sunken) {
                        painter->setRenderHint(QPainter::Antialiasing);
                        painter->setPen(Qt::NoPen);

                        QPalette::ColorRole textRole = !enabled ? QPalette::Text:
                                                       selected ? QPalette::HighlightedText : QPalette::ButtonText;
                        painter->setBrush(option->palette.brush( option->palette.currentColorGroup(), textRole));
                        painter->drawEllipse(checkRect.adjusted(4, 4, -4, -4));
                    }
                } else {
                    // Check box
                    if (menuItem->icon.isNull()) {
                        if (checked || sunken) {
                            QImage image(qt_cleanlooks_menuitem_checkbox_checked);
                            if (menuItem->state & State_Selected) {
                                image.setColor(1, 0x55ffffff);
                                image.setColor(2, 0xAAffffff);
                                image.setColor(3, 0xBBffffff);
                                image.setColor(4, 0xFFffffff);
                                image.setColor(5, 0x33ffffff);
                            } else {
                                image.setColor(1, 0x55000000);
                                image.setColor(2, 0xAA000000);
                                image.setColor(3, 0xBB000000);
                                image.setColor(4, 0xFF000000);
                                image.setColor(5, 0x33000000);
                            }
                            painter->drawImage(QPoint(checkRect.center().x() - image.width() / 2,
                                                      checkRect.center().y() - image.height() / 2), image);
                        }
                    } else if (checked) {
                        int iconSize = qMax(menuItem->maxIconWidth, 20);
                        QRect sunkenRect(option->rect.left() + 2,
                                         option->rect.top() + (option->rect.height() - iconSize) / 2,
                                         iconSize, iconSize);
                        sunkenRect = visualRect(menuItem->direction, menuItem->rect, sunkenRect);

                        QStyleOption opt = *option;
                        opt.state |= State_Sunken;
                        opt.rect = sunkenRect;
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
                                                     QRect(textRect.topRight(), QPoint(menuitem->rect.right(), textRect.bottom())));
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1, 1, 1, 1), text_flags, s.mid(t + 1));
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
                    p->drawText(vTextRect.adjusted(1, 1, 1, 1), text_flags, s.left(t));
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
                int xpos = menuItem->rect.left() + menuItem->rect.width() - 1 - dim;
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
        }
        painter->restore();
        break;
    case CE_MenuHMargin:
    case CE_MenuVMargin:
        break;
    case CE_MenuEmptyArea:
        break;
    case CE_MenuBarEmptyArea:
        painter->save();
        {
            QColor shadow = mergedColors(option->palette.background().color().dark(120),
                                 option->palette.dark().color().light(130), 60);

            QLinearGradient gradient(rect.topLeft(), QPoint(rect.bottomLeft().x(), rect.bottomLeft().y()*2));
            gradient.setColorAt(0, option->palette.button().color());
            gradient.setColorAt(1, option->palette.button().color().dark(110));
            painter->fillRect(rect, gradient);

            if (widget && qobject_cast<const QMainWindow *>(widget->parentWidget())) {
                QPen oldPen = painter->pen();
                painter->setPen(QPen(shadow));
                painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
            }
        }
        painter->restore();
        break;
#ifndef QT_NO_TABBAR
    case CE_TabBarTabShape:
        painter->save();
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QColor tabFrameColor = mergedColors(option->palette.background().color(),
                                                option->palette.dark().color().light(125), 80);

            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth
                                   || tab->shape == QTabBar::RoundedSouth));
            bool selected = tab->state & State_Selected;
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs
                                && tab->position == QStyleOptionTab::Beginning));
            bool onlyTab = tab->position == QStyleOptionTab::OnlyOneTab;
            bool leftCornerWidget = (tab->cornerWidgets & QStyleOptionTab::LeftCornerWidget);
            bool atBeginning = ((tab->position == QStyleOptionTab::Beginning) || onlyTab)
                               && !leftCornerWidget;

            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            bool previousSelected =
                ((!rtlHorTabs
                  && tab->selectedPosition == QStyleOptionTab::PreviousIsSelected)
                 || (rtlHorTabs
                     && tab->selectedPosition == QStyleOptionTab::NextIsSelected));
            bool nextSelected =
                ((!rtlHorTabs
                  && tab->selectedPosition == QStyleOptionTab::NextIsSelected)
                 || (rtlHorTabs
                     && tab->selectedPosition
                     == QStyleOptionTab::PreviousIsSelected));
            int tabBarAlignment = styleHint(SH_TabBar_Alignment, tab, widget);
            bool leftAligned = (!rtlHorTabs && tabBarAlignment == Qt::AlignLeft)
                               || (rtlHorTabs
                                   && tabBarAlignment == Qt::AlignRight);

            bool rightAligned = (!rtlHorTabs && tabBarAlignment == Qt::AlignRight)
                                || (rtlHorTabs
                                    && tabBarAlignment == Qt::AlignLeft);

            QColor light = tab->palette.light().color();
            QColor midlight = tab->palette.midlight().color();
            QColor dark = tab->palette.dark().color();
            QColor shadow = tab->palette.shadow().color();
            QColor background = tab->palette.background().color();
            int borderThinkness = pixelMetric(PM_TabBarBaseOverlap, tab, widget);
            if (selected)
                borderThinkness /= 2;
            QRect r2(option->rect);
            int x1 = r2.left();
            int x2 = r2.right();
            int y1 = r2.top();
            int y2 = r2.bottom();

            QMatrix rotMatrix;
            bool flip = false;
            painter->setPen(shadow);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setMatrix(rotMatrix);
                break;
            case QTabBar::RoundedWest:
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                flip = true;
                painter->setMatrix(rotMatrix);
                break;
            case QTabBar::RoundedEast:
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - rect.width() + 1);
                flip = true;
                painter->setMatrix(rotMatrix);
                break;
            default:
                painter->restore();
                QWindowsStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                int tmp = rect.width();
                rect.setWidth(rect.height());
                rect.setHeight(tmp);
                int temp = x1;
                x1 = y1;
                y1 = temp;
                temp = x2;
                x2 = y2;
                y2 = temp;
            }

            QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
            if (selected) {
                gradient.setColorAt(0, option->palette.background().color().light(104));
                gradient.setColorAt(1, tabFrameColor);
            } else {
                y1 += 2;
                gradient.setColorAt(0, option->palette.button().color());
                gradient.setColorAt(1, option->palette.button().color().dark(115));
            }
            painter->fillRect(rect.adjusted(0, 2, 0, -1), gradient);

            // Delete border
            if (selected) {
                painter->setPen(QPen(option->palette.highlight(), 0));
                painter->drawLine(x1 + 1, y1 + 1, x2 - 1, y1 + 1);
                painter->drawLine(x1 , y1 + 2, x2 , y1 + 2);
            } else {
                painter->setPen(dark);
                painter->drawLine(x1, y2 - 1, x2 + 2, y2 - 1 );
                if (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedWest) {
                    painter->setPen(light);
                    painter->drawLine(x1, y2 , x2, y2 );
                }
            }
            // Left
            if (atBeginning|| selected ) {
                painter->setPen(light);
                painter->drawLine(x1 + 1, y1 + 2 + 1, x1 + 1, y2 - ((onlyOne || atBeginning) && selected && leftAligned ? 0 : borderThinkness));
                painter->drawPoint(x1 + 1, y1 + 1);
                painter->setPen(dark);
                painter->drawLine(x1, y1 + 2, x1, y2 - ((onlyOne || atBeginning)  && leftAligned ? 0 : borderThinkness));
            }
            // Top
            {
                int beg = x1 + (previousSelected ? 0 : 2);
                int end = x2 - (nextSelected ? 0 : 2);
                painter->setPen(light);

                if (!selected)painter->drawLine(beg - 2, y1 + 1, end, y1 + 1);

                if (selected)
                    painter->setPen(QPen(option->palette.highlight().color().dark(150), 0));
                else
                    painter->setPen(dark);
                painter->drawLine(beg, y1 , end, y1);

                if (atBeginning|| selected) {
                    painter->drawPoint(beg - 1, y1 + 1);
                } else if (!atBeginning) {
                    painter->drawPoint(beg - 1, y1);
                    painter->drawPoint(beg - 2, y1);
                    if (!lastTab) {
                        painter->setPen(dark.light(130));
                        painter->drawPoint(end + 1, y1);
                        painter->drawPoint(end + 2 , y1);
                        painter->drawPoint(end + 2, y1 + 1);
                    }
                }
            }
            // Right
            if (lastTab || selected || onlyOne || !nextSelected) {
                painter->setPen(dark);
                painter->drawLine(x2, y1 + 2, x2, y2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                if (selected)
                    painter->setPen(QPen(option->palette.highlight().color().dark(150), 0));
                else
                    painter->setPen(dark);
                painter->drawPoint(x2 - 1, y1 + 1);

                if (selected) {
                    painter->setPen(dark.light(130));
                    painter->drawLine(x2 - 1, y1 + 3, x2 - 1, y2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                }
            }
        }
        painter->restore();
        break;

#endif // QT_NO_TABBAR
    default:
        QWindowsStyle::drawControl(element,option,painter,widget);
        break;
    }
}

/*!
  \reimp
*/
QPalette QCleanLooksStyle::standardPalette () const
{
    QPalette palette = QWindowsStyle::standardPalette();

    palette.setBrush(QPalette::Active, QPalette::Highlight, QColor(98, 140, 178));
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, QColor(145, 141, 126));
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(145, 141, 126));

    QColor backGround(239, 235, 231);
    QColor light = backGround.light(150);
    QColor base = Qt::white;
    QColor dark = QColor(170, 156, 143);
    QColor darkDisabled = QColor(209, 200, 191);

    //### Find the correct disabled text color
    palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(190, 190, 190));

    palette.setBrush(QPalette::Background, backGround);
    palette.setBrush(QPalette::Mid, backGround.dark(130));
    palette.setBrush(QPalette::Light, light);

    palette.setBrush(QPalette::Active, QPalette::Base, base);
    palette.setBrush(QPalette::Inactive, QPalette::Base, base);
    palette.setBrush(QPalette::Disabled, QPalette::Base, backGround);

    palette.setBrush(QPalette::Midlight, palette.mid().color().light(110));

    palette.setBrush(QPalette::All, QPalette::Dark, dark);
    palette.setBrush(QPalette::Disabled, QPalette::Dark, darkDisabled);

    QColor button(239, 235, 230);

    palette.setBrush(QPalette::Button, button);

    QColor shadow = dark.dark(135);
    palette.setBrush(QPalette::Shadow, shadow);
    palette.setBrush(QPalette::Disabled, QPalette::Shadow, shadow.light(150));
    palette.setBrush(QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    return palette;
}

/*!
  \reimp
*/
void QCleanLooksStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                         QPainter *painter, const QWidget *widget) const
{
    QColor borderColor = option->palette.dark().color().light(110);
    QColor alphaCornerColor;
    if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), borderColor);
    } else {
        alphaCornerColor = mergedColors(option->palette.background().color(), borderColor);
    }
    QColor grooveColor = mergedColors(option->palette.dark().color(), option->palette.button().color(), 50);
    QColor gripShadow = grooveColor.dark(110);
    QColor buttonShadow = option->palette.button().color().dark(110);

    QColor gradientStartColor = option->palette.button().color().light(108);
    QColor gradientStopColor = mergedColors(option->palette.button().color().dark(108), option->palette.dark().color().light(140), 70);

    QColor highlightedGradientStartColor = option->palette.button().color();
    QColor highlightedGradientStopColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 85);

    QColor highlightedDarkInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 35);
    QColor highlightedLightInnerBorderColor = mergedColors(option->palette.button().color(), option->palette.highlight().color(), 58);

    QPalette palette = option->palette;

    switch (control) {
#ifndef QT_NO_SPINBOX
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            QPixmap cache;
            QString pixmapName = uniqueName("spinbox", spinBox, spinBox->rect.size());
            if (!UsePixmapCache || !QPixmapCache::find(pixmapName, cache)) {
                cache = QPixmap(spinBox->rect.size());
                cache.fill(Qt::transparent);
                QRect pixmapRect(0, 0, spinBox->rect.width(), spinBox->rect.height());
                QPainter cachePainter(&cache);

                bool isEnabled = (spinBox->state & State_Enabled);
                //bool focus = isEnabled && (spinBox->state & State_HasFocus);
                bool hover = isEnabled && (spinBox->state & State_MouseOver);
                bool sunken = (spinBox->state & State_Sunken);
                bool upIsActive = (spinBox->activeSubControls == SC_SpinBoxUp);
                bool downIsActive = (spinBox->activeSubControls == SC_SpinBoxDown);

                QRect rect = pixmapRect;
                QStyleOptionSpinBox spinBoxCopy = *spinBox;
                spinBoxCopy.rect = pixmapRect;
                QRect upRect = subControlRect(CC_SpinBox, &spinBoxCopy, SC_SpinBoxUp, widget);
                QRect downRect = subControlRect(CC_SpinBox, &spinBoxCopy, SC_SpinBoxDown, widget);
                if (spinBox->direction == Qt::RightToLeft) {
                }
                if (isEnabled) {
                    // gradients
                    QRect buttonRect = upRect.unite(downRect);
                    qt_cleanlooks_draw_gradient(&cachePainter, buttonRect,
                                            gradientStartColor,
                                            gradientStopColor);
                    if(upIsActive) {
                        if (sunken) {
                            cachePainter.fillRect(upRect.adjusted(1, 1, -1, 0), gradientStopColor.dark(110));
                        } else if (hover) {
                            qt_cleanlooks_draw_gradient(&cachePainter, upRect.adjusted(1, 1, -1, -1),
                                                    gradientStartColor.light(120),
                                                    gradientStopColor.light(120));
                        }
                    }
                    if(downIsActive) {
                        if (sunken) {
                            cachePainter.fillRect(downRect.adjusted(1, 0, -1, -1), gradientStopColor.dark(110));

                        } else if (hover) {
                                qt_cleanlooks_draw_gradient(&cachePainter, downRect.adjusted(1, 1, -1, -1),
                                                        gradientStartColor.light(120),
                                                        gradientStopColor.light(120));
                        }
                    }
                }

                // Draw a line edit
                QStyleOptionFrame lineEdit;
                lineEdit.QStyleOption::operator=(*spinBox);
                lineEdit.rect = pixmapRect;
                drawPrimitive(PE_FrameLineEdit, &lineEdit, &cachePainter, widget);

                // outline the up/down buttons
                cachePainter.setPen(borderColor);
                if (spinBox->direction == Qt::RightToLeft) {
                    cachePainter.drawLine(upRect.right(), upRect.top(), upRect.right(), downRect.bottom());
                    cachePainter.setPen(option->palette.light().color().light());
                    cachePainter.drawLine(upRect.right() - 1, upRect.top() + 1, upRect.right() - 1, downRect.bottom() - 1);
                } else {
                    cachePainter.drawLine(upRect.left(), upRect.top(), upRect.left(), downRect.bottom());
                    cachePainter.setPen(option->palette.light().color().light());
                    cachePainter.drawLine(upRect.left() + 1, upRect.top() + 1, upRect.left() + 1, downRect.bottom() - 1);
                }

                if(upIsActive && sunken) {
                    cachePainter.setPen(gradientStopColor.dark(130));
                    cachePainter.drawLine(upRect.left() + 1, upRect.top() + 1, upRect.left() + 1, upRect.bottom() - 1);
                    cachePainter.drawLine(upRect.left() + 1, upRect.top() + 1, upRect.right() - 1, upRect.top() + 1);
                } else {
                    cachePainter.setPen(borderColor);
                    cachePainter.drawLine(upRect.bottomLeft() + QPoint(3, 0), upRect.bottomRight()- QPoint(3, 0));
                }
                if(downIsActive && sunken) {
                    cachePainter.setPen(gradientStopColor.dark(130));
                    cachePainter.drawLine(downRect.left() + 1, downRect.top() + 1, downRect.left() + 1, downRect.bottom() - 1);
                    cachePainter.drawLine(downRect.left() + 1, downRect.top() + 1, downRect.right() - 1, downRect.top() + 1);
                } else {
                    cachePainter.setPen(option->palette.light().color().light());
                    cachePainter.drawLine(downRect.topLeft() + QPoint(3, 0), downRect.topRight() - QPoint(3, 0));
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
                    QImage upArrow(qt_spinbox_button_arrow_up);
                    upArrow.setColor(1, spinBox->palette.foreground().color().rgba());

                    cachePainter.drawImage(upRect.center().x() - upArrow.width() / 2,
                                            upRect.center().y() - upArrow.height() / 2,
                                            upArrow);

                    QImage downArrow(qt_spinbox_button_arrow_down);
                    downArrow.setColor(1, spinBox->palette.foreground().color().rgba());

                    cachePainter.drawImage(downRect.center().x() - downArrow.width() / 2,
                                            downRect.center().y() - downArrow.height() / 2,
                                            downArrow);

                }
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(spinBox->rect.topLeft(), cache);
        }
        break;
#endif // QT_NO_SPINBOX
    case CC_TitleBar:
        painter->save();

        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            bool active = (titleBar->titleBarState & State_Active);
            QRect fullRect = titleBar->rect;
            QPalette palette = option->palette;

            QColor titleBarGradientStart(active ? palette.highlight().color(): palette.background().color());
            QColor titleBarGradientStop(active ? palette.highlight().color().dark(150): palette.background().color().dark(120));
            QColor titleBarFrameBorder(active ? palette.highlight().color().dark(180): palette.dark().color().dark(120));
            QColor titleBarHighlight(active ? palette.highlight().color().light(120): palette.background().color().light(120));
            QColor textColor(active ? 0xffffff : 0xff000000);
            QColor textAlphaColor(active ? 0xffffff : 0xff000000 );

            // Fill titlebar gradient
            qt_cleanlooks_draw_gradient(painter, option->rect.adjusted(1, 1, -1, 0),
                                       titleBarGradientStart,
                                       titleBarGradientStop);

            // Frame and rounded corners
            painter->setPen(titleBarFrameBorder);

            // top outline
            painter->drawLine(fullRect.left() + 5, fullRect.top(), fullRect.right() - 5, fullRect.top());
            painter->drawLine(fullRect.left(), fullRect.top() + 4, fullRect.left(), fullRect.bottom());
            painter->drawPoint(fullRect.left() + 4, fullRect.top() + 1);
            painter->drawPoint(fullRect.left() + 3, fullRect.top() + 1);
            painter->drawPoint(fullRect.left() + 2, fullRect.top() + 2);
            painter->drawPoint(fullRect.left() + 1, fullRect.top() + 3);
            painter->drawPoint(fullRect.left() + 1, fullRect.top() + 4);

            painter->drawLine(fullRect.right(), fullRect.top() + 4, fullRect.right(), fullRect.bottom());
            painter->drawPoint(fullRect.right() - 3, fullRect.top() + 1);
            painter->drawPoint(fullRect.right() - 4, fullRect.top() + 1);
            painter->drawPoint(fullRect.right() - 2, fullRect.top() + 2);
            painter->drawPoint(fullRect.right() - 1, fullRect.top() + 3);
            painter->drawPoint(fullRect.right() - 1, fullRect.top() + 4);

            // draw bottomline
            painter->drawLine(fullRect.right(), fullRect.bottom(), fullRect.left(), fullRect.bottom());

            // top highlight
            painter->setPen(titleBarHighlight);
            painter->drawLine(fullRect.left() + 6, fullRect.top() + 1, fullRect.right() - 6, fullRect.top() + 1);

            // draw title
            QRect textRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarLabel, widget);
            QFont font = painter->font();
            font.setBold(true);
            painter->setFont(font);
            painter->setPen(active? (titleBar->palette.text().color().light(120)) : 
                                     titleBar->palette.text().color() );
                painter->drawText(textRect.adjusted(1, 1, 1, 1), titleBar->text, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
            painter->setPen(titleBar->palette.highlightedText().color());
            if (active)
                painter->drawText(textRect, titleBar->text, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));

            // min button
            if ((titleBar->subControls & SC_TitleBarMinButton) && (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint)) {
                QRect minButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarMinButton, widget);
                qt_cleanlooks_draw_mdibutton(painter, titleBar, minButtonRect);

                int xoffset = 5;//closeButtonRect.width() / 3;
                int yoffset = 5;//closeButtonRect.height() / 3;

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
                QRect maxButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarMaxButton, widget);
                qt_cleanlooks_draw_mdibutton(painter, titleBar, maxButtonRect);

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
                QRect closeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarCloseButton, widget);
                qt_cleanlooks_draw_mdibutton(painter, titleBar, closeButtonRect);

                int xoffset = 5;//closeButtonRect.width() / 3;
                int yoffset = 5;//closeButtonRect.height() / 3;

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
            if ((titleBar->subControls & SC_TitleBarNormalButton) && (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint)) {
                QRect normalButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarNormalButton, widget);
                qt_cleanlooks_draw_mdibutton(painter, titleBar, normalButtonRect);

                int xoffset = 5;//closeButtonRect.width() / 3;
                int yoffset = 5;//closeButtonRect.height() / 3;

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
            if (titleBar->subControls & SC_TitleBarContextHelpButton) {
                bool hover = (titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_MouseOver);
                QRect contextHelpButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarContextHelpButton, widget);

                qt_cleanlooks_draw_mdibutton(painter, titleBar, contextHelpButtonRect);

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
                QRect shadeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarShadeButton, widget);
                qt_cleanlooks_draw_mdibutton(painter, titleBar, shadeButtonRect);

                int xoffset = 5;//closeButtonRect.width() / 3;
                int yoffset = 5;//closeButtonRect.height() / 3;

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
                QRect unshadeButtonRect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarUnshadeButton, widget);
                qt_cleanlooks_draw_mdibutton(painter, titleBar, unshadeButtonRect);

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
                    qt_cleanlooks_draw_mdibutton(painter, titleBar, iconRect);

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
        }
        painter->restore();
        break;
    case CC_ScrollBar:
        painter->save();
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            bool isEnabled = scrollBar->state & State_Enabled;
            bool reverse = scrollBar->direction == Qt::RightToLeft;
            bool horizontal = scrollBar->orientation == Qt::Horizontal;
            bool sunken = scrollBar->state & State_Sunken;

            painter->fillRect(option->rect, option->palette.background());

            QRect rect = scrollBar->rect;
            QRect scrollBarSubLine = subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
            QRect scrollBarAddLine = subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
            QRect scrollBarSlider = subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);
            QRect grooveRect = subControlRect(control, scrollBar, SC_ScrollBarGroove, widget);

            // paint groove
            if (scrollBar->subControls & SC_ScrollBarGroove) {
                painter->setBrush(grooveColor);
                painter->setPen(Qt::NoPen);
                if (horizontal) {
                    painter->drawRect(grooveRect);
                    painter->setPen(borderColor.dark(110));
                    painter->drawLine(grooveRect.topLeft(), grooveRect.topRight());
                    painter->drawLine(grooveRect.bottomLeft(), grooveRect.bottomRight());
                } else {
                    painter->drawRect(grooveRect);
                    painter->setPen(borderColor.dark(110));
                    painter->drawLine(grooveRect.topLeft(), grooveRect.bottomLeft());
                    painter->drawLine(grooveRect.topRight(), grooveRect.bottomRight());
                }
            }
            //paint slider
            if (scrollBar->subControls & SC_ScrollBarSlider) {
                QRect pixmapRect = scrollBarSlider;
                if (horizontal)
                    pixmapRect.adjust(-1, 0, 0, -1);
                else
                    pixmapRect.adjust(0, -1, -1, 0);

                if (isEnabled) {
                    QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                             pixmapRect.center().x(), pixmapRect.bottom());
                    if (!horizontal)
                        gradient = QLinearGradient(pixmapRect.left(), pixmapRect.center().y(),
                                                   pixmapRect.right(), pixmapRect.center().y());
                    if (sunken || (option->state & State_MouseOver && 
                        (scrollBar->activeSubControls & SC_ScrollBarSlider))) {
                        gradient.setColorAt(0, gradientStartColor.light(110));
                        gradient.setColorAt(1, gradientStopColor.light(110));
                    } else {
                        gradient.setColorAt(0, gradientStartColor);
                        gradient.setColorAt(1, gradientStopColor);
                    }
                    painter->setPen(QPen(option->palette.shadow(), 1));
                    painter->setBrush(gradient);
                    painter->drawRect(pixmapRect);


                    //calculate offsets used by highlight and shadow
                    int yoffset, xoffset;
                    if (option->state & State_Horizontal) {
                        xoffset = 0;
                        yoffset = 1;
                    } else {
                        xoffset = 1;
                        yoffset = 0;
                    }
                    //draw slider highlights
                    painter->setPen(QPen(gradientStopColor, 0));
                    painter->drawLine(scrollBarSlider.left() + xoffset,
                                      scrollBarSlider.bottom() - yoffset,
                                      scrollBarSlider.right() - xoffset,
                                      scrollBarSlider.bottom() - yoffset);
                    painter->drawLine(scrollBarSlider.right() - xoffset,
                                      scrollBarSlider.top() + yoffset,
                                      scrollBarSlider.right() - xoffset,
                                      scrollBarSlider.bottom() - yoffset);

                    //draw slider shadow
                    painter->setPen(QPen(gradientStartColor, 0));
                    painter->drawLine(scrollBarSlider.left() + xoffset,
                                      scrollBarSlider.top() + yoffset,
                                      scrollBarSlider.right() - xoffset,
                                      scrollBarSlider.top() + yoffset);
                    painter->drawLine(scrollBarSlider.left() + xoffset,
                                      scrollBarSlider.top() + yoffset,
                                      scrollBarSlider.left() + xoffset,
                                      scrollBarSlider.bottom() - yoffset);
                } else {
                    QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                             pixmapRect.center().x(), pixmapRect.bottom());
                    if (!horizontal) {
                        gradient = QLinearGradient(pixmapRect.left(), pixmapRect.center().y(),
                                                   pixmapRect.right(), pixmapRect.center().y());
                    }
                    if (sunken || (option->state & State_MouseOver)) {
                        gradient.setColorAt(0, gradientStartColor.light(110));
                        gradient.setColorAt(1, gradientStopColor.light(110));
                    } else {
                        gradient.setColorAt(0, gradientStartColor);
                        gradient.setColorAt(1, gradientStopColor);
                    }
                    painter->setPen(borderColor);
                    painter->setBrush(gradient);
                    painter->drawRect(pixmapRect);
                }
                int gripMargin = 4;
                //draw grips
                if (horizontal) {
                    for (int i = -3; i< 6 ; i += 3) {
                        painter->setPen(QPen(gripShadow, 1));
                        painter->drawLine(
                            QPoint(scrollBarSlider.center().x() + i ,
                                   scrollBarSlider.top() + gripMargin),
                            QPoint(scrollBarSlider.center().x() + i,
                                   scrollBarSlider.bottom() - gripMargin));
                        painter->setPen(QPen(palette.light(), 1));
                        painter->drawLine(
                            QPoint(scrollBarSlider.center().x() + i + 1,
                                   scrollBarSlider.top() + gripMargin  ),
                            QPoint(scrollBarSlider.center().x() + i + 1,
                                   scrollBarSlider.bottom() - gripMargin));
                    }
                } else {
                    for (int i = -3; i < 6 ; i += 3) {
                        painter->setPen(QPen(gripShadow, 1));
                        painter->drawLine(
                            QPoint(scrollBarSlider.left() + gripMargin ,
                                   scrollBarSlider.center().y()+ i),
                            QPoint(scrollBarSlider.right() - gripMargin,
                                   scrollBarSlider.center().y()+ i));
                        painter->setPen(QPen(palette.light(), 1));
                        painter->drawLine(
                            QPoint(scrollBarSlider.left() + gripMargin,
                                   scrollBarSlider.center().y() + 1 + i),
                            QPoint(scrollBarSlider.right() - gripMargin,
                                   scrollBarSlider.center().y() + 1 + i));
                    }
                }
            }

            // The SubLine (up/left) buttons
            if (scrollBar->subControls & SC_ScrollBarSubLine) {
                //int scrollBarExtent = pixelMetric(PM_ScrollBarExtent, option, widget);
                QRect pixmapRect = scrollBarSubLine;
                if (isEnabled ) {
                    QRect fillRect = pixmapRect.adjusted(1, 1, -1, -1);
                    // Gradients
                    if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                        qt_cleanlooks_draw_gradient(painter,
                                                    QRect(fillRect),
                                                    gradientStopColor.dark(120),
                                                    gradientStopColor.dark(120),
                                                    horizontal ? TopDown : FromLeft);
                    } else {
                        qt_cleanlooks_draw_gradient(painter,
                                                    QRect(fillRect),
                                                    gradientStartColor.light(105),
                                                    gradientStopColor,
                                                    horizontal ? TopDown : FromLeft);
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
                subButton.setColor(2, option->palette.shadow().color().light(110).rgba());
                if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken) {
                    subButton.setColor(3, gradientStopColor.dark(140).rgba());
                    subButton.setColor(4, gradientStopColor.dark(120).rgba());
                } else {
                    subButton.setColor(3, gradientStartColor.light(105).rgba());
                    subButton.setColor(4, gradientStopColor.rgba());
                }
                subButton.setColor(5, scrollBar->palette.text().color().rgba());
                painter->drawImage(pixmapRect, subButton);

                // Arrows
                if (horizontal) {
                    QImage arrow(reverse ? qt_scrollbar_button_arrow_right : qt_scrollbar_button_arrow_left);
                    arrow.setColor(1, scrollBar->palette.foreground().color().rgba());

                    painter->drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 4), arrow);
                } else {
                    QImage arrow(qt_scrollbar_button_arrow_up);
                    arrow.setColor(1, scrollBar->palette.foreground().color().rgba());
                    painter->drawImage(QPoint(pixmapRect.left() + 4, pixmapRect.top() + 6), arrow);
                }

                // The AddLine (down/right) button
                if (scrollBar->subControls & SC_ScrollBarAddLine) {
                    QString addLinePixmapName = uniqueName("scrollbar_addline", option, QSize(16, 16));
                    QRect pixmapRect = scrollBarAddLine;
                    if (isEnabled) {
                        QRect fillRect = pixmapRect.adjusted(1, 1, -1, -1);
                        // Gradients
                        if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                            qt_cleanlooks_draw_gradient(painter,
                                                        fillRect,
                                                        gradientStopColor.dark(120),
                                                        gradientStopColor.dark(120),
                                                        horizontal ? TopDown: FromLeft);
                        } else {
                            qt_cleanlooks_draw_gradient(painter,
                                                        fillRect,
                                                        gradientStartColor.light(105),
                                                        gradientStopColor,
                                                        horizontal ? TopDown : FromLeft);
                        }
                    }
                    // Details
                    QImage addButton;
                    if (horizontal) {
                        addButton = QImage(reverse ? qt_scrollbar_button_left : qt_scrollbar_button_right);
                    } else {
                        addButton = QImage(qt_scrollbar_button_down);
                    }
                    addButton.setColor(1, alphaCornerColor.rgba());
                    addButton.setColor(2, option->palette.shadow().color().light(110).rgba());
                    if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                        addButton.setColor(3, gradientStopColor.dark(140).rgba());
                        addButton.setColor(4, gradientStopColor.dark(120).rgba());
                    } else {
                        addButton.setColor(3, gradientStartColor.light(105).rgba());
                        addButton.setColor(4, gradientStopColor.rgba());
                    }
                    addButton.setColor(5, scrollBar->palette.text().color().rgba());
                    painter->drawImage(pixmapRect, addButton);

                    // Arrow
                    if (horizontal) {
                        QImage arrow(reverse ? qt_scrollbar_button_arrow_left : qt_scrollbar_button_arrow_right);
                        arrow.setColor(1, scrollBar->palette.foreground().color().rgba());

                        if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                            painter->drawImage(QPoint(pixmapRect.left() + 7, pixmapRect.top() + 5), arrow);
                        } else {
                            painter->drawImage(QPoint(pixmapRect.left() + 6, pixmapRect.top() + 4), arrow);
                        }
                    } else {
                        QImage arrow(qt_scrollbar_button_arrow_down);
                        arrow.setColor(1, scrollBar->palette.foreground().color().rgba());
                        if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken) {
                            painter->drawImage(QPoint(pixmapRect.left() + 5, pixmapRect.top() + 7), arrow);
                        } else {
                            painter->drawImage(QPoint(pixmapRect.left() + 4, pixmapRect.top() + 6), arrow);
                        }
                    }
                }
            }
        }
        painter->restore();
        break;;
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        painter->save();
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            bool sunken = (comboBox->state & (State_Sunken | State_On));
            bool isEnabled = (comboBox->state & State_Enabled);
            bool focus = isEnabled && (comboBox->state & State_HasFocus);
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
                cache.fill(Qt::transparent);
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
                    QStyleOptionFrame  buttonOption;
                    buttonOption.QStyleOption::operator=(*comboBox);
                    buttonOption.rect = rect;
                    buttonOption.state = comboBox->state & (State_Enabled | State_MouseOver);
                    if (sunken) {
                        buttonOption.state |= State_Sunken;
                        buttonOption.state &= ~State_MouseOver;
                    }
                    drawPrimitive(PE_PanelButtonCommand, &buttonOption, &cachePainter, widget);
                    cachePainter.setPen(option->palette.dark().color());
                    if (!sunken) {
                        int borderSize = 2;
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(QPoint(downArrowRect.right() - 1, downArrowRect.top() + borderSize ),
                                                  QPoint(downArrowRect.right() - 1, downArrowRect.bottom() - borderSize));
                            cachePainter.setPen(option->palette.light().color());
                            cachePainter.drawLine(QPoint(downArrowRect.right(), downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.right(), downArrowRect.bottom() - borderSize));
                        } else {
                            cachePainter.drawLine(QPoint(downArrowRect.left() , downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.left() , downArrowRect.bottom() - borderSize));
                            cachePainter.setPen(option->palette.light().color());
                            cachePainter.drawLine(QPoint(downArrowRect.left() + 1, downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.left() + 1, downArrowRect.bottom() - borderSize));
                        }
                    } else {
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(QPoint(downArrowRect.right(), downArrowRect.top() + 2),
                                                  QPoint(downArrowRect.right(), downArrowRect.bottom() - 2));

                        } else {
                            cachePainter.drawLine(QPoint(downArrowRect.left(), downArrowRect.top() + 2),
                                                  QPoint(downArrowRect.left(), downArrowRect.bottom() - 2));
                        }
                    }
                } else {
                    QStyleOptionButton buttonOption;
                    buttonOption.QStyleOption::operator=(*comboBox);
                    buttonOption.rect = rect;
                    buttonOption.state = comboBox->state & (State_Enabled | State_MouseOver);
                    if (sunken) {
                        buttonOption.state |= State_Sunken;
                        buttonOption.state &= ~State_MouseOver;
                    }
                    drawPrimitive(PE_PanelButtonCommand, &buttonOption, &cachePainter, widget);

                    cachePainter.setPen(buttonShadow.dark(102));
                    int borderSize = 4;
                       
                    if (!sunken) {
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(QPoint(downArrowRect.right() + 1, downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.right() + 1, downArrowRect.bottom() - borderSize));
                            cachePainter.setPen(option->palette.light().color());
                            cachePainter.drawLine(QPoint(downArrowRect.right(), downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.right(), downArrowRect.bottom() - borderSize));
                        } else {
                            cachePainter.drawLine(QPoint(downArrowRect.left() - 1, downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.left() - 1, downArrowRect.bottom() - borderSize));
                            cachePainter.setPen(option->palette.light().color());
                            cachePainter.drawLine(QPoint(downArrowRect.left() , downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.left() , downArrowRect.bottom() - borderSize));
                        }
                    } else {
                        cachePainter.setPen(option->palette.dark().color());
                        if (comboBox->direction == Qt::RightToLeft) {
                            cachePainter.drawLine(QPoint(downArrowRect.right() + 1, downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.right() + 1, downArrowRect.bottom() - borderSize));

                        } else {
                            cachePainter.drawLine(QPoint(downArrowRect.left() - 1, downArrowRect.top() + borderSize),
                                                  QPoint(downArrowRect.left() - 1, downArrowRect.bottom() - borderSize));
                        }
                    }
                }

             
                if (comboBox->editable) {
                    // Draw the down arrow
                    QImage downArrow(qt_cleanlooks_arrow_xpm);
                    downArrow.setColor(1, comboBox->palette.foreground().color().rgba());
                    int offset = comboBox->direction == Qt::RightToLeft ? -2 : 2;
                    cachePainter.drawImage(downArrowRect.center().x() - downArrow.width() / 2 + offset,
                                           downArrowRect.center().y() - downArrow.height() / 2 + 1, downArrow);
                } else {
                    // Draw the up/down arrow
                    QImage upArrow(qt_scrollbar_button_arrow_up);
                    upArrow.setColor(1, comboBox->palette.foreground().color().rgba());
                    QImage downArrow(qt_scrollbar_button_arrow_down);
                    downArrow.setColor(1, comboBox->palette.foreground().color().rgba());
                    
                    int offset = comboBox->direction == Qt::RightToLeft ? -2 : 2;
                    
                    cachePainter.drawImage(downArrowRect.center().x() - downArrow.width() / 2 + offset,
                                           downArrowRect.center().y() - upArrow.height() , upArrow);
                    cachePainter.drawImage(downArrowRect.center().x() - downArrow.width() / 2 + offset,
                                           downArrowRect.center().y()  + 3, downArrow);
                }

                // Draw the focus rect
                if ((focus && (option->state & State_KeyboardFocusChange)) && !comboBox->editable) {
                    QStyleOptionFocusRect focus;
                    focus.rect = subControlRect(CC_ComboBox, &comboBoxCopy, SC_ComboBoxEditField, widget)
                                 .adjusted(0, 2, option->direction == Qt::RightToLeft ? 1 : -1, -2);
                    drawPrimitive(PE_FrameFocusRect, &focus, &cachePainter, widget);
                }
                cachePainter.end();
                if (UsePixmapCache)
                    QPixmapCache::insert(pixmapName, cache);
            }
            painter->drawPixmap(comboBox->rect.topLeft(), cache);
        }
        painter->restore();
        break;
#endif // QT_NO_COMBOBOX
#ifndef QT_NO_GROUPBOX
    case CC_GroupBox:
        painter->save();
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            QRect textRect = subControlRect(CC_GroupBox, groupBox, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = subControlRect(CC_GroupBox, groupBox, SC_GroupBoxCheckBox, widget);
            bool flat = groupBox->features & QStyleOptionFrameV2::Flat;

            if(!flat) {
                if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
                    QStyleOptionFrameV2 frame;
                    frame.QStyleOption::operator=(*groupBox);
                    frame.features = groupBox->features;
                    frame.lineWidth = groupBox->lineWidth;
                    frame.midLineWidth = groupBox->midLineWidth;
                    frame.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
                    painter->save();
                    QRegion region(groupBox->rect);
                    bool ltr = groupBox->direction == Qt::LeftToRight;
                    region -= checkBoxRect.unite(textRect).adjusted(ltr ? -4 : 0, 0, ltr ? 0 : 4, 0);
                    painter->setClipRegion(region);
                    frame.palette.setBrush(QPalette::Dark, option->palette.mid().color().light(110));
                    drawPrimitive(PE_FrameGroupBox, &frame, painter);
                    painter->restore();
                }
            }
            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty()) {
                if (!groupBox->text.isEmpty()) {
                    QColor textColor = groupBox->textColor;
                    if (textColor.isValid())
                        painter->setPen(textColor);
                    int alignment = int(groupBox->textAlignment);
                    if (!styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                        alignment |= Qt::TextHideMnemonic;
                    if (flat) {
                        QFont font = painter->font();
                        font.setBold(true);
                        painter->setFont(font);
                        if (groupBox->subControls & SC_GroupBoxCheckBox) {
                            textRect.adjust(checkBoxRect.right() + 4, 0, checkBoxRect.right() + 4, 0);
                        }
                    }

                    painter->drawText(textRect, Qt::TextShowMnemonic | Qt::AlignLeft| alignment, groupBox->text);
                }
                // Draw checkbox
                if (groupBox->subControls & SC_GroupBoxCheckBox) {
                    QStyleOptionButton box;
                    box.QStyleOption::operator=(*groupBox);
                    box.rect = checkBoxRect;
                    drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
                }
            }
        }
        painter->restore();
        break;
#endif // QT_NO_GROUPBOX
#ifndef QT_NO_SLIDER
    case CC_Slider:
        painter->save();
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect groove = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            QRect ticks = subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);

            bool horizontal = slider->orientation == Qt::Horizontal;
            bool ticksAbove = slider->tickPosition & QSlider::TicksAbove;
            bool ticksBelow = slider->tickPosition & QSlider::TicksBelow;

            if ((option->subControls & SC_SliderGroove) && groove.isValid()) {
                    groove.adjust(0, 0, -1, 0);
                    // draw groove
                    painter->setPen(QPen(palette.dark(), 0));

                    QLinearGradient gradient1(groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
                    gradient1.setColorAt(0, palette.dark().color().light(110));
                    gradient1.setColorAt(1, palette.button().color().dark(115));

                    QLinearGradient gradient2(groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
                    gradient2.setColorAt(0, palette.highlight().color().dark(120));
                    gradient2.setColorAt(1, palette.highlight().color().light(108));

                    painter->setBrush(gradient1);
                    painter->drawRect(groove);

                    if (horizontal) {
                        painter->setBrush(gradient2);
                        painter->setPen(QPen(palette.highlight().color().dark(130), 0));
                        painter->drawRect(QRect(groove.left(), groove.top(), handle.left() , groove.height()));
                    } else {
                        painter->setBrush(gradient2);
                        painter->setPen(QPen(palette.highlight().color().dark(150), 0));
                        painter->drawRect(QRect(groove.left(), groove.top(), groove.width() , handle.top()));
                    }
                }

                // draw handle
                if ((option->subControls & SC_SliderHandle) ) {
                    QColor highlightedGradientStartColor = option->palette.button().color();
                    QColor highlightedGradientStopColor = option->palette.light().color();
                    QColor gradientStartColor = mergedColors(option->palette.button().color().light(120),
                                               option->palette.dark().color().light(140), 50);
                    QColor gradientStopColor = gradientStartColor.dark(115);
                    QRect gradRect = handle.adjusted(1, 1, -1, -1);

                    if (option->state & State_MouseOver) {
                        gradientStartColor = gradientStartColor.light(110);
                        gradientStopColor = gradientStopColor.light(110);
                    }

                    // gradient fill
                    QRect innerBorder = gradRect;
                    qt_cleanlooks_draw_gradient(painter, gradRect,
                                                gradientStartColor,
                                                gradientStopColor);
                    painter->setPen(Qt::white);
                    painter->drawLine(innerBorder.topLeft() + QPoint(0, 1), innerBorder.topRight() + QPoint(0, 1));
                    painter->drawLine(innerBorder.topLeft(), innerBorder.bottomLeft());
                    painter->setPen(QPen(option->palette.dark().color().light(130), 0));
                    painter->drawLine(innerBorder.bottomLeft(), innerBorder.bottomRight());
                    painter->drawLine(innerBorder.topRight(), innerBorder.bottomRight());
                    QRect r;
                    if (horizontal)
                        r = handle.adjusted(0, 1, 0, -1);
                    else
                        r = handle.adjusted(1, 0, -1, 0);

                    painter->setPen(QPen(option->palette.dark().color().dark(120), 1));
                    painter->drawLine(QPoint(r.left(), r.top() + 2), QPoint(r.left(), r.bottom() - 2));
                    painter->drawLine(QPoint(r.right(), r.top() + 2), QPoint(r.right(), r.bottom() - 2));
                    painter->drawLine(QPoint(r.left() + 2, r.bottom()), QPoint(r.right() - 2, r.bottom()));
                    painter->drawPoint(QPoint(r.right() - 1, r.bottom() - 1));
                    painter->drawPoint(QPoint(r.right() - 1, r.top() + 1));
                    painter->drawPoint(QPoint(r.left() + 1, r.bottom() - 1));
                    painter->drawPoint(QPoint(r.left() + 1, r.top() + 1));
                    painter->setPen(QPen(option->palette.dark().color().dark(140), 1));
                    painter->drawLine(QPoint(r.left() + 2, r.top()), QPoint(r.right() - 2, r.top()));
                     //draw grips
                    if (horizontal) {
                        for (int i = -3; i< 6 ; i += 3) {
                            painter->setPen(QPen(gripShadow, 1));
                            painter->drawLine(
                                        QPoint(r.center().x() + i , r.top() + 4),
                                        QPoint(r.center().x() + i, r.bottom() - 4));
                            painter->setPen(QPen(palette.light(), 1));
                            painter->drawLine(
                                        QPoint(r.center().x() + i + 1, r.top() + 4 ),
                                        QPoint(r.center().x() + i + 1, r.bottom() - 4 ));
                        }
                    } else {
                        for (int i = -3; i < 6 ; i += 3) {
                            painter->setPen(QPen(gripShadow, 1));
                            painter->drawLine(
                                        QPoint(r.left() + 3, r.center().y()+ i),
                                        QPoint(r.right() - 3, r.center().y()+ i));
                            painter->setPen(QPen(palette.light(), 1));
                            painter->drawLine(
                                        QPoint(r.left() + 3, r.center().y() + 1 + i),
                                        QPoint(r.right() - 3, r.center().y() + 1 + i));

                        }
                    }
                 if (slider->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*slider);
                    fropt.rect = slider->rect;//subElementRect(SE_SliderFocusRect, slider, widget);
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
                }
        }
        painter->restore();
        break;
#endif // QT_NO_SLIDER
        default:
            QWindowsStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

/*!
  \reimp
*/
int QCleanLooksStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    int ret = -1;
    switch (metric) {
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;
    case PM_MessageBoxIconSize:
        ret = 48;
        break;
    case PM_DialogButtonsSeparator:
    case PM_SplitterWidth:
        ret = 6;
        break;
    case PM_ScrollBarSliderMin:
        ret = 26;
        break;
    case PM_MenuPanelWidth: //menu framewidth
        ret = 2;
        break;
    case PM_TitleBarHeight:
        ret = 24;
        break;
    case PM_ScrollBarExtent:
        ret = 15;
        break;
    case PM_SliderThickness:
        ret = 16;
        break;
    case PM_SliderLength:
        ret = 32;
        break;
    case PM_DockWidgetTitleMargin:
        ret = 4;
        break;
    case PM_MenuBarVMargin:
        ret = 1;
        break;
    case PM_DefaultFrameWidth:
        ret = 2;
        break;
    case PM_MenuBarItemSpacing:
        ret = 6;
    case PM_MenuBarHMargin:
        ret = 0;
        break;
    case PM_ToolBarHandleExtent:
        ret = 9;
        break;
    case PM_ToolBarItemSpacing:
        ret = 2;
        break;
    case PM_ToolBarFrameWidth:
        ret = 0;
        break;
    case PM_ToolBarItemMargin:
        ret = 1;
        break;
    default:
        break;
    }

    return ret != -1 ? ret : QWindowsStyle::pixelMetric(metric, option, widget);
}

/*!
  \reimp
*/
QSize QCleanLooksStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                        const QSize &size, const QWidget *widget) const
{
    QSize newSize = QWindowsStyle::sizeFromContents(type, option, size, widget);
    switch (type) {
    case CT_GroupBox:
    case CT_RadioButton:
    case CT_HeaderSection:
    case CT_CheckBox:
        newSize += QSize(0, 1);
        break;
    case CT_ToolButton:
        newSize += QSize(4, 6);
        break;
    case CT_LineEdit:
    case CT_SpinBox:
    case CT_PushButton:
	case CT_ComboBox:
        newSize += QSize(0, 4);
        break;
    case CT_MenuBarItem:
	    newSize += QSize(0, 2);
	break;
    case CT_SizeGrip:
	    newSize += QSize(4, 4);
	break;
    default:
        break;
    }
    return newSize;
}


/*!
  \reimp
*/
void QCleanLooksStyle::polish(QApplication *app)
{
    Q_UNUSED(app);
    // We only need the overhead when shortcuts are sometimes hidden
    if (!styleHint(SH_UnderlineShortcut, 0) && app)
        app->installEventFilter(this);
}

/*!
  \reimp
*/
void QCleanLooksStyle::polish(QWidget *widget)
{
    if (qobject_cast<QAbstractButton*>(widget)
        || qobject_cast<QComboBox *>(widget)
        || qobject_cast<QProgressBar *>(widget)
        || qobject_cast<QScrollBar *>(widget)
#ifndef QT_NO_SPLITTER
        || qobject_cast<QSplitterHandle *>(widget)
#endif
        || qobject_cast<QAbstractSlider *>(widget)
        || qobject_cast<QAbstractSpinBox *>(widget)
        || (widget->inherits("QDockSeparator"))
        || (widget->inherits("QDockWidgetSeparator"))
        ) {
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

/*!
  \reimp
*/
void QCleanLooksStyle::polish(QPalette &pal)
{
    QWindowsStyle::polish(pal);
}

/*!
  \reimp
*/
void QCleanLooksStyle::unpolish(QWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
  \reimp
*/
void QCleanLooksStyle::unpolish(QApplication *app)
{
    QWindowsStyle::unpolish(app);
}

/*!
  \reimp
*/
QRect QCleanLooksStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                      SubControl subControl, const QWidget *widget) const
{
    QRect rect = QWindowsStyle::subControlRect(control, option, subControl, widget);

    switch (control) {
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int tickSize = pixelMetric(PM_SliderTickmarkOffset, option, widget);
            switch (subControl) {
            case SC_SliderHandle: {
                if (slider->orientation == Qt::Horizontal) {
                    rect.setWidth(32);
                    rect.setHeight(15);
                    int centerY = slider->rect.center().y() - rect.height() / 2;
                    if (slider->tickPosition & QSlider::TicksAbove)
                        centerY += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        centerY -= tickSize;
                    rect.moveTop(centerY);
                } else {
                    rect.setWidth(15);
                    rect.setHeight(32);
                    int centerX = slider->rect.center().x() - rect.width() / 2;
                    if (slider->tickPosition & QSlider::TicksAbove)
                        centerX += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        centerX -= tickSize;
                    rect.moveLeft(centerX);
                }
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
#endif // QT_NO_SLIDER
    case CC_ScrollBar:
        break;
#ifndef QT_NO_SPINBOX
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            QSize bs;
            int fw = spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, widget) : 0;
            bs.setHeight(qMax(8, spinbox->rect.height()/2 - fw));
            bs.setWidth(14);
            int y = fw;
            int x, lx, rx;
            x = spinbox->rect.width() - y - bs.width() + 2;
            lx = fw;
            rx = x - fw;
            switch (subControl) {
            case SC_SpinBoxUp:
                rect = QRect(x, y - 1, bs.width(), bs.height() + 1);
                break;
            case SC_SpinBoxDown:
                rect = QRect(x, y + bs.height(), bs.width(), bs.height());
                break;
            case SC_SpinBoxEditField:
                rect = QRect(lx, fw, rx, spinbox->rect.height() - 2*fw);
                break;
            case SC_SpinBoxFrame:
                rect = spinbox->rect;
            default:
                break;
            }
            rect = visualRect(spinbox->direction, spinbox->rect, rect);
        }
        break;
#endif // Qt_NO_SPINBOX
#ifndef QT_NO_GROUPBOX
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            int topMargin = 0;
            int topHeight = 0;
            int verticalAlignment = styleHint(SH_GroupBox_TextLabelVerticalAlignment, groupBox, widget);
            bool flat = groupBox->features & QStyleOptionFrameV2::Flat;
            if (!groupBox->text.isEmpty()) {
                topHeight = groupBox->fontMetrics.height();
                if (verticalAlignment & Qt::AlignVCenter)
                    topMargin = topHeight / 2;
                else if (verticalAlignment & Qt::AlignTop)
                    topMargin = topHeight;
            }
            QRect frameRect = groupBox->rect;
            frameRect.setTop(topMargin);
            if (subControl == SC_GroupBoxFrame) {
                return rect;
            }
            else if (subControl == SC_GroupBoxContents) {
                if( flat ) {
                    int margin = 0;
                    int leftMarginExtension = 16;
                    rect = frameRect.adjusted(leftMarginExtension + margin, margin + topHeight, -margin, -margin);
                }
                break;
            }
            if(flat) {
                if (const QGroupBox *groupBoxWidget = qobject_cast<const QGroupBox *>(widget)) {
                    //Prepare metrics for a bold font
                    QFont font = widget->font();
                    font.setBold(true);
                    QFontMetrics fontMetrics(font);

                    QSize textRect = fontMetrics.boundingRect(groupBoxWidget->title()).size() + QSize(2, 2);
                    if (subControl == SC_GroupBoxCheckBox) {
                        int indicatorWidth = pixelMetric(PM_IndicatorWidth, option, widget);
                        int indicatorHeight = pixelMetric(PM_IndicatorHeight, option, widget);
                        rect.setWidth(indicatorWidth);
                        rect.setHeight(indicatorHeight);
                        rect.moveTop((fontMetrics.height() - indicatorHeight) / 2 + 2);
                    } else if (subControl == SC_GroupBoxLabel) {
                        rect.setSize(textRect);
                    }
                }
            }
        }
        return rect;
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        switch (subControl) {
        case SC_ComboBoxArrow:
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(rect.right() - 18, rect.top() - 2,
                         17, rect.height() + 4);
            rect = visualRect(option->direction, option->rect, rect);
            break;
        case SC_ComboBoxEditField: {
            int frameWidth = pixelMetric(PM_DefaultFrameWidth);
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(option->rect.left() + frameWidth, option->rect.top() + frameWidth,
                         option->rect.width() - 19 - 2 * frameWidth,
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
#endif //QT_NO_GROUPBOX
        case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            SubControl sc = subControl;
            QRect &ret = rect;
            const int indent = 3;
            const int controlTopMargin = 3;
            const int controlBottomMargin = 3;
            const int controlWidthMargin = 2;
            const int controlHeight = tb->rect.height() - controlTopMargin - controlBottomMargin ;
            const int delta = controlHeight + controlWidthMargin;
            int offset = 0;

            bool isMinimized = tb->titleBarState & Qt::WindowMinimized;

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
                else if (sc == SC_TitleBarNormalButton)
                    break;
            case SC_TitleBarMaxButton:
                if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
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
int QCleanLooksStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                               QStyleHintReturn *returnData) const
{
    int ret = 0;
    switch (hint) {
    case SH_EtchDisabledText:
        ret = 1;
        break;
    case SH_MainWindow_SpaceBelowMenuBar:
        ret = 0;
        break;
    case SH_MenuBar_MouseTracking:
        ret = 1;
        break;
    case SH_TitleBar_NoBorder:
        ret = 1;
        break;
    case SH_ItemView_ShowDecorationSelected:
        ret = true;
        break;
    case SH_Table_GridLineColor:
        if (option) {
            ret = option->palette.background().color().dark(120).rgb();
            break;
        }
    case SH_ComboBox_Popup:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            ret = !cmb->editable;
        else
            ret = 0;
        break;
    case SH_WindowFrame_Mask:
        ret = 1;
        if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData)) {
            //left rounded corner
            mask->region = option->rect;
            mask->region -= QRect(option->rect.left(), option->rect.top(), 5, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 1, 3, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 2, 2, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 3, 1, 2);

            //right rounded corner
            mask->region -= QRect(option->rect.right() - 4, option->rect.top(), 5, 1);
            mask->region -= QRect(option->rect.right() - 2, option->rect.top() + 1, 3, 1);
            mask->region -= QRect(option->rect.right() - 1, option->rect.top() + 2, 2, 1);
            mask->region -= QRect(option->rect.right() , option->rect.top() + 3, 1, 2);
        }
        break;
    case SH_DialogButtonLayoutPolicy:
        ret = 3; // GnomeLayout
        break;
    default:
        ret = QWindowsStyle::styleHint(hint, option, widget, returnData);
    break;
    }
    return ret;
}

/*! \reimp */
QRect QCleanLooksStyle::subElementRect(SubElement sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect r = QWindowsStyle::subElementRect(sr, opt, w);
    switch (sr) {
    case SE_PushButtonFocusRect:
        r.adjust(0, 1, 0, -1);
        break;
    case SE_PushButtonContents:
        r.translate(0, 1);
    default:
        break;
    }
    return r;
}


/*!
    \internal
*/
QIcon QCleanLooksStyle::standardIconImplementation(StandardPixmap standardIcon,
                                                  const QStyleOption *option,
                                                  const QWidget *widget) const
{
    return QWindowsStyle::standardIconImplementation(standardIcon, option, widget);
}


QPixmap findIcon(const QString &subpath)
{
    QPixmap pixmap (QString("/usr/share/icons/gnome") + subpath);
    if (pixmap.isNull())
        pixmap.load(QString("/usr/share/icons/hicolor") + subpath);	
    return pixmap;
}


/*!
 \reimp
 */
QPixmap QCleanLooksStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                      const QWidget *widget) const
{
#ifndef QT_NO_IMAGEFORMAT_XPM
    switch (standardPixmap) {
    case SP_MessageBoxInformation:
        {
            QPixmap pixmap = findIcon("/48x48/stock/generic/stock_dialog-info.png");		    
            if (!pixmap.isNull())
                return pixmap;
            break;
        }
    case SP_MessageBoxWarning:
        {
            QPixmap pixmap = findIcon("/48x48/stock/generic/stock_dialog-warning.png");		    
            if (!pixmap.isNull())
                return pixmap;
            break;
        }
    case SP_MessageBoxCritical:
        {
            QPixmap pixmap = findIcon("/48x48/stock/generic/stock_dialog-error.png");		    
            if (!pixmap.isNull())
                return pixmap;
            break;
        }
    case SP_MessageBoxQuestion:
        {
            QPixmap pixmap = findIcon("/48x48/stock/generic/stock_unknown.png");		    
            if (!pixmap.isNull())
                return pixmap;
            break;
        }
    case SP_TitleBarMenuButton:
    case SP_TitleBarShadeButton:
    case SP_TitleBarUnshadeButton:
    case SP_TitleBarMinButton:
    case SP_TitleBarMaxButton:
    case SP_TitleBarContextHelpButton:
        return QWindowsStyle::standardPixmap(standardPixmap, opt, widget);
    case SP_TitleBarNormalButton:
        return QPixmap((const char **)dock_widget_restore_xpm);
    case SP_TitleBarCloseButton:
    case SP_DockWidgetCloseButton:
        return QPixmap((const char **)dock_widget_close_xpm);

    default:
        break;
    }
#endif //QT_NO_IMAGEFORMAT_XPM
    return QWindowsStyle::standardPixmap(standardPixmap, opt, widget);
}

#endif // QT_NO_STYLE_CLEANLOOKS || QT_PLUGIN
