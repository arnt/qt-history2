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

#ifndef QPALETTE_H
#define QPALETTE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qcolor.h"
#include "qbrush.h" // QColor->QBrush conversion
#endif // QT_H

#ifndef QT_NO_PALETTE

class QColorGroup;
class QPalettePrivate;

class Q_GUI_EXPORT QPalette
{
public:
    QPalette();
    QPalette(const QColor &button);
    QPalette(Qt::GlobalColor button);
    QPalette(const QColor &button, const QColor &background);
    QPalette(const QBrush &foreground, const QBrush &button, const QBrush &light,
             const QBrush &dark, const QBrush &mid, const QBrush &text,
             const QBrush &bright_text, const QBrush &base, const QBrush &background);
    QPalette(const QColor &foreground, const QColor &background, const QColor &light,
             const QColor &dark, const QColor &mid, const QColor &text, const QColor &base);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QPalette(const QColorGroup &active, const QColorGroup &disabled, const QColorGroup &inactive);
#endif
    QPalette(const QPalette &palette);
    ~QPalette();
    QPalette &operator=(const QPalette &palette);

    // Do not change the order, the serialization format depends on it
    enum ColorGroup { Active, Disabled, Inactive, NColorGroups, Current, All, Normal = Active };
    enum ColorRole { Foreground, Button, Light, Midlight, Dark, Mid,
                     Text, BrightText, ButtonText, Base, Background, Shadow,
                     Highlight, HighlightedText, Link, LinkVisited,
                     NColorRoles };

    inline ColorGroup currentColorGroup() const { return (ColorGroup)current_group; }
    inline void setCurrentColorGroup(ColorGroup cg) { current_group = cg; }

    inline const QColor &color(ColorGroup cg, ColorRole cr) const
    { return brush(cg, cr).color(); }
    const QBrush &brush(ColorGroup cg, ColorRole cr) const;
    inline void setColor(ColorGroup cg, ColorRole cr, const QColor &color)
    { setBrush(cg, cr, QBrush(color)); }
    inline void setColor(ColorRole cr, const QColor &color)
     { setColor(All, cr, color); }
    inline void setBrush(ColorRole cr, const QBrush &brush)
     { setBrush(All, cr, brush); }
    void setBrush(ColorGroup cg, ColorRole cr, const QBrush &brush);
    void setColorGroup(ColorGroup cr, const QBrush &foreground, const QBrush &button,
                       const QBrush &light, const QBrush &dark, const QBrush &mid,
                       const QBrush &text, const QBrush &bright_text, const QBrush &base,
                       const QBrush &background);
    bool isEqual(ColorGroup cr1, ColorGroup cr2) const;

    inline const QColor &color(ColorRole cr) const { return color(Current, cr); }
    inline const QBrush &brush(ColorRole cr) const { return brush(Current, cr); }
    inline const QBrush &foreground() const { return brush(Foreground); }
    inline const QBrush &button() const { return brush(Button); }
    inline const QBrush &light() const { return brush(Light); }
    inline const QBrush &dark() const { return brush(Dark); }
    inline const QBrush &mid() const { return brush(Mid); }
    inline const QBrush &text() const { return brush(Text); }
    inline const QBrush &base() const { return brush(Base); }
    inline const QBrush &background() const { return brush(Background); }
    inline const QBrush &midlight() const { return brush(Midlight); }
    inline const QBrush &brightText() const { return brush(BrightText); }
    inline const QBrush &buttonText() const { return brush(ButtonText); }
    inline const QBrush &shadow() const { return brush(Shadow); }
    inline const QBrush &highlight() const { return brush(Highlight); }
    inline const QBrush &highlightedText() const { return brush(HighlightedText); }
    inline const QBrush &link() const { return brush(Link); }
    inline const QBrush &linkVisited() const { return brush(LinkVisited); }

#ifdef QT_COMPAT
    inline QT_COMPAT QPalette copy() const { QPalette p = *this; p.detach(); return p; }
    QT_COMPAT QColorGroup normal() const;
    inline QT_COMPAT void setNormal(const QColorGroup &cg) { setColorGroup(Active, cg); }

    QT_COMPAT QColorGroup active() const;
    QT_COMPAT QColorGroup disabled() const;
    QT_COMPAT QColorGroup inactive() const;
    inline QT_COMPAT void setActive(const QColorGroup &cg) { setColorGroup(Active, cg); }
    inline QT_COMPAT void setDisabled(const QColorGroup &cg) { setColorGroup(Disabled, cg); }
    inline QT_COMPAT void setInactive(const QColorGroup &cg) { setColorGroup(Inactive, cg); }
#endif

    bool operator==(const QPalette &p) const;
    inline bool operator!=(const QPalette &p) const { return !(operator==(p)); }
    bool isCopyOf(const QPalette &p) const;

    int serialNumber() const;

    QPalette resolve(const QPalette &) const;
    inline uint resolve() const { return resolve_mask; }
    inline void resolve(uint mask) { resolve_mask = mask; }

private:
    void setColorGroup(ColorGroup cr, const QBrush &foreground, const QBrush &button,
                       const QBrush &light, const QBrush &dark, const QBrush &mid,
                       const QBrush &text, const QBrush &bright_text, const QBrush &base,
                       const QBrush &background, const QBrush &midlight,
                       const QBrush &button_text, const QBrush &shadow,
                       const QBrush &highlight, const QBrush &highlighted_text,
                       const QBrush &link, const QBrush &link_visited);
#ifdef QT_COMPAT
    void setColorGroup(ColorGroup, const QColorGroup &);
    QColorGroup createColorGroup(ColorGroup) const;
#endif
    void init();
    void detach();

    QPalettePrivate *d;
    uint current_group : 4;
    uint resolve_mask : 27;
#ifdef QT_COMPAT
    friend class QColorGroup;
    uint is_colorgroup : 1;
#endif
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &s, const QPalette &p);
};

#ifdef QT_COMPAT
class Q_GUI_EXPORT QColorGroup : public QPalette
{
public:
    inline QColorGroup() : QPalette() { is_colorgroup = 1; }
    inline QColorGroup(const QBrush &foreground, const QBrush &button, const QBrush &light,
                const QBrush &dark, const QBrush &mid, const QBrush &text,
                const QBrush &bright_text, const QBrush &base, const QBrush &background)
        : QPalette(foreground, button, light, dark, mid, text, bright_text, base, background)
    { is_colorgroup = 1; }
    inline QColorGroup(const QColor &foreground, const QColor &background, const QColor &light,
                const QColor &dark, const QColor &mid, const QColor &text, const QColor &base)
        : QPalette(foreground, background, light, dark, mid, text, base) { is_colorgroup = 1; }
    inline QColorGroup(const QColorGroup &cg) : QPalette(cg) { is_colorgroup = 1; }
    inline QColorGroup(const QPalette &pal) : QPalette(pal) { is_colorgroup = 1; }

    inline QT_COMPAT const QColor &foreground() const { return color(Foreground); }
    inline QT_COMPAT const QColor &button() const { return color(Button); }
    inline QT_COMPAT const QColor &light() const { return color(Light); }
    inline QT_COMPAT const QColor &dark() const { return color(Dark); }
    inline QT_COMPAT const QColor &mid() const { return color(Mid); }
    inline QT_COMPAT const QColor &text() const { return color(Text); }
    inline QT_COMPAT const QColor &base() const { return color(Base); }
    inline QT_COMPAT const QColor &background() const { return color(Background); }
    inline QT_COMPAT const QColor &midlight() const { return color(Midlight); }
    inline QT_COMPAT const QColor &brightText() const { return color(BrightText); }
    inline QT_COMPAT const QColor &buttonText() const { return color(ButtonText); }
    inline QT_COMPAT const QColor &shadow() const { return color(Shadow); }
    inline QT_COMPAT const QColor &highlight() const { return color(Highlight); }
    inline QT_COMPAT const QColor &highlightedText() const { return color(HighlightedText); }
    inline QT_COMPAT const QColor &link() const { return color(Link); }
    inline QT_COMPAT const QColor &linkVisited() const { return color(LinkVisited); }
};

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QT_COMPAT QDataStream &operator<<(QDataStream &ds, const QColorGroup &cg);
Q_GUI_EXPORT QT_COMPAT QDataStream &operator>>(QDataStream &ds, QColorGroup &cg);
#endif

inline QColorGroup QPalette::inactive() const { return createColorGroup(Inactive); }
inline QColorGroup QPalette::disabled() const { return createColorGroup(Disabled); }
inline QColorGroup QPalette::active() const { return createColorGroup(Active); }
inline QColorGroup QPalette::normal() const { return createColorGroup(Active); }

#endif

/*****************************************************************************
  QPalette stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &ds, const QPalette &p);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &ds, QPalette &p);
#endif // QT_NO_DATASTREAM

#endif // QT_NO_PALETTE
#endif // QPALETTE_H
