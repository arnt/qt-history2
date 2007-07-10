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

#ifndef QVFBFLICKER_H
#define QVFBFLICKER_H

#include <QPixmap>
#include <QImage>
#include <QDateTime>
#include <QRect>
#include <QVector>

class QVFbFlicker : public QObject
{
    Q_OBJECT
public:
    QVFbFlicker(QObject *parent = 0);
    ~QVFbFlicker();

    /* later, maybe flicker settings. */

    void reset();
    void setSize(const QSize &);
    void drawPixmap(int, int, const QPixmap &, int, int, int, int);

    const QPixmap & flickerMap() const { return view; }

    void setInterval(uint = 0);
    uint interval() const;

    uint framesRemaining() const { return mFramesRemaining; }

signals:
    void flickerMapChanged();

private:
    QPixmap flickerView;
    QPixmap view;
    QDateTime lastDrawn;
    QPixmap lastPixmap;
    QVector<int> depth;
    uint mInterval;
    uint mFramesRemaining;
    QTimer *fadeTimer;
};

#endif //  QVFBFLICKER_H
