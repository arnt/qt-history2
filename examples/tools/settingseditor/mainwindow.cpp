#include <QtGui>

#include "mainwindow.h"
#include "settingstree.h"

MainWindow::MainWindow()
{
    settingsTree = new SettingsTree;
    setCentralWidget(settingsTree);

    createActions();
    createMenus();

    setWindowTitle(tr("Settings Editor"));
}

void MainWindow::openSettings()
{
}

void MainWindow::openIniFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open INI File"),
                               "", tr("INI Files (*.ini *.conf)"));
    if (!fileName.isEmpty()) {
        QSettings *settings = new QSettings(fileName, QSettings::IniFormat);
        settingsTree->setSettings(settings);
        refreshAct->setEnabled(true);
    }
}

void MainWindow::openPropertyList()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                               tr("Open Property List"),
                               "", tr("Property List Files (*.plist)"));
    if (!fileName.isEmpty()) {
        QSettings *settings = new QSettings(fileName, QSettings::NativeFormat);
        settingsTree->setSettings(settings);
        refreshAct->setEnabled(true);
    }
}

void MainWindow::openRegistryPath()
{
    QString path = QInputDialog::getText(this, tr("Open Registry Path"),
                           tr("Enter the path in the Windows registry:"));
    if (!path.isEmpty()) {
        QSettings *settings = new QSettings(path, QSettings::NativeFormat);
        settingsTree->setSettings(settings);
        refreshAct->setEnabled(true);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Settings Editor"),
            tr("The <b>Settings Editor</b> example shows how to access "
               "application settings using Qt."));
}

void MainWindow::createActions()
{
    openSettingsAct = new QAction(tr("&Open Settings..."), this);
    openSettingsAct->setShortcut(tr("Ctrl+O"));
    connect(openSettingsAct, SIGNAL(triggered()), this, SLOT(openSettings()));

    openIniFileAct = new QAction(tr("Open I&NI File..."), this);
    openIniFileAct->setShortcut(tr("Ctrl+N"));
    connect(openIniFileAct, SIGNAL(triggered()), this, SLOT(openIniFile()));

    openPropertyListAct = new QAction(tr("Open Mac &Property List..."), this);
    openPropertyListAct->setShortcut(tr("Ctrl+P"));
    connect(openPropertyListAct, SIGNAL(triggered()),
            this, SLOT(openPropertyList()));

    openRegistryPathAct = new QAction(tr("Open Windows &Registry Path..."),
                                      this);
    openRegistryPathAct->setShortcut(tr("Ctrl+G"));
    connect(openRegistryPathAct, SIGNAL(triggered()),
            this, SLOT(openRegistryPath()));

    refreshAct = new QAction(tr("&Refresh"), this);
    refreshAct->setShortcut(tr("Ctrl+R"));
    refreshAct->setEnabled(false);
    connect(refreshAct, SIGNAL(triggered()), settingsTree, SLOT(refresh()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    autoRefreshAct = new QAction(tr("&Auto-Refresh"), this);
    autoRefreshAct->setCheckable(true);
    connect(autoRefreshAct, SIGNAL(checked(bool)),
            settingsTree, SLOT(setAutoRefresh(bool)));
    connect(autoRefreshAct, SIGNAL(checked(bool)),
            refreshAct, SLOT(setDisabled(bool)));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

#ifndef Q_WS_MAC
    openPropertyListAct->setEnabled(false);
#endif
#ifndef Q_WS_WIN
    openRegistryPathAct->setEnabled(false);
#endif
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openSettingsAct);
    fileMenu->addAction(openIniFileAct);
    fileMenu->addAction(openPropertyListAct);
    fileMenu->addAction(openRegistryPathAct);
    fileMenu->addSeparator();
    fileMenu->addAction(refreshAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addAction(autoRefreshAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}
