#include <qapplication.h>
#include <qlabel.h>
#include <qpixmap.h>

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
    void showPreview( const QUrl &u, const QUrlInfo &ui ) {
	setPixmap( QPixmap( u.path() ) );
    }

};

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv );

//     QString s = QFileDialog::getOpenFileName();
//     qDebug ( "file: %s", s.latin1() );

    QFileDialog *fd = new QFileDialog();
    fd->setContentsPreviewWidget( new MyFilePreview );
        
    fd->resize( 600, 400 );
    a.setMainWidget( fd );
    fd->show();

    a.exec();
}

#include "main.moc"
