#include "mainform.h"
#include "changeitem.h"

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

    QStringList args;
    args << "p4" << "labels";
    start( args );
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
    start( args );
    QApplication::setOverrideCursor( Qt::waitCursor );
}

void MainForm::go()
{
    delete changeListFrom;
    delete changeListTo;
    changeListFrom = 0;
    changeListTo = 0;

    incIntegrates = includeIntegrates->isChecked();
#if !defined(USE_READLINE)
    changesTmp = "";
#endif
    changeListFrom = new QValueList<int>;
    startChanges( changesFrom->currentText() );
}

void MainForm::currentChanged( QListViewItem *li )
{
    if ( li == 0 ) {
	parseDescribe( "" );
    } else {
	if ( process.isRunning() ) {
	    //qWarning( "Process is running!!!!" );
	    QApplication::restoreOverrideCursor();
	}
	QStringList args;
	args << "p4" << "describe" << "-du" << li->text(0);
	//qDebug( args.join( " " ) );
	start( args );
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
#if defined(USE_READLINE)
	    while ( process.canReadLineStdout() ) {
		QString label = QStringList::split( ' ', process.readLineStdout() )[1];
		list->append( label.toInt() );
		if ( list->count() % 500 == 0 ) {
		    qApp->processEvents();
		}
	    }
#else
	    changesTmp = QString(process.readStdout());
	    int sPos = 0;
	    int ePos = 0;
	    while ( TRUE ) {
		ePos = changesTmp.find( '\n', sPos );
		if ( ePos == -1 )
		    break;
		int sTmpPos = changesTmp.find( ' ', sPos );
		int eTmpPos = changesTmp.find( ' ', sTmpPos+1 );
		if ( sTmpPos == -1 || eTmpPos == -1 )
		    qWarning( "parsing error of p4 output" );
		QString label = changesTmp.mid( sTmpPos, eTmpPos-sTmpPos );
		list->append( label.toInt() );
		sPos = ePos + 1;
	    }
	    changesTmp = changesTmp.right( changesTmp.length() - sPos );
#endif
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
	errorView->append( errorString );
    }
}

#define FROM_AT_END ( itFrom == changeListFrom->end() )
#define TO_AT_END ( itTo == changeListTo->end() )

void MainForm::processExited()
{
#if defined(USE_READLINE)
    // We do a processEvents() in readyReadStdout(). This can emit the
    // processExited() signal, even before we have read all data. So make sure
    // that you read all data of the process.
    readyReadStdout();
    readyReadStderr();
#endif

    QString command = process.arguments()[1];

    if ( command == "labels" ) {
	changesTo->insertItem( "#head", 0 );
	changesTo->insertItem( "#have", 0 );

	QStringList args;
	args << "p4" << "dirs" << "//depot/qt/*";
	start( args );
    } else if ( command == "dirs" ) {
	char *qtdir = getenv( "QTDIR" );
	if ( qtdir ) {
	    path->insertItem( QString(qtdir)+"/src", 0 );
	    path->insertItem( QString(qtdir), 1 );
	}
	while ( process.canReadLineStdout() ) {
	    QString depot = process.readLineStdout() + "/src";
	    if ( depot == "//depot/qt/main/src" )
		path->insertItem( depot, 2 );
	    else
		path->insertItem( depot );
	}
    } else if ( command == "changes" ) {
	if ( changeListFrom!=0 ) {
	    if ( changeListTo==0 ) {
#if !defined(USE_READLINE)
		changesTmp = "";
#endif
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
			    changes->insertItem( new ChangeItem( changes, *itTo ) );
			    itTo++;
			}
			break;
		    }
		    while ( *itFrom > *itTo ) {
			changes->insertItem( new ChangeItem( changes, *itTo ) );
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
		errorView->append( QString("%1 changes found\n").arg(changes->childCount()) );
		QApplication::restoreOverrideCursor();
		QApplication::restoreOverrideCursor();
	    }
	}
    } else if ( command == "describe" ) {
	QString desc( process.readStdout() );
	parseDescribe( desc );
	QApplication::restoreOverrideCursor();
    }
}

void MainForm::start( const QStringList& args )
{
    process.setArguments( args );
    if ( !process.start() ) {
	QMessageBox::critical( this, tr("Error starting process"),
		tr("Could not start p4. Please check your path") );
    }
}

void MainForm::parseDescribe( const QString& desc )
{
    static QRegExp filesRE( "Affected files \\.\\.\\." );
    static QRegExp diffRE( "Differences \\.\\.\\." );
    int posFiles = desc.find( filesRE );
    int posDiff = desc.find( diffRE );

    if ( posFiles != -1 ) {
	if ( posDiff != -1 ) {
	    QString _desc = desc.left( posFiles );
	    QString _files = desc.mid( posFiles, posDiff-posFiles );
	    QString _diff = desc.mid( posDiff );
	    setDescFilesDiff( _desc, _files, _diff );
	} else {
	    // ###
	    qDebug( "not implemented yet posFiles=%d posDiff=%d", posFiles, posDiff );
	    setDescFilesDiff( desc, "", "" );
	}
    } else {
	if ( posDiff != -1 ) {
	    // ###
	    qDebug( "not implemented yet posFiles=%d posDiff=%d", posFiles, posDiff );
	    setDescFilesDiff( desc, "", "" );
	} else {
	    setDescFilesDiff( desc, "", "" );
	}
    }

    if ( changes->currentItem() != 0 ) {
	((ChangeItem*)(changes->currentItem()))->setVisitedEnable( TRUE );
    }
    // ### that should go somewhre else
    if ( changes->currentItem() != 0 ) {
	int i = desc.find( '\n' );
	QString tmp = desc.left(i).replace( QRegExp("^Change \\d* by .* on ") , "" );
	changes->currentItem()->setText( 1, tmp );
    }
#if 0
    description->setCursorPosition( 0, 0 );
    description->setText( desc );
    if ( changes->currentItem() != 0 ) {
	int i = desc.find( '\n' );
	QString tmp = desc.left(i).replace( QRegExp("^Change \\d* by .* on ") , "" );
	changes->currentItem()->setText( 1, tmp );
    }
#endif
}

void MainForm::setDescFilesDiff( const QString& de, const QString& f, const QString& di )
{
#if 0
    // ### do some nice syntax highlighting
    di = di.replace( QRegExp("<"), "&lt;" );
    di = di.replace( QRegExp(">"), "&gt;" );
    di = di.replace( QRegExp("^\\+"), "<font color=blue>+</font>" );
#endif
    description->setCursorPosition( 0, 0 );
    description->setText( de );
    affectedFiles->setCursorPosition( 0, 0 );
    affectedFiles->setText( f );
    diff->setCursorPosition( 0, 0 );
    diff->setText( di );
}
