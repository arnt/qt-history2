/****************************************************************************
**
** Definition of QColorGroup and QPalette classes.
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

#ifndef QPALETTE_H
#define QPALETTE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qcolor.h"
#include "qshared.h"
#include "qbrush.h" // QColor->QBrush conversion
#endif // QT_H

#ifndef QT_NO_PALETTE

class Q_EXPORT QColorGroup
{
public:
    QColorGroup();
    QColorGroup(const QColor &foreground, const QColor &button,
		const QColor &light, const QColor &dark, const QColor &mid,
		const QColor &text, const QColor &base);
    QColorGroup(const QBrush &foreground, const QBrush &button,
		const QBrush &light, const QBrush &dark, const QBrush &mid,
		const QBrush &text, const QBrush &bright_text,
		const QBrush &base, const QBrush &background);
    QColorGroup(const QColorGroup &cg);

    ~QColorGroup();

    QColorGroup& operator =(const QColorGroup&);

    // Do not change the order, the serialization format depends on it
    enum ColorRole { Foreground, Button, Light, Midlight, Dark, Mid,
		     Text, BrightText, ButtonText, Base, Background, Shadow,
		     Highlight, HighlightedText, Link, LinkVisited,
		     NColorRoles };

    inline const QColor &color(ColorRole cr) const { return d->br[cr].color(); }
    inline const QBrush &brush(ColorRole cr) const { return d->br[cr]; }
    inline void setColor(ColorRole cr, const QColor &color) { setBrush(cr, QBrush(color)); }
    void setBrush(ColorRole cr, const QBrush &brush);

    inline const QColor &foreground() const { return d->br[Foreground].color(); }
    inline const QColor &button() const { return d->br[Button].color(); }
    inline const QColor &light() const { return d->br[Light].color(); }
    inline const QColor &dark() const { return d->br[Dark].color(); }
    inline const QColor &mid() const { return d->br[Mid].color(); }
    inline const QColor &text() const { return d->br[Text].color(); }
    inline const QColor &base() const { return d->br[Base].color(); }
    inline const QColor &background() const { return d->br[Background].color(); }

    inline const QColor &midlight() const { return d->br[Midlight].color(); }
    inline const QColor &brightText() const { return d->br[BrightText].color(); }
    inline const QColor &buttonText() const { return d->br[ButtonText].color(); }
    inline const QColor &shadow() const { return d->br[Shadow].color(); }
    inline const QColor &highlight() const { return d->br[Highlight].color(); }
    inline const QColor &highlightedText() const {return d->br[HighlightedText].color(); }
    inline const QColor &link() const { return d->br[Link].color(); }
    inline const QColor &linkVisited() const { return d->br[LinkVisited].color(); }

    bool operator==(const QColorGroup &g) const;
    inline bool operator!=(const QColorGroup &g) const { return !(operator==(g)); }

private:
    friend class QPalette;
    void detach() { if (d->ref != 1 || d == shared_default) detach_helper(); }
    void detach_helper();
    struct QColorGroupData
    {
	QAtomic ref;
	QBrush br[QColorGroup::NColorRoles];
    };
    QColorGroupData *d;
    static QColorGroupData *shared_default;
};

class Q_EXPORT QPalette
{
public:
    QPalette();
    QPalette(const QColor &button);
    QPalette(const QColor &button, const QColor &background);
    QPalette(const QColorGroup &active, const QColorGroup &disabled, const QColorGroup &inactive);
    QPalette(const QPalette &palette);
    ~QPalette();
    QPalette &operator=(const QPalette &palette);

    enum ColorGroup { Disabled, Active, Inactive, NColorGroups, Normal=Active };

    inline const QColor &color(ColorGroup cg, QColorGroup::ColorRole cr) const
    { return directBrush(cg, cr).color(); }
    inline const QBrush &brush(ColorGroup cg, QColorGroup::ColorRole cr) const
    { return directBrush(cg, cr); }
    inline void setColor(ColorGroup cg, QColorGroup::ColorRole cr, const QColor &color)
    { setBrush(cg, cr, QBrush(color)); }
    void setBrush(ColorGroup cg, QColorGroup::ColorRole cr, const QBrush &brush);

    inline void setColor(QColorGroup::ColorRole cr, const QColor &color) { setBrush(cr, color); }
    void setBrush(QColorGroup::ColorRole cr, const QBrush &brush);

#ifndef QT_NO_COMPAT
    QPalette copy() const { QPalette p = *this; p.detach(); return p; }
    inline const QColorGroup &normal() const { return active(); }
    inline void setNormal(const QColorGroup &cg) { setActive(cg); }
#endif

    inline const QColorGroup &active() const { return d->active; }
    inline const QColorGroup &disabled() const { return d->disabled; }
    inline const QColorGroup &inactive() const { return d->inactive; }
    void setActive(const QColorGroup &cg);
    void setDisabled(const QColorGroup &cg);
    void setInactive(const QColorGroup &cg);

    bool operator==(const QPalette &p) const;
    inline bool operator!=(const QPalette &p) const { return !(operator==(p)); }
    bool isCopyOf(const QPalette &p);

    int	serialNumber() const { return d->ser_no; }

    static QColorGroup::ColorRole foregroundRoleFromMode(Qt::BackgroundMode mode);
    static QColorGroup::ColorRole backgroundRoleFromMode(Qt::BackgroundMode mode);

private:
    void detach() { if (d->ref != 1) detach_helper(); }
    void detach_helper();
    const QBrush &directBrush(ColorGroup cg, QColorGroup::ColorRole cr) const;
    void directSetBrush(ColorGroup cg, QColorGroup::ColorRole cr, const QBrush &brush);

    struct QPaletteData {
	QAtomic ref;
	QColorGroup disabled;
	QColorGroup active;
	int	    ser_no;
	QColorGroup inactive;
    };
    QPaletteData *d;
    static QPaletteData *shared_default;
    static int palette_count;
};


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<(QDataStream &ds, const QColorGroup &cg);
Q_EXPORT QDataStream &operator>>(QDataStream &ds, QColorGroup &cg);

Q_EXPORT QDataStream &operator<<(QDataStream &ds, const QPalette &p);
Q_EXPORT QDataStream &operator>>(QDataStream &ds, QPalette &p);
#endif // QT_NO_DATASTREAM

#endif // QT_NO_PALETTE
#endif // QPALETTE_H
