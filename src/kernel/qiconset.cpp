/****************************************************************************
** $Id: $
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

#include "qapplication.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"
#include "qimage.h"
#include "qpainter.h"

enum { NumSizes = 2, NumModes = 3, NumStates = 2 };

static QIconFactory *defaultFac = 0;
static short widths[2] = { 22, 32 };
static short heights[2] = { 22, 32 };
static QSingleCleanupHandler<QIconFactory> q_cleanup_icon_factory;

enum IconOrigin { SuppliedFileName, SuppliedPixmap, Manufactured, Generated };

class QIconSetPrivate : public QShared
{
public:
    struct Icon
    {
	IconOrigin origin;
	union {
	    QString *fileName;
	    QPixmap *pixmap;
	};

	Icon() : origin( Generated ) { pixmap = 0; }
	Icon( const Icon& other )
	    : origin( Generated ) {
	    pixmap = 0;
	    operator=( other );
	}
	~Icon() {
	    if ( origin == SuppliedFileName ) {
		delete fileName;
	    } else {
		delete pixmap;
	    }
	}

	Icon& operator=( const Icon& other ) {
	    QPixmap *oldPixmap = 0;
	    QString *oldFileName = 0;
	    if ( origin == SuppliedFileName ) {
		oldFileName = fileName;
	    } else {
		oldPixmap = pixmap;
	    }

	    if ( other.origin == SuppliedFileName ) {
		fileName = new QString( *other.fileName );
	    } else {
		if ( other.pixmap ) {
		    pixmap = new QPixmap( *other.pixmap );
		} else {
		    pixmap = 0;
		}
	    }
	    origin = other.origin;
	    delete oldPixmap;
	    delete oldFileName;
	    return *this;
	}

	void clearCached() {
	    if ( (origin == Manufactured || origin == Generated) && pixmap ) {
		delete pixmap;
		pixmap = 0;
	    }
	}
    };

    Icon icons[NumSizes][NumModes][NumStates];
    QPixmap defaultPix;
    QIconFactory *factory;

    QIconSetPrivate() : factory( 0 ) {}
    ~QIconSetPrivate() {}

    Icon *icon( const QIconSet *iconSet, QIconSet::Size size,
		QIconSet::Mode mode, QIconSet::State state ) {
	Icon *ik = &icons[(int) size - 1][(int) mode][(int) state];

	if ( iconSet ) {
	    if ( ik->origin == SuppliedFileName ) {
		QPixmap *newPixmap = new QPixmap( *ik->fileName );
		delete ik->fileName;

		if ( newPixmap->isNull() ) {
		    delete newPixmap;
		    ik->origin = Generated;
		    ik->pixmap = 0;
		} else {
		    ik->origin = SuppliedPixmap;
		    ik->pixmap = newPixmap;
		}
	    }

	    if ( !ik->pixmap ) {
		QIconFactory *f = factory;
		if ( !f )
		    f = defaultFac;

		if ( f ) {
		    ik->pixmap = f->createPixmap( *iconSet, size, mode, state );
		    if ( ik->pixmap )
			ik->origin = Manufactured;
		}
	    }
	}
	return ik;
    }

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const QIconSetPrivate& ) const { return FALSE; }
#endif
#if defined(Q_CC_HPACC) || defined(Q_CC_GNU) && defined(__arch64__)
    // * workaround for "implicit copy ctor for multi dimension array"
    //   bug (CR JAGad01979) in HP-UX aCC (up to A.03.30 at least)
    // * workaround for "sorry, not implemented: cannot initialize
    //   multi-dimensional array with initializer" in GCC 3.1 / 64-bit
    //   on Solaris
    QIconSetPrivate( const QIconSetPrivate& other ) : QShared() {
	count = other.count;
	for ( int i = 0; i < NumSizes; i++ )
	    for ( int j = 0; j < NumModes; j++ )
		for ( int k = 0; k < NumStates; k++ )
		    icons[i][j][k] = other.icons[i][j][k];
	defaultPix = other.defaultPix;
	factory = other.factory;
    }
#endif
};

typedef QIconSetPrivate::Icon Icon;


/*! \class QIconSet qiconset.h

  \brief The QIconSet class provides a set of icons with different
  styles and sizes.

  \ingroup graphics
  \ingroup images
  \ingroup shared
  \mainclass

  A QIconSet can generate smaller, larger, active, and disabled pixmaps
  from the set of icons it is given. Such pixmaps are used by
  QToolButton, QHeader, QPopupMenu, etc. to show an icon representing a
  particular action.

  The simplest use of QIconSet is to create one from a QPixmap and then
  use it, allowing Qt to work out all the required icon styles and
  sizes. For example:

  \code
    QToolButton *but = new QToolButton( QIconSet( QPixmap("open.xpm") ), ... );
  \endcode

  Using whichever pixmaps you specify as a base, QIconSet provides a
  set of six icons, each with a \l Size and a \l Mode: Small Normal,
  Small Disabled, Small Active, Large Normal, Large Disabled, and
  Large Active.

  An additional set of six icons can be provided for widgets that have
  an "On" or "Off" state, like checkable menu items or toggleable
  toolbuttons. If you provide pixmaps for the "On" state, but not for
  the "Off" state, the QIconSet will provide the "Off" pixmaps. You may
  specify icons for both states in you wish.

  You can set any of the icons using setPixmap().

  When you retrieve a pixmap using pixmap(Size, Mode, State),
  QIconSet will return the icon that has been set or previously
  generated for that size, mode and state combination. If none is
  available, QIconSet will ask the icon factory. If the icon factory
  cannot provide any (the default), QIconSet generates a pixmap based
  on the pixmaps it has been given and returns it.

  The \c Disabled appearance is computed using an algorithm that
  produces results very similar to those used in Microsoft Windows
  95. The \c Active appearance is identical to the \c Normal
  appearance unless you use setPixmap() to set it to something
  special.

  When scaling icons, QIconSet uses \link QImage::smoothScale()
  smooth scaling\endlink, which can partially blend the color component
  of pixmaps.  If the results look poor, the best solution
  is to supply pixmaps in both large and small sizes.

  You can use the static function setIconSize() to set the preferred
  size of the generated large/small icons. The default small size is
  22 x 22, while the default large size is 32 x 32. These sizes only
  affect generated icons.

  The isGenerated() function returns TRUE if an icon was generated by
  QIconSet or by a factory; clearGenerated() clears all cached
  pixmaps.

  \section1 Making Classes that use QIconSet

  If you write your own widgets that have an option to set a small
  pixmap, consider allowing a QIconSet to be set for that pixmap.  The
  Qt class QToolButton is an example of such a widget.

  Provide a method to set a QIconSet, and when you draw the icon, choose
  whichever icon is appropriate for the current state of your widget.
  For example:
  \code
    void MyWidget::drawIcon( QPainter* p, QPoint pos )
    {
	p->drawPixmap( pos, icons->pixmap(QIconSet::Small, isEnabled()) );
    }
  \endcode

  You might also make use of the Active mode, perhaps making your widget
  Active when the mouse is over the widget (see \l QWidget::enterEvent()),
  while the mouse is pressed pending the release that will activate
  the function, or when it is the currently selected item. If the widget
  can be toggled, the "On" mode might be used to draw a different icon.

  \img iconset.png QIconSet

  \sa QIconFactory QPixmap QMainWindow::setUsesBigPixmaps()
      \link guibooks.html#fowler GUI Design Handbook: Iconic Label \endlink
*/


/*!
  \enum QIconSet::Size

  This enum type describes the size at which a pixmap is intended to be
  used.
  The currently defined sizes are:

    \value Automatic  The size of the pixmap is determined from its
		    pixel size. This is a useful default.
    \value Small  The pixmap is the smaller of two.
    \value Large  The pixmap is the larger of two.

  If a Small pixmap is not set by QIconSet::setPixmap(), the Large
  pixmap will be automatically scaled down to the size of a small pixmap
  to generate the Small pixmap when required.  Similarly, a Small pixmap
  will be automatically scaled up to generate a Large pixmap. The
  preferred sizes for large/small generated icons can be set using
  setIconSize().

  \sa setIconSize() iconSize() setPixmap() pixmap() QMainWindow::setUsesBigPixmaps()
*/

/*!
  \enum QIconSet::Mode

  This enum type describes the mode for which a pixmap is intended to be
  used.
  The currently defined modes are:

    \value Normal
	 Display the pixmap when the user is
	not interacting with the icon, but the
	functionality represented by the icon is available.
    \value Disabled
	 Display the pixmap when the
	functionality represented by the icon is not available.
    \value Active
	 Display the pixmap when the
	functionality represented by the icon is available and
	the user is interacting with the icon, for example, moving the
	mouse over it or clicking it.
*/

/*!
  \enum QIconSet::State

  This enum describes the state for which a pixmap is intended to be
  used. The \e state can be:

  \value Off  Display the pixmap when the widget is in an "off" state
  \value On  Display the pixmap when the widget is in an "on" state

  \sa setPixmap() pixmap()
*/

/*!
  Constructs a null icon set.

  \sa setPixmap(), reset()
*/
QIconSet::QIconSet()
    : d( 0 )
{
}

/*!
  Constructs an icon set for which the Normal pixmap is \a pixmap,
  which is assumed to be of size \a size.

  The default for \a size is \c Automatic, which means that QIconSet
  will determine whether the pixmap is Small or Large from its pixel
  size. Pixmaps less than the width of a small generated icon are
  considered to be Small. You can use setIconSize() to set the
  preferred size of a generated icon.

  \sa setIconSize() reset()
*/
QIconSet::QIconSet( const QPixmap& pixmap, Size size )
    : d( 0 )
{
    reset( pixmap, size );
}

/*!  Creates an iconset which uses the pixmap \a smallPix for for
  displaying a small icon, and the pixmap \a largePix for displaying a
  large icon.
*/
QIconSet::QIconSet( const QPixmap& smallPix, const QPixmap& largePix )
    : d( 0 )
{
    reset( smallPix, Small );
    reset( largePix, Large );
}

/*!
  Constructs a copy of \a other. This is very fast.
*/
QIconSet::QIconSet( const QIconSet& other )
    : d( other.d )
{
    if ( d )
	d->ref();
}

/*!
  Destroys the icon set and frees any allocated resources.
*/
QIconSet::~QIconSet()
{
    if ( d && d->deref() )
	delete d;
}

/*!
  Sets this icon set to use pixmap \a pixmap for the Normal pixmap,
  assuming it to be of size \a size.

  This is equivalent to assigning QIconSet(\a pixmap, \a size) to this
  icon set.

  This function does nothing if \a pixmap is a null pixmap.
*/
void QIconSet::reset( const QPixmap& pixmap, Size size )
{
    if ( pixmap.isNull() )
	return;

    detach();
    normalize( size, pixmap.size() );
    setPixmap( pixmap, size, Normal );
    d->defaultPix = pixmap;
    d->factory = 0;
}

/*!
  Sets this icon set to provide pixmap \a pixmap for size \a size, mode \a
  mode and state \a state. The icon set may also use \a pixmap for
  generating other pixmaps if they are not explicitly set.

  The \a size can be one of Automatic, Large or Small.  If Automatic is
  used, QIconSet will determine if the pixmap is Small or Large from its
  pixel size.

  Pixmaps less than the width of a small generated icon are
  considered to be Small. You can use setIconSize() to set the preferred
  size of a generated icon.

  This function does nothing if \a pixmap is a null pixmap.

  \sa reset()
*/
void QIconSet::setPixmap( const QPixmap& pixmap, Size size, Mode mode,
			  State state )
{
    if ( pixmap.isNull() )
	return;

    normalize( size, pixmap.size() );

    detach();
    clearGenerated();

    Icon *icon = d->icon( 0, size, mode, state );
    if ( icon->origin == SuppliedFileName ) {
	delete icon->fileName;
	icon->pixmap = 0;
    }
    icon->origin = SuppliedPixmap;
    if ( icon->pixmap == 0 ) {
	icon->pixmap = new QPixmap( pixmap );
    } else {
	*icon->pixmap = pixmap;
    }
}

/*!
  \overload

  The pixmap is loaded from \a fileName when it becomes necessary.
*/
void QIconSet::setPixmap( const QString& fileName, Size size, Mode mode,
			  State state )
{
    if ( size == Automatic ) {
	setPixmap( QPixmap(fileName), size, mode, state );
    } else {
	detach();
	clearGenerated();

	Icon *icon = d->icon( 0, size, mode, state );
	if ( icon->origin == SuppliedFileName ) {
	    *icon->fileName = fileName;
	} else {
	    delete icon->pixmap;
	    icon->fileName = new QString( fileName );
	    icon->origin = SuppliedFileName;
	}
    }
}

/*!
  Returns a pixmap with size \a size, mode \a mode and state \a
  state, generating one if necessary. Generated pixmaps are cached.
*/
QPixmap QIconSet::pixmap( Size size, Mode mode, State state ) const
{
    if ( !d ) {
	if ( defaultFac ) {
	    QIconSet *that = (QIconSet *) this;
	    that->detach();
	} else {
	    return QPixmap();
	}
    }

    if ( size == Automatic )
	size = Small;

    Icon *icon = d->icon( this, size, mode, state );
    if ( icon->pixmap )
	return *icon->pixmap;

    if ( mode == Active )
	return pixmap( size, Normal, state );

    Size otherSize = ( size == Large ) ? Small : Large;
    Icon *otherSizeIcon = d->icon( this, otherSize, mode, state );

    if ( state == Off ) {
	if ( mode == Disabled &&
	     d->icon(this, size, Normal, Off)->origin != Generated ) {
	    icon->pixmap = createDisabled( size, Off );
	} else if ( otherSizeIcon->origin != Generated ) {
	    icon->pixmap = createScaled( size, otherSizeIcon->pixmap );
	} else if ( mode == Disabled ) {
	    icon->pixmap = createDisabled( size, Off );
	} else if ( !d->defaultPix.isNull() ) {
	    icon->pixmap = new QPixmap( d->defaultPix );
	} else {
	    /*
	      No icons are available for { TRUE, Normal, Off } and
	      { FALSE, Normal, Off }. Try the other 10 combinaisons,
	      best ones first.
	    */
	    const int N = 10;
	    struct {
		bool sameSize;
		Mode mode;
		State state;
	    } tryList[N] = {
		{ TRUE, Active, Off },
		{ TRUE, Normal, On },
		{ TRUE, Active, On },
		{ FALSE, Active, Off },
		{ FALSE, Normal, On },
		{ FALSE, Active, On },
		{ TRUE, Disabled, Off },
		{ TRUE, Disabled, On },
		{ FALSE, Disabled, Off },
		{ FALSE, Disabled, On }
	    };
	    bool sameSize = FALSE;

	    for ( int i = 0; i < N; i++ ) {
		sameSize = tryList[i].sameSize;
		Icon *tryIcon = d->icon( this, sameSize ? size : otherSize,
					 tryList[i].mode, tryList[i].state );
		if ( tryIcon->origin != Generated ) {
		    icon->pixmap = new QPixmap( *tryIcon->pixmap );
		    break;
		}
	    }
	    if ( icon->pixmap ) {
		if ( !sameSize ) {
		    QPixmap *old = icon->pixmap;
		    icon->pixmap = createScaled( size, old );
		    delete old;
		}
	    } else {
		icon->pixmap = new QPixmap;
	    }
	}
    } else { /* ( state == On ) */
	if ( mode == Normal ) {
	    if ( otherSizeIcon->origin != Generated ) {
		icon->pixmap = createScaled( size, otherSizeIcon->pixmap );
	    } else {
		icon->pixmap = new QPixmap( pixmap(size, mode, Off) );
	    }
	} else { /* ( mode == Disabled ) */
	    Icon *offIcon = d->icon( this, size, mode, Off );
	    Icon *otherSizeOffIcon = d->icon( this, otherSize, mode, Off );

	    if ( offIcon->origin != Generated ) {
		icon->pixmap = new QPixmap( *offIcon->pixmap );
	    } else if ( d->icon(this, size, Normal, On)->origin != Generated ) {
		icon->pixmap = createDisabled( size, On );
	    } else if ( otherSizeIcon->origin != Generated ) {
		icon->pixmap = createScaled( size, otherSizeIcon->pixmap );
	    } else if ( otherSizeOffIcon->origin != Generated ) {
		icon->pixmap = createScaled( size, otherSizeOffIcon->pixmap );
	    } else {
		icon->pixmap = createDisabled( size, On );
	    }
	}
    }
    return *icon->pixmap;
}

/*! \overload
    \obsolete

    This is the same as pixmap(\a size, \a enabled, \a state).
*/
QPixmap QIconSet::pixmap( Size size, bool enabled, State state ) const
{
    return pixmap( size, enabled ? Normal : Disabled, state );
}

/*!
  \overload

  Returns the pixmap originally provided to the constructor or to
  reset(). This is the Normal pixmap of unspecified Size.

  \sa reset()
*/
QPixmap QIconSet::pixmap() const
{
    if ( !d )
	return QPixmap();
    return d->defaultPix;
}

/*!
  Returns TRUE if the pixmap with size \a size, mode \a mode and
  state \a state is generated from other pixmaps; otherwise returns
  FALSE.

  A pixmap obtained from a QIconFactory is considered non-generated.
*/
bool QIconSet::isGenerated( Size size, Mode mode, State state ) const
{
    if ( !d )
	return TRUE;
    return d->icon( this, size, mode, state )->origin == Generated;
};

/*!
  Clears all cached pixmaps.
*/
void QIconSet::clearGenerated()
{
    if ( !d )
	return;

    for ( int i = 0; i < NumSizes; i++ ) {
	for ( int j = 0; j < NumModes; j++ ) {
	    for ( int k = 0; k < NumStates; k++ )
		d->icons[i][j][k].clearCached();
	}
    }
}

/*!
  Installs \a factory as the icon factory for this iconset. The icon
  factory is used to generates pixmaps not set by the user.

  If no icon factory is installed, QIconFactory::defaultFactory() is
  used.
*/
void QIconSet::installIconFactory( QIconFactory *factory )
{
    detach();
    d->factory = factory;
}

/*!
  Returns TRUE if the icon set is empty; otherwise returns FALSE.
*/
bool QIconSet::isNull() const
{
    return !d;
}

/*!
  Detaches this icon set from others with which it may share data.

  You will never need to call this function; other QIconSet functions
  call it as necessary.
*/
void QIconSet::detach()
{
    if ( !d ) {
	d = new QIconSetPrivate;
	return;
    }
    if ( d->count != 1 ) {
	d->deref();
	d = new QIconSetPrivate( *d );
	d->count = 1;
    }
}

/*!
  Assigns \a other to this icon set and returns a reference to this
  icon set.

  \sa detach()
*/
QIconSet& QIconSet::operator=( const QIconSet& other )
{
    if ( other.d )
	other.d->ref();

    if ( d && d->deref() )
	delete d;
    d = other.d;
    return *this;
}

/*!
  Set the preferred size for all small or large icons that are
  generated after this call. If \a which is Small, sets the preferred
  size of small generated icons to \a size. Similarly, if \a which is
  Large, sets the preferred size of large generated icons to \a size.

  Note that cached icons will not be regenerated, so it is recommended
  that you set the preferred icon sizes before generating any icon sets.
  Also note that the preferred icon sizes will be ignored for icon sets
  that have been created using both small and large pixmaps.

  \sa iconSize()
*/
void QIconSet::setIconSize( Size which, const QSize& size )
{
    widths[(int) which - 1] = size.width();
    heights[(int) which - 1] = size.height();
}

/*!
    If \a which is Small, returns the preferred size of a small
    generated icon; if \a which is Large, returns the preferred size
    of a large generated icon.

  \sa setIconSize()
*/
const QSize& QIconSet::iconSize( Size which )
{
    // ### replace 'const QSize&' with QSize in Qt 4 and simply this code
    static QSize size;
    size = QSize( widths[(int) which - 1], heights[(int) which - 1] );
    return size;
}

void QIconSet::normalize( Size& which, const QSize& pixSize )
{
    if ( which == Automatic )
	which = pixSize.width() > iconSize( Small ).width() ? Large : Small;
}

QPixmap *QIconSet::createScaled( Size size, const QPixmap *suppliedPix ) const
{
    QImage img = suppliedPix->convertToImage();
    QSize imgSize = iconSize( size );
    if ( size == Small ) {
	imgSize = imgSize.boundedTo( img.size() );
    } else {
	imgSize = imgSize.expandedTo( img.size() );
    }
    img = img.smoothScale( imgSize );

    QPixmap *pixmap = new QPixmap( img );
    if ( !pixmap->mask() ) {
	QBitmap mask;
	mask.convertFromImage( img.createHeuristicMask(),
			       Qt::MonoOnly | Qt::ThresholdDither );
	pixmap->setMask( mask );
    }
    return pixmap;
}

QPixmap *QIconSet::createDisabled( Size size, State state ) const
{
    QBitmap normalMask;
    QPixmap normalPix = pixmap( size, Normal, state );
    QImage img;

    if ( normalPix.mask() ) {
	normalMask = *normalPix.mask();
    } else if ( !normalPix.isNull() ) {
	img = normalPix.convertToImage();
	normalMask.convertFromImage( img.createHeuristicMask(),
				     Qt::MonoOnly | Qt::ThresholdDither );
    }

    QPixmap *pixmap = new QPixmap( normalPix.width() + 1,
				   normalPix.height() + 1 );
    const QColorGroup &dis = QApplication::palette().disabled();
    pixmap->fill( dis.background() );

    QPainter painter;
    painter.begin( pixmap );
    painter.setPen( dis.base() );
    painter.drawPixmap( 1, 1, normalMask );
    painter.setPen( dis.foreground() );
    painter.drawPixmap( 0, 0, normalMask );
    painter.end();

    if ( !normalMask.mask() )
	normalMask.setMask( normalMask );

    QBitmap mask( pixmap->size() );
    mask.fill( Qt::color0 );
    painter.begin( &mask );
    painter.drawPixmap( 0, 0, normalMask );
    painter.drawPixmap( 1, 1, normalMask );
    painter.end();
    pixmap->setMask( mask );
    return pixmap;
}

/*! \class QIconFactory
  \ingroup advanced
  \brief The QIconFactory class is used to create pixmaps for a QIconSet.

  By reimplementing createPixmap(), you can override QIconSet's
  default algorithm for computing pixmaps not supplied by the user.

  \sa QIconSet
*/

/*!
  Constructs an icon factory with parent \a parent and name \a name.
*/
QIconFactory::QIconFactory( QObject *parent, const char *name )
    : QObject( parent, name )
{
}

/*!
  Destroys the object and frees any allocated resources.
*/
QIconFactory::~QIconFactory()
{
}

/*!
  Ceates a pixmap for \a iconSet with a certain \a size, \a mode, and
  \a state. Returns 0 if the default QIconSet algorithm should be
  used to create a pixmap that wasn't supplied by the user.

  It is the caller's responsability to delete the returned object.

  The default implementation always returns 0.
*/
QPixmap *QIconFactory::createPixmap( const QIconSet& /* iconSet */,
				     QIconSet::Size /* size */,
				     QIconSet::Mode /* mode */,
				     QIconSet::State /* state */ )
{
    return 0;
}

/*!
  Returns the default icon factory.

  \sa setDefaultFactory()
*/
QIconFactory *QIconFactory::defaultFactory()
{
    if ( !defaultFac ) {
	defaultFac = new QIconFactory;
	q_cleanup_icon_factory.set( &defaultFac );
    }
    return defaultFac;
}

/*!
  Replaces the default icon factory with \a factory.

  QIconFactory takes ownership of \a factory, and destroys it when it
  is no longer needed.
*/
void QIconFactory::installDefaultFactory( QIconFactory *factory )
{
    if ( !factory )
	return;

    delete defaultFac;
    defaultFac = factory;
    q_cleanup_icon_factory.set( &defaultFac );
}

#endif // QT_NO_ICONSET
