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

#ifndef PROPERTYEDITORVIEW_H
#define PROPERTYEDITORVIEW_H

#include <QMainWindow>

class AbstractFormEditor;

class PropertyEditorView: public QMainWindow
{
    Q_OBJECT
public:
    PropertyEditorView(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~PropertyEditorView();

    virtual QSize sizeHint() const
    { return QMainWindow::sizeHint() + QSize(150, 0); }

protected:
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);

signals:
    void visibilityChanged(bool isVisible);

private:
    AbstractFormEditor *m_core;
};

#endif // PROPERTYEDITORVIEW_H
