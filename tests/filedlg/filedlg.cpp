#include "filedlg.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qurl.h>
#include <qlabel.h>
#include <qpixmap.h>
#include "qnetwork.h"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qtoolbutton.h>

/* XPM */
static const char *fileopen[] = {
    "    16    13        5            1",
    ". c #040404",
    "# c #808304",
    "a c None",
    "b c #f3f704",
    "c c #f3f7f3",
    "aaaaaaaaa...aaaa",
    "aaaaaaaa.aaa.a.a",
    "aaaaaaaaaaaaa..a",
    "a...aaaaaaaa...a",
    ".bcb.......aaaaa",
    ".cbcbcbcbc.aaaaa",
    ".bcbcbcbcb.aaaaa",
    ".cbcb...........",
    ".bcb.#########.a",
    ".cb.#########.aa",
    ".b.#########.aaa",
    "..#########.aaaa",
    "...........aaaaa"
};                                                                              

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    QVBoxLayout *lay = new QVBoxLayout( this );
    lab = new QLabel( "Hello", this );
    lay->addWidget( lab );
    initEd = new QLineEdit( this );
    lay->addWidget( initEd );
    filtEd = new QLineEdit( this );
    lay->addWidget( filtEd );
    QPushButton *but = new QPushButton( "&Get Open!", this );
    lay->addWidget( but );
    connect( but, SIGNAL(clicked()), this, SLOT(bang()) );
    QPushButton *but2 = new QPushButton( "&Save As!", this );
    lay->addWidget( but2 );
    connect( but2, SIGNAL(clicked()), this, SLOT(bop()) );
    QPushButton *but3 = new QPushButton( "Get &Multiple Opens!", this );
    lay->addWidget( but3 );
    connect( but3, SIGNAL(clicked()), this, SLOT(ratatatat()) );
    QPushButton *but4 = new QPushButton( "Open &Dir!", this );
    lay->addWidget( but4 );
    connect( but4, SIGNAL(clicked()), this, SLOT(whoosh()) );
    QPushButton *but5 = new QPushButton( "Test &Filters", this );
    lay->addWidget( but5 );
    connect( but5, SIGNAL(clicked()), this, SLOT(boom()) );
    QPushButton *but6 = new QPushButton( "Test &Preview", this );
    lay->addWidget( but6 );
    connect( but6, SIGNAL(clicked()), this, SLOT(platsch()) );
    QPushButton *but7 = new QPushButton( "Custom Filedialog", this );
    lay->addWidget( but7 );
    connect( but7, SIGNAL(clicked()), this, SLOT(tusch()) );
}

void Main::bang()
{
    QString s = QFileDialog::getOpenFileName( initEd->text(),
                                              filtEd->text() );
    if ( s.isEmpty() )
        lab->setText( "Open: No luck!" );
    else {
        //s.prepend( "Open: " );
        lab->setText( s );
    }
}


void Main::bop()
{
    QString s = QFileDialog::getSaveFileName( initEd->text(),
                                              filtEd->text() );

    if ( s.isEmpty() )
        lab->setText( "Save: No luck!" );
    else {
        s.prepend( "Save: " );
        lab->setText( s );
    }
}


void Main::ratatatat()
{
    QStringList l = QFileDialog::getOpenFileNames( filtEd->text(),
                                                   initEd->text() );
    if ( l.isEmpty() )
        lab->setText( "Opens: No luck!" );
    else {
        QString s;//( "Opens: \n" );
        for( int i = 0; i < (int)l.count(); i++ ) {
            s.append( l[i] );
            s.append( "\n" );
        }
        lab->setText( s );
    }
}


void Main::whoosh()
{
    QString s = QFileDialog::getExistingDirectory( initEd->text() );
    if ( s.isEmpty() )
        lab->setText( "Open Dir: No luck!" );
    else {
        s.prepend( "Open Dir: " );
        lab->setText( s );
    }
}

void Main::boom()
{
    QFileDialog::getOpenFileName(
	 QString::null,
	 "C++ Files (*.cpp;*.cc;*.C;*.cxx;*.c++);;"
	 "Header Files (*.h *.hxx *.h++);;"
	 "Project files (*.pro)" );
}

class Preview : public QLabel
{
    Q_OBJECT

public:
    Preview( QWidget *parent )
	: QLabel( parent ) {
    }

public slots:
    void showPreview( const QUrl &u ) {
	if ( u.isLocalFile() ) {
	    QString path = u.path();
	    QPixmap pix( path );
	    if ( pix.isNull() )
		setText( "This is not a pixmap" );
	    else
		setPixmap( pix );
	} else
	    setText( "I only shoud local files!" );
    }

};

void Main::platsch()
{
    QFileDialog *fd = new QFileDialog( initEd->text(), filtEd->text() );
    fd->setPreviewMode( FALSE, TRUE );
    fd->setContentsPreviewWidget( new Preview( this ) );
    fd->setViewMode( QFileDialog::DetailView | QFileDialog::PreviewContents );
    fd->show();
}

class MyFileDialog : public QFileDialog
{
public:
    MyFileDialog() : QFileDialog() {
 	addWidgets( 0, new QCheckBox( "Open Read-Only", this ), 0 );
	addWidgets( new QLabel( "Choose Something", this ), 
		    new QComboBox( TRUE, this ), 
		    new QPushButton( "Press Me", this ) );
	QPixmap p( fileopen );
	QToolButton *b = new QToolButton( this );
	b->setIconSet( p );
	addToolButton( b, TRUE );
	b = new QToolButton( this );
	b->setIconSet( p );
	addToolButton( b, FALSE );
    };
};

void Main::tusch()
{
    MyFileDialog *fd = new MyFileDialog;
    fd->exec();
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*)
{
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());

    // ...
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);
    qInitNetworkProtocols();

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}

#include "filedlg.moc"
