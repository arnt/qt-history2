/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qiconset.cpp#34 $
**
** Implementation of QIconSet class
**
** Created : 980318
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qiconset.h"

#ifndef QT_NO_ICONSET

#include "qimage.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qpainter.h"


struct QIconSetPrivate: public QShared
{
    struct Variant {
	Variant(): pm(0), generated(0) {}
	~Variant()
	{
	    delete pm;
	}
	void clearGenerate()
	{
	    if ( generated ) {
		delete pm;
		pm = 0;
		generated = FALSE;
	    }
	}
	QPixmap * pm;
	bool generated;
    };
    Variant vsmall;
    Variant vlarge;
    Variant smallActive;
    Variant largeActive;
    Variant smallDisabled;
    Variant largeDisabled;

    QPixmap defpm;

    Variant on_vsmall;
    Variant on_vlarge;
    Variant on_smallActive;
    Variant on_largeActive;
    Variant on_smallDisabled;
    Variant on_largeDisabled;
};


// REVISED: warwick
/*! \class QIconSet qiconset.h

  \brief The QIconSet class provides a set of differently styled and sized
  icons.

  \ingroup misc
  \ingroup shared

  Once a QIconSet is fed some pixmaps
  it can generate smaller, larger, active, and disabled pixmaps.
  Such pixmaps are used by
  QToolButton, QHeader, QPopupMenu, etc. to show an icon representing
  a piece of functionality.

  The simplest usage of QIconSet is to create one from a QPixmap and then
  use it, allowing Qt to work out all the required icon sizes. For example:

  \code
  QToolButton *tb = new QToolButton( QIconSet( QPixmap("open.xpm") ), ... );
  \endcode

  Using whichever pixmaps you specify as a base,
  QIconSet provides a set of six icons, each with
    a \link QIconSet::Size Size\endlink and
    a \link QIconSet::Mode Mode\endlink:
  <ul>
  <li> <i>Small Normal</i> - can be calculated only from Large Normal.
  <li> <i>Small Disabled</i> - calculated from Large Disabled or Small Normal.
  <li> <i>Small Active</i> - same as Small Normal unless you set it.
  <li> <i>Large Normal</i> - can be calculated only from Small Normal.
  <li> <i>Large Disabled</i> - calculated from Small Disabled or Large Normal.
  <li> <i>Large Active</i> - same as Large Normal unless you set it.
  </ul>

  An additional set of six icons can be provided for widgets that have an "On" or 
  "Off" state, like checkable menuitems or toggleable toolbuttons. An QIconSet
  cannot only have pixmaps for the "On" state, and those pixmaps have to be
  set explicitely using setPixmap().

  You can set any of the icons using setPixmap(). When you retrieve
  one using pixmap(Size,Mode,State), QIconSet will compute it from the
  closest other icon and cache it for later.

  The \c Disabled appearance is computed using a "shadow" algorithm
  that produces results very similar to those used in Microsoft
  Windows 95.

  The \c Active appearance is identical to the \c Normal appearance
  unless you use setPixmap() to set it to something special.

  When scaling icons, QIconSet uses \link QImage::smoothScale
  smooth scaling\endlink, which can partially blend the color component
  of pixmaps.  If the results look poor, the best solution
  is to supply both large and small sizes of pixmaps.

  QIconSet provides a function, isGenerated(), that indicates whether
  an icon was set by the application programmer or computed by
  QIconSet itself.

  <h3>Making Classes that use QIconSet</h3>

  If you write your own widgets that have an option to set a small pixmap,
  you should consider (instead, or additionally) allowing a QIconSet to be
  set for that pixmap.  The Qt class QToolButton is an example of such
  a widget.

  Provide a method to set a QIconSet, and when you draw the icon, choose
  whichever icon is appropriate for the current state of your widget.
  For example:
  \code
  void YourWidget::drawIcon( QPainter* p, QPoint pos )
  {
      p->drawPixmap( pos, icons->pixmap(isEnabled(), QIconSet::Small) );
  }
  \endcode

  You might also make use of the Active mode, perhaps making your widget
  Active when the mouse in inside the widget (see QWidget::enterEvent),
  while the mouse is pressed pending the release that will activate
  the function, or when it is the currently selected item. If you widget
  can be toggled, the On mode might be used to draw a different icon.

  \sa QPixmap QLabel QToolButton QPopupMenu
	QIconViewItem::setViewMode() QMainWindow::setUsesBigPixmaps()
  <a href="guibooks.html#fowler">GUI Design Handbook: Iconic Label</a>
  <a href="http://cgl.microsoft.com/clipgallerylive/cgl30/eula.asp?nInterface=0">Microsoft
  Icon Gallery.</a>
*/


/*!
  \enum QIconSet::Size

  This enum type describes the size for which a pixmap is intended to be
  provided.
  The currently defined sizes are:

  <ul>
    <li> \c Automatic - the size of the pixmap is determined from its
		    pixel size. This is a useful default.
    <li> \c Small - the pixmap is the smaller of two.
    <li> \c Large - the pixmap is the larger of two.
  </ul>

  If a Small pixmap is not set by QIconSet::setPixmap(), the
  Large pixmap may be automatically scaled to two-thirds of its size to
  generate the Small pixmap.  Conversely, a Small pixmap will be
  automatically scaled up by 50% to create a Large pixmap if needed.

  \sa setPixmap() pixmap() QIconViewItem::setViewMode()
      QMainWindow::setUsesBigPixmaps()
*/

/*!
  \enum QIconSet::Mode

  This enum type describes the mode for which a pixmap is intended to be
  provided.
  The currently defined modes are:

  <ul>
    <li> \c Normal
	- the pixmap to be displayed when the user is
	not interacting with the icon, but the
	functionality represented by the icon is available.
    <li> \c Disabled
	- the pixmap to be displayed when the
	functionality represented by the icon is not available.
    <li> \c Active
	- the pixmap to be displayed when the
	functionality represented by the icon is available and
	the user is interacting with the icon, such as by pointing
	at it or by invoking it.
  </ul>
*/

/*!
  \enum QIconSet::State

  This enum describes the state for which a pixmap is intended to be provided.
  The \e state can be:

  \value Off - the pixmap to be displayed when the widget is in an Off state
  \value On - the pixmap to be displayed when the widget is in an On state

  \sa setPixmap() pixmap()
*/

/*!
  Constructs an icon set of \link QPixmap::isNull() null\endlink pixmaps.
  Use setPixmap(), reset(), or operator=() to set some pixmaps.

  \sa reset()
*/
QIconSet::QIconSet()
{
    d = 0;
    reset( QPixmap(), Automatic );
}


/*!  Constructs an icon set for which the Normal pixmap is
  \a pixmap, which is assumed to be the given \a size.

  The default for \a size is \c Automatic, which means that
  QIconSet will determine whether the pixmap is Small or Large
  from its pixel size.
  Pixmaps less than 23 pixels wide are considered to be Small.

  \sa reset()
*/
QIconSet::QIconSet( const QPixmap& pixmap, Size size )
{
    d = 0;
    reset( pixmap, size );
}


/*!
  Constructs a copy of \a other.  This is very fast.
*/
QIconSet::QIconSet( const QIconSet& other )
{
    d = other.d;
    if ( d )
	d->ref();
}

/*!  Creates an iconset which uses the pixmap \a smallPix for for
  displaying small a small icon, and \a largePix for displaying a large
  one.
*/

QIconSet::QIconSet( const QPixmap &smallPix, const QPixmap &largePix )
{
    d = 0;
    reset( smallPix, Small );
    reset( largePix, Large );
}

/*!
  Destructs the icon set and frees any allocated resources.
*/
QIconSet::~QIconSet()
{
    if ( d && d->deref() )
	delete d;
}

/*!
  Assigns \a other to this icon set and returns a reference to this
  icon set.

  This is very fast.

  \sa detach()
*/
QIconSet &QIconSet::operator=( const QIconSet &other )
{
    if ( other.d ) {
	other.d->ref();				// beware of other = other
	if ( d->deref() )
	    delete d;
	d = other.d;
	return *this;
    } else {
	if ( d && d->deref() )
	    delete d;
	d = 0;
	return *this;
    }
}

/*!
  Returns TRUE if the icon set is empty. Currently, a QIconSet
  is never empty (although it may contain null pixmaps).
*/
bool QIconSet::isNull() const
{
    return ( d == 0 );
}

/*!
  Sets this icon set to provide \a pm for the Normal pixmap,
  assuming it to be of \a size.

  This is equivalent to assigning QIconSet(pm,size) to this icon set.
*/
void QIconSet::reset( const QPixmap & pm, Size size )
{
    detach();
    if ( size == Small ||
	 ( size == Automatic && pm.width() <= 22 ) )
	setPixmap( pm, Small, Normal );
    else
	setPixmap( pm, Large, Normal );
    d->defpm = pm;
}


/*!
  Sets this icon set to provide \a pm for a \a size, \a mode and \a state.
  It may also use \a pm for deriving some other varieties if those
  are not set.

  The \a size can be one of Automatic, Large or Small.  If Automatic is used,
  QIconSet will determine if the pixmap is Small or Large from its pixel size.
  Pixmaps less than 23 pixels wide are considered to be Small.

  \sa reset()
*/
void QIconSet::setPixmap( const QPixmap & pm, Size size, Mode mode, State state )
{
    detach();
    if ( d ) {
	if ( state == Off ) {
	    d->vsmall.clearGenerate();
	    d->vlarge.clearGenerate();
	    d->smallDisabled.clearGenerate();
	    d->largeDisabled.clearGenerate();
	    d->smallActive.clearGenerate();
	    d->largeActive.clearGenerate();
	} else {
	    d->on_vsmall.clearGenerate();
	    d->on_vlarge.clearGenerate();
	    d->on_smallDisabled.clearGenerate();
	    d->on_largeDisabled.clearGenerate();
	    d->on_smallActive.clearGenerate();
	    d->on_largeActive.clearGenerate();
	}
    } else {
	d = new QIconSetPrivate;
    }
    if ( size == Large || (size == Automatic && pm.width() > 22)) {
	switch( mode ) {
	case Active:
	    if ( state == Off ) {
		if ( !d->largeActive.pm )
		    d->largeActive.pm = new QPixmap();
		*d->largeActive.pm = pm;
	    } else {
		if ( !d->on_largeActive.pm )
		    d->on_largeActive.pm = new QPixmap();
		*d->on_largeActive.pm = pm;
	    }
	    break;
	case Disabled:
	    if ( state == Off ) {
		if ( !d->largeDisabled.pm )
		    d->largeDisabled.pm = new QPixmap();
		*d->largeDisabled.pm = pm;
	    } else {
		if ( !d->on_largeDisabled.pm )
		    d->on_largeDisabled.pm = new QPixmap();
		*d->on_largeDisabled.pm = pm;
	    }
	    break;
	case Normal:
	default:
	    if ( state == Off ) {
		if ( !d->vlarge.pm )
		    d->vlarge.pm = new QPixmap();
		*d->vlarge.pm = pm;
	    } else {
		if ( !d->on_vlarge.pm )
		    d->on_vlarge.pm = new QPixmap();
		*d->on_vlarge.pm = pm;
	    }
	    break;
	}
    } else if ( size == Small  || (size == Automatic && pm.width() <= 22)) {
	switch( mode ) {
	case Active:
	    if ( state == Off ) {
		if ( !d->smallActive.pm )
		    d->smallActive.pm = new QPixmap();
		*d->smallActive.pm = pm;
	    } else {
		if ( !d->on_smallActive.pm )
		    d->on_smallActive.pm = new QPixmap();
		*d->on_smallActive.pm = pm;
	    }
	    break;
	case Disabled:
	    if ( state == Off ) {
		if ( !d->smallDisabled.pm )
		    d->smallDisabled.pm = new QPixmap();
		*d->smallDisabled.pm = pm;
	    } else {
		if ( !d->on_smallDisabled.pm )
		    d->on_smallDisabled.pm = new QPixmap();
		*d->on_smallDisabled.pm = pm;
	    }
	    break;
	case Normal:
	default:
	    if ( state == Off ) {
		if ( !d->vsmall.pm )
		    d->vsmall.pm = new QPixmap();
		*d->vsmall.pm = pm;
	    } else {
		if ( !d->on_vsmall.pm )
		    d->on_vsmall.pm = new QPixmap();
		*d->on_vsmall.pm = pm;
	    }
	    break;
	}
#if defined(QT_CHECK_RANGE)
    } else {
	qWarning("QIconSet::setPixmap: invalid size passed");
#endif
    }
}


/*!
  Sets this icon set to load \a fileName as a pixmap and provide it
  for \a size, \a mode and \a state.
  It may also use the pixmap for deriving some other varieties if those
  are not set.

  The \a size can be one of Automatic, Large or Small.  If Automatic is used,
  QIconSet will determine if the pixmap is Small or Large from its pixel size.
  Pixmaps less than 23 pixels wide are considered to be Small.
*/
void QIconSet::setPixmap( const QString &fileName, Size size, Mode mode, State state )
{
    QPixmap p;
    p.load( fileName );
    if ( !p.isNull() )
	setPixmap( p, size, mode, state );
}


/*!
  Returns a pixmap with \a size, \a mode and \a state, generating one if
  needed. Generated pixmaps are cached.
*/
QPixmap QIconSet::pixmap( Size size, Mode mode, State state ) const
{
    if ( !d ) {
	QPixmap r;
	return r;
    }

    QImage i;
    QIconSetPrivate * p = ((QIconSet *)this)->d;
    QPixmap * pm = 0;
    if ( state == Off ) {
	if ( size == Large ) {
	    switch( mode ) {
	    case Normal:
		if ( !p->vlarge.pm ) {
		    Q_ASSERT( p->vsmall.pm );
		    i = p->vsmall.pm->convertToImage();
		    i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		    p->vlarge.pm = new QPixmap;
		    p->vlarge.generated = TRUE;
		    p->vlarge.pm->convertFromImage( i );
		    if ( !p->vlarge.pm->mask() ) {
			i = i.createHeuristicMask();
			QBitmap tmp;
			tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			p->vlarge.pm->setMask( tmp );
		    }
		}
		pm = p->vlarge.pm;
		break;
	    case Active:
		if ( !p->largeActive.pm ) {
		    p->largeActive.pm = new QPixmap( pixmap( Large, Normal ) );
		    p->largeActive.generated = TRUE;
		}
		pm = p->largeActive.pm;
		break;
	    case Disabled:
		if ( !p->largeDisabled.pm ) {
		    QBitmap tmp;
		    if ( p->vlarge.generated && !p->smallDisabled.generated &&
			 p->smallDisabled.pm && !p->smallDisabled.pm->isNull() ) {
			// if there's a hand-drawn disabled small image,
			// but the normal big one is generated, use the
			// hand-drawn one to generate this one.
			i = p->smallDisabled.pm->convertToImage();
			i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
			p->largeDisabled.pm = new QPixmap;
			p->largeDisabled.pm->convertFromImage( i );
			if ( !p->largeDisabled.pm->mask() ) {
			    i = i.createHeuristicMask();
			    tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			}
		    } else {
			if (pixmap( Large, Normal).mask())
			    tmp = *pixmap( Large, Normal).mask();
			else {
			    QPixmap conv = pixmap( Large, Normal );
			    if ( !conv.isNull() ) {
				i = conv.convertToImage();
				i = i.createHeuristicMask();
				tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			    }
			}
			p->largeDisabled.pm
			    = new QPixmap( p->vlarge.pm->width()+1,
					   p->vlarge.pm->height()+1);
			QColorGroup dis( QApplication::palette().disabled() );
			p->largeDisabled.pm->fill( dis.background() );
			QPainter painter( p->largeDisabled.pm );
			painter.setPen( dis.base() );
			painter.drawPixmap( 1, 1, tmp );
			painter.setPen( dis.foreground() );
			painter.drawPixmap( 0, 0, tmp );
		    }
		    if ( !p->largeDisabled.pm->mask() ) {
			if ( !tmp.mask() )
			    tmp.setMask( tmp );
			QBitmap mask( d->largeDisabled.pm->size() );
			mask.fill( Qt::color0 );
			QPainter painter( &mask );
			painter.drawPixmap( 0, 0, tmp );
			painter.drawPixmap( 1, 1, tmp );
			painter.end();
			p->largeDisabled.pm->setMask( mask );
		    }
		    p->largeDisabled.generated = TRUE;
		}
		pm = p->largeDisabled.pm;
		break;
	    }
	} else {
	    switch( mode ) {
	    case Normal:
		if ( !p->vsmall.pm ) {
		    Q_ASSERT( p->vlarge.pm );
		    i = p->vlarge.pm->convertToImage();
		    i = i.smoothScale( i.width() * 2 / 3, i.height() * 2 / 3 );
		    p->vsmall.pm = new QPixmap;
		    p->vsmall.generated = TRUE;
		    p->vsmall.pm->convertFromImage( i );
		    if ( !p->vsmall.pm->mask() ) {
			i = i.createHeuristicMask();
			QBitmap tmp;
			tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			p->vsmall.pm->setMask( tmp );
		    }
		}
		pm = p->vsmall.pm;
		break;
	    case Active:
		if ( !p->smallActive.pm ) {
		    p->smallActive.pm = new QPixmap( pixmap( Small, Normal ) );
		    p->smallActive.generated = TRUE;
		}
		pm = p->smallActive.pm;
		break;
	    case Disabled:
		if ( !p->smallDisabled.pm ) {
		    QBitmap tmp;
		    if ( p->vsmall.generated && !p->largeDisabled.generated &&
			 p->largeDisabled.pm && !p->largeDisabled.pm->isNull() ) {
			// if there's a hand-drawn disabled large image,
			// but the normal small one is generated, use the
			// hand-drawn one to generate this one.
			i = p->largeDisabled.pm->convertToImage();
			i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
			p->smallDisabled.pm = new QPixmap;
			p->smallDisabled.pm->convertFromImage( i );
			if ( !p->smallDisabled.pm->mask() ) {
			    i = i.createHeuristicMask();
			    tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			}
		    } else {
			if ( pixmap( Small, Normal).mask())
			    tmp = *pixmap( Small, Normal).mask();
			else {
			    QPixmap conv = pixmap( Small, Normal );
			    if ( !conv.isNull() ) {
				i = conv.convertToImage();
				i = i.createHeuristicMask();
				tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			    }
			}
			p->smallDisabled.pm
			    = new QPixmap( p->vsmall.pm->width()+1,
					   p->vsmall.pm->height()+1);
			QColorGroup dis( QApplication::palette().disabled() );
			p->smallDisabled.pm->fill( dis.background() );
			QPainter painter( p->smallDisabled.pm );
			painter.setPen( dis.base() );
			painter.drawPixmap( 1, 1, tmp );
			painter.setPen( dis.foreground() );
			painter.drawPixmap( 0, 0, tmp );
		    }
		    if ( !p->smallDisabled.pm->mask() ) {
 			if ( !tmp.mask() )
 			    tmp.setMask( tmp );
			QBitmap mask( d->smallDisabled.pm->size() );
			mask.fill( Qt::color0 );
			QPainter painter( &mask );
			painter.drawPixmap( 0, 0, tmp );
			painter.drawPixmap( 1, 1, tmp );
			painter.end();
			p->smallDisabled.pm->setMask( mask );
		    }

		    p->smallDisabled.generated = TRUE;
		}
		pm = p->smallDisabled.pm;
		break;
	    }
	}
    } else { // state == On
	if ( size == Large ) {
	    switch( mode ) {
	    case Normal:
		if ( !p->on_vlarge.pm ) {
		    QPixmap *fallback = 0;
		    if ( !(fallback = p->on_vsmall.pm ) ) {
			pm = p->on_vlarge.pm = new QPixmap( Large, Normal, Off );
			break;
		    }
		    Q_ASSERT( fallback );
		    i = fallback->convertToImage();
		    i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		    p->on_vlarge.pm = new QPixmap;
		    p->on_vlarge.generated = TRUE;
		    p->on_vlarge.pm->convertFromImage( i );
		    if ( !p->on_vlarge.pm->mask() ) {
			i = i.createHeuristicMask();
			QBitmap tmp;
			tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			p->on_vlarge.pm->setMask( tmp );
		    }
		}
		pm = p->on_vlarge.pm;
		break;
	    case Active:
		if ( !p->on_largeActive.pm ) {
		    p->on_largeActive.pm = new QPixmap( pixmap( Large, Normal, On ) );
		    p->on_largeActive.generated = TRUE;
		}
		pm = p->on_largeActive.pm;
		break;
	    case Disabled:
		if ( !p->on_largeDisabled.pm ) {
		    QBitmap tmp;
		    if ( p->on_vlarge.generated && !p->on_smallDisabled.generated &&
			 p->on_smallDisabled.pm && !p->on_smallDisabled.pm->isNull() ) {
			// if there's a hand-drawn disabled small image,
			// but the normal big one is generated, use the
			// hand-drawn one to generate this one.
			i = p->on_smallDisabled.pm->convertToImage();
			i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
			p->on_largeDisabled.pm = new QPixmap;
			p->on_largeDisabled.pm->convertFromImage( i );
			if ( !p->on_largeDisabled.pm->mask() ) {
			    i = i.createHeuristicMask();
			    tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			}
		    } else {
			if (pixmap( Large, Normal, On ).mask())
			    tmp = *pixmap( Large, Normal, On ).mask();
			else {
			    QPixmap conv = pixmap( Large, Normal, On );
			    if ( !conv.isNull() ) {
				i = conv.convertToImage();
				i = i.createHeuristicMask();
				tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			    }
			}
			p->on_largeDisabled.pm
			    = new QPixmap( p->on_vlarge.pm->width()+1,
					   p->on_vlarge.pm->height()+1);
			QColorGroup dis( QApplication::palette().disabled() );
			p->on_largeDisabled.pm->fill( dis.background() );
			QPainter painter( p->on_largeDisabled.pm );
			painter.setPen( dis.base() );
			painter.drawPixmap( 1, 1, tmp );
			painter.setPen( dis.foreground() );
			painter.drawPixmap( 0, 0, tmp );
		    }
		    if ( !p->on_largeDisabled.pm->mask() ) {
			if ( !tmp.mask() )
			    tmp.setMask( tmp );
			QBitmap mask( d->on_largeDisabled.pm->size() );
			mask.fill( Qt::color0 );
			QPainter painter( &mask );
			painter.drawPixmap( 0, 0, tmp );
			painter.drawPixmap( 1, 1, tmp );
			painter.end();
			p->on_largeDisabled.pm->setMask( mask );
		    }
		    p->on_largeDisabled.generated = TRUE;
		}
		pm = p->on_largeDisabled.pm;
		break;
	    }
	} else {
	    switch( mode ) {
	    case Normal:
		if ( !p->on_vsmall.pm ) {
		    QPixmap *fallback = 0;
		    if ( !( fallback = p->on_vlarge.pm ) ) {
			pm = p->on_vsmall.pm = new QPixmap( pixmap( Small, Normal, Off ) );
			break;
		    }
		    Q_ASSERT( fallback );
		    i = fallback->convertToImage();
		    i = i.smoothScale( i.width() * 2 / 3, i.height() * 2 / 3 );
		    p->on_vsmall.pm = new QPixmap;
		    p->on_vsmall.generated = TRUE;
		    p->on_vsmall.pm->convertFromImage( i );
		    if ( !p->on_vsmall.pm->mask() ) {
			i = i.createHeuristicMask();
			QBitmap tmp;
			tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			p->on_vsmall.pm->setMask( tmp );
		    }
		}
		pm = p->on_vsmall.pm;
		break;
	    case Active:
		if ( !p->on_smallActive.pm ) {
		    p->on_smallActive.pm = new QPixmap( pixmap( Small, Normal, On ) );
		    p->on_smallActive.generated = TRUE;
		}
		pm = p->on_smallActive.pm;
		break;
	    case Disabled:
		if ( !p->on_smallDisabled.pm ) {
		    QBitmap tmp;
		    if ( p->on_vsmall.generated && !p->on_largeDisabled.generated &&
			 p->on_largeDisabled.pm && !p->on_largeDisabled.pm->isNull() ) {
			// if there's a hand-drawn disabled large image,
			// but the normal small one is generated, use the
			// hand-drawn one to generate this one.
			i = p->on_largeDisabled.pm->convertToImage();
			i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
			p->on_smallDisabled.pm = new QPixmap;
			p->on_smallDisabled.pm->convertFromImage( i );
			if ( !p->on_smallDisabled.pm->mask() ) {
			    i = i.createHeuristicMask();
			    tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			}
		    } else {
			if ( pixmap( Small, Normal, On ).mask())
			    tmp = *pixmap( Small, Normal, On ).mask();
			else {
			    QPixmap conv = pixmap( Small, Normal );
			    if ( !conv.isNull() ) {
				i = conv.convertToImage();
				i = i.createHeuristicMask();
				tmp.convertFromImage( i, Qt::MonoOnly + Qt::ThresholdDither );
			    }
			}
			p->on_smallDisabled.pm
			    = new QPixmap( p->on_vsmall.pm->width()+1,
					   p->on_vsmall.pm->height()+1);
			QColorGroup dis( QApplication::palette().disabled() );
			p->on_smallDisabled.pm->fill( dis.background() );
			QPainter painter( p->on_smallDisabled.pm );
			painter.setPen( dis.base() );
			painter.drawPixmap( 1, 1, tmp );
			painter.setPen( dis.foreground() );
			painter.drawPixmap( 0, 0, tmp );
		    }
		    if ( !p->on_smallDisabled.pm->mask() ) {
 			if ( !tmp.mask() )
 			    tmp.setMask( tmp );
			QBitmap mask( d->on_smallDisabled.pm->size() );
			mask.fill( Qt::color0 );
			QPainter painter( &mask );
			painter.drawPixmap( 0, 0, tmp );
			painter.drawPixmap( 1, 1, tmp );
			painter.end();
			p->on_smallDisabled.pm->setMask( mask );
		    }

		    p->on_smallDisabled.generated = TRUE;
		}
		pm = p->on_smallDisabled.pm;
		break;
	    }
	}
    }
    Q_ASSERT( pm );
    return *pm;
}


/*!
  Returns a pixmap with \a size, \a state and Mode either Normal or Disabled,
  depending on the value of \a enabled.
*/
QPixmap QIconSet::pixmap( Size size, bool enabled, State state ) const
{
    return pixmap( size, enabled ? Normal : Disabled, state );
}


/*!
  Returns TRUE if the variant with \a size, \a mode and \a state was
  automatically generated, and FALSE if it was not. This is mainly
  useful for development purposes.
*/
bool QIconSet::isGenerated( Size size, Mode mode, State state ) const
{
    if ( !d )
	return FALSE;

    if ( state == Off ) {
	if ( size == Large ) {
	    if ( mode == Disabled )
		return d->largeDisabled.generated || !d->largeDisabled.pm;
	    else if ( mode == Active )
		return d->largeActive.generated || !d->largeActive.pm;
	    else
		return d->vlarge.generated || !d->vlarge.pm;
	} else if ( size == Small ) {
	    if ( mode == Disabled )
		return d->smallDisabled.generated || !d->smallDisabled.pm;
	    else if ( mode == Active )
		return d->smallActive.generated || !d->smallActive.pm;
	    else
		return d->vsmall.generated || !d->vsmall.pm;
	}
    } else {
	if ( size == Large ) {
	    if ( mode == Disabled )
		return d->on_largeDisabled.generated || !d->on_largeDisabled.pm;
	    else if ( mode == Active )
		return d->on_largeActive.generated || !d->on_largeActive.pm;
	    else
		return d->on_vlarge.generated || !d->on_vlarge.pm;
	} else if ( size == Small ) {
	    if ( mode == Disabled )
		return d->on_smallDisabled.generated || !d->on_smallDisabled.pm;
	    else if ( mode == Active )
		return d->on_smallActive.generated || !d->on_smallActive.pm;
	    else
		return d->on_vsmall.generated || !d->on_vsmall.pm;
	}
    }
    return FALSE;
}


/*!
  Returns the pixmap originally provided to the constructor or
  to reset().  This is the Normal pixmap of unspecified Size.

  \sa reset()
*/

QPixmap QIconSet::pixmap() const
{
    if ( !d )
	return QPixmap();

    return d->defpm;
}


/*!
  Detaches this icon set from others with which it may share data.

  You will never need to call this function; other QIconSet functions
  call it as necessary.
*/
void QIconSet::detach()
{
    if ( !d )
    {
	d = new QIconSetPrivate;
	return;
    }

    if ( d->count == 1 )
	return;

    QIconSetPrivate * p = new QIconSetPrivate;
    p->vsmall.pm = d->vsmall.pm;
    p->vsmall.generated = d->vsmall.generated;
    p->smallActive.pm = d->smallActive.pm;
    p->smallActive.generated = d->smallActive.generated;
    p->smallDisabled.pm = d->smallDisabled.pm;
    p->smallDisabled.generated = d->smallDisabled.generated;
    p->vlarge.pm = d->vlarge.pm;
    p->vlarge.generated = d->vlarge.generated;
    p->largeActive.pm = d->largeActive.pm;
    p->largeActive.generated = d->largeActive.generated;
    p->largeDisabled.pm = d->largeDisabled.pm;
    p->largeDisabled.generated = d->largeDisabled.generated;
    p->defpm = d->defpm;

    p->on_vsmall.pm = d->on_vsmall.pm;
    p->on_vsmall.generated = d->on_vsmall.generated;
    p->on_smallActive.pm = d->on_smallActive.pm;
    p->on_smallActive.generated = d->on_smallActive.generated;
    p->on_smallDisabled.pm = d->on_smallDisabled.pm;
    p->on_smallDisabled.generated = d->on_smallDisabled.generated;
    p->on_vlarge.pm = d->on_vlarge.pm;
    p->on_vlarge.generated = d->on_vlarge.generated;
    p->on_largeActive.pm = d->on_largeActive.pm;
    p->on_largeActive.generated = d->on_largeActive.generated;
    p->on_largeDisabled.pm = d->on_largeDisabled.pm;
    p->on_largeDisabled.generated = d->on_largeDisabled.generated;

    d->deref();
    d = p;
}

#endif // QT_NO_ICONSET
