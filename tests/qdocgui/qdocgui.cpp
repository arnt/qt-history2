#include "qdocgui.h"
#include <qprocess.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <stdlib.h>

QDocMainWindow::QDocMainWindow( QWidget* parent, const char* name ) : QMainWindow( parent, name )
{
    setCaption( "qdoc GUI" );
    QVBoxLayout* vb = new QVBoxLayout(this);
    vb->setAutoAdd( TRUE );
    classList = new QListView( this );
    classList->addColumn( "Text" );
    classList->setRootIsDecorated( TRUE );
    fileText = new QMultiLineEdit( this );
    connect( classList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(activateEditor(QListViewItem*)));
    fileText->setTextFormat( QTextView::PlainText );
    fileText->show();
    classList->show();
    qtdirenv = getenv( "QTDIR" );
    QPushButton* saveButton = new QPushButton( "Save", this );
    connect( saveButton, SIGNAL(clicked()), this, SLOT(saveFile()));
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
	if ( item->parent()->parent()->text(0).startsWith( "doc" ) )
	    subdir = "/doc/";
	else if ( item->parent()->parent()->text(0).startsWith( "doc" ) )
	    subdir = "/example/";
	else
	    subdir = "/src/";
	filename = qtdirenv + subdir + item->parent()->parent()->text(0) + '/' + item->parent()->text(0);
	QFile f(filename);
	if ( f.open( IO_ReadOnly ) ) {
	    fileText->clear();
	    QTextStream t(&f);
	    while ( !t.eof() ) {
		QString s = t.read();
		fileText->append( s.latin1() );
	    }
	    f.close();
	    int hypfind = item->text(0).find( '-' );
	    QString number = item->text(0).mid( 5, (hypfind - 5 - 1));
	    int line = number.toInt();
	    fileText->setCursorPosition( line, 0, FALSE );
	} else {
	    qWarning("The file could not be opened");
	}

    }
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
    while ( !outputText.isEmpty() ) {    
	newLine = outputText.find( '\n' );
	if ( newLine == -1 )
	    outputText = "";
	text = outputText.left( newLine );
	
	if ( !text.startsWith( "qdoc" ) && !text.isEmpty() ) {
	    qApp->processEvents();
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
	    
	    QListViewItem* dirItem = classList->findItem( dirText, 0, Qt::BeginsWith );
	    if ( ! dirItem ) {
		dirItem = new QListViewItem( classList, dirText );
		classList->setCurrentItem( dirItem );
	    }
	    
	    QListViewItem* classItem = classList->findItem( classText, 0, Qt::BeginsWith );
	    if ( ! classItem ) {
		classItem = new QListViewItem( dirItem, classText );
	    }
	    QListViewItem* warningItem = new QListViewItem( classItem, ( "Line " + linenumber + " - " + warningText ));
	}
	outputText = outputText.right( outputText.length() - ( newLine + 1 ) );
    }
    waitText->hide();
}

void QDocMainWindow::saveFile()
{
    if ( !(filename.isEmpty()) ) {
	QFile f(filename);
        procper = new QProcess;
        procper->addArgument("p4");
        procper->addArgument("edit");
        procper->addArgument(filename);
        if ( procper->start() ) {
            if ( QMessageBox::warning( this, "QDoc GUI", "Are you sure you want to do this?  You are better off using an\n"
		"external editor otherwise P4 thinks the entire file has changed", 
		QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
		if ( f.open( IO_WriteOnly ) ) {
		    QTextStream t( &f );
		    f.close();
		} else {
		    qWarning("The file could not be opened for saving");
		}
	    } 
	} else {
	    qWarning("The file could not be opened for saving on P4");
	}
    }
}

QDocMainWindow::~QDocMainWindow()
{
    // Empty
}

int main( int argc, char** argv )
{
    QApplication a( argc, argv );
    
    QDocMainWindow qdmw;
    qdmw.setGeometry( 100, 100, 300, 300 );
    a.setMainWidget( &qdmw );
    qdmw.show();
    return a.exec();

}
