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

#ifndef QICONSET_H
#define QICONSET_H

#ifndef QT_H
#include "qglobal.h"
#ifdef QT_INCLUDE_COMPAT
#include "qobject.h"
#include "qpixmap.h"
#endif
#endif // QT_H

#ifndef QT_NO_ICONSET

class QPixmap;
class QIconSetData;
class QString;
class QSize;

class Q_GUI_EXPORT QIconSet
{
public:
    enum Size { Automatic, Small, Large };
    enum Mode { Normal, Disabled, Active };
    enum State { On, Off };

    QIconSet() : d(0) {}
    QIconSet(const QPixmap& pixmap, Size size = Automatic);
    QIconSet(const QPixmap& smallPix, const QPixmap& largePix);
    QIconSet(const QIconSet& other);
    ~QIconSet();

    void reset(const QPixmap& pixmap, Size size);

    void setPixmap(const QPixmap& pixmap, Size size, Mode mode = Normal, State state = Off);
    void setPixmap(const QString& fileName, Size size, Mode mode = Normal, State state = Off);
    QPixmap pixmap(Size size, Mode mode, State state = Off) const;
    QPixmap pixmap(Size size, bool enabled, State state = Off) const;
    QPixmap pixmap() const;
    bool isGenerated(Size size, Mode mode, State state = Off) const;
    void clearGenerated();
    typedef QPixmap *(*PixmapGeneratorFn)(const QIconSet &icon, Size size, Mode mode, State state);
    void setPixmapGeneratorFn(PixmapGeneratorFn func);

    inline bool isNull() const { return !d; }

    void detach();

    QIconSet& operator=(const QIconSet& other);

    static void setIconSize(Size which, const QSize& size);
    static QSize iconSize(Size which);
    static void setDefaultPixmapGeneratorFn(PixmapGeneratorFn func);
    inline static PixmapGeneratorFn defaultPixmapGeneratorFn() { return defaultGeneratorFn; }

#ifndef Q_QDOC
    Q_DUMMY_COMPARISON_OPERATOR(QIconSet)
#endif

private:
    void normalize(Size& which, const QSize& pixSize);
    QPixmap *createScaled(Size size, const QPixmap *suppliedPix) const;
    QPixmap *createIcon(Size size, Mode mode, State state) const;
    static QPixmap *defaultGenerator(const QIconSet &icon, Size size, Mode mode, State state);

    QIconSetData *d;
    friend class QIconSetData;
    static PixmapGeneratorFn defaultGeneratorFn;
};

#endif // QT_NO_ICONSET
#endif
