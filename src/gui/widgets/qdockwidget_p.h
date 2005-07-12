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

#ifndef QDOCKWIDGET_P_H
#define QDOCKWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qstyleoption.h>
#include "private/qwidget_p.h"

#ifndef QT_NO_DOCKWIDGET

class QBoxLayout;
class QGridLayout;
class QWidgetResizeHandler;
class QRubberBand;
class QDockWidgetTitleButton;

class QDockWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDockWidget)

    struct DragState {
        QRubberBand *rubberband;
        QRect origin;   // starting position
        QRect current;  // current size of the dockwidget (can be either placed or floating)
        QRect floating; // size of the floating dockwidget
        QPoint offset;
        bool canDrop;
    };

public:
    inline QDockWidgetPrivate()
	: QWidgetPrivate(), state(0), widget(0),
          features(QDockWidget::DockWidgetClosable
                   | QDockWidget::DockWidgetMovable
                   | QDockWidget::DockWidgetFloatable),
          allowedAreas(Qt::AllDockWidgetAreas), top(0), box(0),
          leftSpacer(0), rightSpacer(0), bottomSpacer(0),
          floatButton(0), closeButton(0), resizer(0)
    { }

    void init();
    void toggleView(bool); // private slot
    void toggleTopLevel(); // private slot

    QStyleOptionDockWidget getStyleOption();

    void updateButtons();
    void relayout();
    DragState *state;

    QWidget *widget;

    QDockWidget::DockWidgetFeatures features;
    Qt::DockWidgetAreas allowedAreas;

    QGridLayout *top;
    QBoxLayout *box;
    QSpacerItem *leftSpacer, *rightSpacer, *topSpacer, *bottomSpacer;
    QRect titleArea;
    QDockWidgetTitleButton *floatButton;
    QDockWidgetTitleButton *closeButton;

    QWidgetResizeHandler *resizer;
#ifndef QT_NO_ACTION
    QAction *toggleViewAction;
#endif
};

#endif // QT_NO_DOCKWIDGET
#endif // QDOCKWIDGET_P_H
