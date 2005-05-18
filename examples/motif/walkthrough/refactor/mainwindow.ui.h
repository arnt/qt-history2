/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

// PageEditDialog includes
#include "pageeditdialog.h"

// Qt includes
#include <qapplication.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qstatusbar.h>

#include <qmotifwidget.h>
#include <qmotifdialog.h>

#include <unistd.h>

// X includes
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

// Motif includes
#include <Xm/Xm.h>
#include <Xm/Text.h>

// Demo includes
#include "page-refactor.h"

extern "C" {
#include <Exm/TabB.h>
#include <Xmd/Print.h>
} // extern "C"

void ReadDB(char*);
void SaveDB(char*);

extern Widget notebook, textw, labelw;

extern int modified;


void MainWindow::fileNew()
{
    char buf[128];
    char *str;
    Boolean found = False;
    int i = 0;

    while(! found) {
	sprintf(buf, "untitled%d.todo", i++);
	found = access(buf, F_OK) != 0;
    }

    str = qstrdup( buf );
    ReadDB(str);
    delete [] options.todoFile;
    options.todoFile = str;

    statusBar()->message( tr("Created new file '%1'").
			  arg( QString::fromLocal8Bit( options.todoFile ) ) );

    SetPage(0);
}


void MainWindow::fileOpen()
{
    QString filename =
	QFileDialog::getOpenFileName( QString(), QString(), this );

    if ( ! filename.isEmpty() ) {
	char *str = qstrdup( filename.local8Bit() );
	ReadDB(str);
	delete [] options.todoFile;
	options.todoFile = str;

	statusBar()->message( tr("Opened file '%1'").
			      arg( QString::fromLocal8Bit( options.todoFile ) ) );
    }
}


void MainWindow::fileSave()
{
    SaveDB(options.todoFile);

    statusBar()->message( tr("Saved file '%1'").
			  arg( QString::fromLocal8Bit( options.todoFile ) ) );
}


void MainWindow::fileSaveAs()
{
  QString filename =
      QFileDialog::getSaveFileName( QString(), QString(), this );

  if ( ! filename.isEmpty() ) {
    char *str = qstrdup( filename.local8Bit() );
    SaveDB(str);
    delete [] options.todoFile;
    options.todoFile = str;

    statusBar()->message( tr("Saved file '%1'").
			  arg( QString::fromLocal8Bit( options.todoFile ) ) );
  }
}


// Print callback called by the print dialog
static void
Print(Widget, char *, XmdPrintCallbackStruct *cb)
{
  int i;
  FILE *temp;
  int from, to;

  temp = fopen("/tmp/.todoout", "w");

  if (cb -> first == cb -> last &&
      cb -> first == 0) {
    from = 0;
    to = maxpages - 1;
  } else {
    from = qMax(0, cb -> first - 1);
    to = qMin(maxpages, cb -> last - 1);
  }

  for (i = from; i <= to; i++) {
    if (pages[i] -> label != NULL) {
      fprintf(temp, "Subject: %s\n", pages[i] -> label);
      fprintf(temp, "---------------------------\n\n\n");
    }
    fprintf(temp, "%s", pages[i] -> page);
    if (i != (maxpages - 1)) fprintf(temp, "\f");
  }

  fclose(temp);
  XmdPrintDocument("/tmp/.todoout", cb);
}


void MainWindow::filePrint()
{
    QMotifDialog dialog( this );
    (void) XtCreateWidget( "print dialog", xmdPrintWidgetClass,
			   dialog.shell(), NULL, 0 );

    // the print callback calls QMotifDialog::acceptCallback()
    XtAddCallback( dialog.dialog(), XmdNprintCallback,
		   (XtCallbackProc) QMotifDialog::acceptCallback, &dialog );
    // the cancel callback calls QMotifDialog::rejectCallback()
    XtAddCallback( dialog.dialog(), XmNcancelCallback,
		   (XtCallbackProc) QMotifDialog::rejectCallback, &dialog );

    // the print callback also calls the original Print() function
    XtAddCallback( dialog.dialog(), XmdNprintCallback,
		   (XtCallbackProc) Print, NULL );

    dialog.exec();
}


void MainWindow::fileExit()
{
    qApp->quit();
}


void MainWindow::selProperties()
{
    if (pages[currentPage] == NULL) return;

    PageEditDialog pedlg( this, "page edit dialog", true );

    if (pages[currentPage] -> label != NULL)
	pedlg.titleEdit->setText( pages[currentPage]->label );
    if (pages[currentPage] -> majorTab != NULL)
	pedlg.majorEdit->setText( pages[currentPage]->majorTab );
    if (pages[currentPage] -> minorTab != NULL)
	pedlg.minorEdit->setText( pages[currentPage]->minorTab );

    int result = pedlg.exec();

    if ( result != QDialog::Accepted )
	return;

    char *temp;
    XmString tstr;
    Arg args[5];
    int i;

    QString qstr = pedlg.titleEdit->text().simplifyWhiteSpace();
    pages[currentPage]->label = qstrdup( qstr.local8Bit().data() );

    delete [] pages[currentPage] -> minorTab;
    qstr = pedlg.minorEdit->text().simplifyWhiteSpace();
    temp = qstrdup( qstr.local8Bit().data() );
    if (strlen(temp) > 0)
	pages[currentPage] -> minorTab = temp;
    else {
	delete [] temp;
	pages[currentPage] -> minorTab = NULL;
	if (pages[currentPage] -> minorPB)
	    XtUnmanageChild(pages[currentPage] -> minorPB);
    }

    delete [] pages[currentPage] -> majorTab;
    qstr = pedlg.majorEdit->text().simplifyWhiteSpace();
    temp = qstrdup( qstr.local8Bit().data() );
    if (strlen(temp) > 0)
	pages[currentPage] -> majorTab = temp;
    else {
	delete [] temp;
	pages[currentPage] -> majorTab = NULL;
	if (pages[currentPage] -> majorPB)
	    XtUnmanageChild(pages[currentPage] -> majorPB);
    }

    if (pages[currentPage] -> majorTab != NULL) {
	if (pages[currentPage] -> majorPB == (Widget) 0) {
	    i = 0;
	    XtSetArg(args[i], XmNpageNumber, currentPage + 1); i++;
	    XtSetArg(args[i], XmNnotebookChildType, XmMAJOR_TAB); i++;
	    XtSetArg(args[i], XmNshadowThickness, 1); i++;
	    pages[currentPage] -> majorPB =
		ExmCreateTabButton(notebook, "atab", args, i);
	}
	tstr = XmStringGenerate(pages[currentPage] -> majorTab, NULL,
				XmCHARSET_TEXT, NULL);
	XtSetArg(args[0], ExmNcompoundString, tstr);
	XtSetValues(pages[currentPage] -> majorPB, args, 1);
	XtManageChild(pages[currentPage] -> majorPB);
    }

    if (pages[currentPage] -> minorTab != NULL) {
	if (pages[currentPage] -> minorPB == (Widget) 0) {
	    i = 0;
	    XtSetArg(args[i], XmNpageNumber, currentPage + 1); i++;
	    XtSetArg(args[i], XmNnotebookChildType, XmMINOR_TAB); i++;
	    XtSetArg(args[i], XmNshadowThickness, 1); i++;
	    pages[currentPage] -> minorPB =
		ExmCreateTabButton(notebook, "atab", args, i);
	}
	tstr = XmStringGenerate(pages[currentPage] -> minorTab, NULL,
				XmCHARSET_TEXT, NULL);
	XtSetArg(args[0], ExmNcompoundString, tstr);
	XtSetValues(pages[currentPage] -> minorPB, args, 1);
	XtManageChild(pages[currentPage] -> minorPB);
    }

    /* Get contents before update */
    delete [] pages[currentPage] -> page;
    char *p = XmTextGetString(textw);
    pages[currentPage] -> page = qstrdup( p );
    XtFree( p );

    SetPage(currentPage);
}


void MainWindow::selNewPage()
{
    Arg args[2];

    if (modified && pages[currentPage] != NULL) {
	delete [] pages[currentPage] -> page;
	char *p = XmTextGetString(textw);
	pages[currentPage] -> page = qstrdup( p );
	XtFree( p );
    }
    AdjustPages(currentPage, 1);
    pages[currentPage] = new Page();
    FixPages();
    XtSetArg(args[0], XmNcurrentPageNumber, (currentPage + 1));
    XtSetArg(args[1], XmNlastPageNumber, (maxpages + 1));
    XtSetValues(notebook, args, 2);
    SetPage(currentPage);
}


void MainWindow::selDeletePage()
{
    int result =
	QMessageBox::information( this, "Page Delete Dialog",
				  "Do you want to delete this page?",
				  QMessageBox::Yes, QMessageBox::No );
    if ( result != QMessageBox::Yes )
	return;

    Arg args[2];

    delete pages[currentPage];
    pages[currentPage] = 0;

    AdjustPages(currentPage, -1);

    /* If there are no more pages left,  then create a blank one */
    if (maxpages < 0) {
	pages[0] = new Page();
	pages[0] -> page = XtMalloc(2);
	pages[0] -> page[0] = 0;
	maxpages = 0;
    }

    FixPages();
    XtSetArg(args[0], XmNcurrentPageNumber, (currentPage + 1));
    XtSetArg(args[1], XmNlastPageNumber, (maxpages + 1));
    XtSetValues(notebook, args, 2);
    SetPage(currentPage);
}

// EOF
