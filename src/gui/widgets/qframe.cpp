/****************************************************************************
**
** Implementation of QFrame widget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qframe.h"
#ifndef QT_NO_FRAME
#include "qpainter.h"
#include "qdrawutil.h"
#include "qframe.h"
#include "qbitmap.h"
#include "qstyle.h"
#include "qevent.h"

#include "qframe_p.h"
#define d d_func()
#define q q_func()


QFramePrivate::QFramePrivate()
    : frect(QRect(0, 0, 0, 0)),
      frameStyle(QFrame::NoFrame | QFrame::Plain),
      lineWidth(1),
      margin(0),
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
    \value MShape internal mask

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
    \value MShadow internal; mask for the shadow

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

QFrame::QFrame(QWidget* parent, WFlags f)
    : QWidget(*new QFramePrivate, parent, f)
{
}

/*! \internal */
QFrame::QFrame(QFramePrivate &dd, QWidget* parent, WFlags f)
    : QWidget(dd, parent, f)
{
}

/*! \internal */
QFrame::QFrame(QWidget *parent, const char *name, WFlags f)
    : QWidget(*new QFramePrivate, parent, f)
{
    setObjectName(name);
}

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
    if (!testWState(WState_OwnSizePolicy)) {
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
	clearWState(WState_OwnSizePolicy);
    }
    d->frameStyle = (short)style;
    d->updateFrameWidth();
}

/*!
    \property QFrame::lineWidth
    \brief the line width

    Note that the \e total line width for \c HLine and \c VLine is
    given by frameWidth(), not lineWidth().

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
    \property QFrame::margin
    \brief the width of the margin

    The margin is the distance between the innermost pixel of the
    frame and the outermost pixel of contentsRect(). It is included in
    frameWidth().

    The margin is filled according to backgroundMode().

    The default value is 0.

    \sa setMargin(), lineWidth(), frameWidth()
*/

void QFrame::setMargin(int w)
{
    d->margin = (short)w;
    d->updateFrameWidth();
}

int QFrame::margin() const
{
    return d->margin;
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
    case QFrame::StyledPanel:
	switch (frameShadow) {
	case QFrame::Plain:
	case QFrame::Raised:
	case QFrame::Sunken:
	    frameWidth = lineWidth;
	    break;
	}
	break;
    }

    if (frameWidth == -1)				// invalid style
	frameWidth = 0;

    frameWidth += margin;

    q->setFrameRect(fr);
}


/*!
    \property QFrame::frameWidth
    \brief the width of the frame that is drawn.

    Note that the frame width depends on the \link
    QFrame::setFrameStyle() frame style \endlink, not only the line
    width and the mid-line width. For example, the style \c NoFrame
    always has a frame width of 0, whereas the style \c Panel has a
    frame width equivalent to the line width. The frame width also
    includes the margin.

    \sa lineWidth(), midLineWidth(), frameStyle(), margin()
*/
int QFrame::frameWidth() const
{
    return d->frameWidth;
}


/*!
    Returns the frame's rectangle.

    \sa setFrameRect() contentsRect()
*/

QRect QFrame::frameRect() const
{
    QRect fr = contentsRect();
    fr.addCoords(-d->frameWidth, -d->frameWidth, d->frameWidth, d->frameWidth);
    return fr;
}

/*!
    Sets the frame's rectangle to \a r. The frame's rectangle is the
    rectangle the frame is drawn in. By default, this is the entire
    widget. Calling this function does \e not cause a widget update.

    If \a r is a null rectangle (for example \c{QRect(0, 0, 0, 0)}),
    then the frame rectangle is equivalent to the \link
    QWidget::rect() widget rectangle\endlink.

    \sa contentsRect()
*/

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
    QRect       r     = frameRect();
    int frameShape  = d->frameStyle & QFrame::MShape;
    int frameShadow = d->frameStyle & QFrame::MShadow;
    const QPalette &pal = palette();

    int lw = 0;
    int mlw = 0;
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
	lw = d->frameWidth - d->margin;
	break;
    }

    QStyleOption opt(lw, mlw);
    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (frameShadow == Sunken)
	flags |= QStyle::Style_Sunken;
    else if (frameShadow == Raised)
	flags |= QStyle::Style_Raised;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;
    if (testAttribute(WA_UnderMouse))
	flags |= QStyle::Style_MouseOver;

    switch (frameShape) {

    case Box:
        if (frameShadow == Plain)
            qDrawPlainRect(p, r, pal.foreground(), lw);
        else
            qDrawShadeRect(p, r, pal, frameShadow == Sunken, lw, mlw);
        break;

    case LineEditPanel:
	style().drawPrimitive(QStyle::PE_PanelLineEdit, p, r, pal, flags, opt);
	break;

    case GroupBoxPanel:
	style().drawPrimitive(QStyle::PE_PanelGroupBox, p, r, pal, flags, opt);
	break;

    case TabWidgetPanel:
	style().drawPrimitive(QStyle::PE_PanelTabWidget, p, r, pal, flags, opt);
	break;

    case MenuBarPanel:
	style().drawPrimitive(QStyle::PE_PanelMenuBar, p, r, pal, flags, opt);
	break;
    case ToolBarPanel:
	style().drawPrimitive(QStyle::PE_PanelDockWindow, p, rect(), pal, flags, opt);
        break;

    case StyledPanel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, r, pal.foreground(), lw);
        else
	    style().drawPrimitive(QStyle::PE_Panel, p, r, pal, flags, opt);
        break;

    case PopupPanel:
    {
	int vextra = style().pixelMetric(QStyle::PM_MenuFrameVerticalExtra, this),
	    hextra = style().pixelMetric(QStyle::PM_MenuFrameHorizontalExtra, this);
	if(vextra > 0 || hextra > 0) {
	    QRect fr = frameRect();
	    int   fw = d->frameWidth;
	    if(vextra > 0) {
		style().drawControl(QStyle::CE_MenuVerticalExtra, p, this,
				    QRect(fr.x() + fw, fr.y() + fw, fr.width() - (fw*2), vextra),
				    pal, flags, opt);
		style().drawControl(QStyle::CE_MenuVerticalExtra, p, this,
				    QRect(fr.x() + fw, fr.bottom() - fw - vextra, fr.width() - (fw*2), vextra),
				    pal, flags, opt);
	    }
	    if(hextra > 0) {
		style().drawControl(QStyle::CE_MenuHorizontalExtra, p, this,
				    QRect(fr.x() + fw, fr.y() + fw + vextra, hextra, fr.height() - (fw*2) - vextra),
				    pal, flags, opt);
		style().drawControl(QStyle::CE_MenuHorizontalExtra, p, this,
				    QRect(fr.right() - fw - hextra, fr.y() + fw + vextra, hextra, fr.height() - (fw*2) - vextra),
				    pal, flags, opt);
	    }
	}

        if (frameShadow == Plain)
            qDrawPlainRect(p, r, pal.foreground(), lw);
        else
	    style().drawPrimitive(QStyle::PE_PanelPopup, p, r, pal, flags, opt);
        break;
    }

    case Panel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, r, pal.foreground(), lw);
        else
            qDrawShadePanel(p, r, pal, frameShadow == Sunken, lw);
        break;

    case WinPanel:
        if (frameShadow == Plain)
            qDrawPlainRect(p, r, pal.foreground(), lw);
        else
            qDrawWinPanel(p, r, pal, frameShadow == Sunken);
        break;
    case HLine:
    case VLine:
        if (frameShape == HLine) {
            p1 = QPoint(r.x(), r.height()/2);
            p2 = QPoint(r.x()+r.width(), p1.y());
        }
        else {
            p1 = QPoint(r.x()+r.width()/2, 0);
            p2 = QPoint(p1.x(), r.height());
        }
        if (frameShadow == Plain) {
            QPen oldPen = p->pen();
            p->setPen(QPen(pal.foreground(),lw));
            p->drawLine(p1, p2);
            p->setPen(oldPen);
        }
        else
            qDrawShadeLine(p, p1, p2, pal, frameShadow == Sunken, lw, mlw);
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





Q3Frame::Q3Frame(QWidget* parent, const char* name, WFlags f)
    :QFrame(parent, f)
{
    if (name)
	setObjectName(name);
    setAttribute(WA_LayoutOnEntireRect);
}

Q3Frame::~Q3Frame()
{
}

void Q3Frame::paintEvent(QPaintEvent * event)
{
    QPainter paint( this );
    if ( !contentsRect().contains( event->rect() ) ) {
        paint.save();
        paint.setClipRegion( event->region().intersect(frameRect()) );
        drawFrame( &paint );
        paint.restore();
    }
    if (event->rect().intersects(contentsRect())) {
        paint.setClipRegion(event->region().intersect(contentsRect()));
        drawContents(&paint);
    }
}

/*!
    Virtual function that draws the contents of the frame.

    The QPainter is already open when you get it, and you must leave
    it open. Painter \link QPainter::setWorldMatrix()
    transformations\endlink are switched off on entry. If you
    transform the painter, remember to take the frame into account and
    \link QPainter::resetXForm() reset transformation\endlink before
    returning.

    This function is reimplemented by subclasses that draw something
    inside the frame. It should only draw inside contentsRect(). The
    default function does nothing.

    \sa contentsRect(), QPainter::setClipRect()
*/

void Q3Frame::drawContents(QPainter *)
{
}

/*!
    Draws the frame using the painter \a p and the current frame
    attributes and color group. The rectangle inside the frame is not
    affected.

    This function is virtual, but in general you do not need to
    reimplement it. If you do, note that the QPainter is already open
    and must remain open.

    \sa frameRect(), contentsRect(), drawContents(), frameStyle(), setPalette()
*/

void Q3Frame::drawFrame( QPainter *p )
{
    QFrame::drawFrame(p);
}

void Q3Frame::resizeEvent(QResizeEvent *)
{
    frameChanged();
}

/*!
    Virtual function that is called when the frame style, line width
    or mid-line width changes.

    This function can be reimplemented by subclasses that need to know
    when the frame attributes change.
*/

void Q3Frame::frameChanged()
{
}

#endif //QT_NO_FRAME
