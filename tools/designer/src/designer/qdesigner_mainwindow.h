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

#ifndef QDESIGNER_MAINWINDOW_H
#define QDESIGNER_MAINWINDOW_H

#include <QtCore/QHash>
#include <QtGui/QMainWindow>

class QDesignerWorkbench;
class QDesignerToolWindow;
class QMenu;
class QActionGroup;

class AbstractFormEditor;
class AbstractFormWindow;
class QDesignerFormWindowManager;
class QDesignerFormWindow;
class QDesignerActions;

class QDesignerMainWindow: public QMainWindow
{
    Q_OBJECT
public:
    QDesignerMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~QDesignerMainWindow();

    QDesignerWorkbench *workbench() const;
    AbstractFormEditor *core() const;
    bool readInForm(const QString &fileName) const;
    bool writeOutForm(AbstractFormWindow *formWindow, const QString &fileName) const;


signals:
    void initialized();

private slots:
    void addFormWindow(QDesignerFormWindow *formWindow);
    void removeFormWindow(QDesignerFormWindow *formWindow);

    void addToolWindow(QDesignerToolWindow *toolWindow);
    void removeToolWindow(QDesignerToolWindow *toolWindow);

protected:
    virtual void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *ev);

private:
    void initialize();
    void updateWindowState();

private:
    QDesignerWorkbench *m_workbench;
    QDesignerActions *m_actionManager;
    QActionGroup *m_toolActions;
    QActionGroup *m_windowActions;
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_formMenu;
    QMenu *m_toolMenu;
    QMenu *m_windowMenu;
};


#endif // QDESIGNER_MAINWINDOW_H
