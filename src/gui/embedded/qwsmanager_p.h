/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWSMANAGER_P_H
#define QWSMANAGER_P_H

#include <qregion.h>
#include <qdecoration_qws.h>

class QWidget;
class QMenu;

#ifndef QT_NO_QWS_MANAGER

class QWSManagerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWSManager)
public:
    QWSManagerPrivate();

    bool doPaint(int decorationRegion = QDecoration::All, QDecoration::DecorationState state = QDecoration::Normal);


    int activeRegion;
    QWidget *managed;
    QMenu *popup;

    enum MenuAction {
        NormalizeAction,
        TitleAction,
        BottomRightAction,
        MinimizeAction,
        MaximizeAction,
        CloseAction,
        LastMenuAction
    };
    QAction *menuActions[LastMenuAction];

    static QWidget *active;
    static QPoint mousePos;

    // Region caching to avoid getting a regiontype's
    // QRegion for each mouse move event
    int previousRegionType;
    bool previousRegionRepainted; // Hover/Press handled
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

#endif
#endif
