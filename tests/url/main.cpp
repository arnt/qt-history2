#include <qapplication.h>
#include "qfiledialog.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv );

    QString s = QFileDialog::getOpenFileName();
    qDebug ( "file: %s", s.latin1() );
}
