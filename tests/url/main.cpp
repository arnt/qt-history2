#include <qapplication.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include "qurl.h"
#include "qexturl.h"
#include "qurlinfo.h"
#include "qfiledialog.h"

class MyFilePreview : public QLabel
{
    Q_OBJECT

public:
    MyFilePreview( QWidget *parent = 0 )
	: QLabel( parent ) {}

public slots:
    void showPreview( const QUrl &u, const QUrlInfo & ) {
	setPixmap( QPixmap( u.path() ) );
    }

};

class MyFileDialog : public QFileDialog
{
    Q_OBJECT
    
public:
    MyFileDialog() : QFileDialog() { 
	addWidgets( 0, //new QLabel( "Label", this ), 
		    new QCheckBox( "Open &Read-Only", this ), //new QComboBox( TRUE, this ), 
		    0 );//new QPushButton( "Button", this ) );
    }
    
};

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv );

//     QString s = QFileDialog::getOpenFileName();
//     qDebug ( "file: %s", s.latin1() );

    MyFileDialog *fd = new MyFileDialog();
    fd->setContentsPreviewWidget( new MyFilePreview );

    fd->resize( 600, 400 );
    a.setMainWidget( fd );
    fd->show();

//    QString s = QFileDialog::getOpenFileName( "/bin/ls", "*;;*.txt" );
//     QStringList strlst;
//     strlst = QFileDialog::getOpenFileNames();
//     if ( !strlst.isEmpty() ) {
// 	QStringList::Iterator it = strlst.begin();
// 	for ( ; it != strlst.end(); ++it )
// 	    qDebug( "%s", ( *it ).latin1() );
//     } else
// 	qDebug( "no files chosen" );
    //qDebug( "%s", s.latin1() );

    a.exec();
}

#include "main.moc"
