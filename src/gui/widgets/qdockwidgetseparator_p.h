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

#ifndef QDOCKWINDOWSEPARATOR_P_H
#define QDOCKWINDOWSEPARATOR_P_H

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

class QDockWidget;
class QDockWidgetLayout;
class QPainter;

class QDockWidgetSeparator : public QWidget
{
    Q_OBJECT

public:
    QDockWidgetSeparator(QDockWidgetLayout *l, QWidget *parent);

    QRect calcRect(const QPoint &point);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);

    QDockWidgetLayout *layout;

    struct DragState {
	QPoint origin, last;
	QWidget *prevFocus;
    } *state;

    bool hover;
};

#endif // QT_NO_MAINWINDOW
#endif // QDOCKWINDOWSEPARATOR_P_H
