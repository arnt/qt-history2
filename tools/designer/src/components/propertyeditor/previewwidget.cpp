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

#include "qobject.h"
#include "previewwidget.h"

#include <qevent.h>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
	setupUi(this);
    installEventFilters(this);
}

void PreviewWidget::installEventFilters(QWidget *parent)
{
    QObjectList objlist = parent->children();
	QWidget *wobj;

	for (int i = 0; i < objlist.size(); ++i) {
        wobj = qt_cast<QWidget *>(objlist.at(i));
		if (wobj != 0) {
			installEventFilters(wobj);
			wobj->installEventFilter(this);
			wobj->setFocusPolicy(Qt::NoFocus);
		}
    }
}

void PreviewWidget::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

bool PreviewWidget::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Enter:
    case QEvent::Leave:
	return true; // ignore;
    default:
	break;
    }
    return FALSE;
}
