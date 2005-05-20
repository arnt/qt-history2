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
#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/qdebug.h>

#include <QtDesigner/QDesignerComponents>

// designer
#include "qdesigner.h"
#include "qdesigner_actions.h"
#include "qdesigner_server.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"
#include "qdesigner_toolwindow.h"

QDesigner::QDesigner(int &argc, char **argv)
    : QApplication(argc, argv),
      m_server(0),
      m_client(0)
{
    setOrganizationName(QLatin1String("Trolltech"));
    setApplicationName(QLatin1String("Designer"));

#ifndef Q_WS_MAC
    setWindowIcon(QIcon(QLatin1String(":/trolltech/designer/images/designer.png")));
#endif

    initialize();
}

QDesigner::~QDesigner()
{
    delete m_workbench;
    delete m_server;
    delete m_client;
}

QDesignerWorkbench *QDesigner::workbench() const
{
    return m_workbench;
}

QDesignerServer *QDesigner::server() const
{
    return m_server;
}

void QDesigner::initialize()
{
    // initialize the sub components
    QStringList files;

    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    for (int i = 1; i < argc(); ++i)
    {
        if (QString::fromLocal8Bit(argv()[i]) == QLatin1String("-server")) {
            m_server = new QDesignerServer();
            printf("%d\n", m_server->serverPort());
            fflush(stdout);
        } else if (QString::fromLocal8Bit(argv()[i]) == QLatin1String("-client")) {
            bool ok = true;
            if (i + 1 < argc()) {
                quint16 port = QString::fromLocal8Bit(argv()[++i]).toUShort(&ok);
                if (ok)
                    m_client = new QDesignerClient(port, this);
            }
        } else if (QString::fromLocal8Bit(argv()[i]) == QLatin1String("-resourcedir")) {
            if (i + 1 < argc()) {
                resourceDir = QFile::decodeName(argv()[++i]);
            } else {
                // issue a warning
            }
        } else {
            files.append(QString::fromLocal8Bit(argv()[i]));
        }
    }

    QTranslator *translator = new QTranslator;
    QTranslator *qtTranslator = new QTranslator;
    translator->load(QLatin1String("designer_") + QLocale::system().name().toLower(), resourceDir);
    qtTranslator->load(QLatin1String("qt_") + QLocale::system().name().toLower(), resourceDir);
    installTranslator(translator);
    installTranslator(qtTranslator);

    m_workbench = new QDesignerWorkbench();

    emit initialized();

    if (files.isEmpty()) {
        if (QDesignerSettings().showNewFormOnStartup())
            m_workbench->actionManager()->createForm();
    } else {
        for (int arg = 0; arg < files.count(); ++arg)
            m_workbench->readInForm(files.at(arg));
    }
}

bool QDesigner::event(QEvent *ev)
{
    bool eaten;
    switch (ev->type()) {
    case QEvent::FileOpen:
        m_workbench->readInForm(static_cast<QFileOpenEvent *>(ev)->file());
        eaten = true;
        break;
    case QEvent::Close: {
        QCloseEvent *closeEvent = static_cast<QCloseEvent *>(ev);
        closeEvent->setAccepted(m_workbench->handleClose());
        if (closeEvent->isAccepted()) {
            // We're going down, make sure that we don't get our settings saved twice.
            if (m_mainWindow)
                m_mainWindow->setSaveSettingsOnClose(false);
            eaten = QApplication::event(ev);
        }
        eaten = true;
        break;
    }
    default:
        eaten = QApplication::event(ev);
        break;
    }
    return eaten;
}

void QDesigner::setMainWindow(QDesignerToolWindow *tw)
{
    m_mainWindow = tw;
}

QDesignerToolWindow *QDesigner::mainWindow() const
{
    return m_mainWindow;
}
