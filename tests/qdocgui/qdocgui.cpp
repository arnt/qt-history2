#include "qdocgui.h"

#include <qapplication.h>
#include <qdict.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsettings.h>


QDocListItem::QDocListItem( QListViewItem* after, QString text,
			    QString lineNumber )
    : QListViewItem( after, text )
{
    line = lineNumber;
}


QDocListItem::~QDocListItem()
{
}


QString QDocListItem::key( int, bool ) const
{
    QString key = line;
    return key.rightJustify( 7, '0' );
}



QDocMainWindow::QDocMainWindow( QWidget* parent, const char* name )
    : QDialog( parent, name )
{
    vb = new QVBoxLayout( this );
    classList = new QListView( this );
    classList->addColumn( "Text" );
    classList->setRootIsDecorated( TRUE );
    vb->addWidget( classList );
    QHBoxLayout* hb = new QHBoxLayout( this );
    statusBar = new QLabel( "Ready", this );
    hb->addWidget( statusBar );
    hb->setStretchFactor( statusBar, 2 );
    redo = new QPushButton( "&Repopulate", this );
    redo->setFocusPolicy( NoFocus );
    redo->setAutoDefault( FALSE );
    hb->addWidget( redo );
    QPushButton *quit = new QPushButton( "&Quit", this );
    quit->setFocusPolicy( NoFocus );
    quit->setAutoDefault( FALSE );
    hb->addWidget( quit );
    vb->addLayout( hb );

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    int x = settings.readNumEntry( "/qDocGUI/geometry/x", 0 );
    int y = settings.readNumEntry( "/qDocGUI/geometry/y", 0 );
    int width = settings.readNumEntry( "/qDocGUI/geometry/width", 200 );
    int height = settings.readNumEntry( "/qDocGUI/geometry/height", 200 );
    setGeometry( x, y, width, height );

    msgCount = 0;
    qtdirenv = getenv( "QTDIR" );
    setCaption( QString( "qdocgui -- %1" ).arg( qtdirenv ) );

    populateListView();
    setEditor();
    classList->setFocus();

    connect( classList, SIGNAL(returnPressed(QListViewItem*)),
	     this, SLOT(activateEditor(QListViewItem*)) );
    connect( classList, SIGNAL(doubleClicked(QListViewItem*)),
	     this, SLOT(activateEditor(QListViewItem*)) );
    connect( redo, SIGNAL(clicked()), this, SLOT(populateListView()) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}


void QDocMainWindow::populateListView()
{
    redo->setEnabled( FALSE );
    classList->clear();
    proc = new QProcess( this );
    QDir dir( qtdirenv + "/util/qdoc" );
    if ( ! dir.exists( "qdoc" ) )
	statusBar->setText( QString( "No qdoc to execute in %1" ).
				arg( dir.path() ) );
    else {
	proc->setWorkingDirectory( dir );
	proc->addArgument( dir.path() + "/qdoc" );
	proc->addArgument( dir.path() + "/qdoc.conf" );
	proc->addArgument( "--friendly" );
	proc->addArgument( "-Wall" );
	proc->addArgument( "-W4" );

	connect( proc, SIGNAL(readyReadStderr()), this, SLOT(readOutput()) );
	connect( proc, SIGNAL(processExited()), this, SLOT(finished()) );
	statusBar->setText( "Running qdoc..." );
	if ( !proc->start() ) {
	    QString msg = QString( "Failed to execute %1" ).
			    arg( dir.path() + "/qdoc" );
	    statusBar->setText( msg );
	    qDebug( msg );
	}
    }
}


void QDocMainWindow::readOutput()
{
    outputText.append( QString(proc->readStderr()) );
    statusBar->setText( QString( "%1 messages..." ).arg( ++msgCount ) );
}


void QDocMainWindow::setEditor()
{
    bool ok = FALSE;
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    editor = settings.readEntry( "/qDocGUI/editor" );
    while ( editor.isEmpty() ) {
	editor = QInputDialog::getText(
			"Please enter your editor",
			"qdocgui -- choose editor",
			QLineEdit::Normal, QString::null, &ok, this );
	if ( !editor.isEmpty() )
	    settings.writeEntry( "/qDocGUI/editor", editor );
	else
	    QMessageBox::information(
		    this,
		    "qdocgui - no editor entered",
		    "You didn't choose an editor", QMessageBox::Ok );
    }
}


void QDocMainWindow::activateEditor( QListViewItem * item )
{
    if ( !item )
	return;
    statusBar->setText( "" );
    QString subdir;
    QString filename;
    QString cppfilename;
    classList->update();
    qApp->processEvents();
    bool hasLino = item->text(0).startsWith( "Line" );
    if ( ! hasLino &&
	 ! ( item->text(0).endsWith( ".h" ) ||
	     item->text(0).endsWith( ".cpp" ) ) ) {
	statusBar->setText( "Can only open files or error Lines" );
	return;
    }
    QListViewItem *grandparent = item->parent();
    if ( grandparent && hasLino ) grandparent = grandparent->parent();
    if ( !grandparent ) {
	statusBar->setText( QString( "Failed to find grandparent of " ) +
			    item->text(0) );
	return;
    }
    QString prefix = grandparent->text(0);
    if ( prefix.startsWith( "doc" ) )
	filename = qtdirenv + '/' + prefix + '/' + item->parent()->text(0);
    else if ( prefix.startsWith( "include" ) ) {
	QFile f;
	QString fileText = item->parent()->text(0).
				replace(QRegExp( "\\.h$"), ".doc");
	f.setName(qtdirenv + "/doc/" + fileText);
	if ( f.exists() )
	    filename = qtdirenv + "/doc/" + fileText;
	else {
	    fileText = item->parent()->text(0).
			    replace(QRegExp( "\\.h$"), ".cpp");
	    QDir d;
	    d.setPath( qtdirenv + "/src/" );
	    QStringList lst = d.entryList( "*", QDir::Dirs );
	    QStringList::Iterator i = lst.begin();
	    while ( i != lst.end() ) {
		f.setName(qtdirenv + "/src/" + (*i) + '/' + fileText);
		if ( f.exists() ) {
		    filename = qtdirenv + "/include/" +
				item->parent()->text(0); // Include file first
		    cppfilename = qtdirenv + "/src/" + (*i) +
				    '/' + fileText; // source or doc file second
		    break;
		}
		++i;
	    }
	}
    }
    else if ( prefix.startsWith( "designer" ) )
	filename = qtdirenv + "/tools/designer/" + item->parent()->text(0);
    else if ( prefix.startsWith( "extensions" ) )
	filename = qtdirenv + "/extensions/" + item->parent()->text(0);
    else
	filename = qtdirenv + "/src/" + prefix + '/' + item->parent()->text(0);

    if ( ! hasLino ) {
	int i = filename.findRev( '/' );
	if ( i != -1 )
	    filename = filename.mid( 0, i + 1 ) + item->text(0);
    }
    QString itemtext = item->text(0);
    QRegExp rxp( "(\\d+)" );
    int foundpos = rxp.search( itemtext, 5 );
    if ( foundpos != -1 || ! hasLino ) {
	// yes!
	if ( QDir::home().dirName() == QString("jasmin") ) {
	    QProcess *p4 = new QProcess( this );
	    p4->addArgument( QString("p4") );
	    p4->addArgument( QString("edit") );
	    p4->addArgument( filename );
	    p4->start();
	}

	procedit = new QProcess( this );
	procedit->addArgument( editor );
	if ( hasLino )
	    procedit->addArgument( QString( "+" ) + rxp.cap( 0 ) );
	procedit->addArgument( filename );
	if ( ! cppfilename.isEmpty() )
	    procedit->addArgument( cppfilename );
	connect( procedit, SIGNAL(processExited()), this, SLOT(editorFinished()));
	if ( !procedit->start() ) {
	    QString msg = QString( "Failed to execute %1" ).arg( editor );
	    statusBar->setText( msg );
	    qDebug( msg );
	    // Fix for crappy editors
	}
    }
}


void QDocMainWindow::editorFinished()
{
    QString msg = QString( "%1 warnings" ).arg( warnings );
}


void QDocMainWindow::finished()
{
    statusBar->setText("Sorting...");
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
    while ( ! outputText.isEmpty() ) {
	newLine = outputText.find( '\n' );
	if ( newLine == -1 )
	    outputText = "";
	text = outputText.left( newLine );
	if ( text.startsWith( qtdirenv ) )
	    text = text.mid( qtdirenv.length() + 1 );

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

	    new QDocListItem( classItem,
				( "Line " + linenumber + " - " + warningText ),
				linenumber );
	    count++;
	}
	outputText = outputText.right( outputText.length() - ( newLine + 1 ) );
	classList->sort();
    }
    redo->setEnabled( TRUE );
    warnings = count;
    QString msg = QString( "%1 warnings" ).arg( count );
    statusBar->setText( msg );
    qDebug( msg );
}


QDocMainWindow::~QDocMainWindow()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    settings.writeEntry( "/qDocGUI/geometry/x", x() );
    settings.writeEntry( "/qDocGUI/geometry/y", y() );
    settings.writeEntry( "/qDocGUI/geometry/width", width() );
    settings.writeEntry( "/qDocGUI/geometry/height", height() );
    proc->kill();
}



int main( int argc, char** argv )
{
    QApplication a( argc, argv );

    QDocMainWindow qdmw;
    a.setMainWidget( &qdmw );
    qdmw.show();
    return a.exec();

}
