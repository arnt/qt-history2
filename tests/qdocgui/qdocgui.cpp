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
#include <qsettings.h>

QDocListItem::QDocListItem( QListViewItem* after, QString text, QString lineNumber ) : QListViewItem( after, text )
{
    line = lineNumber;
}

QDocListItem::~QDocListItem()
{
}

QString QDocListItem::key( int column, bool ascending ) const
{
    QString key = line;
    return key.rightJustify( 7, '0' );
}

QDocMainWindow::QDocMainWindow( QWidget* parent, const char* name ) : QMainWindow( parent, name )
{
    setCaption( "qdoc GUI" );
    vb = new QVBoxLayout( this );
    classList = new QListView( this );
    classList->addColumn( "Text" );
    classList->setRootIsDecorated( TRUE );
    connect( classList, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(activateEditor(QListViewItem*)) );
    connect( classList, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(activateEditor(QListViewItem*)) );
    vb->addWidget( classList );
    QHBoxLayout* hb = new QHBoxLayout( this );
    QPushButton *redo = new QPushButton( "&Repopulate", this );
    hb->addWidget( redo );
    connect( redo, SIGNAL(clicked()), this, SLOT(populateListView()) );
    QPushButton *quit = new QPushButton( "&Quit", this );
    hb->addWidget( quit );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
    vb->addLayout( hb );
    qtdirenv = getenv( "QTDIR" );
    init();
}

void QDocMainWindow::init()
{
    populateListView();
}

void QDocMainWindow::populateListView()
{
    waitText = new QLabel( "Currently qdocing", this, "wait", WStyle_Customize | WStyle_NormalBorder );
    vb->addWidget( waitText ); 
    waitText->setCaption( "qdoc GUI - Waiting" );
    waitText->setFont( QFont("times", 36) );
    waitText->setAlignment( AlignCenter );
    waitText->show();
    classList->clear();
    proc = new QProcess( this );
    QDir e( qtdirenv + "/util/qdoc" );
    proc->setWorkingDirectory( e );
    proc->addArgument( qtdirenv + "/util/qdoc/qdoc" );
    proc->addArgument( "-Wall" );
    proc->addArgument( "-W4" );

    connect( proc, SIGNAL(readyReadStderr()), this, SLOT(readOutput()) );
    connect( proc, SIGNAL(processExited()), this, SLOT(finished()) );
    if ( !proc->start() )
	qDebug( "Sommat happened" );
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
	bool ok = FALSE;
	QSettings settings;
	settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
	QString editText = settings.readEntry( "/qDocGUI/editor" );
	while ( editText.isNull() ) {
	    editText = QInputDialog::getText( "Please enter your editor", "Enter editor", QLineEdit::Normal, QString::null, &ok, this );
	    if ( !editText.isNull() )
		settings.writeEntry( "/qDocGUI/editor", editText );
	    else
		QMessageBox::information( this, "No editor specified", "You didn't specify an editor", QMessageBox::Ok );
	}
	if ( item->parent()->parent()->text(0).startsWith( "doc" ) ) {
	    filename = qtdirenv + '/' + item->parent()->parent()->text(0) + '/' + item->parent()->text(0);
	} else if ( item->parent()->parent()->text(0).startsWith( "include" ) ) {
	    QFile f;
	    QString fileText = item->parent()->text(0).replace(QRegExp( "\\.h$"), ".doc");
	    f.setName(qtdirenv + "/doc/" + fileText);
	    if ( f.exists() )
		filename = qtdirenv + "/doc/" + fileText;
	    else {
		fileText = item->parent()->text(0).replace(QRegExp( "\\.h$"), ".cpp");
		QDir d;
		d.setPath( qtdirenv + "/src/" );
		QStringList lst = d.entryList( "*", QDir::Dirs );
		QStringList::Iterator i = lst.begin();
		while ( i != lst.end() ) {
		    f.setName(qtdirenv + "/src/" + (*i) + '/' + fileText);
		    if ( f.exists() ) {
			filename = qtdirenv + "/src/" + (*i) + '/' + fileText;
			break;
		    }
		    ++i;
		}
	    }
	} else if ( item->parent()->parent()->text(0).startsWith( "designer" ) ) {
	    filename = qtdirenv + "/tools/designer/" + item->parent()->text(0);
	} else {
	    filename = qtdirenv + "/src/" + item->parent()->parent()->text(0) + '/' + item->parent()->text(0);
	}

	QString itemtext = item->text(0);
	QRegExp rxp( "(\\d+)" );
	int foundpos = rxp.search( itemtext, 5 );
	if ( foundpos != -1 ) {
	    // yes!
	    if ( QDir::home().dirName() == QString("jasmin") ) {
		QProcess *p4 = new QProcess( this );
		p4->addArgument( QString("p4") );
		p4->addArgument( QString("edit") );
		p4->addArgument( filename );
		p4->start();
	    }

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
    }
}

void QDocMainWindow::editorFinished()
{
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
    int count = 0;
    QDict<QListViewItem> category( 31 );
    QDict<QListViewItem> filename( 4001 );
    QListViewItem *dirItem   = 0;
    QListViewItem *classItem = 0;
    while ( !outputText.isEmpty() ) {
	newLine = outputText.find( '\n' );
	if ( newLine == -1 )
	    outputText = "";
	text = outputText.left( newLine );

	if ( !text.isEmpty() && !text.startsWith( "qdoc" ) ) {
	    if ( text.startsWith( "src" ) )
		text = text.right( text.length() - 4 );
	    else if ( text.startsWith( "tools/designer" ) )
		text = text.right( text.length() - 6 );
	    int slashpos = text.find( '/' );
	    int classfind = text.find( ':' );
	    int secondcolonpos = text.find( ':', classfind + 1 );
	    dirText = text.left( slashpos );
	    classText = text.mid ( slashpos + 1, (classfind - slashpos - 1 ) );
	    linenumber = text.mid( classfind + 1, (secondcolonpos - classfind - 1 ));
	    warningText = text.right( text.length() - secondcolonpos - 1 );
	    if ( warningText.right(1) == '\r' )
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

	    new QDocListItem( classItem, ( "Line " + linenumber + " - " + warningText ), linenumber );
	    count++;
	}
	outputText = outputText.right( outputText.length() - ( newLine + 1 ) );
	classList->sort();
    }
    waitText->hide();

    qDebug( "%d warnings", count );
}

QDocMainWindow::~QDocMainWindow()
{
    // Empty
}

int main( int argc, char** argv )
{
    QApplication a( argc, argv );

    QDocMainWindow qdmw;
    qdmw.setGeometry( 50, 50, 350, 500 );
    a.setMainWidget( &qdmw );
    qdmw.show();
    return a.exec();

}
