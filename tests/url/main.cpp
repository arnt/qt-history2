#include <qapplication.h>

#include "qfiledialog.h"
#include "qftp.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv );

    // register FTP
    qRegisterNetworkProtocol( "ftp", new QFtp );
    
    /*QStringList lst*/ QString s = QFileDialog::getOpenFileName();
    qDebug ( "file: %s", s.latin1() );
//     QStringList::Iterator it = lst.begin();
//     for ( ; it != lst.end(); ++it )
// 	qDebug( "%s", ( *it ).latin1() );
    
    a.exec();
}

#include "main.moc"
