/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QSVGICONENGINE_P_H
#define QSVGICONENGINE_P_H

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_HEADER

QT_MODULE(Svg)

class QSvgIconEnginePrivate;

class Q_SVG_EXPORT QSvgIconEngine : public QIconEngine
{
public:
    QSvgIconEngine();
    virtual ~QSvgIconEngine();
    virtual void paint(QPainter *painter, const QRect &rect,
                       QIcon::Mode mode, QIcon::State state);
    virtual QSize actualSize(const QSize &size, QIcon::Mode mode,
                             QIcon::State state);
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                           QIcon::State state);

    virtual void addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                           QIcon::State state);
    virtual void addFile(const QString &fileName, const QSize &size,
                         QIcon::Mode mode, QIcon::State state);
private:
    QSharedDataPointer<QSvgIconEnginePrivate> d;
};

QT_END_HEADER

#endif
