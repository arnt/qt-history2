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

#include <QtGui/QFileOpenEvent>
#include <QtGui/QCloseEvent>
#include <qdebug.h>

// designer
#include "qdesigner.h"
#include "qdesigner_server.h"
#include "qdesigner_settings.h"
#include "qdesigner_session.h"
#include "qdesigner_mainwindow.h"

extern int qInitResources_formeditor();
extern int qInitResources_widgetbox();

QDesigner::QDesigner(int &argc, char **argv)
    : QApplication(argc, argv),
      m_server(0),
      m_session(0),
      m_mainWindow(0)
{
    setOrganizationDomain(QLatin1String("Trolltech"));
    setApplicationName(QLatin1String("Designer"));

    qInitResources_formeditor();
    qInitResources_widgetbox();

    initialize();
}

QDesigner::~QDesigner()
{
}

QDesignerSession *QDesigner::session() const
{
    return m_session;
}

QDesignerMainWindow *QDesigner::mainWindow() const
{
    return m_mainWindow;
}

QDesignerServer *QDesigner::server() const
{
    return m_server;
}

void QDesigner::initialize()
{
    // initialize the sub components
    m_server = new QDesignerServer(this);
    m_session = new QDesignerSession(this);
    m_mainWindow = new QDesignerMainWindow();

    setMainWidget(m_mainWindow);

    emit initialized();

    for (int i = 1; i < argc(); ++i)
        m_mainWindow->readInForm(QString::fromLocal8Bit(argv()[i]));

    m_mainWindow->show();
}

bool QDesigner::event(QEvent *ev)
{
    bool eaten;
    switch (ev->type()) {
    case QEvent::FileOpen:
        m_mainWindow->readInForm(static_cast<QFileOpenEvent *>(ev)->file());
        eaten = true;
        break;
    case QEvent::Close: {
        QCloseEvent *closeEvent = static_cast<QCloseEvent *>(ev);
        sendEvent(m_mainWindow, closeEvent);
        if (closeEvent->isAccepted())
            eaten = QApplication::event(ev);
        eaten = true;
        break;
    }
    default:
        eaten = QApplication::event(ev);
        break;
    }
    return eaten;
}
