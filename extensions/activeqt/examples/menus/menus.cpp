/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
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

#include "menus.h"

#include "fileopen.xpm"
#include "filesave.xpm"

QMenus::QMenus(QWidget *parent) 
    : QMainWindow(parent, 0) // QMainWindow's default flag is WType_TopLevel
{
    QAction *action;

    QMenu *file = new QMenu(this);

    action = new QAction(QPixmap((const char**)fileopen), "&Open", CTRL+Key_O, this);
    connect(action, SIGNAL(activated()), this, SLOT(fileOpen()));
    file->addAction(action);

    action = new QAction(QPixmap((const char**)filesave),"&Save", CTRL+Key_S, this);
    connect(action, SIGNAL(activated()), this, SLOT(fileSave()));
    file->addAction(action);

    QMenu *edit = new QMenu(this);

    action = new QAction("&Normal", CTRL+Key_N, this);
    action->setToolTip("Normal");
    action->setStatusTip("Toggles Normal");
    action->setCheckable(true);
    connect(action, SIGNAL(activated()), this, SLOT(editNormal()));
    edit->addAction(action);

    action = new QAction("&Bold", CTRL+Key_B, this);
    action->setCheckable(true);
    connect(action, SIGNAL(activated()), this, SLOT(editBold()));
    edit->addAction(action);

    action = new QAction("&Underline", CTRL+Key_U, this);
    action->setCheckable(true);
    connect(action, SIGNAL(activated()), this, SLOT(editUnderline()));
    edit->addAction(action);

    QMenu *advanced = new QMenu(this);
    action = new QAction("&Font...", this);
    connect(action, SIGNAL(activated()), this, SLOT(editAdvancedFont()));
    advanced->addAction(action);

    action = new QAction("&Style...", this);
    connect(action, SIGNAL(activated()), this, SLOT(editAdvancedStyle()));
    advanced->addAction(action);

    edit->addMenu("&Advanced", advanced);

    edit->addSeparator();

    action = new QAction("Una&vailable", CTRL+Key_V, this);
    action->setCheckable(true);
    action->setEnabled(false);
    connect(action, SIGNAL(activated()), this, SLOT(editUnderline()));
    edit->addAction(action);

    QMenu *help = new QMenu(this);

    action = new QAction("&About...", Key_F1, this);
    connect(action, SIGNAL(activated()), this, SLOT(helpAbout()));
    help->addAction(action);

    action = new QAction("&About Qt...", 0, this);
    connect(action, SIGNAL(activated()), this, SLOT(helpAboutQt()));
    help->addAction(action);

    if (!QAxFactory::isServer())
        menuBar()->addMenu("&File", file);
    menuBar()->addMenu("&Edit", edit);
    menuBar()->addMenu("&Help", help);

    editor = new QTextEdit(this, "editor");
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
