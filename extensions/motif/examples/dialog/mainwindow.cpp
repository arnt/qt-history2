#include "mainwindow.h"

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

#include <qapplication.h>
#include <qmessagebox.h>
#include <qmotifdialog.h>


static void dialogCallback( Widget, XtPointer client_data, XtPointer )
{
    MainWindow *mw = (MainWindow *) client_data;
    mw->showDialog();
}

static void customDialogCallback( Widget, XtPointer client_data, XtPointer )
{
    MainWindow *mw = (MainWindow *) client_data;
    mw->showCustomDialog();
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
    : QMotifWidget( 0, xmMainWindowWidgetClass, NULL, 0, "mainwindow" )
{
    Widget menubar = XmCreateMenuBar( motifWidget(), "menubar", NULL, 0 );
    Widget filemenu = XmCreatePulldownMenu( menubar, "filemenu", NULL, 0 );
    Widget item;

    item = XtVaCreateManagedWidget( "Dialog...",
				    xmPushButtonGadgetClass, filemenu,
				    XmNmnemonic, 'D',
				    NULL );
    XtAddCallback( item, XmNactivateCallback, dialogCallback, this );

    item = XtVaCreateManagedWidget( "Custom Dialog...",
				    xmPushButtonGadgetClass, filemenu,
				    XmNmnemonic, 'C',
				    NULL );
    XtAddCallback( item, XmNactivateCallback, customDialogCallback, this );

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
 		   XmNheight, 600,
 		   NULL );

    setCaption( tr("QMotif Dialog Example") );

}

void MainWindow::showDialog()
{
    Arg args[1];
    XmString message =
	XmStringCreateLocalized( "This is an Information dialog.\n\n"
				 "It is one of many predefined Motif dialog types." );
    XtSetArg( args[0], XmNmessageString, message );

    QMotifDialog dialog( QMotifDialog::Information, XtParent( motifWidget() ),
			 args, 1, "predefined motif dialog", TRUE );
    dialog.setCaption( tr("Prefined Motif Dialog") );

    XtAddCallback( dialog.dialog(), XmNokCallback,
                   (XtCallbackProc) QMotifDialog::acceptCallback, &dialog );
    XtAddCallback( dialog.dialog(), XmNcancelCallback,
                   (XtCallbackProc) QMotifDialog::rejectCallback, &dialog );

    XtUnmanageChild( XmMessageBoxGetChild( dialog.dialog(),
					   XmDIALOG_CANCEL_BUTTON ) );
    XtUnmanageChild( XmMessageBoxGetChild( dialog.dialog(),
					   XmDIALOG_HELP_BUTTON ) );

    dialog.exec();

    XmStringFree( message );
}

void MainWindow::showCustomDialog()
{
    QMotifDialog dialog( XtParent( motifWidget() ),
			 NULL, 0, "custom dialog", TRUE );
    dialog.setCaption( tr("Custom Motif Dialog") );

    Widget form = XmCreateForm( dialog.shell(), "custom motif dialog", NULL, 0 );

    XmString str;
    Arg args[11];

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

    str = XmStringCreateLocalized( "This is a Custom dialog." );
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
    QMessageBox::aboutQt( this, tr("QMotif Dialog Example") );
}
