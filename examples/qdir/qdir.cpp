/****************************************************************************
** $Id: //depot/qt/main/examples/qdir/qdir.cpp#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qwidgetstack.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qtextview.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>

class PixmapView : public QScrollView
{
    Q_OBJECT

public:
    PixmapView( QWidget *parent )
	: QScrollView( parent ) {}

    void setPixmap( const QPixmap &pix ) {
	pixmap = pix;
	resizeContents( pixmap.size().width(), pixmap.size().height() );
	viewport()->repaint( FALSE );
    }

    void drawContents( QPainter *p, int, int, int, int ) {
	p->drawPixmap( 0, 0, pixmap );
    }

private:
    QPixmap pixmap;

};

class Preview : public QWidgetStack
{
    Q_OBJECT

public:
    Preview( QWidget *parent )
	: QWidgetStack( parent ) {
	    normalText = new QMultiLineEdit( this );
	    normalText->setReadOnly( TRUE );
	    html = new QTextView( this );
	    pixmap = new PixmapView( this );
	    raiseWidget( normalText );
    }

    void showPreview( const QUrl &u, int size ) {
	if ( u.isLocalFile() ) {
	    QString path = u.path();
	    QFileInfo fi( path );
	    if ( fi.isFile() && (int)fi.size() > size * 1000 ) {
		normalText->setText( tr( "The File\n%1\nis too large, so I don't show it!" ).arg( path ) );
		raiseWidget( normalText );
		return;
	    }
	
	    QPixmap pix( path );
	    if ( pix.isNull() ) {
		if ( fi.isFile() ) {
		    QFile f( path );
		    if ( f.open( IO_ReadOnly ) ) {
			QTextStream ts( &f );
			QString text = ts.read();
			f.close();
			if ( fi.extension().lower().contains( "htm" ) ) {
			    QString url = html->mimeSourceFactory()->makeAbsolute( path, html->context() );
			    html->setText( text, url ); 	
			    raiseWidget( html );
			    return;
			} else {
			    normalText->setText( text ); 	
			    raiseWidget( normalText );
			    return;
			}
		    }
		}
		normalText->setText( QString::null );
		raiseWidget( normalText );
	    } else {
		pixmap->setPixmap( pix );
		raiseWidget( pixmap );
	    }
	} else {
	    normalText->setText( "I only show local files!" );
	    raiseWidget( normalText );
	}
    }

private:
    QMultiLineEdit *normalText;
    QTextView *html;
    PixmapView *pixmap;

};

class PreviewWidget : public QVBox
{
    Q_OBJECT

public:
    PreviewWidget( QWidget *parent )
	: QVBox( parent ) {
	    setSpacing( 5 );
	    setMargin( 5 );
	    QHBox *row = new QHBox( this );
	    row->setSpacing( 5 );
	    (void)new QLabel( tr( "Only show files smaller than: " ), row );
	    sizeSpinBox = new QSpinBox( 1, 10000, 1, row );
	    sizeSpinBox->setSuffix( " KB" );
	    sizeSpinBox->setValue( 64 );
	    row->setFixedHeight( 10 + sizeSpinBox->sizeHint().height() );
	    preview = new Preview( this );
    }

public slots:
    void showPreview( const QUrl &u ) {
	preview->showPreview( u, sizeSpinBox->value() );
    }

private:
    QSpinBox *sizeSpinBox;
    Preview *preview;

};

int main( int argc, char ** argv )
{
    QFileDialog::Mode mode = QFileDialog::ExistingFile;
    QString start;
    QString filter;
    QString caption;
    bool preview = FALSE;
    QApplication a( argc, argv );
    for (int i=1; i<argc; i++) {
	QString arg = argv[i];
	if ( arg == "-any" )
	    mode = QFileDialog::AnyFile;
	else if ( arg == "-dir" )
	    mode = QFileDialog::Directory;
	else if ( arg == "-default" )
	    start = argv[++i];
	else if ( arg == "-filter" )
	    filter = argv[++i];
	else if ( arg == "-preview" )
	    preview = TRUE;
	else if ( arg[0] == '-' ) {
	    qDebug("Usage: qdir [-any | -dir] [ -preview] [-default f] {-filter f} [caption ...]\n"
		   "      -any         Get any filename, need not exist.\n"
		   "      -dir         Return a directory rather than a file.\n"
		   "      -preview  Show a preview widget.\n"
		   "      -default f   Start from directory/file f.\n"
		   "      -filter f    eg. '*.gif' '*.bmp'\n"
		   "      caption ...  Caption for dialog.\n"
		   );
	    return 1;
	} else {
	    if ( !caption.isNull() )
		caption += ' ';
	    caption += arg;
	}
    }

    if ( !start )
	start = QDir::currentDirPath();

    if ( !caption )
	caption = mode == QFileDialog::Directory
		    ? "Choose directory..." : "Choose file...";

    QFileDialog fd( QString::null, filter, 0, 0, TRUE );
    fd.setMode( mode );
    if ( preview ) {
	fd.setPreviewMode( FALSE, TRUE );
	fd.setContentsPreviewWidget( new PreviewWidget( &fd ) );
	fd.setViewMode( QFileDialog::ListView | QFileDialog::PreviewContents );
    }
    fd.setCaption( caption );
    fd.setSelection( start );
    if ( fd.exec() == QDialog::Accepted ) {
	QString result = fd.selectedFile();
	printf("%s\n", (const char*)result);
	return 0;
    } else {
	return 1;
    }
}

#include "qdir.moc"
