#include "qdocgui.h"
#include <qprocess.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include <qinputdialog.h>
#include <qregexp.h>
#include <qdict.h>

QDocMainWindow::QDocMainWindow( QWidget* parent, const char* name ) : QMainWindow( parent, name )
{
    setCaption( "qdoc GUI" );
    QVBoxLayout* vb = new QVBoxLayout(this);
    vb->setAutoAdd( TRUE );
    classList = new QListView( this );
    classList->addColumn( "Text" );
    classList->setRootIsDecorated( TRUE );
    connect( classList, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(activateEditor(QListViewItem*)) );
    connect( classList, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(activateEditor(QListViewItem*)) );
    classList->show();
    qtdirenv = getenv( "QTDIR" );
    waitText = new QLabel( "Currently QDocing", this, "wait", WType_Modal | WStyle_Customize | WStyle_NormalBorder );
    waitText->setCaption( "qdoc GUI - Waiting" );
    waitText->setFont( QFont("times", 36) );
    waitText->setAlignment( AlignCenter );
    waitText->show();
    init();
}

void QDocMainWindow::init()
{
    proc = new QProcess( this );
    QDir e( qtdirenv + "/util/qdoc");
    proc->setWorkingDirectory( e );
    proc->addArgument( qtdirenv + "/util/qdoc/qdoc" );
    proc->addArgument( "-Wall" );
    proc->addArgument( "-W4" );

    connect( proc, SIGNAL(readyReadStderr()), this, SLOT(readOutput()));
    connect( proc, SIGNAL(processExited()), this, SLOT(finished()));
    if ( !proc->start() )
	qDebug("Sommat happened");
}

void QDocMainWindow::readOutput()
{
    outputText.append( proc->readStderr() );
}

void QDocMainWindow::activateEditor( QListViewItem * item )
{
    if ( !item )
	return;
    QString subdir;
    classList->update();
    qApp->processEvents();
    if ( item->text(0).startsWith( "Line" ) ) {
	if ( editText.isNull() ) {
	    bool ok = FALSE;
	    editText = QInputDialog::getText( "Please enter your editor", "Enter editor", QLineEdit::Normal, QString::null, &ok, this );
	}
	if ( !editText.isNull() ) {
	    if ( item->parent()->parent()->text(0).startsWith( "doc" ) )
		subdir = "/doc/";
	    else if ( item->parent()->parent()->text(0).startsWith( "doc" ) )
		subdir = "/example/";
	    else
		subdir = "/src/";
	    filename = qtdirenv + subdir + item->parent()->parent()->text(0) + '/' + item->parent()->text(0);
	    QString itemtext = item->text(0);
	    QRegExp rxp( "(\\d+)" );
	    int foundpos = rxp.search( itemtext, 5 );
	    if ( foundpos != -1 ) {
		QString linenumber = rxp.cap( 0 );
		procedit = new QProcess( this );
		procedit->addArgument( editText );
		procedit->addArgument( QString("+" + linenumber) );
		procedit->addArgument( filename );
		connect( procedit, SIGNAL(processExited()), this, SLOT(editorFinished()));
		if ( !procedit->start() ) {
		    // Fix for crappy editors
		}
		
	    }
	} else {
	    qWarning( "You didn't specify an editor..." );
	}
    }
}

void QDocMainWindow::editorFinished()
{
//    QMessageBox::information( this, "Finished editing!", "You can now submit the files into P4!" );
}


void QDocMainWindow::finished()
{
    waitText->setText("Sorting results");
    waitText->update();
    QString dirText;
    QString classText;
    QString warningText;
    QString linenumber;
    int newLine;
    QString text;
    QDict<QListViewItem> category( 31 );
    QDict<QListViewItem> filename( 4001 );
    QListViewItem *dirItem   = 0;
    QListViewItem *classItem = 0;
    while ( !outputText.isEmpty() ) {    
	newLine = outputText.find( '\n' );
	if ( newLine == -1 )
	    outputText = "";
	text = outputText.left( newLine );
	
	if ( !text.startsWith( "qdoc" ) && !text.isEmpty() ) {
	    if ( text.startsWith( "src" ) )
		text = text.right( text.length() - 4 );
	    int slashpos = text.find( '/' );
	    int classfind = text.find( ':' );
	    int secondcolonpos = text.find( ':', classfind + 1 );
	    dirText = text.left( slashpos );
	    classText = text.mid ( slashpos + 1, (classfind - slashpos - 1 ) );
	    linenumber = text.mid( classfind + 1, (secondcolonpos - classfind - 1 ));
	    warningText = text.right( text.length() - secondcolonpos - 1 );
	    warningText.truncate( warningText.length() - 1 );
	   
	    if ( !category[dirText] ) {
		dirItem = new QListViewItem( classList, dirText );
		category.insert( dirText, dirItem );
	    }
	    else {
		dirItem = category[dirText];
	    }

	    if ( !filename[classText] ) {
		classItem = new QListViewItem( dirItem, classText );
		filename.insert( classText, classItem );
	    }
	    else {
		classItem = filename[classText];
	    }
	    
	    new QListViewItem( classItem, ( "Line " + linenumber + " - " + warningText ));
	}
	outputText = outputText.right( outputText.length() - ( newLine + 1 ) );
    }
    waitText->hide();
}

QDocMainWindow::~QDocMainWindow()
{
    // Empty
}

int main( int argc, char** argv )
{
    QApplication a( argc, argv );
    
    QDocMainWindow qdmw;
    qdmw.setGeometry( 0, 0, 350, 500 );
    a.setMainWidget( &qdmw );
    qdmw.show();
    return a.exec();

}
