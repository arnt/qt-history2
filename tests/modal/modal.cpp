#include <qapplication.h>
#include "modal.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    Dialog d;
    return d.exec();
}
