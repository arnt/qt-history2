/****************************************************************************
**
** Implementation of QColorGroup and QPalette classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpalette.h"

#ifndef QT_NO_PALETTE
#include "qdatastream.h"
#include "qcleanuphandler.h"

/*****************************************************************************
  QColorGroup member functions
 *****************************************************************************/

/*!
    \class QColorGroup qpalette.h
    \brief The QColorGroup class contains a group of widget colors.

    \ingroup appearance
    \ingroup graphics
    \ingroup images

    A color group contains a group of colors used by widgets for
    drawing themselves. We recommend that widgets use color group
    roles such as "foreground" and "base" rather than literal colors
    like "red" or "turquoise". The color roles are enumerated and
    defined in the \l ColorRole documentation.

    The most common use of QColorGroup is like this:

    \code
	QPainter p;
	...
	p.setPen( colorGroup().foreground() );
	p.drawLine( ... )
    \endcode

    It is also possible to modify color groups or create new color
    groups from scratch.

    The color group class can be created using three different
    constructors or by modifying one supplied by Qt. The default
    constructor creates an all-black color group, which can then be
    modified using set functions; there's also a constructor for
    specifying all the color group colors. And there is also a copy
    constructor.

    We strongly recommend using a system-supplied color group and
    modifying that as necessary.

    You modify a color group by calling the access functions
    setColor() and setBrush(), depending on whether you want a pure
    color or a pixmap pattern.

    There are also corresponding color() and brush() getters, and a
    commonly used convenience function to get each ColorRole:
    background(), foreground(), base(), etc.

    \sa QColor QPalette QWidget::colorGroup()
*/

/*!
    \enum QColorGroup::ColorRole

    The ColorRole enum defines the different symbolic color roles used
    in current GUIs.

    The central roles are:

    \value Background  general background color.

    \value Foreground  general foreground color.

    \value Base  used as background color for text entry widgets, for example;
    usually white or another light color.

    \value Text  the foreground color used with \c Base. Usually this
    is the same as the \c Foreground, in which case it must provide good
    contrast with \c Background and \c Base.

    \value Button  general button background color in which buttons need a
    background different from \c Background, as in the Macintosh style.

    \value ButtonText  a foreground color used with the \c Button color.

    There are some color roles used mostly for 3D bevel and shadow
    effects:

    \value Light  lighter than \c Button color.

    \value Midlight  between \c Button and \c Light.

    \value Dark  darker than \c Button.

    \value Mid  between \c Button and \c Dark.

    \value Shadow  a very dark color.
    By default, the shadow color is \c Qt::black.

    All of these are normally derived from \c Background and used in
    ways that depend on that relationship. For example, buttons depend
    on it to make the bevels look attractive, and Motif scroll bars
    depend on \c Mid to be slightly different from \c Background.

    Selected (marked) items have two roles:

    \value Highlight   a color to indicate a selected item or the
    current item. By default, the highlight color is \c Qt::darkBlue.

    \value HighlightedText  a text color that contrasts with \c Highlight.
    By default, the highlighted text color is \c Qt::white.

    Finally, there is a special role for text that needs to be
    drawn where \c Text or \c Foreground would give poor contrast,
    such as on pressed push buttons:

    \value BrightText a text color that is very different from \c
    Foreground and contrasts well with e.g. \c Dark.

    \value Link a text color used for unvisited hyperlinks.
    By default, the link color is \c Qt::blue.

    \value LinkVisited a text color used for already visited hyperlinks.
    By default, the linkvisited color is \c Qt::magenta.

    \value NColorRoles Internal.

    Note that text colors can be used for things other than just
    words; text colors are \e usually used for text, but it's quite
    common to use the text color roles for lines, icons, etc.

    This image shows most of the color roles in use:
    \img palette.png Color Roles
*/

QColorGroup::QColorGroupData *QColorGroup::shared_default = 0;


/*!
    Constructs a color group with all colors set to black.
*/
QColorGroup::QColorGroup()
{
    if (!shared_default) {
	static QCleanupHandler<QColorGroup::QColorGroupData> defColorGroupCleanup;
	shared_default = new QColorGroupData;
	shared_default->ref = 1;
	defColorGroupCleanup.add(&shared_default);
    }
    d = shared_default;
    ++d->ref;
}

/*!
    Constructs a color group that is an independent copy of \a other.
*/
QColorGroup::QColorGroup(const QColorGroup &other)
{
    d = other.d;
    ++d->ref;
}

/*!
    Copies the colors of \a other to this color group.
*/
QColorGroup& QColorGroup::operator =(const QColorGroup& other)
{
    QColorGroupData *x = other.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	delete x;
    return *this;
}

static QColor qt_mix_colors(const QColor &a, const QColor &b)
{
    return QColor((a.red() + b.red()) / 2, (a.green() + b.green()) / 2, (a.blue() + b.blue()) / 2);
}


/*!
    Constructs a color group. You can pass either brushes, pixmaps or
    plain colors for \a foreground, \a button, \a light, \a dark, \a
    mid, \a text, \a bright_text, \a base and \a background.

    \sa QBrush
*/
 QColorGroup::QColorGroup(const QBrush &foreground, const QBrush &button,
			  const QBrush &light, const QBrush &dark,
			  const QBrush &mid, const QBrush &text,
			  const QBrush &bright_text, const QBrush &base,
			  const QBrush &background)
{
    d = new QColorGroupData;
    d->ref = 1;
    d->br[Foreground] = foreground;
    d->br[Button] = button;
    d->br[Light] = light;
    d->br[Dark] = dark;
    d->br[Mid] = mid;
    d->br[Text] = text;
    d->br[BrightText] = bright_text;
    d->br[ButtonText] = text;
    d->br[Base] = base;
    d->br[Background] = background;
    d->br[Midlight] = qt_mix_colors(d->br[Button].color(), d->br[Light].color());
    d->br[Shadow] = Qt::black;
    d->br[Highlight] = Qt::darkBlue;
    d->br[HighlightedText] = Qt::white;
    d->br[Link] = Qt::blue;
    d->br[LinkVisited] = Qt::magenta;
}


/*!\obsolete

  Constructs a color group with the specified colors. The button
  color will be set to the background color.
*/
QColorGroup::QColorGroup(const QColor &foreground, const QColor &background,
			 const QColor &light, const QColor &dark,
			 const QColor &mid, const QColor &text, const QColor &base)
{
    d = new QColorGroupData;
    d->ref = 1;
    d->br[Foreground] = QBrush(foreground);
    d->br[Button] = QBrush(background);
    d->br[Light] = QBrush(light);
    d->br[Dark] = QBrush(dark);
    d->br[Mid] = QBrush(mid);
    d->br[Text] = QBrush(text);
    d->br[BrightText] = d->br[Light];
    d->br[ButtonText] = d->br[Text];
    d->br[Base] = QBrush(base);
    d->br[Background] = QBrush(background);
    d->br[Midlight] = qt_mix_colors(d->br[Button].color(), d->br[Light].color());
    d->br[Shadow] = Qt::black;
    d->br[Highlight] = Qt::darkBlue;
    d->br[HighlightedText] = Qt::white;
    d->br[Link] = Qt::blue;
    d->br[LinkVisited] = Qt::magenta;
}

/*!
    Destroys the color group.
*/
QColorGroup::~QColorGroup()
{
    if (!--d->ref)
	delete d;
}

/*!
   \fn const QColor &QColorGroup::color( ColorRole r ) const
    Returns the color that has been set for color role \a r.

    \sa brush() ColorRole
 */

/*!
    \fn const QBrush &QColorGroup::brush( ColorRole r ) const
    Returns the brush that has been set for color role \a r.

    \sa color() setBrush() ColorRole
*/

/*!
    \fn void QColorGroup::setColor( ColorRole r, const QColor &c )
    Sets the brush used for color role \a r to a solid color \a c.

    \sa brush() setColor() ColorRole
*/

/*!
    Sets the brush used for color role \a r to \a b.

    \sa brush() setColor() ColorRole
*/
void QColorGroup::setBrush(ColorRole r, const QBrush &b)
{
    detach();
    d->br[r] = b;
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

    Returns the mid color of the color group.

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
    \fn const QColor & QColorGroup::link() const

    Returns the unvisited link text color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::linkVisited() const

    Returns the visited link text color of the color group.

    \sa ColorRole
*/

/*!
    \fn bool QColorGroup::operator!=( const QColorGroup &g ) const

    Returns TRUE if this color group is different from \a g; otherwise
    returns  FALSE.

    \sa operator==()
*/

/*!
    Returns TRUE if this color group is equal to \a g; otherwise
    returns FALSE.

    \sa operator!=()
*/
bool QColorGroup::operator==( const QColorGroup &g ) const
{
    if ( d == g.d )
	return true;
    for (int r = 0; r < NColorRoles; ++r)
	if (d->br[r] != g.d->br[r])
	    return false;
    return true;
}

void QColorGroup::detach_helper()
{
    QColorGroupData *x = new QColorGroupData;
    x->ref = 1;
    for (int i = 0; i < NColorRoles; ++i)
	x->br[i] = d->br[i];
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	delete x;
}
/*****************************************************************************
  QPalette member functions
 *****************************************************************************/

/*!
    \class QPalette qpalette.h

    \brief The QPalette class contains color groups for each widget state.

    \ingroup appearance
    \ingroup shared
    \ingroup graphics
    \ingroup images
    \mainclass

    A palette consists of three color groups: \e active, \e disabled,
    and \e inactive. All widgets contain a palette, and all widgets in
    Qt use their palette to draw themselves. This makes the user
    interface easily configurable and easier to keep consistent.

    If you create a new widget we strongly recommend that you use the
    colors in the palette rather than hard-coding specific colors.

    The color groups:
    \list
    \i The active() group is used for the window that has keyboard focus.
    \i The inactive() group is used for other windows.
    \i The disabled() group is used for widgets (not windows) that are
    disabled for some reason.
    \endlist

    Both active and inactive windows can contain disabled widgets.
    (Disabled widgets are often called \e inaccessible or \e{grayed
    out}.)

    In Motif style, active() and inactive() look the same. In Windows
    2000 style and Macintosh Platinum style, the two styles look
    slightly different.

    There are setActive(), setInactive(), and setDisabled() functions
    to modify the palette. (Qt also supports a normal() group; this is
    an obsolete alias for active(), supported for backwards
    compatibility.)

    Colors and brushes can be set for particular roles in any of a
    palette's color groups with setColor() and setBrush().

    You can copy a palette using the copy constructor and test to see
    if two palettes are \e identical using isCopyOf().

    \sa QApplication::setPalette(), QWidget::setPalette(), QColorGroup, QColor
*/

/*!
    \enum QPalette::ColorGroup

    \value Disabled
    \value Active
    \value Inactive
    \value NColorGroups
    \value Normal synonym for Active
*/

/*!
    \obsolete

    \fn const QColorGroup &QPalette::normal() const

    Returns the active color group. Use active() instead.

    \sa setActive() active()
*/

/*!
    \obsolete

    \fn void QPalette::setNormal( const QColorGroup & cg )

    Sets the active color group to \a cg. Use setActive() instead.

    \sa setActive() active()
*/


int QPalette::palette_count = 0;
QPalette::QPaletteData *QPalette::shared_default = 0;

/*!
    Constructs a palette that consists of color groups with only black
    colors.
*/
QPalette::QPalette()
{
    if (!shared_default) {
	shared_default = new QPaletteData;
	shared_default->ref = 1;
	shared_default->ser_no = palette_count++;
	static QCleanupHandler<QPaletteData> defPalCleanup;
	defPalCleanup.add(&shared_default);
    }
    d = shared_default;
    ++d->ref;
}

/*!\obsolete
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. Background will be
  the button color as well.
*/
QPalette::QPalette( const QColor &button )
{
    d = new QPaletteData;
    d->ref = 1;
    d->ser_no = palette_count++;
    QColor bg = button,
	   btn = button,
	   fg,
	   base,
	   disfg;
    int h, s, v;
    bg.hsv( &h, &s, &v );
    if ( v > 128 ) {
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    d->active   = QColorGroup(fg, btn, btn.light(150), btn.dark(), btn.dark(150), fg,
			      Qt::white, base, bg );
    d->disabled = QColorGroup(disfg, btn, btn.light(150), btn.dark(), btn.dark(150), disfg,
			      Qt::white, base, bg );
    d->inactive = d->active;
}

/*!
    Constructs a palette from a \a button color and a \a background.
    The other colors are automatically calculated, based on these
    colors.
*/
QPalette::QPalette( const QColor &button, const QColor &background )
{
    d = new QPaletteData;
    d->ref = 1;
    d->ser_no = palette_count++;
    QColor bg = background,
	   btn = button,
	   fg,
	   base,
	   disfg;
    int h, s, v;
    bg.hsv(&h, &s, &v);
    if ( v > 128 ) {
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    d->active   = QColorGroup(fg, btn, btn.light(150), btn.dark(), btn.dark(150), fg,
			      Qt::white, base, bg );
    d->disabled = QColorGroup(disfg, btn, btn.light(150), btn.dark(), btn.dark(150), disfg,
			      Qt::white, base, bg );
    d->inactive = d->active;
}

/*!
    Constructs a palette that consists of the three color groups \a
    active, \a disabled and \a inactive. See the \link #details
    Detailed Description\endlink for definitions of the color groups
    and \l QColorGroup::ColorRole for definitions of each color role
    in the three groups.

    \sa QColorGroup QColorGroup::ColorRole QPalette
*/
QPalette::QPalette(const QColorGroup &active, const QColorGroup &disabled,
		   const QColorGroup &inactive)
{
    d = new QPaletteData;
    d->ref = 1;
    d->ser_no = palette_count++;
    d->active = active;
    d->disabled = disabled;
    d->inactive = inactive;
}

/*!
    Constructs a copy of \a p.

    This constructor is fast (it uses copy-on-write).
*/
QPalette::QPalette(const QPalette &p)
{
    d = p.d;
    ++d->ref;
}

/*!
    Destroys the palette.
*/
QPalette::~QPalette()
{
    if (!--d->ref)
	delete d;
}

/*!
    Assigns \a p to this palette and returns a reference to this
    palette.

    This is fast (it uses copy-on-write).

    \sa copy()
*/
QPalette &QPalette::operator=(const QPalette &p)
{
    QPaletteData *x = p.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	delete x;
    return *this;
}

/*!
    \fn const QColor &QPalette::color(ColorGroup gr, QColorGroup::ColorRole r) const
    Returns the color in color group \a gr, used for color role \a r.

    \sa brush() setColor() QColorGroup::ColorRole
*/

/*!
    \fn const QBrush &QPalette::brush( ColorGroup gr, QColorGroup::ColorRole r ) const
    Returns the brush in color group \a gr, used for color role \a r.

    \sa color() setBrush() QColorGroup::ColorRole
*/

/*!
    \fn void QPalette::setColor( ColorGroup gr, QColorGroup::ColorRole r, const QColor &c)
    Sets the brush in color group \a gr, used for color role \a r, to
    the solid color \a c.

    \sa setBrush() color() QColorGroup::ColorRole
*/

/*!
    Sets the brush in color group \a gr, used for color role \a r, to
    \a b.

    \sa brush() setColor() QColorGroup::ColorRole
*/
void QPalette::setBrush( ColorGroup gr, QColorGroup::ColorRole r,
			 const QBrush &b)
{
    detach();
    d->ser_no = palette_count++;
    directSetBrush(gr, r, b);
}

/*!
    \overload

    \fn void QPalette::setColor( QColorGroup::ColorRole r, const QColor &c )
    Sets the brush color used for color role \a r to color \a c in all
    three color groups.

    \sa color() setBrush() QColorGroup::ColorRole
*/

/*!
    \overload

    Sets the brush in for color role \a r in all three color groups to
    \a b.

    \sa brush() setColor() QColorGroup::ColorRole active() inactive() disabled()
*/
void QPalette::setBrush(QColorGroup::ColorRole r, const QBrush &b)
{
    detach();
    d->ser_no = palette_count++;
    directSetBrush(Active, r, b);
    directSetBrush(Disabled, r, b);
    directSetBrush(Inactive, r, b);
}

/*!
    \fn QPalette QPalette::copy() const
    \obsolete
    Returns a deep copy of this palette.

    \warning This is slower than the copy constructor and assignment
    operator and offers no benefits.
*/

/*!
    \fn void QPalette::detach()
    Detaches this palette from any other QPalette objects with which
    it might implicitly share QColorGroup objects. In essence, does
    the copying part of copy-on-write.

    Calling this should generally not be necessary; QPalette calls it
    itself when necessary.
*/
void QPalette::detach_helper()
{
    QPaletteData *x = new QPaletteData;
    x->ref = 1;
    x->disabled = d->disabled;
    x->active = d->active;
    x->inactive = d->inactive;
    x->ser_no = palette_count++;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	delete x;
}

/*!
    \fn const QColorGroup & QPalette::disabled() const

    Returns the disabled color group of this palette.

    \sa QColorGroup, setDisabled(), active(), inactive()
*/

/*!
    Sets the \c Disabled color group to \a g.

    \sa disabled() setActive() setInactive()
*/

void QPalette::setDisabled(const QColorGroup &g)
{
    detach();
    d->ser_no = palette_count++;
    d->disabled = g;
}

/*!
    \fn const QColorGroup & QPalette::active() const

    Returns the active color group of this palette.

    \sa QColorGroup, setActive(), inactive(), disabled()
*/

/*!
    Sets the \c Active color group to \a g.

    \sa active() setDisabled() setInactive() QColorGroup
*/

void QPalette::setActive(const QColorGroup &g)
{
    detach();
    d->ser_no = palette_count++;
    d->active = g;
}

/*!
    \fn const QColorGroup & QPalette::inactive() const

    Returns the inactive color group of this palette.

    \sa QColorGroup,  setInactive(), active(), disabled()
*/

/*!
    Sets the \c Inactive color group to \a g.

    \sa active() setDisabled() setActive() QColorGroup
*/

void QPalette::setInactive(const QColorGroup &g)
{
    detach();
    d->ser_no = palette_count++;
    d->inactive = g;
}

/*!
    \fn bool QPalette::operator!=( const QPalette &p ) const

    Returns TRUE (slowly) if this palette is different from \a p;
    otherwise returns FALSE (usually quickly).
*/

/*!
    Returns TRUE (usually quickly) if this palette is equal to \a p;
    otherwise returns FALSE (slowly).
*/

bool QPalette::operator==(const QPalette &p) const
{
    return d->active == p.d->active && d->disabled == p.d->disabled && d->inactive == p.d->inactive;
}

/*!
    \fn int QPalette::serialNumber() const

    Returns a number that uniquely identifies this QPalette object.
    The serial number is intended for caching. Its value may not be
    used for anything other than equality testing.

    Note that QPalette uses copy-on-write, and the serial number
    changes during the lazy copy operation (detach()), not during a
    shallow copy (copy constructor or assignment).

    \sa QPixmap QPixmapCache QCache
*/

/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \relates QColorGroup

    Writes color group, \a g to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QColorGroup &g)
{
    if (s.version() == 1) {
	s << g.foreground()
	  << g.background()
	  << g.light()
	  << g.dark()
	  << g.mid()
	  << g.text()
	  << g.base();
    } else {
	int max = QColorGroup::NColorRoles;
	if (s.version() <= 3)
	    max = 14;
	for (int r = 0 ; r < max ; ++r)
	    s << g.brush((QColorGroup::ColorRole)r);
    }
    return s;
}

/*!
    \related QColorGroup

    Reads a color group from the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QColorGroup &g)
{
    if (s.version() == 1) {
	QColor fg, bg, light, dark, mid, text, base;
	s >> fg >> bg >> light >> dark >> mid >> text >> base;
	QPalette p(bg);
	QColorGroup n(p.active());
	n.setColor(QColorGroup::Foreground, fg);
	n.setColor(QColorGroup::Light, light);
	n.setColor(QColorGroup::Dark, dark);
	n.setColor(QColorGroup::Mid, mid);
	n.setColor(QColorGroup::Text, text);
	n.setColor(QColorGroup::Base, base);
	g = n;
    } else {
	int max = QColorGroup::NColorRoles;
	if (s.version() <= 3)
	    max = 14;

	QBrush tmp;
	for(int r = 0 ; r < max; ++r) {
	    s >> tmp;
	    g.setBrush((QColorGroup::ColorRole)r, tmp);
	}
    }
    return s;
}

/*!
    \relates QPalette

    Writes the palette, \a p to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPalette &p)
{
    return s << p.active()
	     << p.disabled()
	     << p.inactive();
}

static void readV1ColorGroup(QDataStream &s, QColorGroup &g, QPalette::ColorGroup r)
{
    QColor fg, bg, light, dark, mid, text, base;
    s >> fg >> bg >> light >> dark >> mid >> text >> base;
    QPalette p(bg);
    QColorGroup n;
    switch (r) {
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
    n.setColor(QColorGroup::Foreground, fg);
    n.setColor(QColorGroup::Light, light);
    n.setColor(QColorGroup::Dark, dark);
    n.setColor(QColorGroup::Mid, mid);
    n.setColor(QColorGroup::Text, text);
    n.setColor(QColorGroup::Base, base);
    g = n;
}

/*!
    \relates QPalette

    Reads a palette from the stream, \a s into the palette \a p, and
    returns a reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPalette &p)
{
    QColorGroup active, disabled, inactive;
    if ( s.version() == 1 ) {
	readV1ColorGroup(s, active, QPalette::Active);
	readV1ColorGroup(s, disabled, QPalette::Disabled);
	readV1ColorGroup(s, inactive, QPalette::Inactive);
    } else {
	s >> active >> disabled >> inactive;
    }
    QPalette newpal(active, disabled, inactive);
    p = newpal;
    return s;
}
#endif //QT_NO_DATASTREAM

/*!
    Returns TRUE if this palette and \a p are copies of each other,
    i.e. one of them was created as a copy of the other and neither
    was subsequently modified; otherwise returns FALSE. This is much
    stricter than equality.

    \sa operator=() operator==()
*/

bool QPalette::isCopyOf(const QPalette &p)
{
    return d == p.d;
}

const QBrush &QPalette::directBrush(ColorGroup gr, QColorGroup::ColorRole r) const
{
    switch (gr) {
    case Active:
	return d->active.d->br[r];
    case Disabled:
	return d->disabled.d->br[r];
    case Inactive:
	return d->inactive.d->br[r];
    default:
	return d->active.d->br[QColorGroup::Foreground];
    }
}

void QPalette::directSetBrush(ColorGroup gr, QColorGroup::ColorRole r, const QBrush& b)
{
    switch (gr) {
    case Active:
	d->active.setBrush(r, b);
	break;
    case Disabled:
	d->disabled.setBrush(r, b);
	break;
    case Inactive:
	d->inactive.setBrush(r, b);
	break;
    default:
	break;
    }
}

/*!\internal*/
QColorGroup::ColorRole QPalette::foregroundRoleFromMode(Qt::BackgroundMode mode)
{
    switch (mode) {
    case Qt::PaletteButton:
	return QColorGroup::ButtonText;
    case Qt::PaletteBase:
	return QColorGroup::Text;
    case Qt::PaletteDark:
    case Qt::PaletteShadow:
	return QColorGroup::Light;
    case Qt::PaletteHighlight:
	return QColorGroup::HighlightedText;
    case Qt::PaletteBackground:
    default:
	return QColorGroup::Foreground;
    }
}

/*!\internal*/
QColorGroup::ColorRole QPalette::backgroundRoleFromMode(Qt::BackgroundMode mode)
{
    switch (mode) {
    case Qt::PaletteForeground:
	return QColorGroup::Foreground;
    case Qt::PaletteButton:
	return QColorGroup::Button;
    case Qt::PaletteLight:
	return QColorGroup::Light;
    case Qt::PaletteMidlight:
	return QColorGroup::Midlight;
    case Qt::PaletteDark:
	return QColorGroup::Dark;
    case Qt::PaletteMid:
	return QColorGroup::Mid;
    case Qt::PaletteText:
	return QColorGroup::Text;
    case Qt::PaletteBrightText:
	return QColorGroup::BrightText;
    case Qt::PaletteButtonText:
	return QColorGroup::ButtonText;
    case Qt::PaletteBase:
	return QColorGroup::Base;
    case Qt::PaletteShadow:
	return QColorGroup::Shadow;
    case Qt::PaletteHighlight:
	return QColorGroup::Highlight;
    case Qt::PaletteHighlightedText:
	return QColorGroup::HighlightedText;
    case Qt::PaletteLink:
	return QColorGroup::Link;
    case Qt::PaletteLinkVisited:
	return QColorGroup::LinkVisited;
    case Qt::PaletteBackground:
    default:
	return QColorGroup::Background;
    }
}
#endif // QT_NO_PALETTE
