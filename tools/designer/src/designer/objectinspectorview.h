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

#ifndef OBJECTINSPECTORVIEW_H
#define OBJECTINSPECTORVIEW_H

#include <QMainWindow>

class AbstractFormEditor;

class ObjectInspectorView: public QMainWindow
{
    Q_OBJECT
public:
    ObjectInspectorView(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~ObjectInspectorView();

    virtual QSize sizeHint() const
    { return QMainWindow::sizeHint() + QSize(150, 0); }

signals:
    void visibilityChanged(bool isVisible);

protected:
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);

private:
    AbstractFormEditor *m_core;
};

#endif // OBJECTINSPECTORVIEW_H
