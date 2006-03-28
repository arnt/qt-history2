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

#ifndef QDIRECTPAINTER_QWS_H
#define QDIRECTPAINTER_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qregion.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_DIRECTPAINTER
class QDirectPainterPrivate;

class Q_GUI_EXPORT QDirectPainter : public QObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDirectPainter)
public:
    //    explicit QDirectPainter(const QRegion&);
    //~QDirectPainter();

    static QRegion reserveRegion(const QRegion&);
    static QRegion region();

    static uchar* frameBuffer();
    static int screenDepth();
    static int screenWidth();
    static int screenHeight();
    static int linestep();

    static void lock();
    static void unlock();
};

#endif

QT_END_HEADER

#endif // QDIRECTPAINTER_QWS_H
