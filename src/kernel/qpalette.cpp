/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpalette.cpp#55 $
**
** Implementation of QColorGroup and QPalette classes
**
** Created : 950323
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpalette.h"

#ifndef QT_NO_PALETTE
#include "qdatastream.h"
#include "qpixmap.h"

/*****************************************************************************
  QColorGroup member functions
 *****************************************************************************/

// REVISED - arnt
/*!
  \class QColorGroup qpalette.h
  \brief The QColorGroup class contains a group of widget colors.

  \ingroup appearance
  \ingroup drawing

  A color group contains a group of colors used by widgets for drawing
  themselves.  Widgets should not use colors like "red" and "turqoise"
  but rather "foreground" and "base", where possible.  The color roles
  are enumerated and defined in the ColorRole documentation.

  The most common usage of QColorGroup is like this:

  \code
    QPainter p;
    ...
    p.setPen( colorGroup().foreground() );
    p.drawLine( ... )
  \endcode

  See the \l ColorRole documentation below for more details on roles.

  It's also possible to modify color groups or create them from scratch.

  The color group class can be created using three different
  constructors, or by modifying one supplied by the system.  The
  default constructor creates an all-black color group, which can then
  be modified using set functions.  There are two functions that take
  long lists of arguments (slightly different lists - beware!).  And
  there is the copy constructor.

  We strongly recommend using a system-supplied color group, and
  modifying that as necessary.

  You modify a color group by calling the access functions setColor()
  and setBrush(), depending on whether you want a pure color or e.g. a
  pixmap pattern.

  There are also corresponding color() and brush() getters, and a
  commonly used convenience function to get each ColorRole:
  background(), foreground(), base() and so on.

  \sa QColor QPalette QWidget::colorGroup()
*/

/*! \enum QColorGroup::ColorRole

  The ColorRole enum defines the different symbolic color roles used
  in current GUIs.  The central roles are:

  <ul>
  <li> \c Background - general background color.

  <li> \c Foreground - general foreground color.

  <li> \c Base - used as background color for e.g. text entry widgets,
  usually white or another light color.

  <li> \c Text - the foreground color used with \c Base. Usually this
  is the same as the \c Foreground, in what case it must provide good
  contrast both with \c Background and \c Base.

  <li> \c Button - general button background color, where buttons need a
  background different from \c Background, as in the Macintosh style.

  <li> \c ButtonText - a foreground color used with the \c Button color.

  </ul> There are some color roles used mostly for 3D bevel and shadow
  effects: <ul>

  <li> \c Light - lighter than \c Button color.

  <li> \c Midlight - between \c Button and \c Light.

  <li> \c Dark - darker than \c Button.

  <li> \c Mid - between \c Button and \c Dark.

  <li> \c Shadow - a very dark color.

  </ul> All of these are normally derived from \c Background, and used
  in ways that depend on that relationship.  For example, buttons
  depend on it to make the bevels look good, and Motif scroll bars
  depend on \c Mid to be slightly different from \c Background.

  Selected (marked) items have two roles: <ul>

  <li> \c Highlight  - a color to indicate a selected or highlighted item.

  <li> \c HighlightedText - a text color that contrasts to \c Highlight.

  </ul> Finally, there is a special role for text that needs to be
  drawn where \c Text or \c Foreground would provide bad contrast,
  such as on pressed push buttons: <ul>

  <li> \c BrightText - a text color that is very different from \c
  Foreground and contrasts well with e.g. \c Dark.

  </ul>

  Note that text colors can be used for other things than just words:
  text colors are \e usually used for text, but it's quite common to
  have lines, icons and so on that belong with a text color logically.

  This image shows most of the color roles in use:
  <img src="palette.png" width="603" height="166" alt="">
*/




/*!
  Constructs a color group with all colors set to black.
*/

QColorGroup::QColorGroup()
{
    br = new QBrush[NColorRoles];	// all colors become black

    // The d pointer may allow sharing in the future.  The br pointer
    // then will be a redundant pointer that makes possible the inlines
    // in the header file.  Don't forget to add delete d in the destructor.
    // QPalette, the main QColorGroup user, is already shared though,
    // so perhaps not much is to be gained.
    d = 0;
}

/*!
  Constructs a color group that is an independent copy of \a other.
*/
QColorGroup::QColorGroup( const QColorGroup& other )
{
    br = new QBrush[NColorRoles];
    for (int i=0; i<NColorRoles; i++)
	br[i] = other.br[i];
    d = 0;
}

/*!
  Copies the colours of \a other to this color group.
*/
QColorGroup& QColorGroup::operator =(const QColorGroup& other)
{
    for (int i=0; i<NColorRoles; i++)
	br[i] = other.br[i];
    return *this;
}

static QColor qt_mix_colors( QColor a, QColor b)
{
    return QColor( (a.red() + b.red()) / 2, (a.green() + b.green()) / 2, (a.blue() + b.blue()) / 2 );
}


/*!
Constructs a color group. You can pass either brushes, pixmaps or
plain colors for each parameter.

This constructor can be very handy sometimes, but don't overuse it:
Such long lists of arguments are rather error-prone.

\sa QBrush
*/
 QColorGroup::QColorGroup( const QBrush &foreground, const QBrush &button,
			   const QBrush &light, const QBrush &dark,
			   const QBrush &mid, const QBrush &text,
			   const QBrush &bright_text, const QBrush &base,
			   const QBrush &background)
{
    br = new QBrush[NColorRoles];
    br[Foreground]      = foreground;
    br[Button] 	 	= button;
    br[Light] 		= light;
    br[Dark] 		= dark;
    br[Mid] 		= mid;
    br[Text] 		= text;
    br[BrightText] 	= bright_text;
    br[ButtonText] 	= text;
    br[Base] 		= base;
    br[Background] 	= background;
    br[Midlight] 	= qt_mix_colors( br[Button].color(), br[Light].color() );
    br[Shadow]          = Qt::black;
    br[Highlight]       = Qt::darkBlue;
    br[HighlightedText] = Qt::white;
}


/*!\obsolete

  Constructs a color group with the specified colors. The button
  color will be set to the background color.
*/

QColorGroup::QColorGroup( const QColor &foreground, const QColor &background,
			  const QColor &light, const QColor &dark,
			  const QColor &mid,
			  const QColor &text, const QColor &base )
{
    br = new QBrush[NColorRoles];
    br[Foreground]      = QBrush(foreground);
    br[Button]          = QBrush(background);
    br[Light]           = QBrush(light);
    br[Dark]            = QBrush(dark);
    br[Mid]             = QBrush(mid);
    br[Text]            = QBrush(text);
    br[BrightText]      = br[Light];
    br[ButtonText]      = br[Text];
    br[Base]            = QBrush(base);
    br[Background]      = QBrush(background);
    br[Midlight] 	= qt_mix_colors( br[Button].color(), br[Light].color() );
    br[Shadow]          = Qt::black;
    br[Highlight]       = Qt::darkBlue;
    br[HighlightedText] = Qt::white;
}

/*!
  Destructs the color group.
*/

QColorGroup::~QColorGroup()
{
    delete [] br;
}

/*!
  Returns the color that has been set for color role \a r.
  \sa brush() ColorRole
 */
const QColor &QColorGroup::color( ColorRole r ) const
{
    return br[r].color();
}

/*!
  Returns the brush that has been set for color role \a r.

  \sa color() setBrush() ColorRole
*/
const QBrush &QColorGroup::brush( ColorRole r ) const
{
    return br[r];
}

/*!
  Sets the brush used for color role \a r to a solid color \a c.

  \sa brush() setColor() ColorRole
*/
void QColorGroup::setColor( ColorRole r, const QColor &c )
{
    setBrush( r, QBrush(c) );
}

/*!
  Sets the brush used for color role \a r to \a b.

  \sa brush() setColor() ColorRole
*/
void QColorGroup::setBrush( ColorRole r, const QBrush &b )
{
    br[r] = b;
}


/*!
  \fn const QColor & QColorGroup::foreground() const

  Returns the foreground color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::button() const
  Returns the button color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::light() const
  Returns the light color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor& QColorGroup::midlight() const
  Returns the midlight color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::dark() const
  Returns the dark color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::mid() const
  Returns the medium color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::text() const
  Returns the text foreground color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::brightText() const
  Returns the bright text foreground color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::buttonText() const
  Returns the button text foreground color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::base() const
  Returns the base color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::background() const
  Returns the background color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::shadow() const
  Returns the shadow color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::highlight() const
  Returns the highlight color of the color group.

  \sa ColorRole
*/

/*!
  \fn const QColor & QColorGroup::highlightedText() const
  Returns the highlighted text color of the color group.

  \sa ColorRole
*/

/*!
  \fn bool QColorGroup::operator!=( const QColorGroup &g ) const
  Returns TRUE if this color group is different from \e g, or FALSE if
  it is equal to \e g.
  \sa operator!=()
*/

/*!
  Returns TRUE if this color group is equal to \e g, or FALSE if
  it is different from \e g.
  \sa operator==()
*/

bool QColorGroup::operator==( const QColorGroup &g ) const
{
    for( int r = 0 ; r < NColorRoles ; r++ )
	if ( br[r] != g.br[r] )
	    return FALSE;
    return TRUE;
}


/*****************************************************************************
  QPalette member functions
 *****************************************************************************/

/*!
  \class QPalette qpalette.h

  \brief The QPalette class contains color groups for each widget state.

  \ingroup appearance
  \ingroup shared
  \ingroup drawing

  A palette consists of three color groups: a \e active, a \e disabled
  and an \e inactive color group.  All widgets contain a palette, and
  all the widgets in Qt use their palette to draw themselves.  This
  makes the user interface consistent and easily configurable.

  If you make a new widget you are strongly advised to use the colors in
  the palette rather than hard-coding specific colors.

  The color groups are: <ul> <li> The active() group is used for the
  window that has keyboard focus. <li> The inactive() group is used
  for other windows. <li> The disabled() group is used for widgets
  (not windows) that are disabled for some reason. </ul>

  Of course, both active and inactive windows can contain disabled
  widgets.  (Disabled widgets are often called \e inaccessible or \e
  grayed \e out.)

  In Motif style, active() and inactive() look precisely the same.  In
  Windows 2000 style and Macintosh Platinum style, the two styles look
  slightly different.

  There are setActive(), setInactive() and setDisabled() functions to
  modify the palette.  Qt also supports a normal() group; this is an
  obsolete alias for active(), supported for backward compatibility.

  (The split between normal() and active() prior to Qt 2.1 did not
  work except in the simplest of cases, hence the change to the
  current, more powerful design.)

  \sa QApplication::setPalette(), QWidget::setPalette(), QColorGroup, QColor
*/ // ### should mention the constructors, copy stuff and isCopyOf()


static int palette_count = 1;

/*!
  Constructs a palette that consists of color groups with only black colors.
*/

QPalette::QPalette()
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
}

/*!\obsolete
  Constructs a palette from the \e button color. The other colors are
  automatically calculated, based on this color. Background will be
  the button color as well.
*/

QPalette::QPalette( const QColor &button )
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
    QColor bg = button, btn = button, fg, base, disfg;
    int h, s, v;
    bg.hsv( &h, &s, &v );
    if ( v > 128 ) {				// light background
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {					// dark background
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    data->normal   = QColorGroup( fg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), fg, Qt::white, base, bg );
    data->active   = data->normal;
    data->disabled = QColorGroup( disfg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), disfg, Qt::white, base, bg );
    data->inactive = data->active;
}

/*!
  Constructs a palette from a \e button color and a background. The other colors are
  automatically calculated, based on these colors.
*/

QPalette::QPalette( const QColor &button, const QColor &background )
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
    QColor bg = background, btn = button, fg, base, disfg;
    int h, s, v;
    bg.hsv( &h, &s, &v );
    if ( v > 128 ) {				// light background
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {					// dark background
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    data->normal   = QColorGroup( fg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), fg, Qt::white, base, bg );
    data->active   = data->normal;
    data->disabled = QColorGroup( disfg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), disfg, Qt::white, base, bg );
    data->inactive = data->active;
}

/*!
  Constructs a palette that consists of the three color groups \e
  active, \e disabled and \e inactive.  See QPalette for definitions
  of the color groups and QColorGroup::ColorRole for definitions of
  each color role in the three groups.

  \sa QColorGroup QColorGroup::ColorRole QPalette
*/

QPalette::QPalette( const QColorGroup &active, const QColorGroup &disabled,
		    const QColorGroup &inactive )
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
    data->active = active;
    data->normal = data->active;
    data->disabled = disabled;
    data->inactive = inactive;
}

/*!
  Constructs a copy of \a p.

  This constructor is fast (it uses copy-on-write).
*/

QPalette::QPalette( const QPalette &p )
{
    data = p.data;
    data->ref();
}

/*!
  Destructs the palette.
*/

QPalette::~QPalette()
{
    if ( data->deref() )
	delete data;
}

/*!
  Assigns \e p to this palette and returns a reference to this
  palette.

  This is fast (it uses copy-on-write).

  \sa copy()
*/

QPalette &QPalette::operator=( const QPalette &p )
{
    p.data->ref();
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*!
  Returns the color in \a gr used for color role \a r.

  \sa brush() setColor() QColorGroup::ColorRole
*/
const QColor &QPalette::color( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    return directBrush( gr, r ).color();
}

/*!
  Returns the brush in \a gr used for color role \a r.

  \sa color() setBrush() QColorGroup::ColorRole
*/
const QBrush &QPalette::brush( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    return directBrush( gr, r );
}

/*!
  Sets the brush in \a gr used for color role \a r to the solid color \a c.

  \sa setBrush() color() QColorGroup::ColorRole
*/
void QPalette::setColor( ColorGroup gr, QColorGroup::ColorRole r,
			 const QColor &c)
{
    setBrush( gr, r, QBrush(c) );
}

/*!
  Sets the brush in \a gr used for color role \a r to \a b.

  \sa brush() setColor() QColorGroup::ColorRole
*/
void QPalette::setBrush( ColorGroup gr, QColorGroup::ColorRole r,
			 const QBrush &b)
{
    detach();
    data->ser_no = palette_count++;
    if ( gr == Normal )
	gr = Active; // #### remove 3.0
    directBrush( gr, r ) = b;
    if ( gr == Active )
	data->normal = data->active; // ##### remove 3.0
}

/*!
  Sets the color of the brush in \a gr used for color role \a r to \a c.

  \sa color() setBrush() QColorGroup::ColorRole
*/
void QPalette::setColor( QColorGroup::ColorRole r, const QColor &c )
{
    setBrush( r, QBrush(c) );
}

/*!
  Sets the brush in for color role \a r in all three color groups to \a b.

  \sa brush() setColor() QColorGroup::ColorRole active() inactive() disabled()
*/
void QPalette::setBrush( QColorGroup::ColorRole r, const QBrush &b )
{
    detach();
    data->ser_no = palette_count++;
    directBrush( Active,   r ) = b;
    directBrush( Disabled, r ) = b;
    directBrush( Inactive,   r ) = b;
    data->normal = data->active; // #### remove 3.0
}


/*! Return a deep copy of this palette.  This is slower than the copy
constructor and assignment operator and offers no advantages any more.
*/

QPalette QPalette::copy() const
{
    QPalette p( data->active, data->disabled, data->inactive );
    return p;
}


/*!
  Detaches this palette from any other QPalette objects with which it
  might implicitly share QColorGroup objects.  In essence, does the
  copy bit of copy-on-write.

  Calling this should generally not be necessary; QPalette calls it
  itself when necessary.
*/

void QPalette::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*! \fn const QColorGroup & QPalette::normal() const

  \obsolete

  Use active() instead.
*/

/*!\obsolete

  Use setActive() instead.
*/

void QPalette::setNormal( const QColorGroup &g )
{
    setActive( g );
}

/*!
  \fn const QColorGroup & QPalette::disabled() const

  Returns the disabled color group of this palette.

  \sa QColorGroup, setDisabled(), active(), inactive()
*/

/*!
  Sets the \c Disabled color group to \e g.
  \sa disabled() setActive() setInactive()
*/

void QPalette::setDisabled( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->disabled = g;
}

/*!
  \fn const QColorGroup & QPalette::active() const
  Returns the active color group of this palette.
  \sa QColorGroup, setActive(), inactive(), disabled()
*/

/*!
  Sets the \c Active color group to \e g.
  \sa active() setDisabled() setInactive() QColorGroup
*/

void QPalette::setActive( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->active = g;
    data->normal = data->active; //#### alias
}

/*!
  \fn const QColorGroup & QPalette::inactive() const
  Returns the inactive color group of this palette.
  \sa QColorGroup,  setInactive(), active(), disabled()
*/

/*!
  Sets the \c Inactive color group to \e g.
  \sa active() setDisabled() setActive() QColorGroup
*/

void QPalette::setInactive( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->inactive = g;
}


/*!
  \fn bool QPalette::operator!=( const QPalette &p ) const

  Returns TRUE (slowly) if this palette is different from \e p, or
  FALSE (usually quickly) if they are equal.
*/

/*!  Returns TRUE (usually quickly) if this palette is equal to \e p,
or FALSE (slowly) if they are different.
*/

bool QPalette::operator==( const QPalette &p ) const
{
    return data->active == p.data->active &&
	   data->disabled == p.data->disabled &&
	   data->inactive == p.data->inactive;
}


/*!
  \fn int QPalette::serialNumber() const

  Returns a number that uniquely identifies this QPalette object.  The
  serial number is intended for caching.  Its value may not be used
  for anything other than equality testing.

  Note that QPalette uses copy-on-write, and the serial number changes
  during the lazy copy operation (detach()), not during a shallow
  copy (copy constructor or assignment).

  \sa QPixmap QPixmapCache QCache
*/


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
  \relates QColorGroup
  Writes a color group to the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QColorGroup &g )
{
    if ( s.version() == 1 ) {
	s << g.foreground()
	  << g.background()
	  << g.light()
	  << g.dark()
	  << g.mid()
	  << g.text()
	  << g.base();
    } else {
	for( int r = 0 ; r < QColorGroup::NColorRoles ; r++ )
	    s << g.brush( (QColorGroup::ColorRole)r);
    }
    return s;
}

/*!
  \related QColorGroup
  Reads a color group from the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QColorGroup &g )
{
    if ( s.version() == 1 ) {
	QColor fg, bg, light, dark, mid, text, base;
	s >> fg >> bg >> light >> dark >> mid >> text >> base;
	QPalette p( bg );
	QColorGroup n( p.normal() );
	n.setColor( QColorGroup::Foreground, fg );
	n.setColor( QColorGroup::Light, light );
	n.setColor( QColorGroup::Dark, dark );
	n.setColor( QColorGroup::Mid, mid );
	n.setColor( QColorGroup::Text, text );
	n.setColor( QColorGroup::Base, base );
	g = n;
    } else {
	QBrush tmp;
	for( int r = 0 ; r < QColorGroup::NColorRoles; r++ ) {
	    s >> tmp;
	    g.setBrush( (QColorGroup::ColorRole)r, tmp);
	}
    }
    return s;
}


/*!
  \relates QPalette
  Writes a palette to the stream and returns a reference to the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QPalette &p )
{
    return s << p.active()
	     << p.disabled()
	     << p.inactive();
}


void readV1ColorGroup( QDataStream &s, QColorGroup &g,
		       QPalette::ColorGroup r )
{
    QColor fg, bg, light, dark, mid, text, base;
    s >> fg >> bg >> light >> dark >> mid >> text >> base;
    QPalette p( bg );
    QColorGroup n;
    switch ( r ) {
	case QPalette::Disabled:
	    n = p.disabled();
	    break;
	case QPalette::Inactive:
	    n = p.inactive();
	    break;
	default:
	    n = p.active();
	    break;
    }
    n.setColor( QColorGroup::Foreground, fg );
    n.setColor( QColorGroup::Light, light );
    n.setColor( QColorGroup::Dark, dark );
    n.setColor( QColorGroup::Mid, mid );
    n.setColor( QColorGroup::Text, text );
    n.setColor( QColorGroup::Base, base );
    g = n;
}


/*!
  \relates QPalette
  Reads a palette from the stream and returns a reference to the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QPalette &p )
{
    QColorGroup active, disabled, inactive;
    if ( s.version() == 1 ) {
	readV1ColorGroup( s, active, QPalette::Active );
	readV1ColorGroup( s, disabled, QPalette::Disabled );
	readV1ColorGroup( s, inactive, QPalette::Inactive );
    } else {
	s >> active >> disabled >> inactive;
    }
    QPalette newpal( active, disabled, inactive );
    p = newpal;
    return s;
}
#endif //QT_NO_DATASTREAM

/*!  Returns TRUE if this palette and \a p are copies of each other,
  ie. one of them was created as a copy of the other and neither was
  subsequently modified.  This is much stricter than equality.

  \sa operator=, operator==
*/

bool QPalette::isCopyOf( const QPalette & p )
{
    return data && data == p.data;
}

QBrush &QPalette::directBrush( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    if ( (uint)gr > (uint)QPalette::NColorGroups ) {
#if defined(CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorGroup(%i) out of range", gr );
#endif
	return data->normal.br[QColorGroup::Foreground];
    }
    if ( (uint)r >= (uint)QColorGroup::NColorRoles ) {
#if defined(CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorRole(%i) out of range", r );
#endif
	return data->normal.br[QColorGroup::Foreground];
    }
    switch( gr ) {
    case Normal:
    case Active:
	return data->active.br[r];
	//break;
    case Disabled:
	return data->disabled.br[r];
	//break;
    case Inactive:
	return data->inactive.br[r];
	//break;
    default:
	break;
    }
#if defined(CHECK_RANGE)
    qWarning( "QPalette::directBrush: colorGroup(%i) internal error", gr );
#endif
    return data->normal.br[QColorGroup::Foreground]; // Satisfy compiler
}


/*! \base64 palette.png

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

*/
#endif // QT_NO_PALETTE
