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

#include "QtGui/qpainter.h"

QT_MODULE(Gui)

#ifdef Q_WS_QWS

#define QT_NO_DIRECTPAINTER

#ifndef QT_NO_DIRECTPAINTER
class QDirectPainterPrivate;

class Q_GUI_EXPORT QDirectPainter : public QPainter {
public:
    explicit QDirectPainter(QWidget*);
    ~QDirectPainter();

    uchar* frameBuffer();
    int lineStep();
    int transformOrientation();

    int numRects() const;
    const QRect& rect(int i) const;
    QRegion region() const;

    int depth() const;
    int width() const;
    int height() const;
    int xOffset() const;
    int yOffset() const;

    QPoint offset() const;
    QSize size() const;

    void setAreaChanged(const QRect&);

private:
    QDirectPainterPrivate *d;
};

#endif
#endif

#endif // QDIRECTPAINTER_QWS_H
