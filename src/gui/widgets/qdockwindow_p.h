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

#ifndef QDOCKWINDOW_P_H
#define QDOCKWINDOW_P_H

#include "qframe_p.h"

class QBoxLayout;
class QDockWindowTitle;
class QWidgetResizeHandler;

class QDockWindowPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QDockWindow);

public:
    inline QDockWindowPrivate(QMainWindow *parent)
	: QFramePrivate(), mainWindow(parent), widget(0),
          features(QDockWindow::DockWindowClosable
                   | QDockWindow::DockWindowMovable
                   | QDockWindow::DockWindowFloatable),
          area(Qt::DockWindowAreaLeft), allowedAreas(~0u & Qt::DockWindowAreaMask),
          top(0), box(0), title(0), resizer(0)
    { }

    void init();
    void place(Qt::DockWindowArea area, Qt::Orientation direction, bool extend);

    QMainWindow *mainWindow;
    QWidget *widget;

    QDockWindow::DockWindowFeatures features;
    Qt::DockWindowArea area;
    Qt::DockWindowAreas allowedAreas;

    QBoxLayout *top, *box;
    QDockWindowTitle *title;

    QWidgetResizeHandler *resizer;
};

#endif // QDOCKWINDOW_P_H
