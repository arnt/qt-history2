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

#ifndef QICON_H
#define QICON_H

#include "QtCore/qglobal.h"
#include "QtCore/qsize.h"

#ifdef QT_INCLUDE_COMPAT
#include "QtCore/qobject.h"
#include "QtGui/qpixmap.h"
#endif

#ifndef QT_NO_ICON

class QIconPrivate;
class QPixmap;
class QString;

class Q_GUI_EXPORT QIcon
{
public:
    enum Mode { Normal, Disabled, Active };
    enum State { On, Off };

    QIcon();
    QIcon(const QPixmap &pixmap, Qt::IconSize size = Qt::AutomaticIconSize);
    QIcon(const QPixmap &smallPix, const QPixmap &largePix);
    QIcon(const QIcon &other);
    ~QIcon();
    QIcon &operator=(const QIcon &other);

    void reset(const QPixmap &pixmap, Qt::IconSize size);
    void setPixmap(const QPixmap &pixmap, Qt::IconSize size, Mode mode = Normal, State state = Off);
    void setPixmap(const QString &fileName, Qt::IconSize size, Mode mode = Normal, State state = Off);
    QPixmap pixmap(Qt::IconSize size, Mode mode, State state = Off) const;
    QPixmap pixmap(Qt::IconSize size, bool enabled, State state = Off) const;
    QPixmap pixmap() const;
    bool isGenerated(Qt::IconSize size, Mode mode, State state = Off) const;
    void clearGenerated();
    typedef QPixmap *(*PixmapGeneratorFn)(const QIcon &icon, Qt::IconSize size, Mode mode, State state);
    void setPixmapGeneratorFn(PixmapGeneratorFn func);

    bool isNull() const;
    void detach();
    bool isDetached() const;

    static void setPixmapSize(Qt::IconSize which, const QSize &size);
    static QSize pixmapSize(Qt::IconSize which);
    static void setDefaultPixmapGeneratorFn(PixmapGeneratorFn func);
    inline static PixmapGeneratorFn defaultPixmapGeneratorFn() { return defaultGeneratorFn; }

#ifdef QT_COMPAT
    inline static QT_COMPAT void setIconSize(Qt::IconSize which, const QSize &size)
        { setPixmapSize(which, size); }
    inline static QT_COMPAT QSize iconSize(Qt::IconSize which)
        { return pixmapSize(which); }
#endif

    Q_DUMMY_COMPARISON_OPERATOR(QIcon)

private:
    QIconPrivate *d;

    friend class QIconPrivate;
    static PixmapGeneratorFn defaultGeneratorFn;
};
Q_DECLARE_SHARED(QIcon);
Q_DECLARE_TYPEINFO(QIcon, Q_MOVABLE_TYPE);

#ifdef QT_COMPAT
typedef QIcon QIconSet;
#endif

#endif // QT_NO_ICON

#endif // QICON_H
