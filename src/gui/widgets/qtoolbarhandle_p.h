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

#ifndef QTOOLBARHANDLE_P_H
#define QTOOLBARHANDLE_P_H

#include <qwidget.h>

class QToolBar;

class QToolBarHandle : public QWidget
{
    Q_OBJECT
    Qt::Orientation orient;
    struct DragState {
	QPoint offset;
	bool canDrop;
    };
    DragState *state;

public:
    QToolBarHandle(QToolBar *parent);

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
};

#endif // QTOOLBARHANDLE_P_H
