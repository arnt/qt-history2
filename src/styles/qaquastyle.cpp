/****************************************************************************
**
** Implementation of Aqua-like style class
**
** Created : 001129
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#ifndef QT_NO_STYLE_AQUA

#include "qcheckbox.h"
#include "qaquastyle.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qpushbutton.h"
#include "qtoolbutton.h"
#include "qdockwindow.h"
#include "qtabbar.h"
#include "qscrollbar.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qobjcoll.h"
#include "qlayout.h"
#include "qlist.h"
#include "qbuttongroup.h"
#include "qtabwidget.h"
#include "qslider.h"
#include "qwmatrix.h"
#include "qprogressbar.h"
#include "qlistview.h"
#include <limits.h>
#include "../widgets/qtitlebar_p.h"
#include "qpopupmenu.h"
#ifdef Q_WS_MAC
#  include <string.h>
#  include <qt_mac.h>
#endif

static const int aquaSepHeight         = 10;    // separator height
static const int aquaItemFrame         = 2;    // menu item frame width
static const int aquaItemHMargin       = 3;    // menu item hor text margin
static const int aquaItemVMargin       = 2;    // menu item ver text margin
static const int aquaArrowHMargin      = 6;    // arrow horizontal margin
static const int aquaTabSpacing        = 12;   // space between text and tab
static const int aquaCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int aquaRightBorder       = 12;   // right border on aqua
static const int aquaCheckMarkWidth    = 12;   // checkmarks width on aqua

class QAquaStylePrivate
{
public:
    //blinking buttons
    struct buttonState {
    public:
        buttonState() : frame(0), dir(1) {}
        int frame;
        int dir;
    };
    QPushButton * defaultButton;
    buttonState   buttonState;
    int buttonTimerId;
    //animated progress bars
    QList<QProgressBar> progressBars;
    int progressTimerId;
    int progressOff;
    //big focus rects
    QWidget *focusWidget;
    QFrame::Shape oldFrameShape;
    QFrame::Shadow oldFrameShadow;
    int oldFrameWidth;
};
#include "qaquastyle_p.h"

// NOT REVISED
/*!
  \class QAquaStyle qaquastyle.h
  \brief The QAquaStyle class implements the aqua 'Look and Feel'.
  \ingroup appearance

  This class implements the Aqua look and feel. It's an
  experimental class that tries to resemble a Macinosh-like GUI style
  with the QStyle system. The emulation is far from being
  perfect.

  Note that the functions provided by QAquaStyle are reimplementations
  of QStyle functions; see QStyle for their documentation.
*/


/*!
  Constructs a QAquaStyle object.
*/
QAquaStyle::QAquaStyle()
{
    d = new QAquaStylePrivate;
    d->defaultButton = 0;
    d->focusWidget = 0;
    d->progressTimerId = d->buttonTimerId = -1;
    d->progressOff = 0;
}

/*!\reimp
*/
QAquaStyle::~QAquaStyle()
{
    delete d;
}

/*! \reimp
*/
void QAquaStyle::polish( QPalette & pal )
{
    oldPalette = pal;

    QPixmap px;
    qAquaPixmap( "gen_back", px );
    QBrush background( Qt::black, px );
    pal.setBrush( QPalette::Active, QColorGroup::Background, background );
    pal.setBrush( QPalette::Inactive, QColorGroup::Background, background );
    pal.setBrush( QPalette::Disabled, QColorGroup::Background, background );

    pal.setBrush( QPalette::Active, QColorGroup::Button, background );
    pal.setBrush( QPalette::Inactive, QColorGroup::Button, background );
    pal.setBrush( QPalette::Disabled, QColorGroup::Button, background );

    pal.setColor( QPalette::Inactive, QColorGroup::ButtonText,
                  QColor( 148,148,148 ));
    pal.setColor( QPalette::Disabled, QColorGroup::ButtonText,
                  QColor( 148,148,148 ));

    pal.setColor( QPalette::Active, QColorGroup::Highlight,
                  QColor( 0xC2, 0xC2, 0xC2 ) );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight,
                  QColor( 0xC2, 0xC2, 0xC2 ));
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight,
                  QColor( 0xC2, 0xC2, 0xC2 ));

    pal.setColor( QPalette::Active, QColorGroup::HighlightedText, Qt::black);
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, Qt::black);
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText, Qt::black);
}

/*! \reimp
*/
void QAquaStyle::unPolish( QPalette & pal )
{
    pal = oldPalette;
    qApp->setPalette( pal, TRUE );
}

/*! \reimp
*/
void QAquaStyle::polish( QWidget * w )
{
    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn->isDefault() || btn->autoDefault() ){
            btn->installEventFilter( this );
            if( d->buttonTimerId == -1 ){
                d->buttonTimerId = startTimer( 50 );
            }
        }
    }

    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( FALSE );
         if( btn->group() ){
             btn->group()->setMargin( 0 );
             btn->group()->setInsideSpacing( 0 );
         }
    }

     if( w->inherits("QToolBar") ){
         QToolBar * bar = (QToolBar *) w;
         QBoxLayout * layout = bar->boxLayout();
         layout->setSpacing( 0 );
         layout->setMargin( 0 );
     }

     if( w->inherits("QProgressBar") ){
	 d->progressBars.append((QProgressBar*)w);
	 if( d->progressTimerId == -1 ){
	     d->progressTimerId = startTimer( 50 );
	 }
     }

    if ( !w->isTopLevel() ) {
        if( !w->inherits("QSplitter") && w->backgroundPixmap() &&
            (w->backgroundMode() == QWidget::PaletteBackground) && qApp->palette().isCopyOf(w->palette()))
            w->setBackgroundOrigin( QWidget::WindowOrigin );
    }

    if( w->inherits("QFrame") && w->parentWidget() &&
	!w->inherits("QSpinBox") && !w->topLevelWidget()->inherits("QPopupMenu") && !w->inherits("QMenuBar") )
	w->installEventFilter( this );
}

/*! \reimp
*/
void QAquaStyle::unPolish( QWidget * w )
{
    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn == d->defaultButton )
	    d->defaultButton = 0;

        if( d->buttonTimerId != -1 ){
            killTimer( d->buttonTimerId );
            d->buttonTimerId = -1;
        }
    }

    if( w->inherits("QProgressBar") ) {
	d->progressBars.remove((QProgressBar *) w);
	if(d->progressBars.isEmpty() && d->progressTimerId != -1) {
	    killTimer( d->progressTimerId );
	    d->progressTimerId = -1;
	}
    }
	

    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( TRUE );
    }

    if ( !w->isTopLevel() ) {
        if( !w->inherits("QSplitter") && w->backgroundPixmap() &&
            (w->backgroundMode() == QWidget::PaletteBackground) )
            w->setBackgroundOrigin( QWidget::WidgetOrigin );
    }

    if(w == d->focusWidget) {
	if(w->inherits("QFrame")) {
	    QFrame *frm = (QFrame *)w;
	    frm->setFrameShape(d->oldFrameShape);
	    frm->setFrameShadow(d->oldFrameShadow);
	    frm->setLineWidth(d->oldFrameWidth);
	}
	d->focusWidget = NULL;
    }
}

/*! \reimp
  Called to animate the default buttons (emulates the pulsing effect found
  on the Mac).
*/
void QAquaStyle::timerEvent( QTimerEvent * te )
{
    if( te->timerId() == d->buttonTimerId ){
	if( d->defaultButton != 0 && (d->defaultButton->isDefault() ||
				      d->defaultButton->autoDefault()) )
	    d->defaultButton->repaint( FALSE );
    } else if( te->timerId() == d->progressTimerId ) {
	if( !d->progressBars.isEmpty() ) {
	    d->progressOff++;
	    for( QListIterator<QProgressBar> it(d->progressBars); it.current(); ++it)
		(*it)->repaint( FALSE );
	}
    }
}

/*! \reimp
*/
bool QAquaStyle::eventFilter( QObject * o, QEvent * e )
{
    if( o->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) o;

        if( e->type() == QEvent::FocusIn ) {
	    // Kb Focus received - make this the default button
	    d->defaultButton = btn;
	} else if( e->type() == QEvent::FocusOut || e->type() == QEvent::Show ) {

	    // Find the correct button to use as default button
	    QObjectList *list = btn->topLevelWidget()->queryList( "QPushButton" );
	    QObjectListIt it( * list );
	    QPushButton * pb;
	    while ( (pb = (QPushButton*)it.current()) ) {
		++it;
		if( ((e->type() == QEvent::FocusOut) && (pb->isDefault() ||
							 pb->autoDefault() &&
							 (pb != btn)) ) ||
		    ((e->type() == QEvent::Show) && pb->isDefault()) )
		{
		    QPushButton * tmp = d->defaultButton;
		    d->defaultButton = 0;
		    if( tmp != 0 )
			tmp->repaint( FALSE );
		    d->defaultButton = pb;
		    break;
		}
	    }
	    delete list;
        } else if( e->type() == QEvent::Hide ) {
	    if( d->defaultButton == btn )
		d->defaultButton = 0;
	}
    } else if( o->inherits("QFrame") ) {
	QFrame *frm = (QFrame *)o;
	if( (e->type() == QEvent::FocusOut && d->focusWidget == frm) ||
	    (e->type() == QEvent::FocusIn && d->focusWidget) )  {
	    if(d->focusWidget->inherits("QFrame")) {
		//restore it
		QFrame *out = (QFrame *)d->focusWidget;
		out->setFrameShape(d->oldFrameShape);
		out->setFrameShadow(d->oldFrameShadow);
		out->setLineWidth(d->oldFrameWidth);
		out->repaint();
	    }
	    d->focusWidget = NULL;
	}
	if( e->type() == QEvent::FocusIn && !o->inherits("QTable") ) {
	    //save it
	    d->focusWidget = frm;
	    d->oldFrameShape = frm->frameShape();
	    d->oldFrameShadow = frm->frameShadow();
	    d->oldFrameWidth = frm->lineWidth();
	    //set it
	    frm->setFrameShape(QFrame::Panel);
	    frm->setFrameShadow(QFrame::Plain);
	    frm->setLineWidth(2);
	    frm->repaint();
	}
    }
    return FALSE;
}

/*! \reimp */
void QAquaStyle::drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString& text,
			   int len, const QColor* penColor ) const
{
    flags |= NoAccel;
    QWindowsStyle::drawItem( p, r, flags, g, enabled, pixmap, text,
			     len, penColor );
}

/*!
  \reimp
*/
void QAquaStyle::drawPrimitive( PrimitiveOperation op,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   PFlags flags,
				   void **data ) const
{
    switch( op ) {
    case PO_HeaderArrow:
	if(flags & PStyle_Up)
	    drawPrimitive(PO_ArrowUp, p, QRect(r.x(), r.y()+2, r.width(), r.height()-4), cg, 0, data);
	else
	    drawPrimitive(PO_ArrowDown, p, QRect(r.x(), r.y()+2, r.width(), r.height()-4), cg, 0, data);
	break;
    case PO_ArrowUp:
    case PO_ArrowDown:
    case PO_ArrowRight:
    case PO_ArrowLeft: {
	p->save();
	p->setPen( cg.text() );
	QPointArray a;
	if ( op == PO_ArrowDown )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y(), r.x() + (r.width() / 2) , r.bottom());
	else if( op == PO_ArrowRight )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y() + (r.height() / 2), r.x(), r.bottom()); 
	else if( op == PO_ArrowUp ) 
	    a.setPoints( 3, r.x() + (r.width() / 2), r.y(), r.right(), r.bottom(), r.x(), r.bottom()); 
	else
	    a.setPoints( 3, r.x(), r.y() + (r.height() / 2), r.right(), r.y(), r.right(), r.bottom());
	p->setBrush( cg.text() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
	p->restore();
	break; }
    case PO_HeaderSection: {
	QPixmap px;
	QString nstr = QString::number(r.height()), mod;
	if(flags & PStyle_Down )
	    mod = "down_";
	else if(flags & PStyle_Sunken && qAquaActive( cg ))
	    mod = "act_";
	qAquaPixmap( "hdr_" + mod + nstr, px );
	p->drawTiledPixmap( r, px );

	//separator
	p->save();
	p->setPen( gray );
	p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
	p->restore();
	break; }
    case PO_ProgressBarChunk: {
	QPixmap px;
	qAquaPixmap( "progress_" + QString::number(r.height()), px );
	p->drawTiledPixmap( r, px, QPoint((r.x() % px.width()) - d->progressOff, 0) );
	break; }
    case PO_FocusRect:
	break;     // The Mac Aqua style doesn't use focus rectangles
    case PO_DockWindowHandle: {
	p->save();
	p->translate( r.x(), r.y() );

	bool highlight = flags & PStyle_On;
	QColor dark( cg.dark() );
	QColor light( cg.light() );
	unsigned int i;
	if ( flags & PStyle_Vertical ) {
	    int w = r.width();
	    if ( w > 6 ) {
		if ( highlight )
		    p->fillRect( 1, 1, w - 2, 9, cg.highlight() );
		QPointArray a( 2 * ((w-6)/3) );

		int x = 3 + (w%3)/2;
		p->setPen( dark );
		p->drawLine( 1, 8, w-2, 8 );
		for( i=0; 2*i < a.size(); i ++ ) {
		    a.setPoint( 2*i, x+1+3*i, 6 );
		    a.setPoint( 2*i+1, x+2+3*i, 3 );
		}
		p->drawPoints( a );
		p->setPen( light );
		p->drawLine( 1, 9, w-2, 9 );
		for( i=0; 2*i < a.size(); i++ ) {
		    a.setPoint( 2*i, x+3*i, 5 );
		    a.setPoint( 2*i+1, x+1+3*i, 2 );
		}
		p->drawPoints( a );
	    }
	} else {
	    int h = r.height();
	    if ( h > 6 ) {
		if ( highlight )
		    p->fillRect( 1, 1, 8, h - 2, cg.highlight() );
		QPointArray a( 2 * ((h-6)/3) );
		int y = 3 + (h%3)/2;
		p->setPen( dark );
		p->drawLine( 8, 1, 8, h-2 );
		for( i=0; 2*i < a.size(); i ++ ) {
		    a.setPoint( 2*i, 5, y+1+3*i );
		    a.setPoint( 2*i+1, 2, y+2+3*i );
		}
		p->drawPoints( a );
		p->setPen( light );
		p->drawLine( 9, 1, 9, h-2 );
		for( i=0; 2*i < a.size(); i++ ) {
		    a.setPoint( 2*i, 4, y+3*i );
		    a.setPoint( 2*i+1, 1, y+1+3*i );
		}
		p->drawPoints( a );
	    }
	}
	p->restore();
	break; }
    case PO_DockWindowSeparator: {
	QPixmap px;
	if( flags & PStyle_Vertical )
	    qAquaPixmap( "tbar_vsep_" + QString::number(r.width())+ "_" + QString::number(r.height()), px );
	else
	    qAquaPixmap( "tbar_hsep_" + QString::number(r.width())+ "_" + QString::number(r.height()), px );
	p->drawPixmap( r.x(), r.y(), px );
	break; }
    case PO_TabBarBase: {
	QPixmap px;
	if( qAquaActive( cg ) ){
	    if( flags & PStyle_Top )
		qAquaPixmap( "tab_t_top_act", px );
	    else
		qAquaPixmap( "tab_b_top_act", px );
	} else {
	    if( flags & PStyle_Bottom )
		qAquaPixmap( "tab_t_top_dis", px );
	    else
		qAquaPixmap( "tab_b_top_dis", px );
	}

	p->drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), px );
	break; }
    case PO_Indicator: {
	QPixmap px;
	bool down = flags & PStyle_Down;
	bool on  = flags  & PStyle_On;
	bool tri = FALSE; //FIXME?
	if( flags & PStyle_Enabled && qAquaActive( cg ) ){
	    if( down && on )
		qAquaPixmap("chk_psh_t", px);
	    else if( on )
		qAquaPixmap("chk_act_t", px);
	    else if ( down && tri )
		qAquaPixmap("chk_mid_psh_t", px );
	    else if ( tri )
		qAquaPixmap("chk_mid_t", px );
	    else if( !on && down )
		qAquaPixmap("chk_psh_f", px);
	    else
		qAquaPixmap("chk_act_f", px);
	} else {
	    if ( on )
		qAquaPixmap("chk_dis_t", px);
	    else if ( tri )
		qAquaPixmap("chk_mid_dis_t", px);
	    else
		qAquaPixmap("chk_act_f", px);
	}
	p->drawPixmap( r.x(), r.y(), px );
	break; }
    case PO_IndicatorMask: {
	p->fillRect(r.x(), r.y()+2, r.width(), r.height(), color1);
	break; }
    case PO_ExclusiveIndicator: {
	QPixmap px;
	bool down = flags & PStyle_Down;
	bool on  = flags  & PStyle_On;
	bool enabled = flags & PStyle_Enabled;
	if( enabled  && qAquaActive( cg ) ){
	    if( down && on )
		qAquaPixmap("radio_psh_t", px);
	    else if( on )
		qAquaPixmap("radio_t", px);
	    else if( !on && down )
		qAquaPixmap("radio_psh_f", px);
	    else
		qAquaPixmap("radio_f", px);
	} else {
	    on ? qAquaPixmap("radio_dis_t", px) : qAquaPixmap("radio_f", px);
	}
	p->drawPixmap( r.x(), r.y(), px );
	break; }
    case PO_ExclusiveIndicatorMask: {
	QBitmap radio_mask( aqua_radio_mask_xbm_width,
			    aqua_radio_mask_xbm_height,
			    (const uchar *) aqua_radio_mask_xbm_bits, TRUE );
	p->drawPixmap( r.x(), r.y()+3, radio_mask );
	break; }

#ifndef QT_NO_SCROLLBAR
    case PO_ScrollBarAddLine: 
    case PO_ScrollBarSubLine: {
	QPixmap arrow;
	p->setBackgroundMode( OpaqueMode );
	QString mod, prefix;
	if(flags & PStyle_Down )
	    mod += "psh_";
	if( flags & PStyle_Horizontal ) {
	    prefix = "h";
	    if(op == PO_ScrollBarAddLine) 
		mod += "right_";
	    else 
		mod += "left_";
	    mod += QString::number(r.height());
	} else {
	    prefix = "v";
	    if(op == PO_ScrollBarAddLine) 
		mod += "down_";
	    else 
		mod += "up_";
	    mod += QString::number(r.width());
	}
	qAquaPixmap( prefix + "sbr_arw_" + mod, arrow );
	p->drawPixmap( r.x(), r.y(), arrow );
	break; }

    case PO_ScrollBarSubPage: 
    case PO_ScrollBarAddPage: {
	QPixmap fill;
	p->setBackgroundMode( OpaqueMode );
	QString prefix="v";
	if( flags & PStyle_Horizontal )
	    prefix = "h";
	qAquaPixmap( prefix + "sbr_back_fill_" + QString::number(
	    flags & PStyle_Horizontal ? r.height() : r.width()), fill );
	p->drawTiledPixmap( r, fill );
	break; }

    case PO_ScrollBarSlider: {
	QPixmap tip1, tip2, fill;
	QBitmap tip1m, tip2m;
	QRect real_rect;
	QString act;
	if(!qAquaActive( cg ) ) 
	    act = "dis_";
	if( flags & PStyle_Horizontal ) {
	    QString nstr = QString::number(r.height());
	    qAquaPixmap( "hsbr_tip_" + act + "left_" + nstr, tip1 );
	    qAquaPixmap( "hsbr_tip_" + act + "right_" + nstr, tip2 );
	    qAquaPixmap( "hsbr_" + act + "fill_" + nstr, fill );
	    p->drawPixmap( r.x(), r.y(), tip1 );
	    p->drawPixmap( r.x()+r.width()-tip2.width(), r.y(), tip2 );
	    real_rect = QRect(r.x() + tip1.width(), r.y(), r.width() - tip2.width() - tip1.width(), r.height());
	} else {
	    QString nstr = QString::number(r.width());
	    qAquaPixmap( "vsbr_tip_" + act + "up_" + nstr, tip1 );
	    qAquaPixmap( "vsbr_tip_" + act + "down_" + nstr, tip2 );
	    qAquaPixmap( "vsbr_" + act + "fill_" + nstr, fill );
	    p->drawPixmap( r.x(), r.y(), tip1 );
	    p->drawPixmap( r.x(), r.y()+r.height()-tip2.height(), tip2 );
	    real_rect = QRect(r.x(), r.y() + tip1.height(), r.width(), r.height() - tip2.height() - tip1.height());
	}
	QBrush br( Qt::black, fill );
	p->fillRect( real_rect, br );
	break; }
#endif
    default:
	QWindowsStyle::drawPrimitive(op, p, r, cg, flags, data);
	break;
    }
}

/*!
  \reimp
*/
void QAquaStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 CFlags how,
				 void **data ) const
{
    PFlags flags = PStyle_Default;
    if (widget->isEnabled())
	flags |= PStyle_Enabled;

    switch(element) {
    case CE_TabBarTab: {
#ifndef QT_NO_TABBAR
	QPixmap left, mid, right;
	QTabBar * tb = (QTabBar *) widget;
	bool selected = how & CStyle_Selected;
	QString pos;

	if( tb->shape() == QTabBar::RoundedAbove )
	    pos = "t";
	else
	    pos = "b";

	if( qAquaActive( tb->colorGroup() ) ){
	    if( selected ){
		qAquaPixmap( "tab_"+ pos +"_act_left", left );
		qAquaPixmap( "tab_"+ pos +"_act_mid", mid );
		qAquaPixmap( "tab_"+ pos +"_act_right", right );
	    } else {
		qAquaPixmap( "tab_"+ pos +"_dis_left", left );
		qAquaPixmap( "tab_"+ pos +"_dis_mid", mid );
		qAquaPixmap( "tab_"+ pos +"_dis_right", right );
	    }
	} else {
	    if( selected ){
		qAquaPixmap( "tab_"+ pos +"_sel_dis_left", left );
		qAquaPixmap( "tab_"+ pos +"_sel_dis_mid", mid );
		qAquaPixmap( "tab_"+ pos +"_sel_dis_right", right );
	    } else {
		qAquaPixmap( "tab_"+ pos +"_usel_dis_left", left );
		qAquaPixmap( "tab_"+ pos +"_usel_dis_mid", mid );
		qAquaPixmap( "tab_"+ pos +"_usel_dis_right", right );
	    }
	}

	p->drawPixmap( r.x(), r.y(), left );
	p->drawTiledPixmap( r.x() + left.width(), r.y(), r.width()-left.width()*2,
			    r.height(), mid );
	p->drawPixmap( r.x() + r.width() - right.width(), r.y(), right );
#endif
	break; }
    case CE_PopupMenuItem: {
#ifndef QT_NO_POPUPMENU
	QPopupMenu *popupmenu = (QPopupMenu *)widget;
	QMenuItem *mi = (QMenuItem *)data[0];

	const QColorGroup & g = cg;
	QColorGroup itemg = g;
	bool dis = !mi->isEnabled();
	int tab = *((int *)data[1]);
	int maxpmw = *((int *)data[2]);
	bool checkable = popupmenu->isCheckable();
	bool act = how & CStyle_Selected;
	int x, y, w, h;
	r.rect(&x, &y, &w, &h);

	if ( checkable ) 
	    maxpmw = QMAX( maxpmw, 12 ); // space for the checkmarks

	int checkcol = maxpmw;
	QPixmap selectedBackground;
	qAquaPixmap( "sel_back", selectedBackground );

	if ( mi && mi->isSeparator() ) // Aqua separators are just empty menuitems
	    return;

	QBrush fill = act? QBrush( Qt::black, selectedBackground ) :
				g.brush( QColorGroup::Button );
	p->fillRect( x, y, w, h, fill);

	if ( !mi )
	    return;

	bool reverse = QApplication::reverseLayout();

	int xpos = x;
	if ( reverse )
	    xpos += w - checkcol;
	if ( mi->isChecked() ) {
	    // Do nothing
	} else if ( !act ) {
	    p->fillRect(xpos, y, checkcol , h,
			g.brush( QColorGroup::Button ));
	}

	if ( mi->iconSet() ) {              // draw iconset
	    QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
	    if (act && !dis )
		mode = QIconSet::Active;
	    QPixmap pixmap;
	    if ( checkable && mi->isChecked() )
		pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
	    else
		pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
	    int pixw = pixmap.width();
	    int pixh = pixmap.height();
	    if ( act && !dis ) {
		if ( !mi->isChecked() )
		    qDrawShadePanel( p, xpos, y, checkcol, h, g, FALSE, 1,
				     &g.brush( QColorGroup::Button ) );
	    }
	    QRect cr( xpos, y, checkcol, h );
	    QRect pmr( 0, 0, pixw, pixh );
	    pmr.moveCenter( cr.center() );
	    p->setPen( itemg.text() );
	    p->drawPixmap( pmr.topLeft(), pixmap );

	    QBrush fill = act? QBrush( Qt::black, selectedBackground ) :
				  g.brush( QColorGroup::Button );
	    int xp;
	    if ( reverse )
		xp = x;
	    else
		xp = xpos + checkcol + 1;
	    p->fillRect( xp, y, w - checkcol - 1, h, fill);
	} else  if ( checkable ) {  // just "checking"...
	    int mw = checkcol + aquaItemFrame;
	    int mh = h - 2*aquaItemFrame;
	    if ( mi->isChecked() ) {
		int xp = xpos;
		if( reverse )
		    xp -= aquaItemFrame;
		else
		    xp += aquaItemFrame;

		PFlags cflags = PStyle_Default;
		if (! dis)
		    cflags |= PStyle_Enabled;
		if (act)
		    cflags |= PStyle_On;
		drawPrimitive(PO_CheckMark, p, QRect(xp, y+aquaItemFrame, mw, mh), cg, cflags);
	    }
	}

	p->setPen( act ? Qt::white/*g.highlightedText()*/ : g.buttonText() );

	QColor discol;
	if ( dis ) {
	    discol = itemg.text();
	    p->setPen( discol );
	}

	int xm = aquaItemFrame + checkcol + aquaItemHMargin;
	if ( reverse )
	    xpos = aquaItemFrame + tab;
	else
	    xpos += xm;

	if ( mi->custom() ) {
	    int m = aquaItemVMargin;
	    p->save();
	    if ( dis && !act ) {
		p->setPen( g.light() );
		mi->custom()->paint( p, itemg, act, !dis,
				     xpos+1, y+m+1, w-xm-tab+1, h-2*m );
		p->setPen( discol );
	    }
	    mi->custom()->paint( p, itemg, act, !dis,
				 x+xm, y+m, w-xm-tab+1, h-2*m );
	    p->restore();
	}
	QString s = mi->text();
	if ( !s.isNull() ) {                        // draw text
	    int t = s.find( '\t' );
	    int m = aquaItemVMargin;
	    const int text_flags = AlignVCenter|NoAccel | DontClip | SingleLine;
	    if ( t >= 0 ) {                         // draw tab text
		int xp;
		if( reverse )
		    xp = x + aquaRightBorder+aquaItemHMargin+aquaItemFrame - 1;
		else
		    xp = x + w - tab - aquaRightBorder-aquaItemHMargin-aquaItemFrame+1;
		if ( dis && !act ) {
		    p->setPen( g.light() );
		    p->drawText( xp, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
		    p->setPen( discol );
		}
		p->drawText( xp, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
		s = s.left( t );
	    }
	    if ( dis && !act ) {
		p->setPen( g.light() );
		p->drawText( xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t );
		p->setPen( discol );
	    }
	    p->drawText( xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
	} else if ( mi->pixmap() ) {                        // draw pixmap
	    QPixmap *pixmap = mi->pixmap();
	    if ( pixmap->depth() == 1 )
		p->setBackgroundMode( OpaqueMode );
	    p->drawPixmap( xpos, y+aquaItemFrame, *pixmap );
	    if ( pixmap->depth() == 1 )
		p->setBackgroundMode( TransparentMode );
	}
	if ( mi->popup() ) {                        // draw sub menu arrow
	    int dim = (h-2*aquaItemFrame) / 2;
	    PrimitiveOperation arrow;
	    if ( reverse ) {
		arrow = PO_ArrowLeft;
		xpos = x + aquaArrowHMargin + aquaItemFrame;
	    } else {
		arrow = PO_ArrowRight;
		xpos = x+w - aquaArrowHMargin - aquaItemFrame - dim;
	    }
	    if ( act ) {
		if ( !dis )
		    discol = white;
		QColorGroup g2( discol, cg.highlight(),
				white, white,
				dis ? discol : white,
				discol, white );

		drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
			      g2, PStyle_Enabled);
	    } else {
		drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
			      cg, mi->isEnabled() ? PStyle_Enabled : PStyle_Default);
	    }
	}
#endif 
	break; }
    case CE_MenuBarItem: {
	QMenuItem *mi = (QMenuItem *)data[0];
	bool down = flags & PStyle_Down;
	bool active = flags & PStyle_On;
	if( down && active ){
	    QPixmap px;
	    qAquaPixmap( "sel_back", px );
	    p->fillRect( r, QBrush( black, px ) );
	} else {
	    p->fillRect( r, cg.brush( QColorGroup::Button ) );
	}
	drawItem( p, r, AlignCenter|DontClip|SingleLine|ShowPrefix,
		  cg, mi->isEnabled(), mi->pixmap(), mi->text(), -1,
		  (down && active) ? &white : &cg.buttonText() );
	break; }
    case CE_PushButton: {
#ifndef QT_NO_PUSHBUTTON
	QPushButton *btn = (QPushButton *)widget;
	QPixmap left, mid, right;
	QColorGroup g = btn->colorGroup();
	int x=r.x(), y=r.y(), w=r.width(), h=r.height();

	// ### What about buttons that are so small that the pixmaps don't fit?
	if( w < 33 ){
	    drawPrimitive( PO_ButtonCommand, p, r, cg, how, data);
	    return;
	}

	QString hstr = QString::number( h - y );
	if( (btn->isDefault() || btn->autoDefault()) && (d->defaultButton == btn) ) {
	    int & alt = d->buttonState.frame;
	    int & dir = d->buttonState.dir;
	    if( alt > 0 ){
		QString num = QString::number( alt );
		qAquaPixmap( "btn_def_left" + num + "_" + hstr, left );
		qAquaPixmap( "btn_def_mid" + num + "_" + hstr, mid );
		qAquaPixmap( "btn_def_right" + num + "_" + hstr, right );
	    } else {
		qAquaPixmap( "btn_def_left_" + hstr, left );
		qAquaPixmap( "btn_def_mid_" + hstr, mid );
		qAquaPixmap( "btn_def_right_" + hstr, right );
	    }
	    // Pause animation if button is down
	    if( !btn->isDown() ) {
		if( (dir == 1) && (alt == 9) ) dir = -1;
		if( (dir == -1) && (alt == 0) ) dir = 1;
		alt += dir;
	    }
	} else if ( btn->isDown() ) {
	    qAquaPixmap( "btn_def_left_" + hstr, left );
	    qAquaPixmap( "btn_def_mid_" + hstr, mid );
	    qAquaPixmap( "btn_def_right_" + hstr, right );
	} else if ( !btn->isEnabled() ) {
	    qAquaPixmap( "btn_dis_left_" + hstr, left );
	    qAquaPixmap( "btn_dis_mid_" + hstr, mid );
	    qAquaPixmap( "btn_dis_right_" + hstr, right );
	} else if ( btn->isOn() ) {
	    qAquaPixmap( "btn_def_mir_left_" + hstr, left );
	    qAquaPixmap( "btn_def_mir_mid_" + hstr, mid );
	    qAquaPixmap( "btn_def_mir_right_" + hstr, right );
	} else {
	    qAquaPixmap( "btn_nrm_left_" + hstr, left );
	    qAquaPixmap( "btn_nrm_mid_" + hstr, mid );
	    qAquaPixmap( "btn_nrm_right_" + hstr, right );
	}

	QBrush mid_f( Qt::black, mid );
	p->drawPixmap( x, y, left );
	p->drawTiledPixmap( x+left.width(), y, w-x-left.width()*2, h-y, mid );
	p->drawPixmap( w-right.width(), y, right );
#endif
	break; }
    case CE_PushButtonLabel: {
#ifndef QT_NO_PUSHBUTTON
	QPushButton *btn = (QPushButton *)widget;
	bool on = btn->isDown() || btn->isOn();
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );
	if ( btn->isMenuButton() ) {
	    int dx = pixelMetric(PM_MenuButtonIndicator, widget);

	    QColorGroup g( btn->colorGroup() );
	    int xx = x+w-dx-4;
	    int yy = y-3;
	    int hh = h+6;

	    if ( !on ) {
		p->setPen( g.mid() ); // mid
		p->drawLine(xx, yy+2, xx, yy+hh-3);
		p->setPen( g.button() );
		p->drawLine(xx+1, yy+1, xx+1, yy+hh-2);
		p->setPen( g.light() );
		p->drawLine(xx+2, yy+2, xx+2, yy+hh-2);
	    }
	    QRect ar(r.right() - dx, r.y() + 2, dx - 4, r.height() - 4);
	    drawPrimitive(PO_ArrowDown, p, ar, cg, how, data);
	    w -= dx;
	}

	if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
	    QIconSet::Mode mode = btn->isEnabled()
				  ? QIconSet::Normal : QIconSet::Disabled;
	    if ( mode == QIconSet::Normal && btn->hasFocus() )
		mode = QIconSet::Active;
	    QIconSet::State state = QIconSet::Off;
	    if ( btn->isToggleButton() && btn->isOn() )
		state = QIconSet::On;
	    QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode, state );
	    int pixw = pixmap.width();
	    int pixh = pixmap.height();
	    p->drawPixmap( x+2, y+h/2-pixh/2, pixmap );
	    x += pixw + 4;
	    w -= pixw + 4;
	}

	drawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix, cg, btn->isEnabled(),
		  btn->pixmap(), btn->text(), -1);
#endif
	break; }
    default:
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, data);
	break;
    }
}

/*!
  \reimp
*/
int QAquaStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret = 0;
    switch(metric) {
    case PM_SpinBoxFrameWidth:
    case PM_ProgressBarChunkWidth:
	ret = 1;
	break;
    case PM_DockWindowSeparatorExtent:
    case PM_TabBarBaseHeight:
	ret = 8;
	break;
    case PM_TabBarBaseOverlap:
	ret = 2;
	break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;
    case PM_MaximumDragDistance:
	ret = -1;
	break;
    case PM_SliderLength:
	ret = 17;
	break;
    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;
    case PM_IndicatorHeight:
	ret = 18;
	break;
    case PM_IndicatorWidth: 
	ret = 15;
	break;
    case PM_ExclusiveIndicatorWidth:
	ret = 14;
	break;
    case PM_ExclusiveIndicatorHeight:
	ret = 17;
	break;
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
}

/*!
  \reimp
*/
QSize QAquaStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       void **data ) const
{
    QSize sz(contentsSize);
    switch(contents) {
    case CT_PopupMenuItem: {
#ifndef QT_NO_POPUPMENU
	QPopupMenu *popup = (QPopupMenu *) widget;
	bool checkable = popup->isCheckable();
	QMenuItem *mi = (QMenuItem *) data[0];
	int maxpmw = *((int *) data[1]);
	int w = sz.width(), h = sz.height();

	//width
	if ( mi->pixmap() )
	    w += mi->pixmap()->width();     // pixmap only

	if ( !mi->text().isNull() ) {
	    if ( mi->text().find('\t') >= 0 )       // string contains tab
		w += aquaTabSpacing;
	}

	if ( maxpmw ) { // we have iconsets
	    w += maxpmw;
	    w += 6; // add a little extra border around the iconset
	}

	if ( checkable && maxpmw < aquaCheckMarkWidth ) {
	    w += aquaCheckMarkWidth - maxpmw; // space for the checkmarks
	}

	if ( maxpmw > 0 || checkable ) // we have a check-column ( iconsets or checkmarks)
	    w += aquaCheckMarkHMargin; // add space to separate the columns

	w += aquaRightBorder;
	//height
	if( mi->isSeparator() )
	    h = aquaSepHeight;
	else if ( mi->pixmap() )         // pixmap height
	    h = mi->pixmap()->height() + 2*aquaItemFrame;
	else                                        // text height
	    h = popup->fontMetrics().height() + 2*aquaItemVMargin + 2*aquaItemFrame;

	if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
	    h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small,
						QIconSet::Normal ).height() + 2*aquaItemFrame );
	}
	if ( mi->custom() )
	    h = QMAX( h, mi->custom()->sizeHint().height() + 2*aquaItemVMargin +
		      2*aquaItemFrame );
	sz = QSize(w, h);
#endif
	break; }
    case CT_PushButton: 
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, data);
	sz.setWidth(sz.width() + 16);
	break; 
    default:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, data);
	break;
    }
    return sz;
}

QRect 
QAquaStyle::subRect( SubRect r, const QWidget *w ) const
{
    QRect ret;
    switch(r) {
    case SR_PushButtonContents: 
	ret = QWindowsStyle::subRect(r, w);
	ret.setTop( ret.top()+1);
	ret.setLeft( ret.left()+8);
	ret.setBottom( ret.bottom()-1);
	ret.setRight( ret.right()-8);
	break;
    case SR_ComboBoxFocusRect: {
	QRect wrect = w->rect();
	ret = QRect(wrect.x()+4, wrect.y()+4, wrect.width()-8-20, wrect.height()-8);
	break; }
    default:
	ret = QWindowsStyle::subRect(r, w);
	break;
    }
    return ret;
}

/*!
  \reimp
*/
void QAquaStyle::drawComplexControl( ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					CFlags flags,
					SCFlags sub,
					SCFlags subActive,
					void **data ) const
{
    switch(ctrl) {
    case CC_ScrollBar: {
	sub = 0xFFFFFFF; //bleh, must paint all?
	QScrollBar *scrollbar = (QScrollBar *) widget;
	QRect addline, subline, addpage, subpage, slider, first, last;
	bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	subline = querySubControlMetrics(ctrl, widget, SC_ScrollBarSubLine, data);
	addline = querySubControlMetrics(ctrl, widget, SC_ScrollBarAddLine, data);
	subpage = querySubControlMetrics(ctrl, widget, SC_ScrollBarSubPage, data);
	addpage = querySubControlMetrics(ctrl, widget, SC_ScrollBarAddPage, data);
	slider  = querySubControlMetrics(ctrl, widget, SC_ScrollBarSlider,  data);
	first   = querySubControlMetrics(ctrl, widget, SC_ScrollBarFirst,   data);
	last    = querySubControlMetrics(ctrl, widget, SC_ScrollBarLast,    data);

	if ((sub & SC_ScrollBarSubPage) && subpage.isValid())
	    drawPrimitive(PO_ScrollBarSubPage, p, subpage, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarSubPage) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));
	if ((sub & SC_ScrollBarAddPage) && addpage.isValid())
	    drawPrimitive(PO_ScrollBarAddPage, p, addpage, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarAddPage) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));
	if ((sub & SC_ScrollBarFirst) && first.isValid())
	    drawPrimitive(PO_ScrollBarFirst, p, first, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarFirst) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));
	if ((sub & SC_ScrollBarLast) && last.isValid())
	    drawPrimitive(PO_ScrollBarLast, p, last, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarLast) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));
	if ((sub & SC_ScrollBarSubLine) && subline.isValid())
	    drawPrimitive(PO_ScrollBarSubLine, p, subline, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarSubLine) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));
	if ((sub & SC_ScrollBarAddLine) && addline.isValid())
	    drawPrimitive(PO_ScrollBarAddLine, p, addline, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarAddLine) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));
	if ((sub & SC_ScrollBarSlider) && slider.isValid()) {
	    //cleanup
	    QRect eraserect(slider);
	    if(scrollbar->orientation() == Qt::Vertical) {
		if(eraserect.y() < subline.height())
		    eraserect.setY(subline.height());
		if(eraserect.bottom() > addline.y())
		    eraserect.setBottom(addline.y());
	    } else {
		if(eraserect.x() < subline.width())
		    eraserect.setX(subline.width());
		if(eraserect.right() > addline.x())
		    eraserect.setRight(addline.x());
	    }
	    if(eraserect.isValid())
		drawPrimitive(PO_ScrollBarAddPage, p, eraserect, cg,
			      ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			      ((subActive == SC_ScrollBarAddPage) ?
			       PStyle_Down : PStyle_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       PStyle_Horizontal : PStyle_Vertical));
	    //now draw
	    drawPrimitive(PO_ScrollBarSlider, p, slider, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarSlider) ?
			   PStyle_Down : PStyle_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   PStyle_Horizontal : PStyle_Vertical));

	    // ### perhaps this should not be able to accept focus if maxedOut?
	    if (scrollbar->hasFocus()) {
		QRect fr(slider.x() + 2, slider.y() + 2,
			 slider.width() - 5, slider.height() - 5);
		drawPrimitive(PO_FocusRect, p, fr, cg, PStyle_Default);
	    }
	}
	break; }
    case CC_TitleBar: {
	if(sub) {
	    QTitleBar *tb = (QTitleBar *)widget;
	    QPixmap left;
	    if(tb->window)
		qAquaPixmap( "win_act_left_controls", left );
	    else
		qAquaPixmap( "win_act_left", left );
	    p->drawPixmap(0, 0, left);

	    if(sub & SC_TitleBarLabel) {
		QPixmap mid, right;
		qAquaPixmap( "win_act_mid", mid );
		qAquaPixmap( "win_act_right", right );

		p->drawTiledPixmap( left.width(), 0, tb->width() - left.width() - right.width(),
				    mid.height(), mid );
		p->drawPixmap(tb->width() - right.width(), 0, right);
		p->setPen( tb->act || !tb->window ? tb->atextc : tb->itextc );
		p->drawText(left.width(), 0, tb->width() - left.width(), tb->height(),
			    AlignAuto | AlignVCenter | SingleLine | AlignHCenter, tb->cuttext );
	    }
	}
	break; }
    case CC_ListView: {
	QListViewItem *item = (QListViewItem *)data[0];
	int y=r.y(), h=r.height(), bx = r.width() / 2;
	for(QListViewItem *child = item->firstChild(); child && y < h; 
	    y += child->totalHeight(), child = child->nextSibling()) {
	    if(y + child->height() > 0) {
		if ( child->isExpandable() || child->childCount() ) {
		    int linebot = child->height()/2;
		    drawPrimitive( child->isOpen() ? PO_ArrowDown : PO_ArrowRight,
				   p, QRect(bx-3, y+linebot-3, bx+3,linebot+3), cg );
		}
	    }
	}
	break; }
    case CC_SpinWidget: {
	QString wstr = QString::number( r.width() );
	QString hstr = QString::number( r.height() );
	QPixmap btn;
	if(sub & SC_SpinWidgetUp) {
	    if ( subActive & SC_SpinWidgetUp )
		qAquaPixmap( "spinbtn_up_on_" + wstr + "_" + hstr, btn );
	    else 
		qAquaPixmap( "spinbtn_up_off_" + wstr + "_" + hstr, btn );
	    p->drawPixmap( r.x(), r.y(), btn );
	} else if(sub & SC_SpinWidgetDown) {
	    if ( subActive & SC_SpinWidgetDown )
		qAquaPixmap( "spinbtn_down_on_" + wstr + "_" + hstr, btn );
	    else 
		qAquaPixmap( "spinbtn_down_off_" + wstr + "_" + hstr, btn );
	    p->drawPixmap( r.x(), r.y(), btn );
	} else if(SC_SpinWidgetFrame) {
	    QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, data);
	}
	break; }
    case CC_Slider: {
	QSlider *sldr = (QSlider *)widget;

	if ( sub == SC_None )
	    sub = SC_SliderGroove | SC_SliderTickmarks | SC_SliderHandle;

	if ( sub & SC_SliderGroove ) {
	    int offset = 3;
	    QPixmap sldr_l, sldr_mid, sldr_r;
	    QString dir;
	    QSlider * s = (QSlider *) widget;
	    if ( s->orientation() == Horizontal ){
		if ( s->tickmarks() == QSlider::Above ) {
		    dir = "up";
		    offset = 10;
		} else
		    dir = "down";
	    } else if ( s->orientation() == Horizontal ){
		if ( s->tickmarks() == QSlider::Above ) {
		    dir = "left";
		    offset = 10;
		} else
		    dir = "right";
	    }
	    qAquaPixmap( "sldr_grv_tip_left_" + dir, sldr_l );
	    qAquaPixmap( "sldr_grv_mid_" + dir, sldr_mid );
	    qAquaPixmap( "sldr_grv_tip_right_" + dir, sldr_r );

	    if ( s->orientation() == Horizontal ){
		p->drawPixmap( r.x(), r.y() + offset, sldr_l );
		p->drawTiledPixmap( r.x() + sldr_l.width(), r.y() + offset, r.width() - sldr_l.width()*2,
				    sldr_l.height(), sldr_mid );
		p->drawPixmap( r.x() + r.width() - sldr_r.width(), r.y() + offset, sldr_r );
	    } else {
		p->drawPixmap( r.x() + offset, r.y(), sldr_r );
		p->drawTiledPixmap( r.x() + offset, r.y() + sldr_l.height(), sldr_mid.width(),
				    r.height() - sldr_l.height()*2, sldr_mid );
		p->drawPixmap( r.x() + offset, r.y() + r.height() - sldr_l.height(), sldr_l );
	    }
	}
	if ( sub & SC_SliderTickmarks ) 
	    QWindowsStyle::drawComplexControl( ctrl, p, widget, r, cg, flags,
					       SC_SliderTickmarks, subActive,
					       data );
	if ( sub & SC_SliderHandle ) {
	    QRect re = querySubControlMetrics( CC_Slider, widget, SC_SliderHandle, data );
	    QPixmap px;
	    QString hstr = QString::number( re.height() );
	    QString dir;
	    if ( sldr->orientation() == Horizontal ){
		if ( sldr->tickmarks() == QSlider::Above )
		    dir = "up";
		else
		    dir = "down";
	    } else if ( sldr->orientation() == Vertical ) {
		if ( sldr->tickmarks() == QSlider::Above )
		    dir = "left";
		else
		    dir = "right";
	    }
	    if( qAquaActive( cg ) ) {
		qAquaPixmap( "sldr_act_pty_" + dir + "_" + hstr, px );
	    } else {
		qAquaPixmap( "sldr_dis_pty_" + dir + "_" + hstr, px );
	    }
	    sldr->erase(re.x(), re.y(), px.width(), px.height());
	    p->drawPixmap( re.x(), re.y(), px );
	}
	break; }
    case CC_ComboBox: {
	QPixmap left, mid, right;
	QString hstr = QString::number( r.height() );
	bool active = qAquaActive( cg );
	qAquaPixmap( "cmb_act_left_" + hstr, left );
	qAquaPixmap( "cmb_act_mid_" + hstr, mid );
	if( active )
	    qAquaPixmap( "cmb_act_right_" + hstr, right );
	else
	    qAquaPixmap( "cmb_dis_right_" + hstr, right );

	p->drawPixmap( r.x(), r.y(), left );
	p->drawTiledPixmap( r.x() + left.width(), r.y(), r.width() - left.width()*2, r.height(), mid );
	p->drawPixmap( r.x() + r.width() - right.width(), r.y(), right );
	break; }
    case CC_ToolButton:
    {
	QToolButton *toolbutton = (QToolButton *) widget;

	QRect button, menuarea;
	button   = querySubControlMetrics(ctrl, widget, SC_ToolButton, data);
	menuarea = querySubControlMetrics(ctrl, widget, SC_ToolButtonMenu, data);

	bool on = toolbutton->isOn();
	bool down = toolbutton->isDown();
	bool autoraise = toolbutton->autoRaise();
	bool use3d = FALSE;
	bool drawarrow = FALSE;
	Qt::ArrowType arrowType = Qt::DownArrow;

	if (data) {
	    use3d      = *((bool *) data[0]);
	    drawarrow  = *((bool *) data[1]);
	    arrowType  = *((Qt::ArrowType *) data[2]);
	}

	PFlags bflags = PStyle_Default,
	       mflags = PStyle_Default;

	if (toolbutton->isEnabled()) {
	    bflags |= PStyle_Enabled;
	    mflags |= PStyle_Enabled;
	}

	if (down) {
	    bflags |= PStyle_Down;
	    mflags |= PStyle_Down;
	}
	if (on) {
	    bflags |= PStyle_On;
	    mflags |= PStyle_On;
	}
	if (autoraise) {
	    bflags |= PStyle_AutoRaise;
	    mflags |= PStyle_AutoRaise;

	    if (use3d) {
		bflags |= PStyle_MouseOver;
		mflags |= PStyle_MouseOver;

		if (subActive & SC_ToolButton)
		    bflags |= PStyle_Down;
		if (subActive & SC_ToolButtonMenu)
		    mflags |= PStyle_Down;

		if (! on && ! down) {
		    bflags |= PStyle_Raised;
		    mflags |= PStyle_Raised;
		}
	    }
	} else if (! on && ! down) {
	    bflags |= PStyle_Raised;
	    mflags |= PStyle_Raised;
	}

	if (sub & SC_ToolButton) {
	    if (bflags & (PStyle_Down | PStyle_On | PStyle_Raised)) {
		QPixmap px;
		QString w = QString::number( r.width() );
		QString h = QString::number( r.height() );

		QWidget *btn_prnt = toolbutton->parentWidget();
		if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		    QToolBar * bar  = (QToolBar *) btn_prnt;
		    QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		    QObjectListIt it( *l );
		    if ( it.toFirst() == toolbutton ) {
			if ( on || down )
			    qAquaPixmap( "toolbtn_on_left_" + w + "_" + h, px );
			else
			    qAquaPixmap( "toolbtn_off_left_" + w + "_" + h, px );
		    } else if( it.toLast() == toolbutton && !toolbutton->popup() ){
			if ( on || down )
			    qAquaPixmap( "toolbtn_on_right_" + w + "_" + h, px );
			else
			    qAquaPixmap( "toolbtn_off_right_" + w + "_" + h, px );
		    } else {
			if ( on || down )
			    qAquaPixmap( "toolbtn_on_mid_" + w + "_" + h, px );
			else
			    qAquaPixmap( "toolbtn_off_mid_"+ w + "_" + h, px );
		    }
		    delete l;
		} else {
		    if ( down || on )
			qAquaPixmap( "toolbtn_on_mid_" + w + "_" + h, px );
		    else
			qAquaPixmap( "toolbtn_off_mid_" + w + "_" + h, px );
		}
		p->drawPixmap( r.x(), r.y(), px );
	    } else if ( toolbutton->parentWidget() &&
			toolbutton->parentWidget()->backgroundPixmap() &&
			! toolbutton->parentWidget()->backgroundPixmap()->isNull() ) {
		p->drawTiledPixmap( r, *(toolbutton->parentWidget()->backgroundPixmap()),
				    toolbutton->pos() );
	    }

	    if (bflags & (PStyle_Down | PStyle_On))
		button.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
			      pixelMetric(PM_ButtonShiftVertical, widget));

	    if (drawarrow) {
		PrimitiveOperation op;
		switch (arrowType) {
		case Qt::LeftArrow:  op = PO_ArrowLeft;  break;
		case Qt::RightArrow: op = PO_ArrowRight; break;
		case Qt::UpArrow:    op = PO_ArrowUp;    break;
		default:
		case Qt::DownArrow:  op = PO_ArrowDown;  break;
		}

		drawPrimitive(op, p, button, cg, bflags, data);
	    } else {
		QColor btext = cg.buttonText();

		if (toolbutton->iconSet().isNull() &&
		    ! toolbutton->text().isNull() &&
		    ! toolbutton->usesTextLabel()) {
		    drawItem(p, button, AlignCenter | ShowPrefix, cg,
			     bflags & PStyle_Enabled, 0, toolbutton->text(),
			     toolbutton->text().length(), &btext);
		} else {
		    QPixmap pm;
		    QIconSet::Size size =
			toolbutton->usesBigPixmap() ? QIconSet::Large : QIconSet::Small;
		    QIconSet::State state =
			toolbutton->isOn() ? QIconSet::On : QIconSet::Off;
		    QIconSet::Mode mode;
		    if (! toolbutton->isEnabled())
			mode = QIconSet::Disabled;
		    else if (bflags & (PStyle_Down | PStyle_On | PStyle_Raised))
			mode = QIconSet::Active;
		    else
			mode = QIconSet::Normal;
		    pm = toolbutton->iconSet().pixmap( size, mode, state );

		    if ( toolbutton->usesTextLabel() ) {
			p->setFont( toolbutton->font() );

			QRect pr = button, tr = button;
			int fh = p->fontMetrics().height();
			pr.addCoords(0, 0, 0, -fh);
			tr.addCoords(0, tr.height() - fh, 0, 0);
			drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
			drawItem( p, tr, AlignCenter | ShowPrefix, cg,
				  bflags & PStyle_Enabled, 0, toolbutton->textLabel(),
				  toolbutton->textLabel().length(), &btext);
		    } else
			drawItem( p, button, AlignCenter, cg, TRUE, &pm, QString::null );
		}
	    }
	}

	if (sub & SC_ToolButtonMenu) {
	    QPixmap px;
	    QString w = QString::number( menuarea.width() );
	    QString h = QString::number( menuarea.height() );
	    QWidget *btn_prnt = toolbutton->parentWidget();
	    if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		QToolBar * bar  = (QToolBar *) btn_prnt;
		QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		QObjectListIt it( *l );
		if( it.toLast() == toolbutton ){
		    if ( (flags & PStyle_On) || (flags & PStyle_Down) )
			qAquaPixmap( "toolbtn_on_right_" + w + "_" + h, px );
		    else
			qAquaPixmap( "toolbtn_off_right_" + w + "_" + h, px );
		} else {
		    if ( (flags & PStyle_On) || (flags & PStyle_Down) )
			qAquaPixmap( "toolbtn_on_mid_" + w + "_" + h, px );
		    else
			qAquaPixmap( "toolbtn_off_mid_"+ w + "_" + h, px );
		}
		delete l;
	    } else {
		if ( (flags & PStyle_On) || (flags & PStyle_Down) )
		    qAquaPixmap( "toolbtn_on_mid_" + w + "_" + h, px );
		else
		    qAquaPixmap( "toolbtn_off_mid_" + w + "_" + h, px );
	    }
	    p->drawPixmap( menuarea.x(), menuarea.y(), px );
	    drawPrimitive(PO_ArrowDown, p, QRect(menuarea.x()+2, 
						 menuarea.y()+(menuarea.height()-menuarea.width()-4),
						 menuarea.width() - 4, menuarea.width()-2), 
						 cg, mflags, data);
	}

	if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
	    QRect fr = toolbutton->rect();
	    fr.addCoords(3, 3, -3, -3);
	    drawPrimitive(PO_FocusRect, p, fr, cg);
	}
	break; }
    default:
	QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, data);
    }
}

QRect QAquaStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    void **data ) const
{
    QRect rect;
    switch( control ) {
    case CC_ComboBox: {
	rect = QWindowsStyle::querySubControlMetrics(control, w, sc, data);
	if(sc == SC_ComboBoxEditField)
	    rect.setWidth(rect.width() - 5);
	break; }
    case CC_TitleBar: {
	if(sc & SC_TitleBarCloseButton)
	    rect = QRect(7, 3, 15, 17);
	else if(sc & SC_TitleBarMinButton)
	    rect = QRect(31, 3, 15, 17);
	else if(sc & SC_TitleBarMaxButton)
	    rect = QRect(51, 3, 15, 17);
	else if(sc & SC_TitleBarLabel)
	    rect = QRect(66, 3, w->width(), 17);
	else if(sc & SC_TitleBarSysMenu)
	    rect = QRect(-666, -666, 0, 23); //ugh, how bogus!
	break; }
    case CC_ScrollBar:
	switch(sc) {
	case SC_ScrollBarAddLine: 
	case SC_ScrollBarSubLine: {
	    QScrollBar *scr = (QScrollBar *)w;
	    rect = QRect(0, 0, 16, 26);
	    if(sc == SC_ScrollBarAddLine) {
		int x = 0, y = 0;
		if(scr->orientation() == Horizontal) 
		    x = scr->width() - 26;
		else 
		    y = scr->height() - 26;
		rect.moveTopLeft(QPoint(x, y));
	    } 
	    if(scr->orientation() == Horizontal)
		rect.setSize(QSize(rect.height(), rect.width()));
	    break; }
	default:
	    rect = QWindowsStyle::querySubControlMetrics( control, w, sc, data); 
	}
	break;
    default:
	rect = QWindowsStyle::querySubControlMetrics( control, w, sc, data); 
	break;
    }
    return rect;
}

int QAquaStyle::styleHint(StyleHint sh, const QWidget *w, void ***d) const
{
    int ret = 0;
    switch(sh) {
    case SH_TabBar_Alignment:
	ret = Qt::AlignHCenter;
	break;
    default:
	ret = QWindowsStyle::styleHint(sh, w, d);
	break;
    }
    return ret;
}

#ifdef Q_WS_MAC
void QAquaStyle::appearanceChanged()
{
    AquaMode m;
    {
	Collection c=NewCollection();
	GetTheme(c);
	Str255 str;
	long int s = 256;
	if(!GetCollectionItem(c, kThemeVariantNameTag, 0, &s, &str)) {
	    if(!strncmp((char *)str+1, "Blue", str[0]))
		m = AquaModeAqua;
	    else if(!strncmp((char *)str+1, "Graphite", str[0]))
		m = AquaModeGraphite;
	    else
		m = AquaModeUnknown;
	} else {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	}
	DisposeCollection(c);
    }
#if 0
    if(aquaMode != m && qApp && qApp->style().inherits("QAquaStyle")) 
	qApp->setStyle(new QAquaStyle);
#endif
    aquaMode = m;
}
#endif

#if 0
/*! \reimp */
QRect QAquaStyle::comboButtonRect( int x, int y, int w, int h) const
{
    QRect r(x+3, y+3, w-6-20, h-6);
    if( QApplication::reverseLayout() )
        r.moveBy( 16, 0 );
    return r;
}

/*! \reimp */
void QAquaStyle::drawToolBarPanel( QPainter * p, int x, int y,
                                   int w, int h, const QColorGroup & g,
                                   const QBrush * /*fill*/ )
{
    p->fillRect( x, y, w, h, g.brush( QColorGroup::Button ) );
}

/*! \reimp */
void QAquaStyle::drawMenuBarPanel( QPainter * p, int x, int y,
                                   int w, int h, const QColorGroup & g,
                                   const QBrush * /*fill*/ )
{
    p->fillRect( x, y, w, h, g.brush( QColorGroup::Button ) );
}

#endif

#endif /* QT_NO_STYLE_AQUA */


