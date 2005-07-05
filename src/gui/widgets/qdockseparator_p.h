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

#ifndef QDOCKSEPARATOR_P_H
#define QDOCKSEPARATOR_P_H

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

#include <qwidget.h>

#ifndef QT_NO_MAINWINDOW

class QDockWidgetLayout;

class QDockSeparator : public QWidget
{
    Q_OBJECT
public:
    QDockSeparator(QDockWidgetLayout *dock, QWidget *parent);

    void setDock(QDockWidgetLayout *d);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);

    QDockWidgetLayout *dock;
    Qt::Orientation orientation;
    bool hover;

    struct DragState {
	QPoint origin;
	QWidget *prevFocus;
    } *state;
};

#endif // QT_NO_MAINWINDOW
#endif // QDOCKSEPARATOR_P_H
