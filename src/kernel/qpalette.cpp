/****************************************************************************
**
** Implementation of QPalette classes.
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

static QColor qt_mix_colors(QColor a, QColor b)
{
    return QColor((a.red() + b.red()) / 2, (a.green() + b.green()) / 2,
		   (a.blue() + b.blue()) / 2);
}

#ifndef QT_NO_COMPAT
void QColorGroup::init()
{
    is_colorgroup = 1;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &s, const QColorGroup &g)
{
    if(s.version() == 1) {
	// Qt 1.x
	s << g.foreground() << g.background() << g.light()
	  << g.dark() << g.mid() << g.text() << g.base();
    } else {
	int max = QPalette::NColorRoles;
	if(s.version() <= 3) // Qt 2.x
	    max = 14;
	for(int r = 0 ; r < max ; r++)
	    s << g.brush((QPalette::ColorRole)r);
    }
    return s;
}

QDataStream &operator>>(QDataStream &s, QColorGroup &g)
{
    if(s.version() == 1) { 	// Qt 1.x
	QColor fg, bg, light, dark, mid, text, base;
	s >> fg >> bg >> light >> dark >> mid >> text >> base;
	QPalette p(bg);
	QColorGroup n(p.active());
	n.setColor(QPalette::Foreground, fg);
	n.setColor(QPalette::Light, light);
	n.setColor(QPalette::Dark, dark);
	n.setColor(QPalette::Mid, mid);
	n.setColor(QPalette::Text, text);
	n.setColor(QPalette::Base, base);
	g = n;
    } else {
	int max = QPalette::NColorRoles;
	if (s.version() <= 3) // Qt 2.x
	    max = 14;
	QBrush tmp;
	for(int r = 0 ; r < max; r++) {
	    s >> tmp;
	    g.setBrush((QPalette::ColorRole)r, tmp);
	}
    }
    return s;
}
#endif

/*!
    Constructs a palette with the specified \a active, \a disabled and
    \a inactive color groups.
*/
QPalette::QPalette(const QColorGroup &active, const QColorGroup &disabled,
		   const QColorGroup &inactive)
{
    init();
    setColorGroup(Active, active);
    setColorGroup(Disabled, disabled);
    setColorGroup(Inactive, inactive);
}

QColorGroup QPalette::createColorGroup(ColorGroup cr) const
{
    QColorGroup ret(*this);
    ret.setCurrentColorGroup(cr);
    return ret;
}

void QPalette::setColorGroup(ColorGroup cg, const QColorGroup &g)
{
    setColorGroup(cg, g.foreground(), g.button(), g.light(),
		  g.dark(), g.mid(), g.text(), g.brightText(), g.base(),
		  g.background(), g.midlight(), g.buttonText(), g.shadow(),
		  g.highlight(), g.highlightedText(), g.link(),
		  g.linkVisited());
}

#endif

/*!
   \fn const QColor &QPalette::color( ColorRole r ) const

   \overload

    Returns the color that has been set for color role \a r in the current ColorGroup.

    \sa brush() ColorRole
 */

/*!
    \fn const QBrush &QPalette::brush( ColorRole r ) const

    \overload

    Returns the brush that has been set for color role \a r in the current ColorGroup.

    \sa color() setBrush() ColorRole
*/

/*!
    \fn void QPalette::setColor( ColorRole r, const QColor &c )

    \overload

    Sets the brush used for color role \a r in the current ColorGroup to a solid color \a c.

    \sa brush() setColor() ColorRole
*/

/*!
    \fn const QColor & QPalette::setBrush(ColorRole cr, const QBrush &brush) const

    Sets the brush used for color role \a r for the current color group to \a b.

    \sa brush() setColor() ColorRole
*/

/*!
    \fn const QColor & QPalette::foreground() const

    Returns the foreground color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::button() const

    Returns the button color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::light() const

    Returns the light color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor& QPalette::midlight() const

    Returns the midlight color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::dark() const

    Returns the dark color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::mid() const

    Returns the mid color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::text() const

    Returns the text foreground color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::brightText() const

    Returns the bright text foreground color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::buttonText() const

    Returns the button text foreground color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::base() const

    Returns the base color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::background() const

    Returns the background color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::shadow() const

    Returns the shadow color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::highlight() const

    Returns the highlight color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::highlightedText() const

    Returns the highlighted text color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::link() const

    Returns the unvisited link text color of the current color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QPalette::linkVisited() const

    Returns the visited link text color of the current color group.

    \sa ColorRole
*/

/*!
    \class QPalette qpalette.h

    \brief The QPalette class contains color groups for each widget state.

    \ingroup appearance
    \ingroup shared
    \ingroup graphics
    \ingroup images
    \mainclass

    A palette consists of three color groups: \e Active, \e Disabled,
    and \e Inactive. All widgets contain a palette, and all widgets in
    Qt use their palette to draw themselves. This makes the user
    interface easily configurable and easier to keep consistent.

    If you create a new widget we strongly recommend that you use the
    colors in the palette rather than hard-coding specific colors.

    The color groups:
    \list
    \i The Active group is used for the window that has keyboard focus.
    \i The Inactive group is used for other windows.
    \i The Disabled group is used for widgets (not windows) that are
    disabled for some reason.
    \endlist

    Both active and inactive windows can contain disabled widgets.
    (Disabled widgets are often called \e inaccessible or \e{grayed
    out}.)

    In Motif style, Active and Inactive look the same. In Windows
    2000 style and Macintosh Platinum style, the two styles look
    slightly different.

    Colors and brushes can be set for particular roles in any of a
    palette's color groups with setColor() and setBrush().  A color
    group contains a group of colors used by widgets for drawing
    themselves. We recommend that widgets use color group roles such
    as "foreground" and "base" rather than literal colors like "red"
    or "turquoise". The color roles are enumerated and defined in the
    \l ColorRole documentation.

    We strongly recommend using a system-supplied color group and
    modifying that as necessary.

    You modify a color group by calling the access functions
    setColor() and setBrush(), depending on whether you want a pure
    color or a pixmap pattern.

    There are also corresponding color() and brush() getters, and a
    commonly used convenience function to get each ColorRole:
    background(), foreground(), base(), etc.

    You can copy a palette using the copy constructor and test to see
    if two palettes are \e identical using isCopyOf().

    \sa QApplication::setPalette(), QWidget::setPalette(), QColor
*/

/*!
    \enum QPalette::ColorGroup

    \value Current internal.
    \value Disabled
    \value Active
    \value Inactive
    \value NColorGroups internal.
    \value All internal.
    \value Normal synonym for Active
*/

/*!
    \enum QPalette::ColorRole

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

int QPalette::palette_count = 0;
QPalette::QPaletteData *QPalette::shared_default = 0;

/*!
    Constructs a palette that consists of color groups with only black
    colors.
*/
QPalette::QPalette()
{
    if(!shared_default) {
	shared_default = new QPaletteData;
	shared_default->ref = 1;
	shared_default->ser_no = palette_count++;
	static QCleanupHandler<QPaletteData> defPalCleanup;
	defPalCleanup.add(&shared_default);
    }
    d = shared_default;
    ++d->ref;
    current_group = Active; //as a default..
#ifndef QT_NO_COMPAT
    is_colorgroup = 0;
#endif
}

/*!\obsolete
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. Background will be
  the button color as well.
*/
QPalette::QPalette(const QColor &button)
{
    init();
    QColor bg = button,
	   btn = button,
	   fg,
	   base,
	   disfg;
    int h, s, v;
    bg.hsv(&h, &s, &v);
    if(v > 128) {
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    //inactive and active are the same..
    setColorGroup(Active, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
		  QBrush(btn.dark(150)), QBrush(fg), QBrush(Qt::white), QBrush(base), QBrush(bg) );
    setColorGroup(Inactive, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
		  QBrush(btn.dark(150)), QBrush(fg), QBrush(Qt::white), QBrush(base), QBrush(bg) );
    setColorGroup(Disabled, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
		  QBrush(btn.dark(150)), QBrush(disfg), QBrush(Qt::white), QBrush(base),
		  QBrush(bg));
}

/*!
    Constructs a palette. You can pass either brushes, pixmaps or
    plain colors for \a foreground, \a button, \a light, \a dark, \a
    mid, \a text, \a bright_text, \a base and \a background.

    \sa QBrush
*/
QPalette::QPalette(const QBrush &foreground, const QBrush &button,
		   const QBrush &light, const QBrush &dark,
		   const QBrush &mid, const QBrush &text,
		   const QBrush &bright_text, const QBrush &base,
		   const QBrush &background)
{
    init();
    setColorGroup(All, foreground, button, light, dark, mid, text, bright_text,
		  base, background);
}


/*!\obsolete

  Constructs a palette with the specified colors. The button
  color will be set to the background color.
*/
QPalette::QPalette(const QColor &foreground, const QColor &background,
		   const QColor &light, const QColor &dark, const QColor &mid,
		   const QColor &text, const QColor &base)
{
    init();
    setColorGroup(All, QBrush(foreground), QBrush(background), QBrush(light),
		  QBrush(dark), QBrush(mid), QBrush(text), QBrush(light),
		  QBrush(base), QBrush(background));
}

/*!
    Constructs a palette from a \a button color and a \a background.
    The other colors are automatically calculated, based on these
    colors.
*/
QPalette::QPalette(const QColor &button, const QColor &background)
{
    init();
    QColor bg = background, btn = button, fg, base, disfg;
    int h, s, v;
    bg.hsv(&h, &s, &v);
    if(v > 128) {
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    //inactive and active are identical
    setColorGroup(Inactive, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
		  QBrush(btn.dark(150)), QBrush(fg), QBrush(Qt::white), QBrush(base),
		  QBrush(bg));
    setColorGroup(Active, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
		  QBrush(btn.dark(150)), QBrush(fg), QBrush(Qt::white), QBrush(base),
		  QBrush(bg));
    setColorGroup(Disabled, QBrush(disfg), QBrush(btn), QBrush(btn.light(150)),
		  QBrush(btn.dark()), QBrush(btn.dark(150)), QBrush(disfg),
		  QBrush(Qt::white), QBrush(base), QBrush(bg));
}

/*!
    Constructs a copy of \a p.

    This constructor is fast (it uses copy-on-write).
*/
QPalette::QPalette(const QPalette &p)
{
    d = p.d;
    ++d->ref;
#ifndef QT_NO_COMPAT
    is_colorgroup = p.is_colorgroup;
#endif
    current_group = p.current_group;
}

/*!
    Destroys the palette.
*/
QPalette::~QPalette()
{
    if(!--d->ref)
	delete d;
}

/*!\internal*/
void QPalette::init() {
    d = new QPaletteData;
    d->ref = 1;
    d->ser_no = palette_count++;
#ifndef QT_NO_COMPAT
    is_colorgroup = 0;
#endif
    current_group = Active; //as a default..
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
    current_group = p.current_group;
#ifndef QT_NO_COMPAT
    is_colorgroup = p.is_colorgroup;
#endif
    x = qAtomicSetPtr(&d, x);
    if(!--x->ref)
	delete x;
    return *this;
}

/*!
    \fn const QColor &QPalette::color(ColorGroup gr, ColorRole r) const
    Returns the color in color group \a gr, used for color role \a r.

    \sa brush() setColor() ColorRole
*/

/*!
    Returns the brush in color group \a gr, used for color role \a cr.

    \sa color() setBrush() ColorRole
*/
const QBrush &QPalette::brush(ColorGroup gr, ColorRole cr) const
{
    Q_ASSERT(cr < NColorRoles);
    if(gr >= (int)NColorGroups) {
	if(gr == Current) {
	    gr = current_group;
	} else {
	    qWarning("QPalette::brush: Unknown ColorGroup: %d", (int)gr);
	    gr = Active;
	}
    }
    return d->br[gr][cr];
}

/*!
    \fn void QPalette::setColor( ColorGroup gr, ColorRole r, const QColor &c)
    Sets the brush in color group \a gr, used for color role \a r, to
    the solid color \a c.

    \sa setBrush() color() ColorRole
*/

/*!
    Sets the brush in color group \a cg, used for color role \a cr, to
    \a b.

    \sa brush() setColor() ColorRole
*/
void QPalette::setBrush(ColorGroup cg, ColorRole cr, const QBrush &b)
{
    Q_ASSERT(cr < NColorRoles);
    detach();
    d->ser_no = palette_count++;
    if(cg >= (int)NColorGroups) {
	if(cg == All) {
	    for(int i = 0; i < (int)NColorGroups; i++)
		d->br[i][cr] = b;
	    return;
	} else if(cg == Current) {
	    cg = current_group;
	} else {
	    qWarning("QPalette::setBrush: Unknown ColorGroup: %d", (int)cg);
	    cg = Active;
	}
    }
    d->br[cg][cr] = b;
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
    for(int grp = 0; grp < (int)NColorGroups; grp++) {
	for(int role = 0; role < (int)NColorRoles; role++)
	    x->br[grp][role] = d->br[grp][role];
    }
    x->ser_no = palette_count++;
    x = qAtomicSetPtr(&d, x);
    if(!--x->ref)
	delete x;
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
#ifndef QT_NO_COMPAT
    Q_ASSERT(is_colorgroup == p.is_colorgroup);
#endif
    for(int grp = 0; grp < (int)NColorGroups; grp++) {
	for(int role = 0; role < (int)NColorRoles; role++) {
	    if(d->br[grp][role] != p.d->br[grp][role])
		return FALSE;
	}
    }
    return TRUE;
}

/*!
    Returns TRUE (usually quickly) if this ColorGroup \a grp1 is equal to \a grp2;
    otherwise returns FALSE.
*/
bool QPalette::isEqual(QPalette::ColorGroup grp1, QPalette::ColorGroup grp2) const
{
    if(grp1 >= (int)NColorGroups) {
	if(grp1 == Current) {
	    grp1 = current_group;
	} else {
	    qWarning("QPalette::brush: Unknown ColorGroup(1): %d", (int)grp1);
	    grp1 = Active;
	}
    }
    if(grp2 >= (int)NColorGroups) {
	if(grp2 == Current) {
	    grp2 = current_group;
	} else {
	    qWarning("QPalette::brush: Unknown ColorGroup(2): %d", (int)grp2);
	    grp2 = Active;
	}
    }
    if(grp1 == grp2)
	return TRUE;
    for(int role = 0; role < (int)NColorRoles; role++) {
	if(d->br[grp1][role] != d->br[grp2][role])
		return FALSE;
    }
    return TRUE;
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
  QPalette stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM

/*!
    \relates QPalette

    Writes the palette, \a p to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPalette &p)
{
    for(int grp = 0; grp < (int)QPalette::NColorGroups; grp++) {
	for(int role = 0; role < (int)QPalette::NColorRoles; role++)
	    s << p.d->br[grp][role];
    }
    return s;
}

static void readV1ColorGroup(QDataStream &s, QPalette &pal, QPalette::ColorGroup r)
{
    QColor fg, bg, light, dark, mid, text, base;
    s >> fg >> bg >> light >> dark >> mid >> text >> base;
    pal.setColor(r, QPalette::Foreground, fg);
    pal.setColor(r, QPalette::Light, light);
    pal.setColor(r, QPalette::Dark, dark);
    pal.setColor(r, QPalette::Mid, mid);
    pal.setColor(r, QPalette::Text, text);
    pal.setColor(r, QPalette::Base, base);
}

/*!
    \relates QPalette

    Reads a palette from the stream, \a s into the palette \a p, and
    returns a reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPalette &p)
{
    if(s.version() == 1) {
	readV1ColorGroup(s, p, QPalette::Active);
	readV1ColorGroup(s, p, QPalette::Disabled);
	readV1ColorGroup(s, p, QPalette::Inactive);
    } else {
	QBrush tmp;
	for(int grp = 0; grp < (int)QPalette::NColorGroups; grp++) {
	    for(int role = 0; role < (int)QPalette::NColorRoles; role++) {
		s >> tmp;
		p.setBrush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role, tmp);
	    }
	}
    }
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

/*!

    Sets a the group at \a cg. You can pass either brushes, pixmaps or
    plain colors for \a foreground, \a button, \a light, \a dark, \a
    mid, \a text, \a bright_text, \a base and \a background.

    \sa QBrush
*/
void QPalette::setColorGroup(ColorGroup cg, const QBrush &foreground, const QBrush &button,
			      const QBrush &light, const QBrush &dark, const QBrush &mid,
			      const QBrush &text, const QBrush &bright_text, const QBrush &base,
			      const QBrush &background)
{
    setColorGroup(cg, foreground, button, light, dark, mid, text, bright_text, base,
		  background, QBrush(qt_mix_colors(button.color(), light.color())),
		  text, QBrush(Qt::black), QBrush(Qt::darkBlue), QBrush(Qt::white),
		  QBrush(Qt::blue), QBrush(Qt::magenta));
}


/*!\internal*/
void
QPalette::setColorGroup(ColorGroup cg, const QBrush &foreground, const QBrush &button,
			const QBrush &light, const QBrush &dark, const QBrush &mid,
			const QBrush &text, const QBrush &bright_text, const QBrush &base,
			const QBrush &background, const QBrush &midlight,
			const QBrush &button_text, const QBrush &shadow,
			const QBrush &highlight, const QBrush &highlighted_text,
			const QBrush &link, const QBrush &link_visited)
{
    detach();
    d->ser_no = palette_count++;
    setBrush(cg, Foreground, foreground);
    setBrush(cg, Button, button);
    setBrush(cg, Light, light);
    setBrush(cg, Dark, dark);
    setBrush(cg, Mid, mid);
    setBrush(cg, Text, text);
    setBrush(cg, BrightText, bright_text);
    setBrush(cg, Base, base);
    setBrush(cg, Background, background);
    setBrush(cg, Midlight, midlight);
    setBrush(cg, ButtonText, button_text);
    setBrush(cg, Shadow, shadow);
    setBrush(cg, Highlight, highlight);
    setBrush(cg, HighlightedText, highlighted_text);
    setBrush(cg, Link, link);
    setBrush(cg, LinkVisited, link_visited);
}

#endif // QT_NO_PALETTE
