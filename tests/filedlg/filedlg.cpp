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
//#include "qnetwork.h"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qmultilineedit.h>

#include <qimage.h>

/* XPM */
static const char *image_xpm[] = {
"17 15 9 1",
" 	c #7F7F7F",
".	c #FFFFFF",
"X	c #00B6FF",
"o	c #BFBFBF",
"O	c #FF6C00",
"+	c #000000",
"@	c #0000FF",
"#	c #6CFF00",
"$	c #FFB691",
"             ..XX",
" ........o   .XXX",
" .OOOOOOOo.  XXX+",
" .O@@@@@@+++XXX++",
" .O@@@@@@O.XXX+++",
" .O@@@@@@OXXX+++.",
" .O######XXX++...",
" .O#####XXX++....",
" .O##$#$XX+o+....",
" .O#$$$$$+.o+....",
" .O##$$##O.o+....",
" .OOOOOOOO.o+....",
" ..........o+....",
" ooooooooooo+....",
"+++++++++++++...."
};

class ImageIconProvider : public QFileIconProvider
{
    Q_OBJECT
    QStrList fmts;
    QPixmap imagepm;

public:
    ImageIconProvider( QWidget *parent=0, const char *name=0 );
    ~ImageIconProvider();

    const QPixmap * pixmap( const QFileInfo &fi );
};

ImageIconProvider::ImageIconProvider( QWidget *parent, const char *name ) :
    QFileIconProvider( parent, name ),
    imagepm(image_xpm)
{
    fmts = QImage::inputFormats();
}

ImageIconProvider::~ImageIconProvider()
{
}

const QPixmap * ImageIconProvider::pixmap( const QFileInfo &fi )
{
    QString ext = fi.extension().upper();
    if ( fmts.contains(ext) ) {
	return &imagepm;
    } else {
	return QFileIconProvider::pixmap(fi);
    }
}

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
    if ( !initEd->text().isEmpty() ) {
	QFileInfo fi( initEd->text() );
	qDebug( "str:\"%s\" path:\"%s\" abs:\"%s\" dir:%d",
		initEd->text().latin1(),
		fi.dirPath( TRUE ).latin1(),
		fi.absFilePath().latin1()
		, fi.isDir() );
    }
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

class Preview : public QLabel, public QFilePreview
{
    Q_OBJECT

public:
    Preview( QWidget *parent )
	: QLabel( parent ) {
    }

    void previewUrl( const QUrl &u ) {
	if ( u.isLocalFile() ) {
	    QString path = u.path();
	    QPixmap pix( path );
	    if ( pix.isNull() )
		setText( "This is not a pixmap" );
	    else
		setPixmap( pix );
	} else {
	    setText( "I should only show local files!" );
	}
    }

};

void Main::platsch()
{
    QFileDialog *fd = new QFileDialog( initEd->text(), filtEd->text() );
    fd->setContentsPreviewEnabled( TRUE );
    Preview *p = new Preview( 0 );
    fd->setContentsPreview( p, p );
    fd->setViewMode( QFileDialog::Detail );
    fd->setPreviewMode( QFileDialog::Contents );
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
	QPixmap p( image_xpm );
	QToolButton *b = new QToolButton( this );
	b->setIconSet( p );
	addToolButton( b, TRUE );
	b = new QToolButton( this );
	b->setIconSet( p );
	addToolButton( b, FALSE );
	addLeftWidget( new QMultiLineEdit( this ) );
    };
};

void Main::tusch()
{
    MyFileDialog *fd = new MyFileDialog;
    fd->exec();
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    //qInitNetworkProtocols();

    QFileDialog::setIconProvider( new ImageIconProvider );

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}

#include "filedlg.moc"
