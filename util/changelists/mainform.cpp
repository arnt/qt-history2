#include "mainform.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtextview.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qsplitter.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qregexp.h>

#include <stdlib.h>

MainForm::MainForm() :
    changeListFrom(0), changeListTo(0)
{
    connect( quit, SIGNAL(clicked()), SLOT(close()) );
    connect( pathSelect, SIGNAL(clicked()), SLOT(selectPath()) );
    connect( goButton, SIGNAL(clicked()), SLOT(go()) );
    connect( changes, SIGNAL(currentChanged(QListViewItem*)), SLOT(currentChanged(QListViewItem*)) );

    connect( &process, SIGNAL(readyReadStdout()), SLOT(readyReadStdout()) );
    connect( &process, SIGNAL(readyReadStderr()), SLOT(readyReadStderr()) );
    connect( &process, SIGNAL(processExited()), SLOT(processExited()) );

    QValueList<int> sizes;
    sizes << 1 << 3;
    splitter->setSizes( sizes );

    char *qtdir = getenv( "QTDIR" );
    if ( qtdir ) {
	path->insertItem( QString(qtdir)+"/src", 0 );
	path->insertItem( QString(qtdir), 1 );
    }

    QStringList args;
    args << "p4" << "labels";
    process.setArguments( args );
    if ( !process.start() ) {
	QMessageBox::critical( this, tr("Error starting process"),
		tr("Could not start p4. Please check your path") );
    }
}

MainForm::~MainForm()
{
    delete changeListFrom;
    delete changeListTo;
}

void MainForm::selectPath()
{
    QString dir = QFileDialog::getExistingDirectory( path->currentText(), this );
    path->insertItem( dir, 0 );
}

void MainForm::startChanges( QString label )
{
    QStringList args;

    if ( label[0] != '#' )
	label = "@" + label;

    QString file = path->currentText();
    if ( file[file.length()-1] != '/' )
	file += "/";
    file += "..." + label;

    if ( incIntegrates )
	args << "p4" << "changes" << "-i" << file;
    else
	args << "p4" << "changes" << file;

    //qDebug( args.join( " " ) );
    process.kill();
    process.setArguments( args );
    if ( !process.start() ) {
	QMessageBox::critical( this, tr("Error starting process"),
		tr("Could not start p4. Please check your path") );
    }
    QApplication::setOverrideCursor( Qt::waitCursor );
}

void MainForm::go()
{
    delete changeListFrom;
    delete changeListTo;
    changeListFrom = 0;
    changeListTo = 0;

    incIntegrates = includeIntegrates->isChecked();
    changeListFrom = new QValueList<int>;
    startChanges( changesFrom->currentText() );
}

void MainForm::currentChanged( QListViewItem *li )
{
    if ( li == 0 ) {
	description->setText( "" );
    } else {
	if ( process.isRunning() ) {
	    //qWarning( "Process is running!!!!" );
	    QApplication::restoreOverrideCursor();
	}
	QStringList args;
	args << "p4" << "describe" << "-du" << li->text(0);
	//qDebug( args.join( " " ) );
	process.setArguments( args );
	if ( !process.start() ) {
	    QMessageBox::critical( this, tr("Error starting process"),
		    tr("Could not start p4. Please check your path") );
	}
	QApplication::setOverrideCursor( Qt::waitCursor );
    }
}

void MainForm::readyReadStdout()
{
    QString command = process.arguments()[1];
    
    if ( command == "labels" ) {
	while ( process.canReadLineStdout() ) {
	    QString label = QStringList::split( ' ', process.readLineStdout() )[1];
	    changesFrom->insertItem( label, 0 );
	    changesTo->insertItem( label, 0 );
	}
    } else if ( command == "changes" ) {
	QValueList<int> *list;
	if ( changeListTo != 0 ) {
	    list = changeListTo;
	} else if ( changeListFrom != 0 ) {
	    list = changeListFrom;
	}
	if ( list ) {
	    while ( process.canReadLineStdout() ) {
		QString label = QStringList::split( ' ', process.readLineStdout() )[1];
		list->append( label.toInt() );
		if ( list->count() % 500 == 0 ) {
		    qApp->processEvents();
		}
	    }
	} else {
	    qWarning( "Something went terribly wrong" );
	}
    }
}

void MainForm::readyReadStderr()
{
    while ( process.canReadLineStderr() ) {
	QString errorString =
	    QString( "%1: %2" ).arg(
		    process.arguments().join( " " ) ).arg(
		    process.readLineStderr() );
	errorView->setText( errorString );
    }
}

#define FROM_AT_END ( itFrom == changeListFrom->end() )
#define TO_AT_END ( itTo == changeListTo->end() )

void MainForm::processExited()
{
    // We do a processEvents() in readyReadStdout(). This can emit the
    // processExited() signal, even before we have read all data. So make sure
    // that you read all data of the process.
    readyReadStdout();
    readyReadStderr();

    QString command = process.arguments()[1];

    if ( command == "labels" ) {
	changesTo->insertItem( "#have", 0 );
	changesTo->insertItem( "#head", 0 );
    } else if ( command == "changes" ) {
	if ( changeListFrom!=0 ) {
	    if ( changeListTo==0 ) {
		changeListTo = new QValueList<int>;
		startChanges( changesTo->currentText() );
	    } else {
		changes->clear();
		qHeapSort( *changeListFrom );
		qHeapSort( *changeListTo );
		QValueList<int>::iterator itFrom, itTo;
		itFrom = changeListFrom->begin();
		itTo = changeListTo->begin();
		while ( !TO_AT_END ) {
		    if ( FROM_AT_END ) {
			while ( !TO_AT_END ) {
			    changes->insertItem( new QListViewItem( changes, QString::number( *itTo ) ) );
			    itTo++;
			}
			break;
		    }
		    while ( *itFrom > *itTo ) {
			changes->insertItem( new QListViewItem( changes, QString::number( *itTo ) ) );
			itTo++;
			if ( TO_AT_END )
			    break;
		    }
		    if ( *itFrom == *itTo ) {
			itFrom++;
			itTo++;
		    } else {
			itFrom++;
		    }
		}
		QApplication::restoreOverrideCursor();
		QApplication::restoreOverrideCursor();
	    }
	}
    } else if ( command == "describe" ) {
	QString desc( process.readStdout() );
#if 0
	// ### do some nice syntax highlighting
	desc = desc.replace( QRegExp("<"), "&lt;" );
	desc = desc.replace( QRegExp(">"), "&gt;" );
	desc = desc.replace( QRegExp("^\\+"), "<font color=blue>+</font>" );
#endif
	description->setCursorPosition( 0, 0 );
	description->setText( desc );
	if ( changes->currentItem() != 0 ) {
	    int i = desc.find( '\n' );
	    QString tmp = desc.left(i).replace( QRegExp("^Change \\d* by .* on ") , "" );
	    changes->currentItem()->setText( 1, tmp );
	}
	QApplication::restoreOverrideCursor();
    }
}
