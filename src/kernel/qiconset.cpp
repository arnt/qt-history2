/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qiconset.cpp#6 $
**
** Implementation of QIconSet class
**
** Created : 980318
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qiconset.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qapp.h"
#include "qpainter.h"

struct QIconSetPrivate: public QShared
{
    struct Variant {
	Variant(): pm(0), generated(0) {}
	~Variant()
	{
	    delete pm;
	}
	void clear()
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
    Variant small;
    Variant large;
    Variant smallActive;
    Variant largeActive;
    Variant smallDisabled;
    Variant largeDisabled;
    QPixmap defpm;
};


/*! \class QIconSet qiconset.h

  \brief The QIconSet class provides a set of icons (normal, disabled,
  various sizes) for e.g. buttons.

  \ingroup misc

  QIconSet must be fed at least one icon, and can generate the other
  variants from the ones it is fed, or use programmer-specified icons.
*/




/*!  Constructs an icon set that will generate its members from \a
  defaultPixmap, which is assumed to be of \a defaultSize.

  The default for \a defaultSize is \c Automatic, which means that
  QIconSet will determine the icon's size from its actual size.

  \sa reset()
*/

QIconSet::QIconSet( const QPixmap & defaultPixmap, Size defaultSize )
{
    d = 0;
    reset( defaultPixmap, defaultSize );
}


/*!  Constructs an a copy of \a other.  This is very fast. */

QIconSet::QIconSet( const QIconSet & other )
{
    d = other.d;
    if ( d )
	d->ref();
}


/*! Destroys the icon set and frees any allocated resources. */

QIconSet::~QIconSet()
{
    if ( d && d->deref() )
	delete d;
}


/*!
  Assigns \e other to this icon set and returns a reference to this
  icon set.

  This is very fast.
*/

QIconSet &QIconSet::operator=( const QIconSet &p )
{
    if ( p.d ) {
	p.d->ref();				// beware of p = p
	if ( d->deref() )
	    delete d;
	d = p.d;
	return *this;
    } else {
	if ( d && d->deref() )
	    delete d;
	d = 0;
	return *this;
    }
}



/*!

*/

void QIconSet::reset( const QPixmap & pm, Size s )
{
    if ( s == Small ||
	 (s == Automatic && pm.width() < 19 ) )
	setPixmap( pm, Small, Normal );
    else
	setPixmap( pm, Large, Normal );
    d->defpm = pm;
}


/*!  Sets this icon set to display \a pn in size \a s/mode \a m, and
  perhaps to use \a pm for deriving some other varieties.
*/

void QIconSet::setPixmap( const QPixmap & pm, Size s, Mode m )
{
    if ( d ) {
	d->small.clear();
	d->large.clear();
	d->smallDisabled.clear();
	d->largeDisabled.clear();
	d->smallActive.clear();
	d->largeActive.clear();
    } else {
	d = new QIconSetPrivate;
    }
    if ( s == Large ) {
	switch( m ) {
	case Active:
	    d->largeActive.pm = new QPixmap( pm );
	    break;
	case Disabled:
	    d->largeDisabled.pm = new QPixmap( pm );
	    break;
	case Normal:
	default:
	    d->large.pm = new QPixmap( pm );
	    break;
	}
    } else if ( s == Small ) {
	switch( m ) {
	case Active:
	    d->smallActive.pm = new QPixmap( pm );
	    break;
	case Disabled:
	    d->smallDisabled.pm = new QPixmap( pm );
	    break;
	case Normal:
	default:
	    d->small.pm = new QPixmap( pm );
	    break;
	}
    }
}


/*!  Sets this icon set to load \a fileName as a pixmap and display it
  in size \a s/mode \a m, and perhaps to use \a pm for deriving some
  other varieties.
*/

void QIconSet::setPixmap( const char * fileName, Size s, Mode m )
{
    QPixmap p;
    p.load( fileName );
    if ( !p.isNull() )
	setPixmap( p, s, m );
}


/*!

*/

QPixmap QIconSet::pixmap( Size s, Mode m ) const
{
    QImage i;
    QIconSetPrivate * p = ((QIconSet *)this)->d;
    QPixmap * pm = 0;
    if ( s == Large ) {
	switch( m ) {
	case Normal:
	    if ( !p->large.pm ) {
		ASSERT( p->small.pm );
		i = p->small.pm->convertToImage();
		i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		p->large.pm = new QPixmap;
		p->large.generated = TRUE;
		p->large.pm->convertFromImage( i );
	    }
	    pm = p->large.pm;
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
		if ( p->large.generated && !p->smallDisabled.generated ) {
		    // if there's a hand-drawn disabled small image,
		    // but the normal big one is generated, use the
		    // hand-drawn one to generate this one.
		    i = p->smallDisabled.pm->convertToImage();
		    i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		    p->largeDisabled.pm = new QPixmap;
		    p->largeDisabled.pm->convertFromImage( i );
		    if ( !p->largeDisabled.pm->mask() ) {
			i = i.createHeuristicMask();
			tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    }
		} else {
		    i = pixmap( Large, Normal ).convertToImage();
		    i = i.createHeuristicMask();
		    QBitmap tmp;
		    tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    p->largeDisabled.pm
			= new QPixmap( p->large.pm->width()+1,
				       p->large.pm->height()+1);
		    QColorGroup dis( QApplication::palette()->disabled() );
		    p->largeDisabled.pm->fill( dis.background() );
		    QPainter painter( p->largeDisabled.pm );
		    painter.setPen( dis.base() );
		    painter.drawPixmap( 1, 1, tmp );
		    painter.setPen( dis.foreground() );
		    painter.drawPixmap( 0, 0, tmp );
		}
		if ( !p->largeDisabled.pm->mask() ) {
		    QBitmap mask( d->largeDisabled.pm->size() );
		    mask.fill( color0 );
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
	switch( m ) {
	case Normal:
	    if ( !p->small.pm ) {
		ASSERT( p->large.pm );
		i = p->large.pm->convertToImage();
		i = i.smoothScale( i.width() * 2 / 3, i.height() * 2 / 3 );
		p->small.pm = new QPixmap;
		p->small.generated = TRUE;
		p->small.pm->convertFromImage( i );
	    }
	    pm = p->small.pm;
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
		if ( p->small.generated && !p->largeDisabled.generated ) {
		    // if there's a hand-drawn disabled large image,
		    // but the normal small one is generated, use the
		    // hand-drawn one to generate this one.
		    i = p->largeDisabled.pm->convertToImage();
		    i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		    p->smallDisabled.pm = new QPixmap;
		    p->smallDisabled.pm->convertFromImage( i );
		    if ( !p->smallDisabled.pm->mask() ) {
			i = i.createHeuristicMask();
			tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    }
		} else {
		    i = pixmap( Small, Normal ).convertToImage();
		    i = i.createHeuristicMask();
		    tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    p->smallDisabled.pm
			= new QPixmap( p->small.pm->width()+1,
				       p->small.pm->height()+1);
		    QColorGroup dis( QApplication::palette()->disabled() );
		    p->smallDisabled.pm->fill( dis.background() );
		    QPainter painter( p->smallDisabled.pm );
		    painter.setPen( dis.base() );
		    painter.drawPixmap( 1, 1, tmp );
		    painter.setPen( dis.foreground() );
		    painter.drawPixmap( 0, 0, tmp );
		}
		if ( !p->smallDisabled.pm->mask() ) {
		    QBitmap mask( d->smallDisabled.pm->size() );
		    mask.fill( color0 );
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
    ASSERT( pm );
    return *pm;
}


/*!

*/

bool QIconSet::isGenerated( Size s, Mode m ) const
{
    if ( s == Large ) {
	if ( m == Disabled )
	    return d->largeDisabled.generated;
	else if ( m == Active )
	    return d->largeActive.generated;
	else
	    return d->large.generated;
    } else if ( s == Small ) {
	if ( m == Disabled )
	    return d->smallDisabled.generated;
	else if ( m == Active )
	    return d->smallActive.generated;
	else
	    return d->small.generated;
    }
    return FALSE;
}


/*!  Returns the pixmap originally provided to the constructor or
  reset().

  \sa reset()
*/

QPixmap QIconSet::pixmap() const
{
    return d->defpm;
}
