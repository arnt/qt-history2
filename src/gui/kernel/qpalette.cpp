/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpalette.h"
#include "qapplication.h"
#ifndef QT_NO_PALETTE
#include "qdatastream.h"
#include "qcleanuphandler.h"
#include "qvariant.h"

static int qt_palette_count = 1;
class QPalettePrivate {
public:
    QPalettePrivate() : ref(1), ser_no(qt_palette_count++) { }
    QAtomic ref;
    QBrush br[QPalette::NColorGroups][QPalette::NColorRoles];
    int ser_no;
};

static QColor qt_mix_colors(QColor a, QColor b)
{
    return QColor((a.red() + b.red()) / 2, (a.green() + b.green()) / 2,
                   (a.blue() + b.blue()) / 2);
}

#ifdef QT3_SUPPORT

#ifndef QT_NO_DATASTREAM
QDataStream &qt_stream_out_qcolorgroup(QDataStream &s, const QColorGroup &g)
{
    if(s.version() == 1) {
        // Qt 1.x
        s << g.color(QPalette::Foreground) << g.color(QPalette::Background)
          << g.color(QPalette::Light) << g.color(QPalette::Dark)
          << g.color(QPalette::Mid) << g.color(QPalette::Text) << g.color(QPalette::Base);
    } else {
        int max = QPalette::NColorRoles;
        if(s.version() <= 3) // Qt 2.x
            max = 14;
        for(int r = 0 ; r < max ; r++)
            s << g.brush((QPalette::ColorRole)r);
    }
    return s;
}

QDataStream &qt_stream_in_qcolorgroup(QDataStream &s, QColorGroup &g)
{
    if(s.version() == 1) {         // Qt 1.x
        QColor fg, bg, light, dark, mid, text, base;
        s >> fg >> bg >> light >> dark >> mid >> text >> base;
        QPalette p(bg);
        p.setColor(QPalette::Active, QPalette::Foreground, fg);
        p.setColor(QPalette::Active, QPalette::Light, light);
        p.setColor(QPalette::Active, QPalette::Dark, dark);
        p.setColor(QPalette::Active, QPalette::Mid, mid);
        p.setColor(QPalette::Active, QPalette::Text, text);
        p.setColor(QPalette::Active, QPalette::Base, base);
        g = p;
        g.setCurrentColorGroup(QPalette::Active);
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

QDataStream &operator<<(QDataStream &s, const QColorGroup &g)
{
    return qt_stream_out_qcolorgroup(s, g);
}

QDataStream &operator>>(QDataStream &s, QColorGroup &g)
{
    return qt_stream_in_qcolorgroup(s, g);
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
    setColorGroup(cg, g.brush(Foreground), g.brush(Button), g.brush(Light),
                  g.brush(Dark), g.brush(Mid), g.brush(Text), g.brush(BrightText), g.brush(Base),
                  g.brush(Background), g.brush(Midlight), g.brush(ButtonText), g.brush(Shadow),
                  g.brush(Highlight), g.brush(HighlightedText), g.brush(Link),
                  g.brush(LinkVisited));
}

#endif

/*!
   \fn const QColor &QPalette::color(ColorRole r) const

   \overload

    Returns the color that has been set for color role \a r in the current ColorGroup.

    \sa brush() ColorRole
 */

/*!
    \fn const QBrush &QPalette::brush(ColorRole r) const

    \overload

    Returns the brush that has been set for color role \a r in the current ColorGroup.

    \sa color() setBrush() ColorRole
*/

/*!
    \fn void QPalette::setColor(ColorRole r, const QColor &c)

    \overload

    Sets the brush used for color role \a r in the current ColorGroup to a solid color \a c.

    \sa brush() setColor() ColorRole
*/

/*!
    \fn void QPalette::setBrush(ColorRole cr, const QBrush &brush)

    Sets the brush used to color role \a cr and brush \a brush.

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
    \fn ColorGroup QPalette::currentColorGroup() const

    Returns the palette's current color group.
*/

/*!
    \fn void QPalette::setCurrentColorGroup(ColorGroup cg)

    Set the palette's current color group to \a cg.
*/

/*!
    \class QPalette qpalette.h

    \brief The QPalette class contains color groups for each widget state.

    \ingroup appearance
    \ingroup shared
    \ingroup multimedia
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

    We strongly recommend that you use a system-supplied color group
    and modify that as necessary.

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

    \value Disabled
    \value Active
    \value Inactive
    \value Normal synonym for Active

    \omitvalue All
    \omitvalue NColorGroups
    \omitvalue Current
*/

/*!
    \enum QPalette::ColorRole

    The ColorRole enum defines the different symbolic color roles used
    in current GUIs.

    The central roles are:

    \value Background  A general background color.

    \value Foreground  A general foreground color.

    \value Base  Used as the background color for text entry widgets;
                 usually white or another light color.

    \value Text  The foreground color used with \c Base. This is usually
                 the same as the \c Foreground, in which case it must provide
                 good contrast with \c Background and \c Base.

    \value Button  The general button background color in which buttons need a
                   background that is different from \c Background, as in the
                   Macintosh style.

    \value ButtonText  A foreground color used with the \c Button color.

    There are some color roles used mostly for 3D bevel and shadow
    effects:

    \value Light  Lighter than \c Button color.

    \value Midlight  Between \c Button and \c Light.

    \value Dark  Darker than \c Button.

    \value Mid  Between \c Button and \c Dark.

    \value Shadow  A very dark color. By default, the shadow color is
                   \c Qt::black.

    All of these are normally derived from \c Background, and used in
    ways that depend on that relationship. For example, buttons depend
    on it to make the bevels look attractive, and Motif scroll bars
    depend on \c Mid to be slightly different from \c Background.

    Selected (marked) items have two roles:

    \value Highlight   A color to indicate a selected item or the current
                       item. By default, the highlight color is
                       \c Qt::darkBlue.

    \value HighlightedText  A text color that contrasts with \c Highlight.
    By default, the highlighted text color is \c Qt::white.

    Finally, there is a special role for text that needs to be
    drawn where \c Text or \c Foreground would give poor contrast,
    such as on pressed push buttons:

    \value BrightText  A text color that is very different from
                       \c Foreground, and contrasts well with e.g. \c Dark.

    \value Link  A text color used for unvisited hyperlinks.
                 By default, the link color is \c Qt::blue.

    \value LinkVisited  A text color used for already visited hyperlinks.
                        By default, the linkvisited color is \c Qt::magenta.

    \omitvalue NColorRoles

    Note that text colors can be used for things other than just
    words; text colors are \e usually used for text, but it's quite
    common to use the text color roles for lines, icons, etc.

    This image shows most of the color roles in use:
    \img palette.png Color Roles
*/

/*!
    Constructs a palette object that uses the application's default palette.

    \sa QApplication::setPalette(), QApplication::palette()
*/
QPalette::QPalette()
    : d(QApplication::palette().d),
      current_group(Active),
      resolve_mask(0)
{
    d->ref.ref();
}

static void qt_palette_from_color(QPalette &pal, const QColor & button)
{
    QColor bg = button,
           btn = button,
           fg,
           base,
           disfg;
    int h, s, v;
    bg.getHsv(&h, &s, &v);
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
    pal.setColorGroup(QPalette::Active, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
                      QBrush(btn.dark(150)), QBrush(fg), QBrush(Qt::white), QBrush(base), QBrush(bg));
    pal.setColorGroup(QPalette::Inactive, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
                      QBrush(btn.dark(150)), QBrush(fg), QBrush(Qt::white), QBrush(base), QBrush(bg));
    pal.setColorGroup(QPalette::Disabled, QBrush(fg), QBrush(btn), QBrush(btn.light(150)), QBrush(btn.dark()),
                      QBrush(btn.dark(150)), QBrush(disfg), QBrush(Qt::white), QBrush(base),
                      QBrush(bg));
}


/*!
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. Background will be
  the button color as well.
*/
QPalette::QPalette(const QColor &button)
{
    init();
    qt_palette_from_color(*this, button);
}

/*!
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. Background will be
  the button color as well.
*/
QPalette::QPalette(Qt::GlobalColor button)
{
    init();
    qt_palette_from_color(*this, button);
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

  Constructs a palette with the specified \a foreground, \a
  background, \a light, \a dark, \a mid, \a text, and \a base colors.
  The button color will be set to the background color.
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
    bg.getHsv(&h, &s, &v);
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
    d->ref.ref();
    resolve_mask = p.resolve_mask;
    current_group = p.current_group;
}

/*!
    Destroys the palette.
*/
QPalette::~QPalette()
{
    if(!d->ref.deref())
        delete d;
}

/*!\internal*/
void QPalette::init() {
    d = new QPalettePrivate;
    resolve_mask = 0;
    current_group = Active; //as a default..
}

/*!
    Assigns \a p to this palette and returns a reference to this
    palette.

    This is fast (it uses copy-on-write).
*/
QPalette &QPalette::operator=(const QPalette &p)
{
    QPalettePrivate *x = p.d;
    x->ref.ref();
    resolve_mask = p.resolve_mask;
    current_group = p.current_group;
    x = qAtomicSetPtr(&d, x);
    if(!x->ref.deref())
        delete x;
    return *this;
}

/*!
   Returns the palette as a QVariant
*/
QPalette::operator QVariant() const
{
    extern bool qRegisterGuiVariant();
    static const bool b = qRegisterGuiVariant();
    Q_UNUSED(b)
    return QVariant(QVariant::Palette, this);
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
            gr = (ColorGroup)current_group;
        } else {
            qWarning("QPalette::brush: Unknown ColorGroup: %d", (int)gr);
            gr = Active;
        }
    }
    return d->br[gr][cr];
}

/*!
    \fn void QPalette::setColor(ColorGroup gr, ColorRole r, const QColor &c)
    Sets the brush in color group \a gr, used for color role \a r, to
    the solid color \a c.

    \sa setBrush() color() ColorRole
*/

/*!
    \overload

    Sets the brush in color group \a cg, used for color role \a cr, to
    \a b.

    \sa brush() setColor() ColorRole
*/
void QPalette::setBrush(ColorGroup cg, ColorRole cr, const QBrush &b)
{
    Q_ASSERT(cr < NColorRoles);
    detach();
    if(cg >= (int)NColorGroups) {
        if(cg == All) {
            for(int i = 0; i < (int)NColorGroups; i++)
                d->br[i][cr] = b;
            resolve_mask |= (1<<cr);
            return;
        } else if(cg == Current) {
            cg = (ColorGroup)current_group;
        } else {
            qWarning("QPalette::setBrush: Unknown ColorGroup: %d", (int)cg);
            cg = Active;
        }
    }
    d->br[cg][cr] = b;
    resolve_mask |= (1<<cr);
}


/*!
    \fn void QPalette::detach()
    Detaches this palette from any other QPalette objects with which
    it might implicitly share QColorGroup objects. In essence, does
    the copying part of copy-on-write.

    Calling this should generally not be necessary; QPalette calls it
    itself when necessary.
*/
void QPalette::detach()
{
    if (d->ref != 1) {
        QPalettePrivate *x = new QPalettePrivate;
        for(int grp = 0; grp < (int)NColorGroups; grp++) {
            for(int role = 0; role < (int)NColorRoles; role++)
                x->br[grp][role] = d->br[grp][role];
        }
        x = qAtomicSetPtr(&d, x);
        if(!x->ref.deref())
            delete x;
    }
}

/*!
    \fn bool QPalette::operator!=(const QPalette &p) const

    Returns true (slowly) if this palette is different from \a p;
    otherwise returns false (usually quickly).
*/

/*!
    Returns true (usually quickly) if this palette is equal to \a p;
    otherwise returns false (slowly).
*/
bool QPalette::operator==(const QPalette &p) const
{
    if (isCopyOf(p))
        return true;
    for(int grp = 0; grp < (int)NColorGroups; grp++) {
        for(int role = 0; role < (int)NColorRoles; role++) {
            if(d->br[grp][role] != p.d->br[grp][role])
                return false;
        }
    }
    return true;
}

#ifdef QT3_SUPPORT
bool QColorGroup::operator==(const QColorGroup &other) const
{
    if (isCopyOf(other))
        return true;
    for (int role = 0; role < int(NColorRoles); role++) {
        if(d->br[current_group][role] != other.d->br[other.current_group][role])
            return false;
    }
    return true;
}

/*!
   Returns the color group as a QVariant
*/
QColorGroup::operator QVariant() const
{
    extern bool qRegisterGuiVariant();
    static const bool b = qRegisterGuiVariant();
    Q_UNUSED(b)
    return QVariant(QVariant::ColorGroup, this);
}
#endif

/*!
    \fn bool QPalette::isEqual(ColorGroup cg1, ColorGroup cg2) const

    Returns true (usually quickly) if color group \a cg1 is equal to
    \a cg2; otherwise returns false.
*/
bool QPalette::isEqual(QPalette::ColorGroup group1, QPalette::ColorGroup group2) const
{
    if(group1 >= (int)NColorGroups) {
        if(group1 == Current) {
            group1 = (ColorGroup)current_group;
        } else {
            qWarning("QPalette::brush: Unknown ColorGroup(1): %d", (int)group1);
            group1 = Active;
        }
    }
    if(group2 >= (int)NColorGroups) {
        if(group2 == Current) {
            group2 = (ColorGroup)current_group;
        } else {
            qWarning("QPalette::brush: Unknown ColorGroup(2): %d", (int)group2);
            group2 = Active;
        }
    }
    if(group1 == group2)
        return true;
    for(int role = 0; role < (int)NColorRoles; role++) {
        if(d->br[group1][role] != d->br[group2][role])
                return false;
    }
    return true;
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
int QPalette::serialNumber() const
{
    return d->ser_no;
}

/*!
    Returns a new QPalette that has attributes copied from \a other.
*/
QPalette QPalette::resolve(const QPalette &other) const
{
    if (*this == other && resolve_mask == other.resolve_mask
        || resolve_mask == 0) {
        QPalette o = other;
        o.resolve_mask = resolve_mask;
        return o;
    }

    QPalette palette(*this);
    palette.detach();

    for(int role = 0; role < (int)NColorRoles; role++)
        if (!(resolve_mask & (1<<role)))
            for(int grp = 0; grp < (int)NColorGroups; grp++)
                palette.d->br[grp][role] = other.d->br[grp][role];

    return palette;
}

/*!
    \fn uint QPalette::resolve() const
    \internal
*/

/*!
    \fn void QPalette::resolve(uint mask)
    \internal
*/


/*****************************************************************************
  QPalette stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM

static const int NumOldRoles = 7;
static const int oldRoles[7] = { QPalette::Foreground, QPalette::Background, QPalette::Light,
                                 QPalette::Dark, QPalette::Mid, QPalette::Text, QPalette::Base };

/*!
    \relates QPalette

    Writes the palette, \a p to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPalette &p)
{
    for (int grp = 0; grp < (int)QPalette::NColorGroups; grp++) {
        if (s.version() == 1) {
            // Qt 1.x
            for (int i = 0; i < NumOldRoles; ++i)
                s << p.d->br[grp][oldRoles[i]].color();
        } else {
            int max = QPalette::NColorRoles;
            if (s.version() <= 3) // Qt 2.x
                max = 14;
            for (int r = 0; r < max; r++)
                s << p.d->br[grp][r];
        }
    }
    return s;
}

static void readV1ColorGroup(QDataStream &s, QPalette &pal, QPalette::ColorGroup grp)
{
    for (int i = 0; i < NumOldRoles; ++i) {
        QColor col;
        s >> col;
        pal.setColor(grp, (QPalette::ColorRole)oldRoles[i], col);
    }
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
        p = QPalette();
        readV1ColorGroup(s, p, QPalette::Active);
        readV1ColorGroup(s, p, QPalette::Disabled);
        readV1ColorGroup(s, p, QPalette::Inactive);
    } else {
        int max = QPalette::NColorRoles;
        if (s.version() <= 3) { // Qt 2.x
            p = QPalette();
            max = 14;
        }

        QBrush tmp;
        for(int grp = 0; grp < (int)QPalette::NColorGroups; ++grp) {
            for(int role = 0; role < max; ++role) {
                s >> tmp;
                p.setBrush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role, tmp);
            }
        }
    }
    return s;
}
#endif //QT_NO_DATASTREAM

/*!
    Returns true if this palette and \a p are copies of each other,
    i.e. one of them was created as a copy of the other and neither
    was subsequently modified; otherwise returns false. This is much
    stricter than equality.

    \sa operator=() operator==()
*/

bool QPalette::isCopyOf(const QPalette &p) const
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

/*!
    \fn QPalette QPalette::copy() const

    Use simple assignment instead.
*/

/*!
    \fn QColorGroup QPalette::normal() const

    Use createColorGroup(Active) instead.
*/

/*!
    \fn void QPalette::setNormal(const QColorGroup &colorGroup)

    Use setColorGroup(Active, colorGroup) instead.
*/

/*!
    \fn QColorGroup QPalette::active() const
*/

/*!
    \fn QColorGroup QPalette::disabled() const
*/

/*!
    \fn QColorGroup QPalette::inactive() const
*/

/*!
    \fn void QPalette::setActive(const QColorGroup &colorGroup)

    Use setColorGroup(Active, colorGroup) instead.
*/

/*!
    \fn void QPalette::setDisabled(const QColorGroup &colorGroup)

    Use setColorGroup(Disabled, colorGroup) instead.
*/

/*!
    \fn void QPalette::setInactive(const QColorGroup &colorGroup)

    Use setColorGroup(Inactive, colorGroup) instead.
*/

/*! \class QColorGroup
    \compat
*/

/*! \fn QColorGroup::QColorGroup()

    Use QPalette() instead.
*/

/*! \fn QColorGroup::QColorGroup(const QBrush &foreground, const QBrush &button, \
                                 const QBrush &light, const QBrush &dark, const QBrush &mid, \
                                 const QBrush &text, const QBrush &bright_text,
                                 const QBrush &base, const QBrush &background)

    Use QPalette(\a foreground, \a button, \a light, \a dark, \a mid,
    \a text, \a bright_text, \a base, \a background) instead.
*/

/*! \fn QColorGroup::QColorGroup(const QColor &foreground, const QColor &background, \
                                 const QColor &light, const QColor &dark, const QColor &mid, \
                                 const QColor &text, const QColor &base)

    Use QColorGroup(\a foreground, \a background, \a light, \a dark,
    \a mid, \a text, \a base) instead.
*/

/*! \fn QColorGroup::QColorGroup(const QColorGroup &other)

    Use QPalette(\a other) instead.
*/

/*! \fn QColorGroup::QColorGroup(const QPalette &pal)

    Use QPalette(\a pal) instead.
*/

/*! \fn const QColor &QColorGroup::foreground() const

    Use QPalette::foreground().color() instead.
*/

/*! \fn const QColor &QColorGroup::button() const

    Use QPalette::button().color() instead.
*/

/*! \fn const QColor &QColorGroup::light() const

    Use QPalette::light().color() instead.
*/

/*! \fn const QColor &QColorGroup::dark() const

    Use QPalette::dark().color() instead.
*/

/*! \fn const QColor &QColorGroup::mid() const

    Use QPalette::mid().color() instead.
*/

/*! \fn const QColor &QColorGroup::text() const

    Use QPalette::text().color() instead.
*/

/*! \fn const QColor &QColorGroup::base() const

    Use QPalette::base().color() instead.
*/

/*! \fn const QColor &QColorGroup::background() const

    Use QPalette::background().color() instead.
*/

/*! \fn const QColor &QColorGroup::midlight() const

    Use QPalette::midlight().color() instead.
*/

/*! \fn const QColor &QColorGroup::brightText() const

    Use QPalette::brightText().color() instead.
*/

/*! \fn const QColor &QColorGroup::buttonText() const

    Use QPalette::buttonText().color() instead.
*/

/*! \fn const QColor &QColorGroup::shadow() const

    Use QPalette::shadow().color() instead.
*/

/*! \fn const QColor &QColorGroup::highlight() const

    Use QPalette::highlight().color() instead.
*/

/*! \fn const QColor &QColorGroup::highlightedText() const

    Use QPalette::highlightedText().color() instead.
*/

/*! \fn const QColor &QColorGroup::link() const

    Use QPalette::link().color() instead.
*/

/*! \fn const QColor &QColorGroup::linkVisited() const

    Use QPalette::linkVisited().color() instead.
*/

/*! \fn QDataStream &operator<<(QDataStream &ds, const QColorGroup &colorGroup)
    \relates QColorGroup
    \compat
*/

/*! \fn QDataStream &operator>>(QDataStream &ds, QColorGroup &colorGroup)
    \relates QColorGroup
    \compat
*/

/*! \fn bool QColorGroup::operator==(const QColorGroup &other)

    Returns true if this color group is equal to \a other; otherwise
    returns false.
*/

/*! \fn bool QColorGroup::operator!=(const QColorGroup &other)

    Returns true if this color group is not equal to \a other;
    otherwise returns false.
*/

#endif // QT_NO_PALETTE
