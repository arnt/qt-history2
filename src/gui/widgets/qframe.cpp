/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qframe.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qapplication.h"

#include "qframe_p.h"

QFramePrivate::QFramePrivate()
    : frect(QRect(0, 0, 0, 0)),
      frameStyle(QFrame::NoFrame | QFrame::Plain),
      lineWidth(1),
      midLineWidth(0),
      frameWidth(0),
      leftFrameWidth(0), rightFrameWidth(0), 
      topFrameWidth(0), bottomFrameWidth(0),
      oldFrameStyle(QFrame::NoFrame | QFrame::Plain)
{
}

/*!
    \class QFrame
    \brief The QFrame class is the base class of widgets that can have a frame.

    \ingroup abstractwidgets
    \mainclass

    QMenu uses this to "raise" the menu above the surrounding
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
    placeholder frames without any contents.

    The frame style is specified by a \l{QFrame::Shape}{frame shape} and
    a \l{QFrame::Shadow}{shadow style} that is used to visually separate
    the frame from surrounding widgets. These properties can be set
    together using the setFrameStyle() function and read with frameStyle().

    The frame shapes are \l NoFrame, \l Box, \l Panel, \l StyledPanel,
    HLine and \l VLine; the shadow styles are \l Plain, \l Raised and
    \l Sunken.

    A frame widget has three attributes that describe the thickness of the
    border: \l lineWidth, \l midLineWidth, and \l frameWidth.

    \list
    \o The line width is the width of the frame border. It can be modified
       to customize the frame's appearance.

    \o The mid-line width specifies the width of an extra line in the
       middle of the frame, which uses a third color to obtain a special
       3D effect. Notice that a mid-line is only drawn for \l Box, \l
       HLine and \l VLine frames that are raised or sunken.

    \o The frame width is determined by the frame style, and the frameWidth()
       function is used to obtain the value defined for the style used.
    \endlist

    The margin between the frame and the contents of the frame can be
    customized with the QWidget::setContentsMargins() function.

    \target picture
    This table shows some of the combinations of styles and line widths:

    \image frames.png Table of frame styles
*/


/*!
    \enum QFrame::Shape

    This enum type defines the shapes of frame available.

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
    \value WinPanel draws a rectangular panel that can be raised or
    sunken like those in Windows 95. Specifying this shape sets
    the line width to 2 pixels. WinPanel is provided for compatibility.
    For GUI style independence we recommend using StyledPanel instead.

    \omitvalue GroupBoxPanel
    \omitvalue ToolBarPanel
    \omitvalue MenuBarPanel
    \omitvalue PopupPanel
    \omitvalue LineEditPanel
    \omitvalue TabWidgetPanel

    When it does not call QStyle, Shape interacts with QFrame::Shadow,
    the lineWidth() and the midLineWidth() to create the total result.
    See the picture of the frames in the main class documentation.

    \sa QFrame::Shadow QFrame::style() QStyle::drawPrimitive()
*/


/*!
    \enum QFrame::Shadow

    This enum type defines the types of shadow that are used to give
    a 3D effect to frames.

    \value Plain  the frame and contents appear level with the
    surroundings; draws using the palette foreground color (without
    any 3D effect)
    \value Raised the frame and contents appear raised; draws a 3D
    raised line using the light and dark colors of the current color
    group
    \value Sunken the frame and contents appear sunken; draws a 3D
    sunken line using the light and dark colors of the current color
    group

    Shadow interacts with QFrame::Shape, the lineWidth() and the
    midLineWidth(). See the picture of the frames in the main class
    documentation.

    \sa QFrame::Shape lineWidth() midLineWidth()
*/

/*!
    \enum QFrame::StyleMask

    This enum defines two constants that can be used to extract the
    two components of frameStyle():

    \value Shadow_Mask The \l Shadow part of frameStyle()
    \value Shape_Mask  The \l Shape part of frameStyle()

    \omitvalue MShadow
    \omitvalue MShape

    Normally, you don't need to use these, since frameShadow() and
    frameShape() already extract the \l Shadow and the \l Shape parts
    of frameStyle().

    \sa frameStyle(), setFrameStyle()
*/

/*!
    Constructs a frame widget with frame style \l NoFrame and a
    1-pixel frame width.

    The \a parent and \a f arguments are passed to the QWidget
    constructor.
*/

QFrame::QFrame(QWidget* parent, Qt::WindowFlags f)
    : QWidget(*new QFramePrivate, parent, f)
{
}

/*! \internal */
QFrame::QFrame(QFramePrivate &dd, QWidget* parent, Qt::WindowFlags f)
    : QWidget(dd, parent, f)
{
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QFrame::QFrame(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QWidget(*new QFramePrivate, parent, f)
{
    setObjectName(QString::fromAscii(name));
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
    Q_D(const QFrame);
    return d->frameStyle;
}

/*!
    \property QFrame::frameShape
    \brief the frame shape value from the frame style

    \sa frameStyle(), frameShadow()
*/

QFrame::Shape QFrame::frameShape() const
{
    Q_D(const QFrame);
    return (Shape) (d->frameStyle & Shape_Mask);
}

void QFrame::setFrameShape(QFrame::Shape s)
{
    Q_D(QFrame);
    setFrameStyle((d->frameStyle & Shadow_Mask) | s);
}


/*!
    \property QFrame::frameShadow
    \brief the frame shadow value from the frame style

    \sa frameStyle(), frameShape()
*/
QFrame::Shadow QFrame::frameShadow() const
{
    Q_D(const QFrame);
    return (Shadow) (d->frameStyle & Shadow_Mask);
}

void QFrame::setFrameShadow(QFrame::Shadow s)
{
    Q_D(QFrame);
    setFrameStyle((d->frameStyle & Shape_Mask) | s);
}

/*!
    Sets the frame style to \a style.

    The \a style is the bitwise OR between a frame shape and a frame
    shadow style. See the picture of the frames in the main class
    documentation.

    The frame shapes are given in \l{QFrame::Shape} and the shadow
    styles in \l{QFrame::Shadow}.

    If a mid-line width greater than 0 is specified, an additional
    line is drawn for \l Raised or \l Sunken \l Box, \l HLine, and \l
    VLine frames. The mid-color of the current color group is used for
    drawing middle lines.

    \sa frameStyle()
*/

void QFrame::setFrameStyle(int style)
{
    Q_D(QFrame);
    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        switch (style & Shape_Mask) {
        case HLine:
            setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            break;
        case VLine:
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
            break;
        default:
            if ((d->frameStyle & Shape_Mask) == HLine || (d->frameStyle & Shape_Mask) == VLine)
                setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        }
        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }
    d->frameStyle = (short)style;
    update();
    d->updateFrameWidth();
    d->oldFrameStyle = (short)style;
}

/*!
    \property QFrame::lineWidth
    \brief the line width

    Note that the \e total line width for frames used as separators
    (\l HLine and \l VLine) is specified by \l frameWidth.

    The default value is 1.

    \sa midLineWidth, frameWidth
*/

void QFrame::setLineWidth(int w)
{
    Q_D(QFrame);
    if (short(w) == d->lineWidth)
        return;
    d->lineWidth = short(w);
    d->updateFrameWidth();
}

int QFrame::lineWidth() const
{
    Q_D(const QFrame);
    return d->lineWidth;
}

/*!
    \property QFrame::midLineWidth
    \brief the width of the mid-line

    The default value is 0.

    \sa lineWidth, frameWidth
*/

void QFrame::setMidLineWidth(int w)
{
    Q_D(QFrame);
    if (short(w) == d->midLineWidth)
        return;
    d->midLineWidth = short(w);
    d->updateFrameWidth();
}

int QFrame::midLineWidth() const
{
    Q_D(const QFrame);
    return d->midLineWidth;
}

/*!
  \internal
  Updates the frame widths from the style.
*/
void QFramePrivate::updateStyledFrameWidths()
{
    Q_Q(const QFrame);
    QStyleOptionFrameV2 opt;
    opt.initFrom(q);
    QRect cr = q->style()->subElementRect(QStyle::SE_FrameContents, &opt, q);
    leftFrameWidth = cr.left() - opt.rect.left();
    topFrameWidth = cr.top() - opt.rect.top();
    rightFrameWidth = opt.rect.right() - cr.right(), 
    bottomFrameWidth = opt.rect.bottom() - cr.bottom();
    frameWidth = qMax(qMax(leftFrameWidth, rightFrameWidth), 
                      qMax(topFrameWidth, bottomFrameWidth));
}

/*!
  \internal
  Updated the frameWidth parameter.
*/

void QFramePrivate::updateFrameWidth()
{
    Q_Q(QFrame);
    QRect fr = q->frameRect();

    int frameShape  = frameStyle & QFrame::Shape_Mask;
    int frameShadow = frameStyle & QFrame::Shadow_Mask;

    frameWidth = -1;

    switch (frameShape) {

    case QFrame::NoFrame:
        frameWidth = 0;
        break;

    case QFrame::Box:
    case QFrame::HLine:
    case QFrame::VLine:
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

    case QFrame::StyledPanel:
        updateStyledFrameWidths();
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

    Note that the frame width depends on the \l{QFrame::setFrameStyle()}{frame style},
    not only the line width and the mid-line width. For example, the style specified
    by \l NoFrame always has a frame width of 0, whereas the style \l Panel has a
    frame width equivalent to the line width.

    \sa lineWidth(), midLineWidth(), frameStyle()
*/
int QFrame::frameWidth() const
{
    Q_D(const QFrame);
    return d->frameWidth;
}


/*!
    \property QFrame::frameRect
    \brief the frame's rectangle

    The frame's rectangle is the rectangle the frame is drawn in. By
    default, this is the entire widget. Setting the rectangle does
    does \e not cause a widget update. The frame rectangle is
    automatically adjusted when the widget changes size.

    If you set the rectangle to a null rectangle (for example,
    QRect(0, 0, 0, 0)), then the resulting frame rectangle is
    equivalent to the \link QWidget::rect() widget rectangle\endlink.
*/

QRect QFrame::frameRect() const
{
    Q_D(const QFrame);
    QRect fr = contentsRect();
    if ((d->oldFrameStyle & QFrame::Shape_Mask) == QFrame::StyledPanel) {
        fr.adjust(-d->leftFrameWidth, -d->topFrameWidth, d->rightFrameWidth, d->bottomFrameWidth);
    } else
        fr.adjust(-d->frameWidth, -d->frameWidth, d->frameWidth, d->frameWidth);
    return fr;
}

void QFrame::setFrameRect(const QRect &r)
{
    Q_D(QFrame);
    QRect cr = r.isValid() ? r : rect();
    if ((d->frameStyle & QFrame::Shape_Mask) == StyledPanel) {
        cr.adjust(d->leftFrameWidth, d->topFrameWidth, -d->rightFrameWidth, -d->bottomFrameWidth);
    } else
        cr.adjust(d->frameWidth, d->frameWidth, -d->frameWidth, -d->frameWidth);
    setContentsMargins(cr.left(), cr.top(), rect().right() - cr.right(), rect().bottom() - cr.bottom());
}

/*!\reimp
*/
QSize QFrame::sizeHint() const
{
    Q_D(const QFrame);
    //   Returns a size hint for the frame - for HLine and VLine
    //   shapes, this is stretchable one way and 3 pixels wide the
    //   other.  For other shapes, QWidget::sizeHint() is used.
    switch (d->frameStyle & Shape_Mask) {
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
    Q_D(QFrame);
    QPoint      p1, p2;
    QStyleOptionFrame opt;
    opt.init(this);
    int frameShape  = d->frameStyle & QFrame::Shape_Mask;
    int frameShadow = d->frameStyle & QFrame::Shadow_Mask;

    int lw = 0;
    int mlw = 0;
    opt.rect = frameRect();
    switch (frameShape) {
    case QFrame::Box:
    case QFrame::HLine:
    case QFrame::VLine:
    case QFrame::StyledPanel:
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
    if (frameShadow == Sunken)
        opt.state |= QStyle::State_Sunken;
    else if (frameShadow == Raised)
        opt.state |= QStyle::State_Raised;

    switch (frameShape) {
    case Box:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), lw);
        else
            qDrawShadeRect(p, opt.rect, opt.palette, frameShadow == Sunken, lw, mlw);
        break;


    case StyledPanel:
        style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
        break;

    case Panel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), lw);
        else
            qDrawShadePanel(p, opt.rect, opt.palette, frameShadow == Sunken, lw);
        break;

    case WinPanel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), lw);
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
            p->setPen(QPen(opt.palette.foreground().color(), lw));
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
    Q_D(QFrame);
    if(ev->type() == QEvent::StyleChange)
        d->updateFrameWidth();
    QWidget::changeEvent(ev);
}

/*! \reimp */
bool QFrame::event(QEvent *e)
{
    return QWidget::event(e);
}
