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

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>

#include "mainwindow.h"

#include <QtMotif/QMotifWidget>

#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>

MainWindow::MainWindow()
    : QMainWindow()
{
    QMenu *filemenu = new QMenu(tr("&File"), this);
    filemenu->addAction( tr("&Quit"), qApp, SLOT(quit()) );

    menuBar()->addMenu(filemenu);
    statusBar()->showMessage( tr("This is a QMainWindow with an XmText widget.") );

    customwidget = new QMotifWidget("form", xmFormWidgetClass, this);

    XmString str;
    Arg args[6];

    str = XmStringCreateLocalized( "Push Button (XmPushButton)" );
    XtSetArg( args[0], XmNlabelString, str );
    XtSetArg( args[1], XmNleftAttachment, XmATTACH_FORM );
    XtSetArg( args[2], XmNrightAttachment, XmATTACH_FORM );
    XtSetArg( args[3], XmNbottomAttachment, XmATTACH_FORM );
    Widget button =
	XmCreatePushButton( customwidget->motifWidget(), "Push Button", args, 4 );
    XmStringFree( str );

    XtSetArg( args[0], XmNeditMode, XmMULTI_LINE_EDIT );
    XtSetArg( args[1], XmNleftAttachment, XmATTACH_FORM );
    XtSetArg( args[2], XmNrightAttachment, XmATTACH_FORM );
    XtSetArg( args[3], XmNtopAttachment, XmATTACH_FORM );
    XtSetArg( args[4], XmNbottomAttachment, XmATTACH_WIDGET );
    XtSetArg( args[5], XmNbottomWidget, button );
    Widget texteditor =
	XmCreateScrolledText( customwidget->motifWidget(), "Text Editor", args, 6 );

    XtManageChild( texteditor );
    XtManageChild( button );

    setCentralWidget( customwidget );

    resize( 400, 600 );
}
