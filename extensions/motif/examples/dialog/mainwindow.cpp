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
#include <QEvent>

#include "mainwindow.h"
#include "dialog.h"

#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>


static void motifDialogCallback( Widget, XtPointer client_data, XtPointer )
{
    MainWindow *mw = (MainWindow *) client_data;
    mw->showMotifDialog();
}

static void qtDialogCallback( Widget, XtPointer client_data, XtPointer )
{
    MainWindow *mw = (MainWindow *) client_data;
    mw->showQtDialog();
}

static void quitCallback( Widget, XtPointer client_data, XtPointer )
{
    MainWindow *mw = (MainWindow *) client_data;
    mw->close();
}


MainWindow::MainWindow()
    : QMotifWidget("mainwindow", xmMainWindowWidgetClass, 0)
{
    Widget menubar = XmCreateMenuBar( motifWidget(), "menubar", NULL, 0 );
    Widget filemenu = XmCreatePulldownMenu( menubar, "filemenu", NULL, 0 );
    Widget item;

    item = XtVaCreateManagedWidget( "Motif Dialog...",
				    xmPushButtonGadgetClass, filemenu,
				    XmNmnemonic, 'C',
				    NULL );
    XtAddCallback( item, XmNactivateCallback, motifDialogCallback, this );

    item = XtVaCreateManagedWidget( "Qt Dialog...",
				    xmPushButtonGadgetClass, filemenu,
				    XmNmnemonic, 'Q',
				    NULL );
    XtAddCallback( item, XmNactivateCallback, qtDialogCallback, this );

    item = XtVaCreateManagedWidget( "sep",
				    xmSeparatorGadgetClass, filemenu,
				    NULL );

    item = XtVaCreateManagedWidget( "Exit",
				    xmPushButtonGadgetClass, filemenu,
				    XmNmnemonic, 'x',
				    NULL );
    XtAddCallback( item, XmNactivateCallback, quitCallback, this );

    XmString str = XmStringCreateLocalized( "File" );
    item = XtVaCreateManagedWidget( "File",
				    xmCascadeButtonWidgetClass, menubar,
				    XmNlabelString, str,
				    XmNmnemonic, 'F',
				    XmNsubMenuId, filemenu,
				    NULL );
    XmStringFree( str );

    Arg args[2];
    XtSetArg( args[0], XmNeditMode, XmMULTI_LINE_EDIT );
    Widget texteditor =
	XmCreateScrolledText( motifWidget(), "texteditor",
			      args, 1 );

    XtManageChild( menubar );
    XtManageChild( texteditor );

    // pick a nice default size
    XtVaSetValues( motifWidget(),
 		   XmNwidth, 400,
 		   XmNheight, 300,
 		   NULL );

    setWindowTitle(tr("QMotif Dialog Example"));

}

void MainWindow::showMotifDialog()
{
    QMotifDialog dialog(this);
    dialog.setWindowTitle(tr("Custom Motif Dialog"));

    Widget form = XmCreateForm( dialog.shell(), "custom motif dialog", NULL, 0 );

    XmString str;
    Arg args[9];

    str = XmStringCreateLocalized( "Close" );
    XtSetArg( args[0], XmNlabelString, str );
    XtSetArg( args[1], XmNshowAsDefault, True );
    XtSetArg( args[2], XmNleftAttachment, XmATTACH_POSITION );
    XtSetArg( args[3], XmNleftPosition, 40 );
    XtSetArg( args[4], XmNrightAttachment, XmATTACH_POSITION );
    XtSetArg( args[5], XmNrightPosition, 60 );
    XtSetArg( args[7], XmNbottomAttachment, XmATTACH_FORM );
    XtSetArg( args[6], XmNtopOffset, 10 );
    XtSetArg( args[8], XmNbottomOffset, 10 );
    Widget button = XmCreatePushButton( form, "Close", args, 9 );
    XmStringFree( str );

    str =
        XmStringCreateLocalized( "This is a custom Motif-based dialog using\n"
                                 "QMotifDialog with a QWidget-based parent." );
    XtSetArg( args[0], XmNlabelString, str );
    XtSetArg( args[1], XmNleftAttachment, XmATTACH_FORM );
    XtSetArg( args[2], XmNrightAttachment, XmATTACH_FORM );
    XtSetArg( args[3], XmNtopAttachment, XmATTACH_FORM );
    XtSetArg( args[4], XmNbottomAttachment, XmATTACH_WIDGET );
    XtSetArg( args[5], XmNbottomWidget, button );
    XtSetArg( args[6], XmNtopOffset, 10 );
    XtSetArg( args[7], XmNbottomOffset, 10 );
    Widget label = XmCreateLabelGadget( form, "label", args, 8 );
    XmStringFree( str );

    XtManageChild( button );
    XtManageChild( label );
    XtManageChild( form );

    XtAddCallback( button, XmNactivateCallback,
		   (XtCallbackProc) QMotifDialog::acceptCallback, &dialog );

    dialog.exec();
}

void MainWindow::showQtDialog()
{
    // custom Qt-based dialog using a Motif-based parent
    CustomDialog customdialog(motifWidget());
    customdialog.exec();
}
