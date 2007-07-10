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

#include "qvfbflicker.h"
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QDebug>

#include <sys/time.h>

QVFbFlicker::QVFbFlicker(QObject *parent)
    : QObject(parent), mInterval(500)
{
}

QVFbFlicker::~QVFbFlicker()
{
}

void QVFbFlicker::setInterval(uint i)
{
    mInterval = i;
}

uint QVFbFlicker::interval() const
{
    return mInterval;
}

void QVFbFlicker::setSize(const QSize &s)
{
    flickerView = QPixmap(s);
    flickerView.fill(QColor(0, 0, 0, 0));
    lastPixmap = QPixmap(s);
    lastPixmap.fill(QColor(0, 0, 0, 0));
    view = QPixmap(s);
    view.fill(QColor(0, 0, 0, 0));

    depth.resize(s.width()*s.height());
    depth.fill(0);
}

void QVFbFlicker::drawPixmap(int dx, int dy, const QPixmap &pixmap, int sx, int sy, int sw, int sh)
{
    QImage asImage = pixmap.toImage();
    QImage lastImage = lastPixmap.toImage();

    // fade existing drawn map.
    {
        QPainter p(&flickerView);
        p.setPen(Qt::red);
        p.setBrush(Qt::red);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(0,0, flickerView.width(), flickerView.height(), QColor(255,0,0,240));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // and the really dumb way of doing pixels.
        if (mInterval == 0) {
            int x;
            int y;
            for (x = sx; x < sx + sw; ++x) {
                for (y = sy; y < sy + sh; ++y) {
                    if (lastImage.pixel(x+dx, y+dy) != asImage.pixel(x, y))
                        p.drawPoint(x+dx, y+dy);
                }
            }
        } else {
            struct timeval tv;
            gettimeofday(&tv, 0);

            uint mark = (tv.tv_sec % 1000) * 1000 + tv.tv_usec / 1000000;
            int x;
            int y;
            for (x = sx; x < sx + sw; ++x) {
                for (y = sy; y < sy + sh; ++y) {
                    int depthIndex = (y+dy)*view.width() + x+dx;
                    uint lastMark = depth[depthIndex];
                    if (x == 10 &&  y == 10) {
                        qDebug() << "depths(10,10) -"
                            << lastMark << mark << mInterval;
                    }
                    if (lastImage.pixel(x+dx, y+dy) != asImage.pixel(x, y)) {
                        if (lastMark != 0 && (mark - lastMark) < mInterval)

                            p.drawPoint(x+dx, y+dy);
                        depth[depthIndex] = mark;
                    } else if (lastMark != 0 && mark - lastMark >= mInterval) {
                        depth[depthIndex] = 0;
                    }
                }
            }
        }
    } 
    {
        QPainter p(&lastPixmap);
        p.drawPixmap(dx, dy, pixmap, sx, sy, sw, sh);
    }
    {
        QPainter p(&view);
        p.drawPixmap(dx, dy, pixmap, sx, sy, sw, sh);
        p.drawPixmap(0, 0, flickerView);
    }
#if 0
    QDateTime nextDrawn = QDateTime::currentDateTime();
    qDebug() << "append - rect:" << area;
    if (lastDrawn.secsTo(nextDrawn) > 2)
    {
        flickerView.fill(QColor(0,0,0,0));
        lastDrawn = nextDrawn;
        return;
    }

    lastDrawn = nextDrawn;
    //qDebug() << "detect";

    QRegion areaR(area);
    if (areaR.intersects(lastDirty)) {
        QRegion areaI = areaR.intersected(lastDirty);

        // fade flickerview by half.
        QPainter p(&flickerView);
#if 1
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(0,0, flickerView.width(), flickerView.height(), QColor(0,0,0,128));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
        p.setPen(Qt::red);
        p.setBrush(QColor(255, 0, 0, 128));
        p.setClipRegion(areaI);
        p.drawRect(areaI.boundingRect());
    }
    lastDirty = areaR;
#endif
}
