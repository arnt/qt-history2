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

#include <qaction.h>
#include <qaxfactory.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qtextedit.h>
#include <qpixmap.h>

#include "menus.h"

#include "fileopen.xpm"
#include "filesave.xpm"

QMenus::QMenus(QWidget *parent) 
    : QMainWindow(parent, 0) // QMainWindow's default flag is WType_TopLevel
{
    QAction *action;

    QMenu *file = new QMenu(this);

    action = new QAction(QPixmap((const char**)fileopen), "&Open", this);
    action->setShortcut(tr("CTRL+O"));
    connect(action, SIGNAL(triggered()), this, SLOT(fileOpen()));
    file->addAction(action);

    action = new QAction(QPixmap((const char**)filesave),"&Save", this);
    action->setShortcut(tr("CTRL+S"));
    connect(action, SIGNAL(triggered()), this, SLOT(fileSave()));
    file->addAction(action);

    QMenu *edit = new QMenu(this);

    action = new QAction("&Normal", this);
    action->setShortcut(tr("CTRL+N"));
    action->setToolTip("Normal");
    action->setStatusTip("Toggles Normal");
    action->setCheckable(true);
    connect(action, SIGNAL(triggered()), this, SLOT(editNormal()));
    edit->addAction(action);

    action = new QAction("&Bold", this);
    action->setShortcut(tr("CTRL+B"));
    action->setCheckable(true);
    connect(action, SIGNAL(triggered()), this, SLOT(editBold()));
    edit->addAction(action);

    action = new QAction("&Underline", this);
    action->setShortcut(tr("CTRL+U"));
    action->setCheckable(true);
    connect(action, SIGNAL(triggered()), this, SLOT(editUnderline()));
    edit->addAction(action);

    QMenu *advanced = new QMenu(this);
    action = new QAction("&Font...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(editAdvancedFont()));
    advanced->addAction(action);

    action = new QAction("&Style...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(editAdvancedStyle()));
    advanced->addAction(action);

    edit->addMenu(advanced)->setText("&Advanced");

    edit->addSeparator();

    action = new QAction("Una&vailable", this);
    action->setShortcut(tr("CTRL+V"));
    action->setCheckable(true);
    action->setEnabled(false);
    connect(action, SIGNAL(triggered()), this, SLOT(editUnderline()));
    edit->addAction(action);

    QMenu *help = new QMenu(this);

    action = new QAction("&About...", this);
    action->setShortcut(tr("F1"));
    connect(action, SIGNAL(triggered()), this, SLOT(helpAbout()));
    help->addAction(action);

    action = new QAction("&About Qt...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(helpAboutQt()));
    help->addAction(action);

    if (!QAxFactory::isServer())
        menuBar()->addMenu(file)->setText("&File");
    menuBar()->addMenu(edit)->setText("&Edit");
    menuBar()->addMenu(help)->setText("&Help");

    editor = new QTextEdit(this);
    setCentralWidget(editor);

    statusBar();
}

void QMenus::fileOpen()
{
    editor->append("File Open selected.");
}

void QMenus::fileSave()
{
    editor->append("File Save selected.");
}

void QMenus::editNormal()
{
    editor->append("Edit Normal selected.");
}

void QMenus::editBold()
{
    editor->append("Edit Bold selected.");
}

void QMenus::editUnderline()
{
    editor->append("Edit Underline selected.");
}

void QMenus::editAdvancedFont()
{
    editor->append("Edit Advanced Font selected.");
}

void QMenus::editAdvancedStyle()
{
    editor->append("Edit Advanced Style selected.");
}

void QMenus::helpAbout()
{
    QMessageBox::about(this, "About QMenus", 
			"This example implements an in-place ActiveX control with menus and status messages.");
}

void QMenus::helpAboutQt()
{
    QMessageBox::aboutQt(this);
}
