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

#ifndef QDESIGNER_TOOLWINDOW_H
#define QDESIGNER_TOOLWINDOW_H

#include <QMainWindow>
#include <QPointer>

class QDesignerWorkbench;

class QDesignerToolWindow: public QMainWindow
{
    Q_OBJECT
public:
    QDesignerToolWindow(QDesignerWorkbench *workbench, QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~QDesignerToolWindow();

    QDesignerWorkbench *workbench() const;
    QAction *action() const;

    virtual QRect geometryHint() const;

protected:
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);
    virtual void changeEvent(QEvent *e);

private:
    QDesignerWorkbench *m_workbench;
    QAction *m_action;
};

#endif // QDESIGNER_TOOLWINDOW_H
