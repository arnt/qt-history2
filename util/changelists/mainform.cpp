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
#include <qsyntaxhighlighter.h>

#include <stdlib.h>

class DiffSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    DiffSyntaxHighlighter( QTextEdit *e ) : QSyntaxHighlighter( e ) {}

    int highlightParagraph( const QString &text, int ) {
	QFont bold = textEdit()->font();
	bold.setBold( true );

	if ( text[0] == '+' )
	    setFormat( 0, text.length(), blue );
	else if ( text[0] == '-' )
	    setFormat( 0, text.length(), red );
	else if ( text.startsWith( "====" ) || text.startsWith( "@@" ) )
	    setFormat( 0, text.length(), bold );
	else
	    setFormat( 0, text.length(), textEdit()->colorGroup().text() );
	
	return 0;
    }
};

MainForm::MainForm(bool g) :
    grouped(g), changeListFrom(0), changeListTo(0), changeDateTo(0), changeWhoTo(0)
{
    if(grouped)
	changes->setRootIsDecorated( true );

    connect( quit, SIGNAL(clicked()), SLOT(close()) );
    connect( pathSelect, SIGNAL(clicked()), SLOT(selectPath()) );
    connect( goButton, SIGNAL(clicked()), SLOT(go()) );
    connect( changes, SIGNAL(currentChanged(QListViewItem*)), SLOT(currentChanged(QListViewItem*)) );

    connect( &process, SIGNAL(readyReadStdout()), SLOT(readyReadStdout()) );
    connect( &process, SIGNAL(readyReadStderr()), SLOT(readyReadStderr()) );
    connect( &process, SIGNAL(processExited()), SLOT(processExited()) );

    QList<int> sizes;
    sizes << 1 << 3;
    splitter->setSizes( sizes );

    QStringList args;
    args << "p4" << "labels";
    start( args );

    (void)new DiffSyntaxHighlighter( diff );
}

MainForm::~MainForm()
{
    delete changeListFrom;
    delete changeListTo;
    delete changeDateTo;
}

void MainForm::selectPath()
{
    QString dir = QFileDialog::getExistingDirectory( path->currentText(), this );
    path->insertItem( dir, 0 );
}

void MainForm::startChanges( QString label )
{
    QStringList args;

    if ( !label.isEmpty() ) {
	if ( label[0] != '#' )
	    label = "@" + label;
    }

    QString file = path->currentText();
    if(!file.contains("...")) {
	if ( file[(int)file.length()-1] != '/' )
	    file += "/";
	file += "...";
    }
    file += label;

    args << "p4" << "changes" << file;

    process.kill();
    start( args );
}

void MainForm::go()
{
    delete changeListFrom;
    delete changeListTo;
    delete changeDateTo;
    changeListFrom = 0;
    changeListTo = 0;
    changeDateTo = 0;

    if(grouped) 
	changeWhoTo = new QMap<int, QString>;
    changeListFrom = new QList<int>;
    if ( allChanges->isChecked() ) {
	changeListTo = new QList<int>;
	changeDateTo = new QMap<int,QString>;
	startChanges( "" );
    } else {
	startChanges( changesFrom->currentText() );
    }
    parseDescribe( "" );
}

void MainForm::currentChanged( QListViewItem *li )
{
    if(li->rtti() != ChangeItem::changeRTTI()) {
	setDescFilesDiff("","","");
	return;
    }

    if ( li == 0 ) {
	parseDescribe( "" );
    } else {
	if ( process.isRunning() ) {
	    //qWarning( "Process is running!!!!" );
	}
	QStringList args;
	args << "p4" << "describe" << "-du" << li->text(0);
	start( args );
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
    while ( QApplication::overrideCursor() )
	QApplication::restoreOverrideCursor();
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
	QList<int> *list;
	if ( changeListTo != 0 ) {
	    list = changeListTo;
	} else if ( changeListFrom != 0 ) {
	    list = changeListFrom;
	}
	if ( list ) {
	    QApplication::setOverrideCursor( Qt::waitCursor );
	    QString changesTmp = QString(process.readStdout());
	    int sPos = 0;
	    int ePos = 0;
	    while ( true ) {
		ePos = changesTmp.find( '\n', sPos );
		if ( ePos == -1 )
		    break;
		int sTmpPos = changesTmp.find( ' ', sPos );
		int eTmpPos = changesTmp.find( ' ', sTmpPos+1 );
		if ( sTmpPos == -1 || eTmpPos == -1 )
		    qWarning( "parsing error of p4 output" );
		QString label = changesTmp.mid( sTmpPos, eTmpPos-sTmpPos );
		int labelInt = label.toInt();
		list->append( labelInt );
		if ( changeDateTo ) {
		    sTmpPos = changesTmp.find( ' ', eTmpPos+1 );
		    eTmpPos = changesTmp.find( ' ', sTmpPos+1 );
		    changeDateTo->insert( labelInt, changesTmp.mid( sTmpPos, eTmpPos-sTmpPos ) );
		}
		if( changeWhoTo ) {
		    sTmpPos = changesTmp.find( ' ', eTmpPos+1 );
		    eTmpPos = changesTmp.find( ' ', sTmpPos+1 );
		    changeWhoTo->insert( labelInt, changesTmp.mid( sTmpPos, eTmpPos-sTmpPos ) );
		}
		sPos = ePos + 1;
	    }

	    if ( changeListTo == 0 ) {
		changeListTo = new QList<int>;
		changeDateTo = new QMap<int,QString>;
		startChanges( changesTo->currentText() );
	    } else {
		QDict<QListViewItem> roots;

		changes->clear();
		qHeapSort( *changeListFrom );
		qHeapSort( *changeListTo );
		QList<int>::iterator itFrom, itTo;
		itFrom = changeListFrom->begin();
		itTo = changeListTo->begin();
		while ( !TO_AT_END ) {
		    if ( FROM_AT_END ) {
			while ( !TO_AT_END ) {
			    bool added = false;
			    if(changeWhoTo && (*changeWhoTo).contains(*itTo)) {
				QString who = (*changeWhoTo)[*itTo].section('@', 0, 0);
				QListViewItem *root = roots[who];
				if(!root) {
				    roots.insert(who, root = new QListViewItem(changes, who));
//				    root->setOpen(true);
				}
				if(root) {
				    added = true;
				    (void) new ChangeItem( root, *itTo, (*changeDateTo)[*itTo] );
				}
			    }
			    if(!added)
				changes->insertItem( new ChangeItem( changes, *itTo, (*changeDateTo)[*itTo] ) );
			    itTo++;
			}
			break;
		    }
		    while ( *itFrom > *itTo ) {
			bool added = false;
			if(changeWhoTo && (*changeWhoTo).contains(*itTo)) {
			    QString who = (*changeWhoTo)[*itTo].section('@', 0, 0);
			    QListViewItem *root = roots[who];
			    if(!root) {
				roots.insert(who, root = new QListViewItem(changes, who));
//				root->setOpen(true);
			    }
			    if(root) {
				added = true;
				(void)new ChangeItem( root, *itTo, (*changeDateTo)[*itTo] );
			    }
			}
			if(!added)
			    changes->insertItem( new ChangeItem( changes, *itTo, (*changeDateTo)[*itTo] ) );
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
		if(roots.isEmpty()) {
		    errorView->append( QString("%1 changes found\n").arg(changes->childCount()) );
		} else {
		    int cnt = 0;
		    for(QDictIterator<QListViewItem> it(roots); it.current(); ++it) {
			int t = it.current()->childCount();
			it.current()->setText(1, QString::number(t));
			cnt += t;
		    }
		    errorView->append( QString("%1 changes found\n").arg(cnt) );
		}

		delete changeListFrom;
		delete changeListTo;
		delete changeDateTo;
		changeListFrom = 0;
		changeListTo = 0;
		changeDateTo = 0;
	    }
	    QApplication::restoreOverrideCursor();
	}
    } else if ( command == "describe" ) {
	QString desc( process.readStdout() );
	parseDescribe( desc );
    }
}

void MainForm::start( const QStringList& args )
{
    //qDebug( args.join( " " ) );
    process.setArguments( args );
    if ( !process.start() ) {
	QMessageBox::critical( this, tr("Error starting process"),
		tr("Could not start p4. Please check your path") );
    } else {
	QApplication::setOverrideCursor( Qt::waitCursor );
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
	((ChangeItem*)(changes->currentItem()))->setVisitedEnable( true );
    }
}

void MainForm::setDescFilesDiff( const QString& de, const QString& f, const QString& di )
{
    description->setText( de );
    affectedFiles->setText( f );
    diff->setText( di );
}
