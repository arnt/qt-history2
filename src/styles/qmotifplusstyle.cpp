/****************************************************************************
** $Id: $
**
** Implementation of QMotifPlusStyle class
**
** Created : 2000.07.27
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmotifplusstyle.h"

#ifndef QT_NO_STYLE_MOTIFPLUS

#include "qmenubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qframe.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qradiobutton.h"
#include "qdrawutil.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qguardedptr.h"
#include "qlayout.h"


struct QMotifPlusStylePrivate
{
    QMotifPlusStylePrivate()
        : hoverWidget(0), hovering(FALSE), sliderActive(FALSE), mousePressed(FALSE),
          scrollbarElement(0), lastElement(0), ref(1), hoverPalette(0)
    { ; }

    QGuardedPtr<QWidget> hoverWidget;
    QPalette oldpalette, prelight_palette;
    bool hovering, sliderActive, mousePressed;
    int scrollbarElement, lastElement, ref;
    QPoint mousePos;
    QPalette *hoverPalette;
};

static QMotifPlusStylePrivate * singleton = 0;


static void drawMotifPlusShade(QPainter *p,
			       const QRect &r,
			       const QColorGroup &g,
			       bool sunken,
			       const QBrush *fill = 0)
{
    QPen oldpen = p->pen();
    QPointArray a(4);
    int x, y, w, h;

    r.rect(&x, &y, &w, &h);

    if (sunken) p->setPen(g.dark()); else p->setPen(g.light());
    a.setPoint(0, x, y + h - 1);
    a.setPoint(1, x, y);
    a.setPoint(2, x, y);
    a.setPoint(3, x + w - 1, y);
    p->drawLineSegments(a);

    if (sunken) p->setPen(Qt::black); else p->setPen(g.button());
    a.setPoint(0, x + 1, y + h - 2);
    a.setPoint(1, x + 1, y + 1);
    a.setPoint(2, x + 1, y + 1);
    a.setPoint(3, x + w - 2, y + 1);
    p->drawLineSegments(a);

    if (sunken) p->setPen(g.button()); else p->setPen(g.dark());
    a.setPoint(0, x + 2, y + h - 2);
    a.setPoint(1, x + w - 2, y + h - 2);
    a.setPoint(2, x + w - 2, y + h - 2);
    a.setPoint(3, x + w - 2, y + 2);
    p->drawLineSegments(a);

    if (sunken) p->setPen(g.light()); else p->setPen(Qt::black);
    a.setPoint(0, x + 1, y + h - 1);
    a.setPoint(1, x + w - 1, y + h - 1);
    a.setPoint(2, x + w - 1, y + h - 1);
    a.setPoint(3, x + w - 1, y);
    p->drawLineSegments(a);

    if (fill)
	p->fillRect(x + 2, y + 2, w - 4, h - 4, *fill);
    else
	p->fillRect(x + 2, y + 2, w - 4, h - 4, QBrush(g.button()));

    p->setPen(oldpen);
}


/*!
  \class QMotifPlusStyle qmotifplusstyle.h
  \brief The QMotifPlusStyle class provides a more sophisticated Motif-ish look and feel.
  \ingroup appearance

 This class implements a Motif-ish look and feel with more
 sophisticated bevelling as used by the GIMP Toolkit (GTK+) for
 Unix/X11.
*/

/*!
  Constructs a QMotifPlusStyle

  If \a hoveringHighlight is FALSE (the default), then the style will not
  highlight push buttons, checkboxes, radiobuttons, comboboxes, scrollbars
  or sliders.
 */
QMotifPlusStyle::QMotifPlusStyle(bool hoveringHighlight) : QMotifStyle(TRUE)
{
    if ( !singleton )
        singleton = new QMotifPlusStylePrivate;
    else
        singleton->ref++;

    useHoveringHighlight = hoveringHighlight;
    useHoveringHighlight = TRUE;
}

/*! \reimp */
QMotifPlusStyle::~QMotifPlusStyle()
{
    if ( singleton && singleton->ref-- <= 0) {
        delete singleton;
        singleton = 0;
    }
}


/*! \reimp */
void QMotifPlusStyle::polish(QPalette &)
{

}


/*! \reimp */
void QMotifPlusStyle::polish(QWidget *widget)
{
    if (widget->inherits("QFrame") &&
        ((QFrame *) widget)->frameStyle() == QFrame::Panel)
        ((QFrame *) widget)->setFrameStyle(QFrame::WinPanel);

#ifndef QT_NO_MENUBAR
    if (widget->inherits("QMenuBar") &&
        ((QMenuBar *) widget)->frameStyle() != QFrame::NoFrame)
        ((QMenuBar *) widget)->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
#endif

    if (widget->inherits("QToolBar"))
        widget->layout()->setMargin(2);

    if (useHoveringHighlight) {
	if (widget->inherits("QButton") ||
	    widget->inherits("QComboBox"))
	    widget->installEventFilter(this);

	if (widget->inherits("QScrollBar") ||
	    widget->inherits("QSlider")) {
	    widget->setMouseTracking(TRUE);
	    widget->installEventFilter(this);
	}
    }

    QMotifStyle::polish(widget);
}


/*! \reimp */
void QMotifPlusStyle::unPolish(QWidget *widget)
{
    widget->removeEventFilter(this);
    QMotifStyle::unPolish(widget);
}


/*! \reimp */
void QMotifPlusStyle::polish(QApplication *app)
{
    QPalette pal = app->palette();

    QColor bg = pal.color(QPalette::Active, QColorGroup::Background);

    if (bg.red()   == 0xc0 &&
        bg.green() == 0xc0 &&
        bg.blue()  == 0xc0) {
        // assume default palette... no -bg arg or color read from RESOURCE_MANAGER

        QColor gtkdf(0x75, 0x75, 0x75);
        QColor gtksf(0xff, 0xff, 0xff);
        QColor gtkbg(0xd6, 0xd6, 0xd6);
        QColor gtksl(0x00, 0x00, 0x9c);

        pal.setColor(QPalette::Active, QColorGroup::Background, gtkbg);
        pal.setColor(QPalette::Inactive, QColorGroup::Background, gtkbg);
        pal.setColor(QPalette::Disabled, QColorGroup::Background, gtkbg);

        pal.setColor(QPalette::Active, QColorGroup::Button, gtkbg);
        pal.setColor(QPalette::Inactive, QColorGroup::Button, gtkbg);
        pal.setColor(QPalette::Disabled, QColorGroup::Button, gtkbg);

        pal.setColor(QPalette::Active, QColorGroup::Highlight, gtksl);
        pal.setColor(QPalette::Inactive, QColorGroup::Highlight, gtksl);
        pal.setColor(QPalette::Disabled, QColorGroup::Highlight, gtksl);

        pal.setColor(QPalette::Active, QColorGroup::HighlightedText, gtksf);
        pal.setColor(QPalette::Inactive, QColorGroup::HighlightedText, gtksf);
        pal.setColor(QPalette::Disabled, QColorGroup::HighlightedText, gtkdf);
    }

    {
        QColorGroup active(pal.color(QPalette::Active,
                                     QColorGroup::Foreground),           // foreground
                           pal.color(QPalette::Active,
                                     QColorGroup::Button),               // button
                           pal.color(QPalette::Active,
                                     QColorGroup::Background).light(),   // light
                           pal.color(QPalette::Active,
                                     QColorGroup::Background).dark(142), // dark
                           pal.color(QPalette::Active,
                                     QColorGroup::Background).dark(110), // mid
                           pal.color(QPalette::Active,
                                     QColorGroup::Text),                 // text
                           pal.color(QPalette::Active,
                                     QColorGroup::BrightText),           // bright text
                           pal.color(QPalette::Active,
                                     QColorGroup::Base),                 // base
                           pal.color(QPalette::Active,
                                     QColorGroup::Background)),          // background


            disabled(pal.color(QPalette::Disabled,
                               QColorGroup::Foreground),                 // foreground
                     pal.color(QPalette::Disabled,
                               QColorGroup::Button),                     // button
                     pal.color(QPalette::Disabled,
                               QColorGroup::Background).light(),         // light
                     pal.color(QPalette::Disabled,
                               QColorGroup::Background).dark(156),       // dark
                     pal.color(QPalette::Disabled,
                               QColorGroup::Background).dark(110),       // mid
                     pal.color(QPalette::Disabled,
                               QColorGroup::Text),                       // text
                     pal.color(QPalette::Disabled,
                               QColorGroup::BrightText),                 // bright text
                     pal.color(QPalette::Disabled,
                               QColorGroup::Base),                       // base
                     pal.color(QPalette::Disabled,
                               QColorGroup::Background));                // background

        active.setColor(QColorGroup::Highlight,
                        pal.color(QPalette::Active, QColorGroup::Highlight));
        disabled.setColor(QColorGroup::Highlight,
                          pal.color(QPalette::Disabled, QColorGroup::Highlight));

        active.setColor(QColorGroup::HighlightedText,
                        pal.color(QPalette::Active, QColorGroup::HighlightedText));
        disabled.setColor(QColorGroup::HighlightedText,
                          pal.color(QPalette::Disabled, QColorGroup::HighlightedText));

        pal.setActive(active);
        pal.setInactive(active);
        pal.setDisabled(disabled);
    }

    singleton->oldpalette = pal;

    QColor prelight;

    if ( (bg.red() + bg.green() + bg.blue()) / 3 > 128)
	prelight = pal.color(QPalette::Active,
			     QColorGroup::Background).light(110);
    else
	prelight = pal.color(QPalette::Active,
			     QColorGroup::Background).light(120);

    QColorGroup active2(pal.color(QPalette::Active,
				  QColorGroup::Foreground), // foreground
			prelight,                           // button
			prelight.light(),                   // light
			prelight.dark(156),                 // dark
			prelight.dark(110),                 // mid
			pal.color(QPalette::Active,
				  QColorGroup::Text),       // text
			pal.color(QPalette::Active,
				  QColorGroup::BrightText), // bright text
			pal.color(QPalette::Active,
				  QColorGroup::Base),       // base
			prelight);                          // background

    singleton->prelight_palette = pal;
    singleton->prelight_palette.setActive(active2);
    singleton->prelight_palette.setInactive(active2);

    app->setPalette(pal);
}


/*! \reimp */
void QMotifPlusStyle::unPolish(QApplication *app)
{
    app->setPalette(singleton->oldpalette);
}


/*! \reimp */
void QMotifPlusStyle::polishPopupMenu(QPopupMenu *menu)
{
    menu->setMouseTracking(TRUE);
}



/*! \reimp */
int QMotifPlusStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ScrollBarExtent:
	ret = 15;
	break;

    case PM_ButtonDefaultIndicator:
	ret = 5;
	break;

    case PM_ButtonMargin:
	ret = 4;
	break;

    case PM_SliderThickness:
	ret = 15;
	break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
	ret = 10;
	break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	ret = 11;
	break;

    default:
	ret = QMotifStyle::pixelMetric(metric, widget);
	break;
    }

    return ret;
}


/*! \reimp */
void QMotifPlusStyle::drawPrimitive( PrimitiveElement pe,
				     QPainter *p,
				     const QRect &r,
				     const QColorGroup &cg,
				     SFlags flags,
				     void **data ) const
{
    switch (pe) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_HeaderSection:
	drawMotifPlusShade( p, r, cg, bool(flags & (Style_Down | Style_On)));
	break;

    case PE_Panel:
    case PE_PanelPopup:
    case PE_PanelMenuBar:
    case PE_PanelDockWindow:
	drawMotifPlusShade( p, r, cg, bool(flags & Style_Sunken));
	break;

    case PE_Indicator:
	{
	    QBrush fill;
	    if (flags & Style_On)
		fill = cg.brush(QColorGroup::Mid);
	    else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & Style_NoChange) {
		qDrawPlainRect(p, r, cg.text(), 1, &fill);
		p->drawLine(r.topRight(), r.bottomLeft());
	    } else
		drawMotifPlusShade(p, r, cg, (flags & Style_On), &fill);
	    break;
	}

    case PE_ExclusiveIndicator:
	{
	    QPen oldpen =  p->pen();
	    QPointArray thick(8);
	    QPointArray thin(4);
	    int x, y, w, h;
	    r.rect(&x, &y, &w, &h);

	    p->fillRect(x, y, w, h, cg.button());


	    if (flags & Style_On) {
		thick.setPoint(0, x, y + (h / 2));
		thick.setPoint(1, x + (w / 2), y);
		thick.setPoint(2, x + 1, y + (h / 2));
		thick.setPoint(3, x + (w / 2), y + 1);
		thick.setPoint(4, x + (w / 2), y);
		thick.setPoint(5, x + w - 1, y + (h / 2));
		thick.setPoint(6, x + (w / 2), y + 1);
		thick.setPoint(7, x + w - 2, y + (h / 2));
		p->setPen(cg.dark());
		p->drawLineSegments(thick);

		thick.setPoint(0, x + 1, y + (h / 2) + 1);
		thick.setPoint(1, x + (w / 2), y + h - 1);
		thick.setPoint(2, x + 2, y + (h / 2) + 1);
		thick.setPoint(3, x + (w / 2), y + h - 2);
		thick.setPoint(4, x + (w / 2), y + h - 1);
		thick.setPoint(5, x + w - 2, y + (h / 2) + 1);
		thick.setPoint(6, x + (w / 2), y + h - 2);
		thick.setPoint(7, x + w - 3, y + (h / 2) + 1);
		p->setPen(cg.light());
		p->drawLineSegments(thick);

		thin.setPoint(0, x + 2, y + (h / 2));
		thin.setPoint(1, x + (w / 2), y + 2);
		thin.setPoint(2, x + (w / 2), y + 2);
		thin.setPoint(3, x + w - 3, y + (h / 2));
		p->setPen(Qt::black);
		p->drawLineSegments(thin);

		thin.setPoint(0, x + 3, y + (h / 2) + 1);
		thin.setPoint(1, x + (w / 2), y + h - 3);
		thin.setPoint(2, x + (w / 2), y + h - 3);
		thin.setPoint(3, x + w - 4, y + (h / 2) + 1);
		p->setPen(cg.mid());
		p->drawLineSegments(thin);
	    } else {
		thick.setPoint(0, x, y + (h / 2));
		thick.setPoint(1, x + (w / 2), y);
		thick.setPoint(2, x + 1, y + (h / 2));
		thick.setPoint(3, x + (w / 2), y + 1);
		thick.setPoint(4, x + (w / 2), y);
		thick.setPoint(5, x + w - 1, y + (h / 2));
		thick.setPoint(6, x + (w / 2), y + 1);
		thick.setPoint(7, x + w - 2, y + (h / 2));
		p->setPen(cg.light());
		p->drawLineSegments(thick);

		thick.setPoint(0, x + 2, y + (h / 2) + 1);
		thick.setPoint(1, x + (w / 2), y + h - 2);
		thick.setPoint(2, x + 3, y + (h / 2) + 1);
		thick.setPoint(3, x + (w / 2), y + h - 3);
		thick.setPoint(4, x + (w / 2), y + h - 2);
		thick.setPoint(5, x + w - 3, y + (h / 2) + 1);
		thick.setPoint(6, x + (w / 2), y + h - 3);
		thick.setPoint(7, x + w - 4, y + (h / 2) + 1);
		p->setPen(cg.dark());
		p->drawLineSegments(thick);

		thin.setPoint(0, x + 2, y + (h / 2));
		thin.setPoint(1, x + (w / 2), y + 2);
		thin.setPoint(2, x + (w / 2), y + 2);
		thin.setPoint(3, x + w - 3, y + (h / 2));
		p->setPen(cg.button());
		p->drawLineSegments(thin);

		thin.setPoint(0, x + 1, y + (h / 2) + 1);
		thin.setPoint(1, x + (w / 2), y + h - 1);
		thin.setPoint(2, x + (w / 2), y + h - 1);
		thin.setPoint(3, x + w - 2, y + (h / 2) + 1);
		p->setPen(Qt::black);
		p->drawLineSegments(thin);
	    }

	    p->setPen(oldpen);
	    break;
	}

    case PE_ArrowDown:
    case PE_ArrowLeft:
    case PE_ArrowRight:
    case PE_ArrowUp:
	{
	    QPen oldpen = p->pen();
	    QBrush oldbrush = p->brush();
	    QPointArray poly(3);
	    bool down = (flags & Style_Down);
	    int x, y, w, h;
	    r.rect(&x, &y, &w, &h);

	    p->save();
	    p->setBrush(cg.button());

	    switch (pe) {
	    case PE_ArrowUp:
		{
		    poly.setPoint(0, x + (w / 2), y );
		    poly.setPoint(1, x, y + h - 1);
		    poly.setPoint(2, x + w - 1, y + h - 1);
		    p->drawPolygon(poly);

		    if (down)
			p->setPen(cg.button());
		    else
			p->setPen(cg.dark());
		    p->drawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);

		    if (down)
			p->setPen(cg.light());
		    else
			p->setPen(black);
		    p->drawLine(x, y + h - 1, x + w - 1, y + h - 1);

		    if (down)
			p->setPen(cg.button());
		    else
			p->setPen(cg.dark());
		    p->drawLine(x + w - 2, y + h - 1, x + (w / 2), y + 1);

		    if (down)
			p->setPen(cg.light());
		    else
			p->setPen(black);
		    p->drawLine(x + w - 1, y + h - 1, x + (w / 2), y);

		    if (down)
			p->setPen(black);
		    else
			p->setPen(cg.button());
		    p->drawLine(x + (w / 2), y + 1, x + 1, y + h - 1);

		    if (down)
			p->setPen(cg.dark());
		    else
			p->setPen(cg.light());
		    p->drawLine(x + (w / 2), y, x, y + h - 1);
		    break;
		}

	    case PE_ArrowDown:
		{
		    poly.setPoint(0, x + w - 1, y);
		    poly.setPoint(1, x, y);
		    poly.setPoint(2, x + (w / 2), y + h - 1);
		    p->drawPolygon(poly);

		    if (down)
			p->setPen(black);
		    else
			p->setPen(cg.button());
		    p->drawLine(x + w - 2, y + 1, x + 1, y + 1);

		    if (down)
			p->setPen(cg.dark());
		    else
			p->setPen(cg.light());
		    p->drawLine(x + w - 1, y, x, y);

		    if (down)
			p->setPen(black);
		    else
			p->setPen(cg.button());
		    p->drawLine(x + 1, y, x + (w / 2), y + h - 2);

		    if (down)
			p->setPen(cg.dark());
		    else
			p->setPen(cg.light());
		    p->drawLine(x, y, x + (w / 2), y + h - 1);

		    if (down)
			p->setPen(cg.button());
		    else
			p->setPen(cg.dark());
		    p->drawLine(x + (w / 2), y + h - 2, x + w - 2, y);

		    if (down)
			p->setPen(cg.light());
		    else
			p->setPen(black);
		    p->drawLine(x + (w / 2), y + h - 1, x + w - 1, y);
		    break;
		}

	    case PE_ArrowLeft:
		{
		    poly.setPoint(0, x, y + (h / 2));
		    poly.setPoint(1, x + w - 1, y + h - 1);
		    poly.setPoint(2, x + w - 1, y);
		    p->drawPolygon(poly);

		    if (down)
			p->setPen(cg.button());
		    else
			p->setPen(cg.dark());
		    p->drawLine(x + 1, y + (h / 2), x + w - 1, y + h - 1);

		    if (down)
			p->setPen(cg.light());
		    else
			p->setPen(black);
		    p->drawLine(x, y + (h / 2), x + w - 1, y + h - 1);

		    if (down)
			p->setPen(cg.button());
		    else
			p->setPen(cg.dark());
		    p->drawLine(x + w - 2, y + h - 1, x + w - 2, y + 1);

		    if (down)
			p->setPen(cg.light());
		    else
			p->setPen(black);
		    p->drawLine(x + w - 1, y + h - 1, x + w - 1, y);

		    if (down)
			p->setPen(black);
		    else
			p->setPen(cg.button());
		    p->drawLine(x + w - 1, y + 1, x + 1, y + (h / 2));

		    if (down)
			p->setPen(cg.dark());
		    else
			p->setPen(cg.light());
		    p->drawLine(x + w - 1, y, x, y + (h / 2));
		    break;
		}

	    case PE_ArrowRight:
		{
		    poly.setPoint(0, x + w - 1, y + (h / 2));
		    poly.setPoint(1, x, y);
		    poly.setPoint(2, x, y + h - 1);
		    p->drawPolygon(poly);

		    if (down)
			p->setPen(black);
		    else
			p->setPen(cg.button());
		    p->drawLine( x + w - 1, y + (h / 2), x + 1, y + 1);

		    if (down)
			p->setPen(cg.dark());
		    else
			p->setPen(cg.light());
		    p->drawLine(x + w - 1, y + (h / 2), x, y);

		    if (down)
			p->setPen(black);
		    else
			p->setPen(cg.button());
		    p->drawLine(x + 1, y + 1, x + 1, y + h - 2);

		    if (down)
			p->setPen(cg.dark());
		    else
			p->setPen(cg.light());
		    p->drawLine(x, y, x, y + h - 1);

		    if (down)
			p->setPen(cg.button());
		    else
			p->setPen(cg.dark());
		    p->drawLine(x + 1, y + h - 2, x + w - 1, y + (h / 2));

		    if (down)
			p->setPen(cg.light());
		    else
			p->setPen(black);
		    p->drawLine(x, y + h - 1, x + w - 1, y + (h / 2));
		    break;
		}

	    default:
		break;
	    }

	    p->restore();
	    p->setBrush(oldbrush);
	    p->setPen(oldpen);
   	    break;
	}

    default:
	QMotifStyle::drawPrimitive(pe, p, r, cg, flags, data);
	break;
    }
}


void QMotifPlusStyle::drawControl( ControlElement element,
				   QPainter *p,
				   const QWidget *widget,
				   const QRect &r,
				   const QColorGroup &cg,
				   SFlags how,
				   void **data = 0 ) const
{
    switch (element) {
    case CE_PushButton:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect br = r;
	    int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

	    if (button->isEnabled())
		how |= Style_Enabled;
	    if (button->isOn())
		how |= Style_On;
	    if (button->isDown())
		how |= Style_Down;
	    else if (! button->isFlat() && ! (how & Style_Down))
		how |= Style_Raised;

	    if (button->isDefault() || button->autoDefault()) {
		if ( button->isDefault())
		    drawMotifPlusShade(p, br, cg, TRUE,
				       &cg.brush(QColorGroup::Background));

		br.setCoords(br.left()   + dbi,
			     br.top()    + dbi,
			     br.right()  - dbi,
			     br.bottom() - dbi);
	    }

	    br.addCoords(1, 1, -1, -1);
	    drawPrimitive(PE_ButtonCommand, p, br, cg, how);
	    break;
	}

    case CE_CheckBoxLabel:
	{
	    const QCheckBox *checkbox = (const QCheckBox *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     how & Style_Enabled, checkbox->pixmap(), checkbox->text());

	    if (checkbox->hasFocus()) {
		QRect fr = visualRect(subRect(SR_CheckBoxFocusRect, widget), widget);
		drawPrimitive(PE_FocusRect, p, fr, cg, how);
	    }
	    break;
	}

    case CE_RadioButtonLabel:
	{
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     how & Style_Enabled, radiobutton->pixmap(), radiobutton->text());

	    if (radiobutton->hasFocus()) {
		QRect fr = visualRect(subRect(SR_RadioButtonFocusRect, widget), widget);
		drawPrimitive(PE_FocusRect, p, fr, cg, how);
	    }

	    break;
	}

    case CE_MenuBarItem:
	{
	    if (! data)
		break;

	    QMenuItem *mi = (QMenuItem *) data[0];
	    bool enabled = mi->isEnabled();
	    bool active = how & Style_Active;
	    if (enabled && active)
		drawMotifPlusShade(p, r, singleton->prelight_palette.active(), FALSE);
	    else
		p->fillRect(r, cg.button());

	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine,
		     cg, enabled, mi->pixmap(), mi->text(), -1, &cg.buttonText());
	    break;
	}


#ifndef QT_NO_POPUPMENU
    case CE_PopupMenuItem:
	{
	    if (! widget || ! data)
		break;

	    QPopupMenu *popupmenu = (QPopupMenu *) widget;
	    QMenuItem *mi = (QMenuItem *) data[0];
	    if ( !mi )
		break;

	    int tab = *((int *) data[1]);
	    int maxpmw = *((int *) data[2]);
	    bool dis = ! mi->isEnabled();
	    bool checkable = popupmenu->isCheckable();
	    bool act = how & Style_Selected;
	    int x, y, w, h;
	    const QColorGroup &g = ((act && !dis) ?
				    singleton->prelight_palette.active() : cg);

	    r.rect(&x, &y, &w, &h);


	    if (checkable)
		maxpmw = QMAX(maxpmw, 15);

	    int checkcol = maxpmw;

	    if (mi && mi->isSeparator()) {
		p->setPen( g.dark() );
		p->drawLine( x, y, x+w, y );
		p->setPen( g.light() );
		p->drawLine( x, y+1, x+w, y+1 );
		return;
	    }

	    if ( act && !dis )
		drawMotifPlusShade(p, QRect(x, y, w, h), g, FALSE,
				   &g.brush(QColorGroup::Button));
	    else
		p->fillRect(x, y, w, h, g.brush( QColorGroup::Button ));

	    if ( !mi )
		return;

	    if ( mi->isChecked() ) {
		if ( mi->iconSet() ) {
		    qDrawShadePanel( p, x+2, y+2, checkcol, h-2*2,
				     g, TRUE, 1, &g.brush( QColorGroup::Midlight ) );
		}
	    } else if ( !act ) {
		p->fillRect(x+2, y+2, checkcol, h-2*2,
			    g.brush( QColorGroup::Button ));
	    }

	    if ( mi->iconSet() ) {              // draw iconset
		QIconSet::Mode mode = (!dis) ? QIconSet::Normal : QIconSet::Disabled;

		if (act && !dis)
		    mode = QIconSet::Active;

		QPixmap pixmap;
		if ( checkable && mi->isChecked() )
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode,
						    QIconSet::On );
		else
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );

		int pixw = pixmap.width();
		int pixh = pixmap.height();

		QRect cr( x + 2, y+2, checkcol, h-2*2 );
		QRect pmr( 0, 0, pixw, pixh );

		pmr.moveCenter(cr.center());

		p->setPen( cg.text() );
		p->drawPixmap( pmr.topLeft(), pixmap );

	    } else if (checkable) {
		int mw = checkcol;
		int mh = h - 4;

		if (mi->isChecked()) {
		    SFlags cflags = Style_Default;
		    if (! dis)
			cflags |= Style_Enabled;
		    if (act)
			cflags |= Style_On;

		    drawPrimitive(PE_CheckMark, p, QRect(x+2, y+2, mw, mh), cg, cflags);
		}
	    }

	    p->setPen( g.buttonText() );

	    QColor discol;
	    if (dis) {
		discol = cg.text();
		p->setPen( discol );
	    }

	    if (mi->custom()) {
		p->save();
		mi->custom()->paint(p, cg, act, !dis, x + checkcol + 4, y + 2,
				    w - checkcol - tab - 3, h - 4);
		p->restore();
	    }

	    QString s = mi->text();
	    if ( !s.isNull() ) {                        // draw text
		int t = s.find( '\t' );
		int m = 2;
		const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
		if ( t >= 0 ) {                         // draw tab text
		    p->drawText( x+w-tab-2-2,
				 y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
		}
		p->drawText(x + checkcol + 4, y + 2, w - checkcol -tab - 3, h - 4,
			    text_flags, s, t);
	    } else if (mi->pixmap()) {
		QPixmap *pixmap = mi->pixmap();

		if (pixmap->depth() == 1) p->setBackgroundMode(OpaqueMode);
		p->drawPixmap(x + checkcol + 2, y + 2, *pixmap);
		if (pixmap->depth() == 1) p->setBackgroundMode(TransparentMode);
	    }

	    if (mi->popup()) {
		int hh = h / 2;
		drawPrimitive(PE_ArrowRight, p,
			      QRect(x + w - hh - 6, y + (hh / 2), hh, hh), g,
			      ((act && mi->isEnabled()) ?
			       Style_Down : Style_Default) |
			      ((mi->isEnabled()) ? Style_Enabled : Style_Default));
	    }
	    break;
	}
#endif // QT_NO_POPUPMENU

    default:
	QMotifStyle::drawControl(element, p, widget, r, cg, how, data);
	break;
    }
}


QRect QMotifPlusStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect;

    switch (r) {
    case SR_PushButtonFocusRect:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    int dfi = pixelMetric(PM_ButtonDefaultIndicator, widget);

	    rect = button->rect();
	    if (button->isDefault() || button->autoDefault())
		rect.addCoords(dfi, dfi, -dfi, -dfi);

	    break;
	}

    case SR_CheckBoxIndicator:
	{
	    int h = pixelMetric( PM_IndicatorHeight );
	    rect.setRect(( widget->rect().height() - h ) / 2,
			 ( widget->rect().height() - h ) / 2,
			 pixelMetric( PM_IndicatorWidth ), h );
	    break;
	}

    case SR_RadioButtonIndicator:
	{
	    int h = pixelMetric( PM_ExclusiveIndicatorHeight );
	    rect.setRect( ( widget->rect().height() - h ) / 2,
			  ( widget->rect().height() - h ) / 2,
			  pixelMetric( PM_ExclusiveIndicatorWidth ), h );
	    break;
	}

    case SR_CheckBoxFocusRect:
    case SR_RadioButtonFocusRect:
       	rect = widget->rect();
	break;

    default:
	rect = QMotifStyle::subRect(r, widget);
	break;
    }

    return rect;
}


/*! \reimp */
void QMotifPlusStyle::drawComplexControl(ComplexControl control,
			    QPainter *p,
			    const QWidget *widget,
			    const QRect &r,
			    const QColorGroup &cg,
			    SFlags how,
			    SCFlags controls,
			    SCFlags active,
			    void **data ) const
{
    switch (control) {
    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    QRect addline, subline, addpage, subpage, slider, first, last;
	    bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	    subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, data);
	    addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, data);
	    subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, data);
	    addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, data);
	    slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  data);
	    first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   data);
	    last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    data);

	    bool skipUpdate = FALSE;
	    if (singleton->hovering) {
		if (addline.contains(singleton->mousePos)) {
		    skipUpdate =
			(singleton->scrollbarElement == SC_ScrollBarAddLine);
		    singleton->scrollbarElement = SC_ScrollBarAddLine;
		} else if (subline.contains(singleton->mousePos)) {
		    skipUpdate =
			(singleton->scrollbarElement == SC_ScrollBarSubLine);
		    singleton->scrollbarElement = SC_ScrollBarSubLine;
		} else if (slider.contains(singleton->mousePos)) {
		    skipUpdate =
			(singleton->scrollbarElement == SC_ScrollBarSlider);
		    singleton->scrollbarElement = SC_ScrollBarSlider;
		} else {
		    skipUpdate =
			(singleton->scrollbarElement == 0);
		    singleton->scrollbarElement = 0;
		}
	    } else
		singleton->scrollbarElement = 0;

	    if (skipUpdate && singleton->scrollbarElement == singleton->lastElement)
		break;

	    singleton->lastElement = singleton->scrollbarElement;

	    if (controls == (SC_ScrollBarAddLine | SC_ScrollBarSubLine |
			     SC_ScrollBarAddPage | SC_ScrollBarSubPage |
			     SC_ScrollBarFirst | SC_ScrollBarLast | SC_ScrollBarSlider))
		drawMotifPlusShade(p, widget->rect(), cg, TRUE,
				   &cg.brush(QColorGroup::Mid));

	    if ((controls & SC_ScrollBarSubLine) && subline.isValid())
		drawPrimitive(PE_ScrollBarSubLine, p, subline,
			      ((active == SC_ScrollBarSubLine ||
				singleton->scrollbarElement == SC_ScrollBarSubLine) ?
			       singleton->prelight_palette.active(): cg),
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, addline,
			      ((active == SC_ScrollBarAddLine ||
				singleton->scrollbarElement == SC_ScrollBarAddLine) ?
			       singleton->prelight_palette.active(): cg),
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarFirst) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, last, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarLast) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, slider,
			      ((active == SC_ScrollBarSlider ||
				singleton->scrollbarElement == SC_ScrollBarSlider) ?
			       singleton->prelight_palette.active(): cg),
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSlider) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (scrollbar->hasFocus()) {
		    QRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
		}
	    }

	    break;
	}

    default:
	QMotifStyle::drawComplexControl(control, p, widget, r, cg, how,
					controls, active, data);
    }
}


/*! \reimp */
bool QMotifPlusStyle::eventFilter(QObject *object, QEvent *event)
{
    switch(event->type()) {
    case QEvent::MouseButtonPress:
        {
	    singleton->mousePressed = TRUE;

            if (! object->inherits("QSlider"))
		break;

	    singleton->sliderActive = TRUE;
            break;
        }

    case QEvent::MouseButtonRelease:
        {
	    singleton->mousePressed = FALSE;

            if (! object->inherits("QSlider"))
		break;

	    singleton->mousePressed = FALSE;
	    ((QWidget *) object)->repaint(FALSE);
	    break;
        }

    case QEvent::Enter:
        {
            if (! object->isWidgetType())
		break;

	    singleton->hoverWidget = (QWidget *) object;
	    if (! singleton->hoverWidget->isEnabled()) {
		singleton->hoverWidget = 0;
		break;
	    }

	    if (object->inherits("QScrollBar") ||
		object->inherits("QSlider")) {
		singleton->hoverWidget->repaint(FALSE);
	    } else if (object->inherits("QPushButton")) {
		QPalette pal = singleton->hoverWidget->palette();

		if (singleton->hoverWidget->ownPalette())
		    singleton->hoverPalette = new QPalette(pal);

		pal.setColor(QPalette::Active, QColorGroup::Button,
			     singleton->prelight_palette.color(QPalette::Active,
							       QColorGroup::Button));
		pal.setColor(QPalette::Inactive, QColorGroup::Button,
			     singleton->prelight_palette.color(QPalette::Inactive,
							       QColorGroup::Button));
		singleton->hoverWidget->setPalette(pal);
	    } else
		singleton->hoverWidget->setPalette(singleton->prelight_palette);

	    break;
	}

    case QEvent::Leave:
	{
	    if (object != singleton->hoverWidget)
		break;

	    if (singleton->hoverPalette) {
		singleton->hoverWidget->setPalette(*(singleton->hoverPalette));
		delete singleton->hoverPalette;
		singleton->hoverPalette = 0;
	    } else {
		singleton->hoverWidget->unsetPalette();
	    }

	    singleton->hoverWidget = 0;

	    break;
	}

    case QEvent::MouseMove:
	{
	    if (! object->isWidgetType() ||
		object != singleton->hoverWidget)
		break;

	    if (! object->inherits("QScrollBar") && ! object->inherits("QSlider"))
		break;

	    singleton->mousePos = ((QMouseEvent *) event)->pos();
	    if (! singleton->mousePressed) {
		singleton->hovering = TRUE;
		singleton->hoverWidget->repaint(FALSE);
		singleton->hovering = FALSE;
	    }

	    break;
	}

    default:
	break;
    }

    return QMotifStyle::eventFilter(object, event);
}



// /*!
//   \reimp
// */
// void QMotifPlusStyle::drawPushButton(QPushButton *button, QPainter *p)
// {
//     int x1, y1, x2, y2;
//     button->rect().coords(&x1, &y1, &x2, &y2);

//     if (button->isDefault())
//         drawButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
//                    qApp->palette().active(), TRUE);

//     if (button->isDefault() || button->autoDefault()) {
//         x1 += buttonDefaultIndicatorWidth();
//         y1 += buttonDefaultIndicatorWidth();
//         x2 -= buttonDefaultIndicatorWidth();
//         y2 -= buttonDefaultIndicatorWidth();
//     }

//     QBrush fill;
//     if (button->isDown() || button->isOn())
//         fill = button->colorGroup().brush(QColorGroup::Mid);
//     else
//         fill = button->colorGroup().brush(QColorGroup::Button);

//     if ( !button->isFlat() || button->isOn() || button->isDown() )
//         drawButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
//                    button->colorGroup(), button->isOn() || button->isDown(), &fill);
// }

// /*!
//   \reimp
// */
// void QMotifPlusStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
//                                       const QColorGroup &g, bool sunken,
//                                       bool editable, bool,
//                                       const QBrush *fill)
// {
//     drawButton(p, x, y, w, h, g, sunken, fill);

//     if (editable) {
//         QRect r = comboButtonRect(x, y, w, h);
//         drawButton(p, r.x() - defaultFrameWidth(),
//                    r.y() - defaultFrameWidth(),
//                    r.width() + (defaultFrameWidth() * 2),
//                    r.height() + (defaultFrameWidth() * 2),
//                    g, TRUE);
//     }

//     int indent = ((y + h) / 2) - 6;
//     int xpos = x;
//     if( QApplication::reverseLayout() )
//         xpos += indent;
//     else
//         xpos += w - indent - 13;
//     drawArrow(p, Qt::DownArrow, TRUE, xpos, indent,
//         13, 13, g, TRUE, fill);
// }


// /*!
//   \reimp
// */
// QRect QMotifPlusStyle::comboButtonRect( int x, int y, int w, int h ) const
// {
//     QRect r(x + (defaultFrameWidth() * 2), y + (defaultFrameWidth() * 2),
//             w - (defaultFrameWidth() * 4), h - (defaultFrameWidth() * 4));

//     int indent = ((y + h) / 2) - defaultFrameWidth();
//     r.setRight(r.right() - indent - 13);
//     if( QApplication::reverseLayout() )
//         r.moveBy( indent + 13, 0 );
//     return r;
// }


// /*!
//   \reimp
// */
// QRect QMotifPlusStyle::comboButtonFocusRect(int x, int y, int w, int h ) const
// {
//     return comboButtonRect(x, y, w, h);
// }

// #define HORIZONTAL      (sb->orientation() == QScrollBar::Horizontal)
// #define VERTICAL        !HORIZONTAL
// #define MOTIF_BORDER    defaultFrameWidth()
// #define SLIDER_MIN      buttonDim

// /*!
//   \reimp
// */
// void QMotifPlusStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb,
//                                              int sliderStart, uint controls,
//                                              uint activeControl )
// {
// #define ADD_LINE_ACTIVE ( activeControl == AddLine )
// #define SUB_LINE_ACTIVE ( activeControl == SubLine )
//     QColorGroup g  = sb->colorGroup();
//     QColorGroup pg = singleton->prelight_palette.active();

//     int sliderMin, sliderMax, sliderLength, buttonDim;
//     scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

//     if (sliderStart > sliderMax) { // sanity check
//         sliderStart = sliderMax;
//     }

//     int b = MOTIF_BORDER;
//     int dimB = buttonDim;
//     QRect addB;
//     QRect subB;
//     QRect addPageR;
//     QRect subPageR;
//     QRect sliderR;
//     int addX, addY, subX, subY;
//     int length = HORIZONTAL ? sb->width()  : sb->height();
//     int extent = HORIZONTAL ? sb->height() : sb->width();

//     if ( HORIZONTAL ) {
//         subY = addY = ( extent - dimB ) / 2;
//         subX = b;
//         addX = length - dimB - b;
//     } else {
//         subX = addX = ( extent - dimB ) / 2;
//         subY = b;
//         addY = length - dimB - b;
//     }

//     subB.setRect( subX,subY,dimB,dimB );
//     addB.setRect( addX,addY,dimB,dimB );

//     int sliderEnd = sliderStart + sliderLength;
//     int sliderW = extent - b*2;
//     if ( HORIZONTAL ) {
//         subPageR.setRect( subB.right() + 1, b,
//                           sliderStart - subB.right() - 1 , sliderW );
//         addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
//         sliderR .setRect( sliderStart, b, sliderLength, sliderW );
//     } else {
//         subPageR.setRect( b, subB.bottom() + 1, sliderW,
//                           sliderStart - subB.bottom() - 1 );
//         addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
//         sliderR .setRect( b, sliderStart, sliderW, sliderLength );
//     }

//     bool skipUpdate = FALSE;
//     if (singleton->hovering) {
//         if (addB.contains(singleton->mousePos)) {
//             skipUpdate = (singleton->scrollbarElement == AddLine);
//             singleton->scrollbarElement = AddLine;
//         } else if (subB.contains(singleton->mousePos)) {
//             skipUpdate = (singleton->scrollbarElement == SubLine);
//             singleton->scrollbarElement = SubLine;
//         } else if (sliderR.contains(singleton->mousePos)) {
//             skipUpdate = (singleton->scrollbarElement == Slider);
//             singleton->scrollbarElement = Slider;
//         } else
//             singleton->scrollbarElement = 0;
//     } else
//         singleton->scrollbarElement = 0;

//     if (skipUpdate) return;

//     if ( controls == ( AddLine | SubLine | AddPage | SubPage |
//                        Slider | First | Last ) )
//         drawButton(p, sb->rect().x(), sb->rect().y(),
//                    sb->rect().width(), sb->rect().height(), g, TRUE,
//                    &g.brush(QColorGroup::Mid));

//     if ( controls & AddLine )
//         drawArrow( p, VERTICAL ? DownArrow : RightArrow,
//                    ADD_LINE_ACTIVE, addB.x(), addB.y(),
//                    addB.width(), addB.height(),
//                    (ADD_LINE_ACTIVE ||
//                     singleton->scrollbarElement == AddLine) ? pg : g, TRUE );
//     if ( controls & SubLine )
//         drawArrow( p, VERTICAL ? UpArrow : LeftArrow,
//                    SUB_LINE_ACTIVE, subB.x(), subB.y(),
//                    subB.width(), subB.height(),
//                    (SUB_LINE_ACTIVE ||
//                     singleton->scrollbarElement == SubLine) ? pg : g, TRUE );

//     QBrush fill = g.brush( QColorGroup::Mid );
//     if (sb->backgroundPixmap() ){
//         fill = QBrush( g.mid(), *sb->backgroundPixmap() );
//     }

//     if ( controls & SubPage )
//         p->fillRect( subPageR, fill );

//     if ( controls & AddPage )
//         p->fillRect( addPageR, fill );

//     if ( controls & Slider ) {
//         QPoint bo = p->brushOrigin();
//         p->setBrushOrigin(sliderR.topLeft());
//         if ( sliderR.isValid() ) {
//             drawBevelButton( p, sliderR.x(), sliderR.y(),
//                              sliderR.width(), sliderR.height(),
//                              (activeControl & Slider ||
//                               singleton->scrollbarElement == Slider) ? pg : g,
//                              FALSE,
//                              (activeControl & Slider ||
//                               singleton->scrollbarElement == Slider) ?
//                              &pg.brush( QColorGroup::Button ) :
//                              &g.brush( QColorGroup::Button ) );
//         }

//         p->setBrushOrigin(bo);
//     }

// }


// /*!
//   \reimp
// */
// void QMotifPlusStyle::drawTab(QPainter *p, const QTabBar *tabbar, QTab *tab,
//                               bool selected)
// {
//     QColorGroup g = tabbar->colorGroup();
//     QPen oldpen = p->pen();
//     QRect fr(tab->rect());

//     if (! selected) {
//         if (tabbar->shape() == QTabBar::RoundedAbove ||
//             tabbar->shape() == QTabBar::TriangularAbove) {
//             fr.setTop(fr.top() + 2);
//         } else {
//             fr.setBottom(fr.bottom() - 2);
//         }
//     }

//     fr.setWidth(fr.width() - 3);

//     p->fillRect(fr.left() + 1, fr.top() + 1, fr.width() - 2, fr.height() - 2,
//                 (selected) ? g.brush(QColorGroup::Button)
//                            : g.brush(QColorGroup::Mid));

//     if (tabbar->shape() == QTabBar::RoundedAbove) {
//         // "rounded" tabs on top
//         fr.setBottom(fr.bottom() - 1);

//         p->setPen(g.light());
//         p->drawLine(fr.left(), fr.top() + 1, fr.left(), fr.bottom() - 1);
//         p->drawLine(fr.left() + 1, fr.top(), fr.right() - 1, fr.top());
//         if (! selected) p->drawLine(fr.left(), fr.bottom(),
//                                     fr.right() + 3, fr.bottom());

//         if (fr.left() == 0)
//             p->drawLine(fr.left(), fr.bottom(), fr.left(), fr.bottom() + 1);

//         p->setPen(g.dark());
//         p->drawLine(fr.right() - 1, fr.top() + 2,
//                     fr.right() - 1, fr.bottom() - 1);

//         p->setPen(black);
//         p->drawLine(fr.right(), fr.top() + 1, fr.right(), fr.bottom() - 1);
//     } else if (tabbar->shape() == QTabBar::RoundedBelow) {
//         // "rounded" tabs on bottom
//         fr.setTop(fr.top() + 1);

//         p->setPen(g.dark());
//         p->drawLine(fr.right() + 3, fr.top() - 1,
//                     fr.right() - 1, fr.top() - 1);
//         p->drawLine(fr.right() - 1, fr.top(),
//                     fr.right() - 1, fr.bottom() - 2);
//         p->drawLine(fr.right() - 1, fr.bottom() - 2,
//                     fr.left() + 2,  fr.bottom() - 2);
//         if (! selected) {
//             p->drawLine(fr.right(), fr.top() - 1,
//                         fr.left() + 1,  fr.top() - 1);

//             if (fr.left() != 0)
//                 p->drawPoint(fr.left(), fr.top() - 1);
//         }

//         p->setPen(black);
//         p->drawLine(fr.right(), fr.top(),
//                     fr.right(), fr.bottom() - 2);
//         p->drawLine(fr.right() - 1, fr.bottom() - 1,
//                     fr.left(), fr.bottom() - 1);
//         if (! selected)
//             p->drawLine(fr.right() + 3, fr.top(), fr.left(), fr.top());
//         else
//             p->drawLine(fr.right() + 3, fr.top(), fr.right(), fr.top());

//         p->setPen(g.light());
//         p->drawLine(fr.left(), fr.top() + 1,
//                     fr.left(), fr.bottom() - 2);

//         if (selected) {
//             p->drawPoint(fr.left(), fr.top());
//             if (fr.left() == 0)
//                 p->drawPoint(fr.left(), fr.top() - 1);

//             p->setPen(g.button());
//             p->drawLine(fr.left() + 2, fr.top() - 1, fr.left() + 1, fr.top() - 1);
//         }
//     } else {
//         // triangular drawing code
//         QCommonStyle::drawTab(p, tabbar, tab, selected);
//     }

//     p->setPen(oldpen);
// }


// /*!
//   \reimp
// */
// void QMotifPlusStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
//                                  const QColorGroup &g, Orientation orientation,
//                                  bool, bool)
// {
//     QRect sliderR(x, y, w, h);
//     QColorGroup cg = g;

//     if ( (singleton->hovering && sliderR.contains(singleton->mousePos)) ||
//          singleton->sliderActive )
//         cg = singleton->prelight_palette.active();

//     if (orientation == Horizontal) {
//         drawButton(p, x, y, w / 2, h, cg, FALSE,
//                    &cg.brush(QColorGroup::Button));
//         drawButton(p, x + (w / 2), y, w / 2, h, cg, FALSE,
//                    &cg.brush(QColorGroup::Button));
//     } else {
//         drawButton(p, x, y, w, h / 2, cg, FALSE,
//                    &cg.brush(QColorGroup::Button));
//         drawButton(p, x, y + (h / 2), w, h / 2, cg, FALSE,
//                    &cg.brush(QColorGroup::Button));
//     }
// }


// /*!
//   \reimp
// */
// void QMotifPlusStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
//                                        const QColorGroup& g, QCOORD,
//                                        Orientation )
// {
//     drawButton(p, x, y, w, h, g, TRUE, &g.brush(QColorGroup::Mid));
// }


#endif // QT_NO_STYLE_MOTIFPLUS
