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

#ifndef QWSMANAGER_QWS_H
#define QWSMANAGER_QWS_H

#include "qpixmap.h"
#include "qobject.h"
#include "qdecoration_qws.h"
#include "qevent.h"

#ifndef QT_NO_QWS_MANAGER

class QAction;
class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSManagerPrivate;

class QWSManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWSManager)
public:
    QWSManager(QWidget *);
    ~QWSManager();

    static QDecoration *newDefaultDecoration();

    QWidget *widget();
    void maximize();

    static QWidget *grabbedMouse();

    QRegion region();
    QRegion &cachedRegion()
    { return cached_region.region; }

protected slots:
    void menuTriggered(QAction *item);
    void styleMenuTriggered(QAction *item);

protected:
    void handleMove(QPoint g);

    virtual bool event(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *) {}
    virtual void paintEvent(QPaintEvent *);
    bool repaintRegion(int region, QDecoration::DecorationState state);

    void menu(const QPoint &);
    void close();
    void minimize();
    void toggleMaximize();


private:
    int previousRegionType;
    bool previousRegionRepainted; // Handled Hover or Pressed?
    struct RegionCaching {
        int regionType;
        QRegion region;
        Qt::WFlags windowFlags;
        QRect windowGeometry;
    } cached_region;

    bool newCachedRegion(const QPoint &pos);
    int cachedRegionAt()
    { return cached_region.regionType; }
};

#include "qdecorationdefault_qws.h"

#endif // QT_NO_QWS_MANAGER

#endif // QWSMANAGER_QWS_H
