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
#include "QtGui/qpixmap.h"

class QIconPrivate;
class QIconEngine;

class Q_GUI_EXPORT QIcon
{
public:
    enum Mode { Normal, Disabled, Active };
    enum State { On, Off };

    QIcon();
    QIcon(const QPixmap &pixmap);
    QIcon(const QIcon &other);
    explicit QIcon(const QString &fileName); // file or resource name
    explicit QIcon(QIconEngine *engine);
    ~QIcon();
    QIcon &operator=(const QIcon &other);

    QPixmap pixmap(const QSize &size, Mode mode = Normal, State state = Off) const;
    inline QPixmap pixmap(int w, int h, Mode mode = Normal, State state = Off) const
        { return pixmap(QSize(w, h), mode, state); }
    QPixmap pixmap(Qt::IconSize size, Mode mode = Normal, State state = Off) const;

    void paint(QPainter *painter, const QRect &rect, Qt::Alignment alignment = Qt::AlignCenter, Mode mode = Normal, State state = Off) const;
    inline void paint(QPainter *painter, int x, int y, int w, int h, Qt::Alignment alignment = Qt::AlignCenter, Mode mode = Normal, State state = Off) const
        { paint(painter, QRect(x, y, w, h), alignment, mode, state); }

    bool isNull() const;
    bool isDetached() const;

    void addPixmap(const QPixmap &pixmap, Mode mode = Normal, State state = Off);

    static QSize sizeHint(Qt::IconSize size);

#ifdef QT_COMPAT
    enum Size { Small, Large, Automatic = Small };
    static QT_COMPAT void setPixmapSize(Size which, const QSize &size);
    static QT_COMPAT QSize pixmapSize(Size which);
    inline QT_COMPAT void reset(const QPixmap &pixmap, Size size) { *this = QIcon(pixmap); }
    inline QT_COMPAT void setPixmap(const QPixmap &pixmap, Size, Mode mode = Normal, State state = Off)
        { addPixmap(pixmap, mode, state); }
    inline QT_COMPAT void setPixmap(const QString &fileName, Size, Mode mode = Normal, State state = Off)
        { addPixmap(QPixmap(fileName), mode, state); }
    QT_COMPAT QPixmap pixmap(Size size, Mode mode, State state = Off) const;
    QT_COMPAT QPixmap pixmap(Size size, bool enabled, State state = Off) const;
    QT_COMPAT QPixmap pixmap() const;
#endif

    Q_DUMMY_COMPARISON_OPERATOR(QIcon)

private:
    QIconPrivate *d;
};

Q_DECLARE_SHARED(QIcon);
Q_DECLARE_TYPEINFO(QIcon, Q_MOVABLE_TYPE);

#ifdef QT_COMPAT
typedef QIcon QIconSet;
#endif


#endif
