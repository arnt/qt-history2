/****************************************************************************
** $Id: $
**
** Definition of Aqua-like style class
**
** Created : 001129
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software. This file and its contents may
** not be distributed onto any other platform or included in any other licensed
** package unless explicit permission is granted.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcheckbox.h"

#ifndef QT_NO_STYLE_AQUA

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
#include "qaquastyle_p.h"
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
static QColor highlightColor = QColor( 0xC2, 0xC2, 0xC2 ); //color of highlighted text
static bool scrollbar_arrows_together = FALSE; //whether scroll arrows go together

class QAquaFocusWidget : public QWidget
{
    Q_OBJECT
public:
    QAquaFocusWidget( );
    ~QAquaFocusWidget() {}
    void setFocusWidget( QWidget * widget );
    QWidget* widget() { return d; }
    QSize sizeHint() { return QSize( 0, 0 ); }
    
public slots:
    void objDestroyed(QObject *);

protected:
    bool eventFilter( QObject * o, QEvent * e );
    void paintEvent( QPaintEvent * );
    
private:
    QWidget *d;
    QPixmap pmt, pmb, pml, pmr, pmtl, pmtr, pmbl, pmbr;
};

QAquaFocusWidget::QAquaFocusWidget( )
    : QWidget( NULL, "magicFocusWidget" ), d( NULL )
{
    qAquaPixmap( "focus_t", pmt );
    qAquaPixmap( "focus_l", pml );
    qAquaPixmap( "focus_b", pmb );
    qAquaPixmap( "focus_r", pmr );
    qAquaPixmap( "focus_tl", pmtl );
    qAquaPixmap( "focus_bl", pmbl );
    qAquaPixmap( "focus_tr", pmtr );
    qAquaPixmap( "focus_br", pmbr );
}

void QAquaFocusWidget::setFocusWidget( QWidget * widget )
{
    if (d == widget)
	return;
    hide();
    if (d)
	d->removeEventFilter( this );
    if (widget && widget->parentWidget()) {
	d = widget;
	reparent( d->parentWidget(), pos() );
	raise();
	d->installEventFilter( this );
	setGeometry( widget->x() - 3, widget->y() - 3, widget->width() + 6, widget->height() + 6 );
	setMask( QRegion( rect() ) - QRegion( 5, 5, width() - 10, height() - 10 ) );  
	show();
    } else {
	d = NULL;
    }
}

void QAquaFocusWidget::objDestroyed(QObject * o)
{
    if (o == d) {
	hide();
	d = NULL;
    }
}

bool QAquaFocusWidget::eventFilter( QObject * o, QEvent * e )
{
    if (o != d)
	return FALSE;
    switch (e->type()) {
    case QEvent::Move: {
	QMoveEvent *me = (QMoveEvent*)e;
	move( me->pos().x() - 2, me->pos().y() - 2 );
	break;
    }
    case QEvent::Resize: {
	QResizeEvent *re = (QResizeEvent*)e;
	resize( re->size().width() + 4, re->size().height() + 4 );
	setMask( QRegion( rect() ) - QRegion( 5, 5, width() - 10, height() - 10 ) );  
	break;
    }
    case QEvent::Reparent: {
	QWidget *w = (QWidget*)o;
	reparent( w->parentWidget(), pos() );
	raise();
	break;
    }
    default:
	break;
    }
    return FALSE;
}

void QAquaFocusWidget::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.drawTiledPixmap( 4, 0, width() - 8, pmt.height(), pmt );
    p.drawTiledPixmap( 4, height() - pmb.height(), width() - 8, pmb.height(), pmb );
    p.drawTiledPixmap( 0, 5, pml.width(), height() - 10, pml );
    p.drawTiledPixmap( width() - pmr.width(), 5, pml.width(), height() - 10, pmr );
    p.drawPixmap( 0, 0, pmtl );
    p.drawPixmap( 0, height() - pmbl.height(), pmbl );
    p.drawPixmap( width() - pmtr.width(), 0, pmtr );
    p.drawPixmap( width() - pmbr.width(), height() - pmbr.height(), pmbr );
}

class QAquaStylePrivate : public QObject
{
    Q_OBJECT
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
    QPtrList<QProgressBar> progressBars;
    int progressTimerId;
    int progressOff;
    //big focus rects
    QAquaFocusWidget *focusWidget;

public slots:
    void objDestroyed(QObject *);
};
#include "qaquastyle.moc"

void QAquaStylePrivate::objDestroyed(QObject *o)
{
    if(o == defaultButton)
	defaultButton = NULL;
}

// NOT REVISED
/*!
  \class QAquaStyle qaquastyle.h
  \brief The QAquaStyle class implements the aqua 'Look and Feel'.
  \ingroup appearance

  This class implements the Aqua look and feel. It's an
  experimental class that tries to resemble a Macintosh-like GUI style
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

/*!\reimp */
QAquaStyle::~QAquaStyle()
{
    delete d;
}

/*! \reimp */
void QAquaStyle::polish( QApplication* app )
{
    QPixmap px;
    qAquaPixmap( "gen_back", px );
    QBrush background( Qt::black, px );
    QPalette pal = app->palette();
    pal.setBrush( QColorGroup::Background, background );
    pal.setBrush( QColorGroup::Button, background );

    pal.setColor( QPalette::Inactive, QColorGroup::ButtonText,
                  QColor( 148,148,148 ));
    pal.setColor( QPalette::Disabled, QColorGroup::ButtonText,
                  QColor( 148,148,148 ));

    pal.setColor( QPalette::Active, QColorGroup::Highlight, highlightColor );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight, QColor( 0xC2, 0xC2, 0xC2 ) );
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight, QColor( 0xC2, 0xC2, 0xC2 ) );

    pal.setColor( QColorGroup::HighlightedText, Qt::black);

    app->setPalette( pal, TRUE );
}

/*! \reimp */
void QAquaStyle::polish( QWidget * w )
{
    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn->isDefault() || btn->autoDefault() ){
	    QObject::connect(btn, SIGNAL(destroyed(QObject*)), d, SLOT(objDestroyed(QObject*)));
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

    if( w->parentWidget() && 
	(w->inherits("QLineEdit") || (w->inherits("QTextEdit") && !w->inherits("QTextView")))) {
	QObject::connect(w, SIGNAL(destroyed(QObject*)), d, SLOT(objDestroyed(QObject*)));
	w->installEventFilter( this );
    }
}

/*! \reimp */
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

    if(d->focusWidget && d->focusWidget->widget() == w)
	d->focusWidget->hide();
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
	    d->progressOff--;
	    for( QPtrListIterator<QProgressBar> it(d->progressBars); it.current(); ++it)
		(*it)->repaint( FALSE );
	}
    }
}

/*! \reimp */
bool QAquaStyle::eventFilter( QObject * o, QEvent * e )
{
    if(d->focusWidget && d->focusWidget->widget() &&
       (o->inherits("QLineEdit") || (o->inherits("QTextEdit") && !o->inherits("QTextView"))) &&
       ((e->type() == QEvent::FocusOut && d->focusWidget->widget() == o) ||
	(e->type() == QEvent::FocusIn && d->focusWidget->widget() != o)))  { //restore it
	d->focusWidget->setFocusWidget( NULL );
    }
    if( o && e->type() == QEvent::FocusIn ) {
	if( (o->inherits("QLineEdit") || (o->inherits("QTextEdit") && !o->inherits("QTextView")))) {
	    if (!d->focusWidget)
		d->focusWidget = new QAquaFocusWidget();
	    d->focusWidget->setFocusWidget( (QWidget*)o );
	} else if( o->inherits("QPushButton") ) { // Kb Focus received - make this the default button
	    d->defaultButton = (QPushButton *) o;
	}
    } else if( e->type() == QEvent::Hide && d->defaultButton == o ) {
	d->defaultButton = NULL;
    } else if( o && (e->type() == QEvent::FocusOut || e->type() == QEvent::Show) &&
	       o->inherits("QPushButton") ) {
	QPushButton *btn = (QPushButton *)o;
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
    }
    return FALSE;
}

/*! \reimp */
void QAquaStyle::drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString& text,
			   int len, const QColor* penColor ) const
{
    //No accelerators drawn here!
    QWindowsStyle::drawItem( p, r, flags | NoAccel, g, enabled, pixmap, text,
			     len, penColor );
}

/*!
  \reimp
*/
void QAquaStyle::drawPrimitive( PrimitiveElement pe,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   SFlags flags,
				   const QStyleOption& opt ) const
{
    switch( pe ) {
    case PE_HeaderArrow:
	if(flags & Style_Up)
	    drawPrimitive(PE_ArrowUp, p, QRect(r.x(), r.y()+2, r.width(), r.height()-4), cg, 0, opt);
	else
	    drawPrimitive(PE_ArrowDown, p, QRect(r.x(), r.y()+2, r.width(), r.height()-4), cg, 0, opt);
	break;

    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
	p->save();
	p->setPen( cg.text() );
	QPointArray a;
	if ( pe == PE_ArrowDown )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y(), r.x() + (r.width() / 2) , r.bottom());
	else if( pe == PE_ArrowRight )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y() + (r.height() / 2), r.x(), r.bottom());
	else if( pe == PE_ArrowUp )
	    a.setPoints( 3, r.x() + (r.width() / 2), r.y(), r.right(), r.bottom(), r.x(), r.bottom());
	else
	    a.setPoints( 3, r.x(), r.y() + (r.height() / 2), r.right(), r.y(), r.right(), r.bottom());
	p->setBrush( cg.text() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
	p->restore();
	break; }

    case PE_HeaderSection: {
	QPixmap px;
	QString nstr = QString::number(r.height()), mod;
	if(flags & Style_Down )
	    mod = "down_";
	else if(flags & Style_Sunken && qAquaActive( cg ))
	    mod = "act_";
	qAquaPixmap( "hdr_" + mod + nstr, px );
	p->drawTiledPixmap( r, px );
	//separator
	p->save();
	p->setPen( gray );
	p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
	p->restore();
	break; }

    case PE_ProgressBarChunk: {
	QPixmap px;
	qAquaPixmap( "progress_" + QString::number(r.height()), px );
	p->drawTiledPixmap( r, px, QPoint((r.x() % px.width()) - d->progressOff, 0) );
	break; }

    case PE_FocusRect:
	break;     // The Mac Aqua style doesn't use focus rectangles

    case PE_DockWindowHandle:
	{
	    p->save();
	    p->translate( r.x(), r.y() );
	
	    bool highlight = flags & Style_On;
	    QColor dark( cg.dark() );
	    QColor light( cg.light() );
	    unsigned int i;
	    if ( flags & Style_Horizontal ) {
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
	    } else {
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
		p->restore();
	    }
	}
	break;

    case PE_DockWindowSeparator: {
	QPixmap px;
	if( flags & Style_Horizontal )
	    qAquaPixmap( "tbar_hsep_" + QString::number(r.width())+ "_" + QString::number(r.height()), px );
	else
	    qAquaPixmap( "tbar_vsep_" + QString::number(r.width())+ "_" + QString::number(r.height()), px );
	
	p->drawPixmap( r.x(), r.y(), px );
	break; }

    case PE_TabBarBase: {
	QPixmap px;
	QString mod = "act";
	if( !qAquaActive( cg ) )
	    mod = "dis";
	if( flags & Style_Top )
	    qAquaPixmap( "tab_t_top_" + mod, px );
	else
	    qAquaPixmap( "tab_b_top_" + mod, px );
	p->drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), px );
	break; }

    case PE_Indicator: {
	QPixmap px;
	bool down = flags & Style_Down;
	bool on  = flags  & Style_On;
	bool tri = FALSE; //FIXME?
	if( flags & Style_Enabled && qAquaActive( cg ) ){
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
	p->drawPixmap( r.x(), r.y(), px, 0, 1 );
	break; }

    case PE_IndicatorMask: {
	p->fillRect(r.x(), r.y()+1, r.width(), r.height()-2, color1);
	break; }

    case PE_ExclusiveIndicator: {
	QPixmap px;
	bool down = flags & Style_Down;
	bool on  = flags  & Style_On;
	bool enabled = flags & Style_Enabled;
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
	    if(on)
		qAquaPixmap("radio_dis_t", px) ;
	    else
		qAquaPixmap("radio_f", px);
	}
	p->drawPixmap( r.x(), r.y(), px, 0, 0 );
	break; }

    case PE_ExclusiveIndicatorMask: {
	QBitmap radio_mask( aqua_radio_mask_xbm_width,
			    aqua_radio_mask_xbm_height,
			    (const uchar *) aqua_radio_mask_xbm_bits, TRUE );
	p->drawPixmap( r.x(), r.y()+1, radio_mask, 0, 1 );
	break; }

#ifndef QT_NO_SCROLLBAR
    case PE_ScrollBarAddLine:
    case PE_ScrollBarSubLine: {
	QPixmap arrow;
	p->setBackgroundMode( OpaqueMode );
	QString mod, prefix, join;
	if(flags & Style_Down )
	    mod += "psh_";
	if( flags & Style_Horizontal ) {
	    prefix = "h";
	    if(pe == PE_ScrollBarAddLine)
		mod += "right_";
	    else
		mod += "left_";
	    mod += QString::number(r.height());
	} else {
	    prefix = "v";
	    if(pe == PE_ScrollBarAddLine)
		mod += "down_";
	    else
		mod += "up_";
	    mod += QString::number(r.width());
	}
	if( scrollbar_arrows_together )
	    join = "joined_";
	qAquaPixmap( prefix + "sbr_" + join + "arw_" + mod, arrow );
	p->drawPixmap( r.x(), r.y(), arrow );
	break; }

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage: {

	QPixmap fill;
	p->setBackgroundMode( OpaqueMode );
	QString prefix="v";
	if( flags & Style_Horizontal )
	    prefix = "h";
	qAquaPixmap( prefix + "sbr_back_fill_" + QString::number(
	    flags & Style_Horizontal ? r.height() : r.width()), fill );
	QRect fillr = r;
	if( scrollbar_arrows_together) {
	    QPixmap cap;
	    qAquaPixmap( prefix + "sbr_joined_back_cap_" + QString::number(
		flags & Style_Horizontal ? r.height() : r.width()), cap );
	    p->drawPixmap( r.x(), r.y(), cap );
	    if(flags & Style_Horizontal)
		fillr.setX( r.x() + cap.width() );
	    else
		fillr.setY( r.y() + cap.height() );
	}
	p->drawTiledPixmap( fillr, fill );
	break; }

    case PE_ScrollBarSlider: {
	QPixmap tip1, tip2, fill;
	QBitmap tip1m, tip2m;
	QRect real_rect;
	QString act;
	if(!qAquaActive( cg ) )
	    act = "dis_";
	if( flags & Style_Horizontal ) {
	    QString nstr = QString::number(r.height());
	    qAquaPixmap( "hsbr_tip_" + act + "left_" + nstr, tip1 );
	    qAquaPixmap( "hsbr_tip_" + act + "right_" + nstr, tip2 );
	    qAquaPixmap( "hsbr_" + act + "fill_" + nstr, fill );
	    p->drawPixmap( r.x(), r.y(), tip1 );
	    p->drawPixmap( r.x()+r.width()-tip2.width(), r.y(), tip2 );
	    real_rect = QRect(r.x() + tip1.width(), r.y(), r.width() - tip2.width() - tip1.width(),
			      r.height());
	} else {
	    QString nstr = QString::number(r.width());
	    qAquaPixmap( "vsbr_tip_" + act + "up_" + nstr, tip1 );
	    qAquaPixmap( "vsbr_tip_" + act + "down_" + nstr, tip2 );
	    qAquaPixmap( "vsbr_" + act + "fill_" + nstr, fill );
	    p->drawPixmap( r.x(), r.y(), tip1 );
	    p->drawPixmap( r.x(), r.y()+r.height()-tip2.height(), tip2 );
	    real_rect = QRect(r.x(), r.y() + tip1.height(), r.width(),
			      r.height() - tip2.height() - tip1.height());
	}
	p->fillRect( real_rect, QBrush( Qt::black, fill ) );
	break; }
#endif
    default:
	QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, opt);
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
				 SFlags how,
				 const QStyleOption& opt ) const
{
    SFlags flags = Style_Default;
    if (widget->isEnabled())
	flags |= Style_Enabled;

    switch(element) {
    case CE_TabBarTab: {
#ifndef QT_NO_TABBAR
	if(!widget)
	    break;

	QPixmap left, mid, right;
	QTabBar * tb = (QTabBar *) widget;
	bool selected = how & Style_Selected;
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
	if(!widget || opt.isDefault())
	    break;
	QPopupMenu *popupmenu = (QPopupMenu *)widget;
	QMenuItem *mi = opt.menuItem();
	if ( !mi )
	    break;

	const QColorGroup & g = cg;
	QColorGroup itemg = g;
	bool dis = !mi->isEnabled();
	int tab = opt.tabWidth();
	int maxpmw = opt.maxIconWidth();
	bool checkable = popupmenu->isCheckable();
	bool act = how & Style_Active;
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

		SFlags cflags = Style_Default;
		if (! dis)
		    cflags |= Style_Enabled;
		if (act)
		    cflags |= Style_On;
		drawPrimitive(PE_CheckMark, p, QRect(xp, y+aquaItemFrame, mw, mh), cg, cflags);
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
	    PrimitiveElement arrow;
	    if ( reverse ) {
		arrow = PE_ArrowLeft;
		xpos = x + aquaArrowHMargin + aquaItemFrame;
	    } else {
		arrow = PE_ArrowRight;
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
			      g2, Style_Enabled);
	    } else {
		drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
			      cg, mi->isEnabled() ? Style_Enabled : Style_Default);
	    }
	}
#endif
	break; }

    case CE_MenuBarItem: {
	if (opt.isDefault())
	    break;

	QMenuItem *mi = opt.menuItem();
	if (!mi)
	    break;
	bool down = how & Style_Down;
	bool active = how & Style_Active;
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
	if(!widget)
	    break;
	QPushButton *btn = (QPushButton *)widget;
	QPixmap left, mid, right;
	QColorGroup g = btn->colorGroup();
	int x=r.x(), y=r.y(), w=r.width(), h=r.height();

	// ### What about buttons that are so small that the pixmaps don't fit?
	if( w < 33 ){
	    drawPrimitive( PE_ButtonCommand, p, r, cg, how, opt);
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
	if(!widget)
	    break;
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
	    drawPrimitive(PE_ArrowDown, p, ar, cg, how, opt);
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
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, opt);
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
    case PM_ScrollBarSliderMin:
	ret = 24;
	break;
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
				       const QStyleOption& opt ) const
{
    QSize sz(contentsSize);
    switch(contents) {
    case CT_PopupMenuItem: {
#ifndef QT_NO_POPUPMENU
	if(!widget || opt.isDefault())
	    break;
	const QPopupMenu *popup = (const QPopupMenu *) widget;
	bool checkable = popup->isCheckable();
	QMenuItem *mi = opt.menuItem();
	int maxpmw = opt.maxIconWidth();
	int w = sz.width(), h = sz.height();

	if (mi->isSeparator()) {
	    w = 10;
	    h = aquaSepHeight;
	} else {
	    if (mi->pixmap())
		h = QMAX(h, mi->pixmap()->height() + 4);
	    else
		h = QMAX(h, popup->fontMetrics().height() + 8);

	    if (mi->iconSet() != 0)
		h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						  QIconSet::Normal).height() + 4);
	}

	if (! mi->text().isNull()) {
	    if (mi->text().find('\t') >= 0)
		w += 12;
	}

	if (maxpmw)
	    w += maxpmw + 6;
	if (checkable && maxpmw < 20)
	    w += 20 - maxpmw;
	if (checkable || maxpmw > 0)
	    w += 2;
	w += 12;
	sz = QSize(w, h);
#endif
	break; }

    case CT_PushButton:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	sz.setWidth(sz.width() + 16);
	break;

    default:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	break;
    }
    return sz;
}

/*!
  \reimp
*/
QRect QAquaStyle::subRect( SubRect r, const QWidget *w ) const
{
    QRect ret, wrect(w->rect());
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

    case SR_RadioButtonContents: {
	QRect ir = subRect(SR_RadioButtonIndicator, w);
	ret.setRect(ir.right() + 5, wrect.y(),
		    wrect.width() - ir.width() - 5, wrect.height());
	break; }

    case SR_CheckBoxContents: {
	QRect ir = subRect(SR_CheckBoxIndicator, w);
	ret.setRect(ir.right() + 5, wrect.y(),
		    wrect.width() - ir.width() - 5, wrect.height());
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
					SFlags flags,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt ) const
{
    switch(ctrl) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	if(!widget)
	    break;
	sub = 0xFFFFFFF; //bleh, must paint all?
	QScrollBar *scrollbar = (QScrollBar *) widget;
	QRect addline, subline, addpage, subpage, slider, first, last;
	bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	subline = querySubControlMetrics(ctrl, widget, SC_ScrollBarSubLine, opt);
	addline = querySubControlMetrics(ctrl, widget, SC_ScrollBarAddLine, opt);
	subpage = querySubControlMetrics(ctrl, widget, SC_ScrollBarSubPage, opt);
	addpage = querySubControlMetrics(ctrl, widget, SC_ScrollBarAddPage, opt);
	slider  = querySubControlMetrics(ctrl, widget, SC_ScrollBarSlider,  opt);
	first   = querySubControlMetrics(ctrl, widget, SC_ScrollBarFirst,   opt);
	last    = querySubControlMetrics(ctrl, widget, SC_ScrollBarLast,    opt);

	if ((sub & SC_ScrollBarSubPage) && subpage.isValid())
	    drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarSubPage) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarAddPage) && addpage.isValid())
	    drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarAddPage) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarFirst) && first.isValid())
	    drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarFirst) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarLast) && last.isValid())
	    drawPrimitive(PE_ScrollBarLast, p, last, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarLast) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarSubLine) && subline.isValid())
	    drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarSubLine) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarAddLine) && addline.isValid())
	    drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarAddLine) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarSlider) && slider.isValid()) {
	    //cleanup
	    QRect eraserect(slider);
	    if(scrollbar->orientation() == Qt::Vertical) {
		if(!scrollbar_arrows_together && eraserect.y() < subline.height())
		    eraserect.setY(subline.height());
		if(eraserect.bottom() > addline.y())
		    eraserect.setBottom(addline.y());
	    } else {
		if(!scrollbar_arrows_together && eraserect.x() < subline.width())
		    eraserect.setX(subline.width());
		if(eraserect.right() > addline.x())
		    eraserect.setRight(addline.x());
	    }
	    if(eraserect.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, eraserect, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((subActive == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    //now draw
	    drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarSlider) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));

	    // ### perhaps this should not be able to accept focus if maxedOut?
	    if (scrollbar->hasFocus()) {
		QRect fr(slider.x() + 2, slider.y() + 2,
			 slider.width() - 5, slider.height() - 5);
		drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
	    }
	}
	break; }
#endif

    case CC_TitleBar: {
	if(!widget)
	    break;
	if(sub) {
	    QTitleBar *tb = (QTitleBar *)widget;
	    QPixmap left;
	    if(tb->window())
		qAquaPixmap( "win_act_left_controls", left );
	    else
		qAquaPixmap( "win_act_left", left );
	    p->drawPixmap(0, 0, left);

	    if(sub & SC_TitleBarLabel) {
		QColorGroup cgroup = tb->isActive() || !tb->window() ? tb->palette().active() : tb->palette().inactive();
		QPixmap mid, right;
		qAquaPixmap( "win_act_mid", mid );
		qAquaPixmap( "win_act_right", right );

		p->drawTiledPixmap( left.width(), 0, tb->width() - left.width() - right.width(),
				    mid.height(), mid );
		p->drawPixmap(tb->width() - right.width(), 0, right);
		p->setPen( cgroup.highlightedText() );
		p->drawText(left.width(), 0, tb->width() - left.width(), tb->height(),
			    AlignAuto | AlignVCenter | SingleLine | AlignHCenter, tb->visibleText() );
	    }
	}
	break; }

    case CC_ListView: {
	if (opt.isDefault())
	    break;
	QListViewItem *item = opt.listViewItem();
	int y=r.y(), h=r.height();
	for(QListViewItem *child = item->firstChild(); child && y < h;
	    y += child->totalHeight(), child = child->nextSibling()) {
	    if(y + child->height() > 0) {
		if ( child->isExpandable() || child->childCount() )
		    drawPrimitive( child->isOpen() ? PE_ArrowDown : PE_ArrowRight, p,
				   QRect(r.right() - 10, (y + child->height()/2) - 4, 9, 9), cg );
	    }
	}
	break; }

    case CC_SpinWidget: {
	QPixmap btn;
	if(sub & SC_SpinWidgetUp) {
	    QRect sr = visualRect( querySubControlMetrics( CC_SpinWidget, widget,
							   SC_SpinWidgetUp ), widget );
	    QString wstr = QString::number( sr.width() );
	    QString hstr = QString::number( sr.height() );
	    if ( subActive & SC_SpinWidgetUp )
		qAquaPixmap( "spinbtn_up_on_" + wstr + "_" + hstr, btn );
	    else
		qAquaPixmap( "spinbtn_up_off_" + wstr + "_" + hstr, btn );
	    p->drawPixmap( sr.x(), sr.y(), btn );
	}
	if(sub & SC_SpinWidgetDown) {
	    QRect sr = visualRect( querySubControlMetrics( CC_SpinWidget, widget,
							   SC_SpinWidgetDown ), widget );
	    QString wstr = QString::number( sr.width() );
	    QString hstr = QString::number( sr.height() );
	    if ( subActive & SC_SpinWidgetDown )
		qAquaPixmap( "spinbtn_down_on_" + wstr + "_" + hstr, btn );
	    else
		qAquaPixmap( "spinbtn_down_off_" + wstr + "_" + hstr, btn );
	    p->drawPixmap( sr.x(), sr.y(), btn );
	}
	if(sub & SC_SpinWidgetFrame)
	    QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, SC_SpinWidgetFrame, subActive, opt);
	break; }

    case CC_Slider: {
	if(!widget)
	    break;
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
					       opt );
	if ( sub & SC_SliderHandle ) {
	    QRect re = querySubControlMetrics( CC_Slider, widget, SC_SliderHandle, opt );
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

    case CC_ComboBox:
	{
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
	    break;
	}

    case CC_ToolButton: {
	if(!widget)
	    break;
	QToolButton *toolbutton = (QToolButton *) widget;

	QRect button, menuarea;
	button   = querySubControlMetrics(ctrl, widget, SC_ToolButton, opt);
	menuarea = querySubControlMetrics(ctrl, widget, SC_ToolButtonMenu, opt);

	bool on = toolbutton->isOn();
	bool down = toolbutton->isDown();

	SFlags bflags = flags,
	       mflags = flags;

	if (subActive & SC_ToolButton)
	    bflags |= Style_Down;
	if (subActive & SC_ToolButtonMenu)
	    mflags |= Style_Down;

	if (sub & SC_ToolButton) {
	    if (bflags & (Style_Down | Style_On | Style_Raised)) {
		QPixmap px;
		QString w = QString::number( r.width() );
		QString h = QString::number( r.height() );

		QWidget *btn_prnt = toolbutton->parentWidget();
		QString mod = (down || on) ? "on_" : "off_";
		if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		    QToolBar * bar  = (QToolBar *) btn_prnt;
		    if( bar->orientation() == Qt::Vertical )
			mod += "v";
		    QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		    QObjectListIt it( *l );
		    if ( it.toFirst() == toolbutton )
			mod += "left";
		    else if( it.toLast() == toolbutton && !toolbutton->popup() )
			mod += "right";
		    else
			mod += "mid";
		    delete l;
		} else {
		    mod += "mid";
		}
		qAquaPixmap( "toolbtn_" + mod + "_" + w + "_" + h, px );
		p->drawPixmap( r.x(), r.y(), px );
	    } else if ( toolbutton->parentWidget() &&
			toolbutton->parentWidget()->backgroundPixmap() &&
			! toolbutton->parentWidget()->backgroundPixmap()->isNull() ) {
		p->drawTiledPixmap( r, *(toolbutton->parentWidget()->backgroundPixmap()),
				    toolbutton->pos() );
	    }
	}

	if (sub & SC_ToolButtonMenu) {
	    QPixmap px;
	    QString w = QString::number( menuarea.width() );
	    QString h = QString::number( menuarea.height() );
	    QWidget *btn_prnt = toolbutton->parentWidget();
	    QString mod = (down || on || (subActive & SC_ToolButtonMenu)) ? "on_" : "off_";
	    if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		QToolBar * bar  = (QToolBar *) btn_prnt;
		if( bar->orientation() == Qt::Vertical )
		    mod += "v";
		QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		QObjectListIt it( *l );
		if ( it.toFirst() == toolbutton ) {
		    if( bar->orientation() == Qt::Horizontal )
			mod += "left";
		} else if( it.toLast() == toolbutton && !toolbutton->popup() ){
		    if( bar->orientation() == Qt::Horizontal )
			mod += "right";
		} else {
		    mod += "mid";
		}
		delete l;
	    } else {
		mod += "mid";
	    }
	    qAquaPixmap( "toolbtn_" + mod + "_" + w + "_" + h, px );
	    p->drawPixmap( menuarea.x(), menuarea.y(), px );
	    drawPrimitive(PE_ArrowDown, p, QRect(menuarea.x()+2,
						 menuarea.y()+(menuarea.height()-menuarea.width()-4),
						 menuarea.width() - 4, menuarea.width()-2),
			  cg, mflags, opt);
	}

	if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
	    QRect fr = toolbutton->rect();
	    fr.addCoords(3, 3, -3, -3);
	    drawPrimitive(PE_FocusRect, p, fr, cg);
	}
	break; }

    default:
	QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, opt);
    }
}

/*!
  \reimp
*/
QRect QAquaStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    const QStyleOption& opt ) const
{
    QRect rect;
    switch( control ) {
    case CC_ComboBox: {
	rect = QWindowsStyle::querySubControlMetrics(control, w, sc, opt);
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

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	if(!w)
	    break;
	QScrollBar *scr = (QScrollBar *)w;
	switch(sc) {
	case SC_ScrollBarAddLine:
	case SC_ScrollBarSubLine: {
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
	    rect = QWindowsStyle::querySubControlMetrics( control, w, sc, opt);
	}
	if( scrollbar_arrows_together ) {
	    switch(sc) {
	    case SC_ScrollBarAddLine:
		if(scr->orientation() == Horizontal)
		    rect.setRect( scr->width() - 17, 0, 17, 15 );
		else
		    rect.setRect( 0, scr->height() - 17, 16, 17 );
		break;
	    case SC_ScrollBarSubLine:
		if(scr->orientation() == Horizontal)
		    rect.setRect( scr->width() - (22 + 17), 0, 22, 15 );
		else
		    rect.setRect( 0, scr->height() - (20 + 17), 16, 20 );
		break;
	    case SC_ScrollBarGroove: 
		if(scr->orientation() == Horizontal)
		    rect.setX( rect.x() + 7 );
		else
		    rect.setY( rect.y() + 5 );
		//fall through
	    case SC_ScrollBarSubPage: 
		if(sc == SC_ScrollBarSubPage) {
		    if(scr->orientation() == Horizontal) 
			rect.setWidth(rect.width() + 20);
		    else
			rect.setHeight(rect.height() + 20);
		}
		//fall through
	    case SC_ScrollBarAddPage: {
		int sbextent = pixelMetric(PM_ScrollBarExtent, w);
		if(scr->orientation() == Horizontal)
		    rect.moveBy( -sbextent, 0 );
		else
		    rect.moveBy( 0, -sbextent );
		break; }
	    default: 
		break;
	    }
	}
	break; }
#endif

    default:
	rect = QWindowsStyle::querySubControlMetrics( control, w, sc, opt);
	break;
    }
    return rect;
}

/*!
  \reimp
*/
int QAquaStyle::styleHint(StyleHint sh, const QWidget *w, QStyleHintReturn *d) const
{
    int ret = 0;
    switch(sh) {
    case SH_Widget_ShareActivation:
	ret = TRUE;
	break;
    case SH_Header_ArrowAlignment:
	ret = Qt::AlignRight;
	break;
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
    bool changed = FALSE;
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
	if(m != aquaMode) {
	    aquaMode = m;
	    changed = TRUE;
	}

	RGBColor color;
	s = sizeof(color);
	if(!GetCollectionItem(c, kThemeHighlightColorTag, 0, &s, &color)) {
	    QColor qc(color.red/256, color.green/256, color.blue/256);
	    if(highlightColor != qc) {
		changed = TRUE;
		highlightColor = qc;
		QPalette pal = qApp->palette();
		pal.setColor( QPalette::Active, QColorGroup::Highlight, highlightColor );
		qApp->setPalette( pal, TRUE );
	    }
	} else {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	}

	ThemeScrollBarArrowStyle arrows;
	GetThemeScrollBarArrowStyle(&arrows);
	bool sat = (arrows == kThemeScrollBarArrowsLowerRight);
	if(sat != scrollbar_arrows_together) {
	    scrollbar_arrows_together = sat;
	    changed = TRUE;
	}

	//cleanup
	DisposeCollection(c);
    }
#if 0
    if(changed && qApp && qApp->style().inherits("QAquaStyle"))
	qApp->setStyle(new QAquaStyle);
#endif
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

