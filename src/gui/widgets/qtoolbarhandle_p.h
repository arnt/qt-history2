/****************************************************************************
**
** Definition of QToolBarHandle widget class.
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

#ifndef QTOOLBARHANDLE_P_H
#define QTOOLBARHANDLE_P_H

#include <qwidget.h>

class QToolBar;

class QToolBarHandle : public QWidget
{
public:
    QToolBarHandle(QToolBar *parent);

    Qt::Orientation orientation();

    QSize sizeHint() const;

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    struct DragState {
	QPoint offset;
	bool canDrop;
    };
    DragState *state;
};

#endif // QTOOLBARHANDLE_P_H
