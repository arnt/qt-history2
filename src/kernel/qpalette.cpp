/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpalette.cpp#42 $
**
** Implementation of QColorGroup and QPalette classes
**
** Created : 950323
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpalette.h"
#include "qdatastream.h"
#include "qpixmap.h"

/*****************************************************************************
  QColorGroup member functions
 *****************************************************************************/

/*!
  \class QColorGroup qpalette.h
  \brief The QColorGroup class contains a group of widget colors.

  \ingroup color
  \ingroup drawing

  A color group contains a group of colors used by widgets for drawing
  themselves.  Widgets should not use colors like "red" and "turqoise"
  but rather "foreground" and "base", where possible.

  We have identified fourty-two distinct color roles:
  <ol>
  <li>Foreground (graphics foreground color)
  <li>Button (general button color)
  <li>Light (lighter than button color, for shadow effects)
  <li>Midlight (between Button and Light, for shadow effects)
  <li>Dark (darker than the button color, for shadow effects)
  <li>Medium (between button color  and dark, used for shadow and contrast
    effects)
  <li>Text (usually the same as the foreground color, but sometimes text
    and other foreground are not the same)
  <li>BrightText (a text color that contrasts to the Dark color)
  <li>ButtonText (a text color that contrasts to the Button  color)
  <li>Base (used as background color for some widgets). Usually white or
    another light color.
  <li>Background (general background color)
  <li>Shadow (a very dark color used for shadow effects, usually black)
  <li>Highlight  (a color to indicate a selected or highlighted item)
  <li>HighlightedText  (a text color that contrasts to Highlight)
  </ol>

  A QPalette contains three color groups.

  The current widget color group is returned by QWidget::colorGroup().

  \sa QColor, QPalette
*/


/*!
  Constructs a color group with all colors set to black.
*/

QColorGroup::QColorGroup()
{						// all colors become black
}

/*!
  Constructs a color group that is an independent copy of another color group.
*/
QColorGroup::QColorGroup(const QColorGroup& other)
{
    for (int i=0; i<=MaxColorRole; i++)
	br[i] = other.br[i];
}

/*!
  Copies the colours of \a other to this color group.
*/
QColorGroup& QColorGroup::operator =(const QColorGroup& other)
{
    for (int i=0; i<=MaxColorRole; i++)
	br[i] = other.br[i];
    return *this;
}

/*!
Constructs a color group. You can pass either brushes, pixmaps or
plain colors for each parameter.

\sa QBrush
*/
 QColorGroup::QColorGroup( const QBrush &foreground, const QBrush &button,
	  const QBrush &light, const QBrush &dark, const QBrush &mid,
	  const QBrush &text,  const QBrush &bright_text, const QBrush &base,
	  const QBrush &background)
 {
     br[Foreground]      = foreground;
     br[Button] 	 = button;
     br[Light] 		 = light;
     br[Dark] 		 = dark;
     br[Mid] 		 = mid;
     br[Text] 		 = text;
     br[BrightText] 	 = bright_text;
     br[ButtonText] 	 = text;
     br[Base] 		 = base;
     br[Background] 	 = background;
     br[Midlight] 	 = QBrush( br[Button].color().light(115));
     br[Shadow]          = Qt::black;
     br[Highlight]       = Qt::darkBlue;
     br[HighlightedText] = Qt::white;
 }


/*!\obsolete
  Constructs a color group with the specified colors. The background
  color will be set to the button color.
*/

QColorGroup::QColorGroup( const QColor &foreground, const QColor &button,
			  const QColor &light, const QColor &dark,
			  const QColor &mid,
			  const QColor &text, const QColor &base )
{
    br[Foreground]      = QBrush(foreground);
    br[Button]          = QBrush(button);
    br[Light]           = QBrush(light);
    br[Dark]            = QBrush(dark);
    br[Mid]             = QBrush(mid);
    br[Text]            = QBrush(text);
    br[BrightText]      = br[Text];
    br[ButtonText]      = br[Text];
    br[Base]            = QBrush(base);
    br[Background]      = QBrush(button);
    br[Midlight]        = QBrush(br[Button].color().light(115));
    br[Shadow]          = Qt::black;
    br[Highlight]       = Qt::darkBlue;
    br[HighlightedText] = Qt::white;
}

/*!
  Destroys the color group.
*/

QColorGroup::~QColorGroup()
{
}

/*!
  Returns the color that has been set for color role \a r.
  \sa brush()
 */
const QColor &QColorGroup::color( ColorRole r ) const
{
    return br[r].color();
}

/*!
  Returns the brush that has been set for color role \a r.
*/
const QBrush &QColorGroup::brush( ColorRole r ) const
{
    return br[r];
}

/*!
  Sets the brush used for color role \a r to a solid color \a c.
*/
void QColorGroup::setColor( ColorRole r, const QColor &c )
{
    setBrush( r, QBrush(c) );
}

/*!
  Sets the brush used for color role \a r to \a b.
*/
void QColorGroup::setBrush( ColorRole r, const QBrush &b )
{
    br[r] = b;
}


/*!
  \fn const QColor & QColorGroup::foreground() const
  Returns the foreground color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::button() const
  Returns the button color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::light() const
  Returns the light color of the color group.
*/

/*!
  \fn const QColor& QColorGroup::midlight() const
  Returns the midlight color of the color group. Currently, this is
  a lightened version of the button color, but this may change
  in the future, to return a <tt>const QColor &</tt> from the
  palette.
*/

/*!
  \fn const QColor & QColorGroup::dark() const
  Returns the dark color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::mid() const
  Returns the medium color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::text() const
  Returns the text foreground color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::brightText() const
  Returns the bright text foreground color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::buttonText() const
  Returns the button text foreground color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::base() const
  Returns the base color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::background() const
  Returns the background color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::shadow() const
  Returns the shadow color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::highlight() const
  Returns the highlight color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::highlightedText() const
  Returns the highlighted text color of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillForeground() const
  Returns the foreground brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillButton() const
  Returns the button brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillLight() const
  Returns the light brush of the color group.
*/

/*!
  \fn const QBrush& QColorGroup::fillMidlight() const
  Returns the midlight brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillDark() const
  Returns the dark brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillMid() const
  Returns the medium brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillText() const
  Returns the text foreground brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillBrightText() const
  Returns the bright text foreground brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillButtonText() const
  Returns the button text foreground brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillBase() const
  Returns the base brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillBackground() const
  Returns the background brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillShadow() const
  Returns the shadow brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillHighlight() const
  Returns the highlight brush of the color group.
*/

/*!
  \fn const QBrush & QColorGroup::fillHighlightedText() const
  Returns the highlighted text brush of the color group.
*/

/*!
  \fn bool QColorGroup::operator!=( const QColorGroup &g ) const
  Returns TRUE if this color group is different from \e g, or FALSE if
  it is equal to \e g.
  \sa operator!=()
*/


/*!
  \fn void QColorGroup::setForeground( const QBrush& b)
  Sets the foreground brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setButton( const QBrush& b)
  Sets the button brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setLight( const QBrush& b)
  Sets the light brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setMidlight( const QBrush& b)
  Sets the midlight brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setDark( const QBrush& b)
  Sets the dark brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setMid( const QBrush& b)
  Sets the medium brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setText( const QBrush& b)
  Sets the text foreground brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setBrightText( const QBrush& b)
  Sets the bright text foreground brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setButtonText( const QBrush& b)
  Sets the button text foreground brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setBase( const QBrush& b)
  Sets the base brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setBackground( const QBrush& b)
  Sets the background brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setShadow( const QBrush& b)
  Sets the shadow brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setHighlight( const QBrush& b)
  Sets the highlight brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/

/*!
  \fn void QColorGroup::setHighlightedText( const QBrush& b)
  Sets the highlighted text brush of the color group. Note that you can also pass a plain QColor instead of a QBrush.
*/


/*!
  Returns TRUE if this color group is equal to \e g, or FALSE if
  it is different from \e g.
  \sa operator==()
*/

bool QColorGroup::operator==( const QColorGroup &g ) const
{
    for( int r = 0 ; r < MaxColorRole + 1 ; r++ )
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

  \ingroup color
  \ingroup shared
  \ingroup drawing

  A palette consists of three color groups: a \e normal, a \e disabled
  and an \e active color group.	 All \link QWidget widgets\endlink
  contain a palette, and all the widgets in Qt use their palette to draw
  themselves.  This makes the user interface consistent and easily
  configurable.

  If you make a new widget you are strongly advised to use the colors in
  the palette rather than hard-coding specific colors.

  The \e active group is used for the widget in focus.	Normally it
  contains the same colors as \e normal so as not to overwhelm the user
  with bright and flashing colors, but if you need to you can change it.

  The \e disabled group is used for widgets that are currently
  inactive or not usable.

  The \e normal color group is used in all other cases.

  \sa QApplication::setPalette(), QWidget::setPalette(), QColorGroup, QColor
*/


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
}

/*!
  Constructs a palette from a \e button color and a background. The other colors are
  automatically calculated, based on this color.
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
}

/*!
  Constructs a palette that consists of the three color groups \e normal,
  \e disabled and \e active.
*/

QPalette::QPalette( const QColorGroup &normal, const QColorGroup &disabled,
		    const QColorGroup &active )
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no   = palette_count++;
    data->normal   = normal;
    data->disabled = disabled;
    data->active   = active;
}

/*!
  Constructs a palette that is a
  \link shclass.html shallow copy\endlink of \e p.
  \sa copy()
*/

QPalette::QPalette( const QPalette &p )
{
    data = p.data;
    data->ref();
}

/*!
  Destroys the palette.
*/

QPalette::~QPalette()
{
    if ( data->deref() )
	delete data;
}

/*!
  Assigns \e p to this palette and returns a reference to this palette.
  Note that a \e shallow copy of \a p is used.
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
  \sa brush()
*/
const QColor &QPalette::color( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    return directBrush( gr, r ).color();
}

/*!
  Returns the brush in \a gr used for color role \a r.
*/
const QBrush &QPalette::brush( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    return directBrush( gr, r );
}

/*!
  Sets the brush in \a gr used for color role \a r to the solid color \a c.
*/
void QPalette::setColor(ColorGroup gr,QColorGroup::ColorRole r,const QColor &c)
{
    setBrush( gr, r, QBrush(c) );
}

/*!
  Sets the brush in \a gr used for color role \a r to \a b.
*/
void QPalette::setBrush(ColorGroup gr,QColorGroup::ColorRole r,const QBrush &b)
{
    detach();
    data->ser_no = palette_count++;
    directBrush( gr, r ) = b;
}

/*!
  Sets the color of the brush in \a gr used for color role \a r to \a c.
*/
void QPalette::setColor( QColorGroup::ColorRole r, const QColor &c )
{
    setBrush( r, QBrush(c) );
}

/*!
  Sets the brush in all color groups (Normal, Disabled, and Active)
  that is used for color role \a r to \a b.
*/
void QPalette::setBrush( QColorGroup::ColorRole r, const QBrush &b )
{
    detach();
    data->ser_no = palette_count++;
    directBrush( Normal,   r ) = b;
    directBrush( Disabled, r ) = b;
    directBrush( Active,   r ) = b;
}


/*!
  Returns a
  \link shclass.html deep copy\endlink of the palette.
*/

QPalette QPalette::copy() const
{
    QPalette p( data->normal, data->disabled, data->active );
    return p;
}


/*!
  Detaches this palette from any other QPalette objects with which
  it might implicitly share \link QColorGroup QColorGroups. \endlink

  Calling this should generally not be necessary; QPalette calls this
  itself when necessary.
*/

void QPalette::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  \fn const QColorGroup & QPalette::normal() const
  Returns the normal color group of this palette.
  \sa QColorGroup, disabled(), active(), setNormal()
*/

/*!
  Sets the \c normal color group to \e g.
  \sa normal()
*/

void QPalette::setNormal( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->normal = g;
}

/*!
  \fn const QColorGroup & QPalette::disabled() const
  Returns the disabled color group of this palette.
  \sa QColorGroup, normal(), active(), setDisabled()
*/

/*!
  Sets the \c disabled color group to \e g.
  \sa disabled()
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
  \sa QColorGroup, normal(), disabled(), setActive()
*/

/*!
  Sets the \c active color group to \e g.
  \sa active()
*/

void QPalette::setActive( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->active = g;
}


/*!
  \fn bool QPalette::operator!=( const QPalette &p ) const
  Returns TRUE if this palette is different from \e p, or FALSE if they
  are equal.
*/

/*!
  Returns TRUE if this palette is equal to \e p, or FALSE if they
  are different.
*/

bool QPalette::operator==( const QPalette &p ) const
{
    return data->normal == p.data->normal &&
	   data->disabled == p.data->disabled &&
	   data->active == p.data->active;
}


/*!
  \fn int QPalette::serialNumber() const

  Returns a number that uniquely identifies this QPalette object. The
  serial number is very useful for caching.

  \sa QPixmap, QPixmapCache
*/


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

/*!
  \relates QColorGroup
  Writes a color group to the stream.

  Serialization format:
  <ol>
  <li> QBrush foreground
  <li> QBrush button
  <li> QBrush light
  <li> QBrush midLight
  <li> QBrush dark
  <li> QBrush mid
  <li> QBrush text
  <li> QBrush brightText
  <li> QBrush ButtonText
  <li> QBrush base
  <li> QBrush background
  <li> QBrush shadow
  <li> QBrush highlight
  <li> QBrush highlightedText
  </ol>
  The colors are serialized in the listed order.
*/

QDataStream &operator<<( QDataStream &s, const QColorGroup &g )
{
    for( int r = 0 ; r < QColorGroup::MaxColorRole + 1 ; r++ )
	s << g.brush( (QColorGroup::ColorRole)r);
    return s;
}

/*!
  \related QColorGroup
  Reads a color group from the stream.
*/

QDataStream &operator>>( QDataStream &s, QColorGroup &g )
{
    QBrush tmp;
    for( int r = 0 ; r < QColorGroup::MaxColorRole + 1 ; r++ ) {
	s >> tmp;
	g.setBrush( (QColorGroup::ColorRole)r, tmp);
    }
    return s;
}


/*!
  \relates QPalette
  Writes a palette to the stream and returns a reference to the stream.

  Serialization format:
  <ol>
  <li> QColorGroup normal
  <li> QColorGroup disabled
  <li> QColorGroup active
  </ol>
  The color groups are serialized in the listed order.
*/

QDataStream &operator<<( QDataStream &s, const QPalette &p )
{
    return s << p.normal()
	     << p.disabled()
	     << p.active();
}

/*!
  \relates QPalette
  Reads a palette from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPalette &p )
{
    QColorGroup normal, disabled, active;
    s >> normal >> disabled >> active;
    QPalette newpal( normal, disabled, active );
    p = newpal;
    return s;
}


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
    if ( (uint)gr > (uint)QPalette::MaxColorGroup ) {
#if defined(CHECK_RANGE)
	warning( "QPalette::directBrush: colorGroup(%i) out of range", gr );
#endif
	return data->normal.br[QColorGroup::Foreground];
    }
    if ( (uint)r > (uint)QColorGroup::MaxColorRole ) {
#if defined(CHECK_RANGE)
	warning( "QPalette::directBrush: colorRole(%i) out of range", r );
#endif
	return data->normal.br[QColorGroup::Foreground];
    }
    switch( gr ) {
    case Normal:
	return data->normal.br[r];
	break;
    case Disabled:
	return data->disabled.br[r];
	break;
    case Active:
	return data->active.br[r];
	break;
    };
#if defined(CHECK_RANGE)
    warning( "QPalette::directBrush: colorGroup(%i) internal error", gr );
#endif
    return data->normal.br[QColorGroup::Foreground]; // Satisfy compiler
}
