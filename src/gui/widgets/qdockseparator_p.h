/****************************************************************************
**
** Definition of QDockSeparator widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

#ifndef QDOCKSEPARATOR_P_H
#define QDOCKSEPARATOR_P_H

#include <qwidget.h>

class QDockWindowLayout;

class QDockSeparator : public QWidget
{
public:
    QDockSeparator(QDockWindowLayout *dock, QWidget *parent);

    void setDock(QDockWindowLayout *d);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    QDockWindowLayout *dock;
    Qt::Orientation orientation;

    struct DragState {
	QPoint origin;
	QWidget *prevFocus;
    } *state;
};

#endif // QDOCKSEPARATOR_P_H
