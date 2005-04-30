/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

// Local includes
#include "pageeditdialog.h"

// Qt includes
#include <qapplication.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qstatusbar.h>

#include <qmotifdialog.h>

#include <unistd.h>

// Motif includes
#include <Xm/Xm.h>

// Demo includes
#include "page.h"

extern "C" {
#include <Xmd/Print.h>
}

extern int modified;


// Helper function
static
void AdjustPages(int startpage, int ins)
{
  int i;

  /* ins is either +1 or -1 for insert or delete a page */

  if (ins > 0) {
    for(i = maxpages; i >= startpage; i--)
      pages[i + 1] = pages[i];
    maxpages += 1;
  } else {
    for(i = startpage; i <= maxpages; i++)
      pages[i] = pages[i + 1];
    maxpages -= 1;
  }
}

// Print callback called by the print dialog
static
void Print(Widget, char *, XmdPrintCallbackStruct *cb)
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


void MainWindow::fileNew()
{
    char buf[128];
    Boolean found = False;
    int i = 0;

    while(! found) {
	sprintf(buf, "untitled%d.todo", i++);
	found = access(buf, F_OK) != 0;
    }

    delete [] options.todoFile;
    options.todoFile = qstrdup( buf );
    readDB( options.todoFile );

    statusBar()->message( tr("Created new file '%1'").
			  arg( QString::fromLocal8Bit( options.todoFile ) ) );

    setPage( 0 );
}


void MainWindow::fileOpen()
{
    QString filename =
	QFileDialog::getOpenFileName( QString(), QString(), this );

    if ( ! filename.isEmpty() ) {
	delete [] options.todoFile;
	options.todoFile = qstrdup( filename.local8Bit() );
	readDB( options.todoFile );

	statusBar()->message( tr("Opened file '%1'").
			      arg( QString::fromLocal8Bit( options.todoFile ) ) );

	setPage( currentPage );
    }
}


void MainWindow::fileSave()
{
    saveDB( options.todoFile );

    statusBar()->message( tr("Saved file '%1'").
			  arg( QString::fromLocal8Bit( options.todoFile ) ) );
}


void MainWindow::fileSaveAs()
{
    QString filename =
	QFileDialog::getSaveFileName( QString(), QString(), this );

    if ( ! filename.isEmpty() ) {
	delete [] options.todoFile;
	options.todoFile = qstrdup( filename.local8Bit() );
	saveDB( options.todoFile );

	statusBar()->message( tr("Saved file '%1'").
			      arg( QString::fromLocal8Bit( options.todoFile ) ) );
    }
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
    }

    delete [] pages[currentPage] -> majorTab;
    qstr = pedlg.majorEdit->text().simplifyWhiteSpace();
    temp = qstrdup( qstr.local8Bit().data() );
    if (strlen(temp) > 0)
	pages[currentPage] -> majorTab = temp;
    else {
	delete [] temp;
	pages[currentPage] -> majorTab = NULL;
    }

    /* Get contents before update */
    delete pages[currentPage] -> page;
    pages[currentPage] -> page = qstrdup( textedit->text().local8Bit() );

    setPage(currentPage);
}


void MainWindow::selNewPage()
{
    if (modified && pages[currentPage] != NULL) {
	delete pages[currentPage] -> page;
	pages[currentPage] -> page = qstrdup( textedit->text().local8Bit() );
    }
    AdjustPages(currentPage, 1);
    pages[currentPage] = new Page();
    spinbox->setMaxValue( maxpages + 1 );
    spinbox->setValue( currentPage + 1 );
    setPage(currentPage);
}


void MainWindow::selDeletePage()
{
    int result =
	QMessageBox::information( this, "Page Delete Dialog",
				  "Do you want to delete this page?",
				  QMessageBox::Yes, QMessageBox::No );
    if ( result != QMessageBox::Yes )
	return;

    delete pages[currentPage];
    pages[currentPage] = 0;

    AdjustPages(currentPage, -1);

    /* If there are no more pages left,  then create a blank one */
    if (maxpages < 0) {
	pages[0] = new Page();
	maxpages = 0;
    }

    spinbox->setMaxValue( maxpages + 1 );
    spinbox->setValue( currentPage + 1 );
    setPage(currentPage);
}


void MainWindow::setPage( int pageNumber )
{
    currentPage = pageNumber;
    if (pageNumber <= maxpages)
	spinbox->setValue( pageNumber + 1 );

    if ( pages[pageNumber] ) {
	textedit->setText( QString::fromLocal8Bit( pages[pageNumber]->page ) );
	textedit->setCursorPosition( pages[pageNumber]->lasttoppos,
				     pages[pageNumber]->lastcursorpos );

	QString labeltext;
	if ( pages[pageNumber]->label ) {
	    labeltext =
		QString::fromLocal8Bit( pages[pageNumber]->label );
	} else {
	    labeltext =
		QString::fromLocal8Bit("Page %1" ). arg( pageNumber + 1 );
	}

	if ( pages[pageNumber]->majorTab ) {
	    labeltext +=
		QString::fromLocal8Bit( " - " ) +
		QString::fromLocal8Bit( pages[pageNumber]->majorTab );
	}

	if ( pages[pageNumber]->minorTab ) {
	    labeltext +=
		QString::fromLocal8Bit( " - " ) +
		QString::fromLocal8Bit( pages[pageNumber]->minorTab );
	}

	textlabel->setText( labeltext );
    } else {
	textedit->clear();

	QString labeltext =
	    QString::fromLocal8Bit("Page %1 (Invalid Page)").arg( pageNumber + 1 );
	textlabel->setText( labeltext );
    }
}


void MainWindow::pageChange( int pageNumber )
{
    if ( modified && pages[currentPage] ) {
	delete pages[currentPage]->page;
	pages[currentPage]->page = qstrdup( textedit->text().local8Bit() );

	textedit->getCursorPosition( &pages[currentPage]->lasttoppos,
				     &pages[currentPage]->lastcursorpos );
    }

    setPage( pageNumber - 1 );
}


void MainWindow::textChanged()
{
    modified = 1;
}

// EOF
