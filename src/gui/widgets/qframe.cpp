/****************************************************************************
**
** Implementation of QFrame widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qframe.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qframe.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"

#include "qframe_p.h"
#define d d_func()
#define q q_func()


QFramePrivate::QFramePrivate()
    : frect(QRect(0, 0, 0, 0)),
      frameStyle(QFrame::NoFrame | QFrame::Plain),
      lineWidth(1),
      midLineWidth(0),
      frameWidth(0)
{
}

/*!
    \class QFrame
    \brief The QFrame class is the base class of widgets that can have a frame.

    \ingroup abstractwidgets

    QPopupMenu uses this to "raise" the menu above the surrounding
    screen. QProgressBar has a "sunken" look. QLabel has a flat look.
    The frames of widgets like these can be changed.

    \code
    QLabel label(...);
    label.setFrameStyle(QFrame::Panel | QFrame::Raised);
    label.setLineWidth(2);

    QProgressBar pbar(...);
    label.setFrameStyle(QFrame::NoFrame);
    \endcode

    The QFrame class can also be used directly for creating simple
    frames without any contents, although usually you would use a
    QHBox or QVBox because they automatically lay out the widgets you
    put inside the frame.

    A frame widget has four attributes: frameStyle(), lineWidth(),
    midLineWidth(), and margin().

    The frame style is specified by a \link QFrame::Shape frame
    shape\endlink and a \link QFrame::Shadow shadow style\endlink. The
    frame shapes are \c NoFrame, \c Box, \c Panel, \c StyledPanel, \c
    PopupPanel, \c WinPanel, \c ToolBarPanel, \c MenuBarPanel, \c
    HLine and \c VLine; the shadow styles are \c Plain, \c Raised and
    \c Sunken.

    The line width is the width of the frame border.

    The mid-line width specifies the width of an extra line in the
    middle of the frame, which uses a third color to obtain a special
    3D effect. Notice that a mid-line is only drawn for \c Box, \c
    HLine and \c VLine frames that are raised or sunken.

    The margin is the gap between the frame and the contents of the
    frame.

    \target picture
    This table shows the most useful combinations of styles and widths
    (and some rather useless ones):

    \img frames.png Table of frame styles
*/


/*!
    \enum QFrame::Shape

    This enum type defines the shapes of a QFrame's frame.

    \value NoFrame  QFrame draws nothing
    \value Box  QFrame draws a box around its contents
    \value Panel  QFrame draws a panel to make the contents appear
    raised or sunken
    \value StyledPanel  draws a rectangular panel with a look that
    depends on the current GUI style. It can be raised or sunken.
    \value HLine  QFrame draws a horizontal line that frames nothing
    (useful as separator)
    \value VLine  QFrame draws a vertical line that frames nothing
    (useful as separator)
    \value GroupBoxPanel draws a rectangular panel
    \value WinPanel draws a rectangular panel that can be raised or
    sunken like those in Windows 95. Specifying this shape sets
    the line width to 2 pixels. WinPanel is provided for compatibility.
    For GUI style independence we recommend using StyledPanel instead.
    \value ToolBarPanel
    \value MenuBarPanel
    \value PopupPanel
    \value LineEditPanel is used to draw a frame suitable for line edits. The
    look depends upon the current GUI style.
    \value TabWidgetPanel is used to draw a frame suitable for tab widgets. The
    look depends upon the current GUI style.
    \omitvalue MShape

    When it does not call QStyle, Shape interacts with QFrame::Shadow,
    the lineWidth() and the midLineWidth() to create the total result.
    See the \link #picture picture of the frames\endlink in the class
    description.

    \sa QFrame::Shadow QFrame::style() QStyle::drawPrimitive()
*/


/*!
    \enum QFrame::Shadow

    This enum type defines the 3D effect used for QFrame's frame.

    \value Plain  the frame and contents appear level with the
    surroundings; draws using the palette foreground color (without
    any 3D effect)
    \value Raised the frame and contents appear raised; draws a 3D
    raised line using the light and dark colors of the current color
    group
    \value Sunken the frame and contents appear sunken; draws a 3D
    sunken line using the light and dark colors of the current color
    group
    \omitvalue MShadow

    Shadow interacts with QFrame::Shape, the lineWidth() and the
    midLineWidth(). See the \link #picture picture of the frames\endlink
    in the class description.

    \sa QFrame::Shape lineWidth() midLineWidth()
*/


/*!
    Constructs a frame widget with frame style \c NoFrame and a
    1-pixel frame width.

    The \a parent and \a f arguments are passed to the QWidget
    constructor.
*/

QFrame::QFrame(QWidget* parent, Qt::WFlags f)
    : QWidget(*new QFramePrivate, parent, f)
{
}

/*! \internal */
QFrame::QFrame(QFramePrivate &dd, QWidget* parent, Qt::WFlags f)
    : QWidget(dd, parent, f)
{
}

#ifdef QT_COMPAT
QFrame::QFrame(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(*new QFramePrivate, parent, f)
{
    setObjectName(name);
}
#endif

/*!
  Destroys the frame.
 */
QFrame::~QFrame()
{
}

/*!
    Returns the frame style.

    The default value is QFrame::NoFrame.

    \sa setFrameStyle(), frameShape(), frameShadow()
*/
int QFrame::frameStyle() const
{
    return d->frameStyle;
}

/*!
    \property QFrame::frameShape
    \brief the frame shape value from the frame style

    \sa frameStyle(), frameShadow()
*/

QFrame::Shape QFrame::frameShape() const
{
    return (Shape) (d->frameStyle & MShape);
}

void QFrame::setFrameShape(QFrame::Shape s)
{
    setFrameStyle((d->frameStyle & MShadow) | s);
}


/*!
    \property QFrame::frameShadow
    \brief the frame shadow value from the frame style

    \sa frameStyle(), frameShape()
*/
QFrame::Shadow QFrame::frameShadow() const
{
    return (Shadow) (d->frameStyle & MShadow);
}

void QFrame::setFrameShadow(QFrame::Shadow s)
{
    setFrameStyle((d->frameStyle & MShape) | s);
}

/*!
    Sets the frame style to \a style.

    The \a style is the bitwise OR between a frame shape and a frame
    shadow style. See the \link #picture illustration\endlink in the
    class documentation.

    The frame shapes are given in \l{QFrame::Shape} and the shadow
    styles in \l{QFrame::Shadow}.

    If a mid-line width greater than 0 is specified, an additional
    line is drawn for \c Raised or \c Sunken \c Box, \c HLine, and \c
    VLine frames. The mid-color of the current color group is used for
    drawing middle lines.

    \sa \link #picture Illustration\endlink, frameStyle()
*/

void QFrame::setFrameStyle(int style)
{
    if (!testWState(Qt::WState_OwnSizePolicy)) {
        switch (style & MShape) {
        case HLine:
            setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            break;
        case VLine:
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
            break;
        default:
            if ((d->frameStyle & MShape) == HLine || (d->frameStyle & MShape) == VLine)
                setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        }
        clearWState(Qt::WState_OwnSizePolicy);
    }
    d->frameStyle = (short)style;
    d->updateFrameWidth();
}

/*!
    \property QFrame::lineWidth
    \brief the line width

    Note that the \e total line width for \c HLine and \c VLine is
    specified by frameWidth(), not lineWidth().

    The default value is 1.

    \sa midLineWidth(), frameWidth()
*/

void QFrame::setLineWidth(int w)
{
    d->lineWidth = (short)w;
    d->updateFrameWidth();
}

int QFrame::lineWidth() const
{
    return d->lineWidth;
}

/*!
    \property QFrame::midLineWidth
    \brief the width of the mid-line

    The default value is 0.

    \sa lineWidth(), frameWidth()
*/

void QFrame::setMidLineWidth(int w)
{
    d->midLineWidth = (short)w;
    d->updateFrameWidth();
}

int QFrame::midLineWidth() const
{
    return d->midLineWidth;
}


/*!
  \internal
  Updated the frameWidth parameter.
*/

void QFramePrivate::updateFrameWidth()
{
    QRect fr = q->frameRect();

    int frameShape  = frameStyle & QFrame::MShape;
    int frameShadow = frameStyle & QFrame::MShadow;

    frameWidth = -1;

    switch (frameShape) {

    case QFrame::NoFrame:
        frameWidth = 0;
        break;

    case QFrame::Box:
    case QFrame::HLine:
    case QFrame::VLine:
    case QFrame::GroupBoxPanel:
        switch (frameShadow) {
        case QFrame::Plain:
            frameWidth = lineWidth;
            break;
        case QFrame::Raised:
        case QFrame::Sunken:
            frameWidth = (short)(lineWidth*2 + midLineWidth);
            break;
        }
        break;

    case QFrame::LineEditPanel:
    case QFrame::TabWidgetPanel:
    case QFrame::PopupPanel:
    case QFrame::StyledPanel:
        frameWidth = q->style().pixelMetric(QStyle::PM_DefaultFrameWidth, q);
        break;

    case QFrame::MenuBarPanel:
        frameWidth = q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q);
        break;
    case QFrame::ToolBarPanel:
        frameWidth = q->style().pixelMetric(QStyle::PM_DockWindowFrameWidth, q);
        break;

    case QFrame::WinPanel:
        frameWidth = 2;
        break;


    case QFrame::Panel:
        switch (frameShadow) {
        case QFrame::Plain:
        case QFrame::Raised:
        case QFrame::Sunken:
            frameWidth = lineWidth;
            break;
        }
        break;
    }

    if (frameWidth == -1)                                // invalid style
        frameWidth = 0;

    q->setFrameRect(fr);
}


/*!
    \property QFrame::frameWidth
    \brief the width of the frame that is drawn.

    Note that the frame width depends on the \link
    QFrame::setFrameStyle() frame style \endlink, not only the line
    width and the mid-line width. For example, the style \c NoFrame
    always has a frame width of 0, whereas the style \c Panel has a
    frame width equivalent to the line width.

    \sa lineWidth(), midLineWidth(), frameStyle()
*/
int QFrame::frameWidth() const
{
    return d->frameWidth;
}


/*!
    \property QFrame::frameRect
    \brief the frame's rectangle

    The frame's rectangle is the rectangle the frame is drawn in. By
    default, this is the entire widget. Setting the rectangle does
    does \e not cause a widget update. The frame rectangle is
    automatically adjusted when the widget changes size.

    If you set the rectangle to a null rectangle (for example
    \c{QRect(0, 0, 0, 0)}), then the resulting frame rectangle is
    equivalent to the \link QWidget::rect() widget rectangle\endlink.
*/

QRect QFrame::frameRect() const
{
    QRect fr = contentsRect();
    fr.addCoords(-d->frameWidth, -d->frameWidth, d->frameWidth, d->frameWidth);
    return fr;
}

void QFrame::setFrameRect(const QRect &r)
{
    QRect cr = r;
    cr.addCoords(d->frameWidth, d->frameWidth, -d->frameWidth, -d->frameWidth);
    setContentsMargins(cr.left(), cr.top(), rect().right() - cr.right(), rect().bottom() - cr.bottom());
}

/*!\reimp
*/
QSize QFrame::sizeHint() const
{
    //   Returns a size hint for the frame - for HLine and VLine
    //   shapes, this is stretchable one way and 3 pixels wide the
    //   other.  For other shapes, QWidget::sizeHint() is used.
    switch (d->frameStyle & MShape) {
    case HLine:
        return QSize(-1,3);
    case VLine:
        return QSize(3,-1);
    default:
        return QWidget::sizeHint();
    }
}

/*!\reimp
*/

void QFrame::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    drawFrame(&paint);
}

/*!
    \internal

    Mostly for the sake of Q3Frame
 */
void QFrame::drawFrame(QPainter *p)
{
    QPoint      p1, p2;
    QStyleOptionFrame opt(0);
    int frameShape  = d->frameStyle & QFrame::MShape;
    int frameShadow = d->frameStyle & QFrame::MShadow;

    int lw = 0;
    int mlw = 0;
    opt.rect = frameRect();
    opt.palette = palette();
    switch (frameShape) {
    case QFrame::Box:
    case QFrame::HLine:
    case QFrame::VLine:
    case QFrame::GroupBoxPanel:
        lw = d->lineWidth;
        mlw = d->midLineWidth;
        break;
    default:
        // most frame styles do not handle customized line and midline widths
        // (see updateFrameWidth()).
        lw = d->frameWidth;
        break;
    }
    opt.lineWidth = lw;
    opt.midLineWidth = mlw;
    opt.state = QStyle::Style_Default;
    if (isEnabled())
        opt.state |= QStyle::Style_Enabled;
    if (frameShadow == Sunken)
        opt.state |= QStyle::Style_Sunken;
    else if (frameShadow == Raised)
        opt.state |= QStyle::Style_Raised;
    if (hasFocus())
        opt.state |= QStyle::Style_HasFocus;
    if (testAttribute(Qt::WA_UnderMouse))
        opt.state |= QStyle::Style_MouseOver;

    switch (frameShape) {
    case Box:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground(), lw);
        else
            qDrawShadeRect(p, opt.rect, opt.palette, frameShadow == Sunken, lw, mlw);
        break;

    case LineEditPanel:
        style().drawPrimitive(QStyle::PE_PanelLineEdit, &opt, p, this);
        break;

    case GroupBoxPanel:
        style().drawPrimitive(QStyle::PE_PanelGroupBox, &opt, p, this);
        break;

    case TabWidgetPanel:
        style().drawPrimitive(QStyle::PE_PanelTabWidget, &opt, p, this);
        break;

    case MenuBarPanel:
        style().drawPrimitive(QStyle::PE_PanelMenuBar, &opt, p, this);
        break;
    case ToolBarPanel:
        opt.rect = rect();
        style().drawPrimitive(QStyle::PE_PanelDockWindow, &opt, p, this);
        break;

    case StyledPanel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground(), lw);
        else
            style().drawPrimitive(QStyle::PE_Panel, &opt, p, this);
        break;

    case PopupPanel:
    {
        int vmargin = style().pixelMetric(QStyle::PM_MenuVMargin, this),
            hmargin = style().pixelMetric(QStyle::PM_MenuHMargin, this);
        if (vmargin > 0 || hmargin > 0) {
            QStyleOptionMenuItem menuOpt(0);
            menuOpt.palette = opt.palette;
            menuOpt.state = opt.state;
            menuOpt.menuItemType = QStyleOptionMenuItem::Margin;
            menuOpt.checkState = QStyleOptionMenuItem::NotCheckable;
            menuOpt.maxIconWidth = lw;
            menuOpt.tabWidth = mlw;
            QRect fr = frameRect();
            int   fw = d->frameWidth;
            if (vmargin > 0) {
                menuOpt.rect.setRect(fr.x() + fw, fr.y() + fw, fr.width() - (fw*2), vmargin);
                style().drawControl(QStyle::CE_MenuVMargin, &menuOpt, p, this);
                menuOpt.rect.setRect(fr.x() + fw, fr.bottom() - fw - vmargin, fr.width() - (fw*2),
                                     vmargin);
                style().drawControl(QStyle::CE_MenuVMargin, &menuOpt, p, this);
            }
            if (hmargin > 0) {
                menuOpt.rect.setRect(fr.x() + fw, fr.y() + fw + vmargin, hmargin,
                                     fr.height() - (fw * 2) - vmargin);
                style().drawControl(QStyle::CE_MenuHMargin, &menuOpt, p, this);
                menuOpt.rect.setRect(fr.right() - fw - hmargin, fr.y() + fw + vmargin, hmargin,
                                     fr.height() - (fw * 2) - vmargin);
                style().drawControl(QStyle::CE_MenuHMargin, &opt, p, this);
            }
        }
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground(), lw);
        else
            style().drawPrimitive(QStyle::PE_PanelPopup, &opt, p, this);
        break;
    }

    case Panel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground(), lw);
        else
            qDrawShadePanel(p, opt.rect, opt.palette, frameShadow == Sunken, lw);
        break;

    case WinPanel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground(), lw);
        else
            qDrawWinPanel(p, opt.rect, opt.palette, frameShadow == Sunken);
        break;
    case HLine:
    case VLine:
        if (frameShape == HLine) {
            p1 = QPoint(opt.rect.x(), opt.rect.height() / 2);
            p2 = QPoint(opt.rect.x() + opt.rect.width(), p1.y());
        } else {
            p1 = QPoint(opt.rect.x()+opt.rect.width() / 2, 0);
            p2 = QPoint(p1.x(), opt.rect.height());
        }
        if (frameShadow == Plain) {
            QPen oldPen = p->pen();
            p->setPen(QPen(opt.palette.foreground(), lw));
            p->drawLine(p1, p2);
            p->setPen(oldPen);
        } else {
            qDrawShadeLine(p, p1, p2, opt.palette, frameShadow == Sunken, lw, mlw);
        }
        break;
    }
}


/*!\reimp
 */
void QFrame::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        d->updateFrameWidth();
    QWidget::changeEvent(ev);
}


