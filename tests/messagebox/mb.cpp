#include <qapplication.h>
#include <qmessagebox.h>



int main( int argc, char **argv )
{
    QApplication a(argc,argv);

    QMessageBox::information(0,"Info caption","info text\nmore stuff",
			     "Bt1", "Bt2", "Bt3", 2, 1 );
    QMessageBox::warning(0,"Warning caption","warning text\nmore stuff",
			     "wBt1", "wBt2", "wBt3", 0, 2 );
    QMessageBox::critical(0,"Critical caption","info text\nmore stuff",
			     "CBt1", "CBt2", "CBt3", 1, 0 );
    QMessageBox::about(0,"About caption","About text");
    QMessageBox::aboutQt(0,"About Qt caption");
}
