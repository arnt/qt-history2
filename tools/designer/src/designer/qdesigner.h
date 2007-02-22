/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDESIGNER_H
#define QDESIGNER_H

#include <QtCore/QPointer>
#include <QtGui/QApplication>


#define qDesigner \
    (static_cast<QDesigner*>(QCoreApplication::instance()))

class QDesignerSettings;
class QDesignerWorkbench;
class QDesignerToolWindow;
class QDesignerServer;
class QDesignerClient;
class QErrorMessage;

class QDesigner: public QApplication
{
    Q_OBJECT
public:
    QDesigner(int &argc, char **argv);
    virtual ~QDesigner();

    QDesignerWorkbench *workbench() const;
    QDesignerServer *server() const;
    QDesignerToolWindow *mainWindow() const;
    void setMainWindow(QDesignerToolWindow *tw);

protected:
    bool event(QEvent *ev);

signals:
    void initialized();

public slots:
    void showErrorMessage(const char *message);

private slots:
    void initialize();
    void callCreateForm();

private:
    void showErrorMessageBox(const QString &);

    QDesignerServer *m_server;
    QDesignerClient *m_client;
    QDesignerWorkbench *m_workbench;
    QPointer<QDesignerToolWindow> m_mainWindow;
    QPointer<QErrorMessage> m_errorMessageDialog;

    QString m_initializationErrors;
    QString m_lastErrorMessage;
    bool m_suppressNewFormShow;
};

#endif // QDESIGNER_H
