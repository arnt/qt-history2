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

#include "qglobal.h"
#include "qsize.h"

#ifdef QT_INCLUDE_COMPAT
#include "qobject.h"
#include "qpixmap.h"
#endif

#ifndef QT_NO_ICON

class QIconPrivate;
class QPixmap;
class QString;

class Q_GUI_EXPORT QIcon
{
public:
#ifdef QT_COMPAT
    enum Size {
        Automatic = Qt::AutomaticIconSize,
        Small = Qt::SmallIconSize,
        Large = Qt::LargeIconSize
    };
#endif
    enum Mode { Normal, Disabled, Active };
    enum State { On, Off };

    QIcon();
    QIcon(const QPixmap &pixmap, QIcon::Size size = QIcon::Automatic);
    QIcon(const QPixmap &smallPix, const QPixmap &largePix);
    QIcon(const QIcon &other);
    ~QIcon();

    void reset(const QPixmap &pixmap, QIcon::Size size);
    void setPixmap(const QPixmap &pixmap, QIcon::Size size, QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    void setPixmap(const QString &fileName, QIcon::Size size, QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    QPixmap pixmap(QIcon::Size size, QIcon::Mode mode, QIcon::State state = QIcon::Off) const;
    QPixmap pixmap(QIcon::Size size, bool enabled, QIcon::State state = QIcon::Off) const;
    QPixmap pixmap() const;
    bool isGenerated(QIcon::Size size, QIcon::Mode mode, QIcon::State state = QIcon::Off) const;
    void clearGenerated();
    typedef QPixmap *(*PixmapGeneratorFn)(const QIcon &icon, QIcon::Size size, QIcon::Mode mode, QIcon::State state);
    void setPixmapGeneratorFn(PixmapGeneratorFn func);

    bool isNull() const;
    void detach();

    QIcon &operator=(const QIcon &other);

    static void setPixmapSize(QIcon::Size which, const QSize &size);
    static QSize pixmapSize(QIcon::Size which);
    static void setDefaultPixmapGeneratorFn(PixmapGeneratorFn func);
    inline static PixmapGeneratorFn defaultPixmapGeneratorFn() { return defaultGeneratorFn; }

#ifdef QT_COMPAT
    inline static QT_COMPAT void setIconSize(QIcon::Size which, const QSize &size)
        { setPixmapSize(which, size); }
    inline static QT_COMPAT QSize iconSize(QIcon::Size which)
        { return pixmapSize(which); }
#endif

    Q_DUMMY_COMPARISON_OPERATOR(QIcon)

private:
    QIconPrivate *d;

    friend class QIconPrivate;
    static PixmapGeneratorFn defaultGeneratorFn;
};

#ifdef QT_COMPAT
typedef QIcon QIconSet;
#endif

#endif // QT_NO_ICON
#endif
