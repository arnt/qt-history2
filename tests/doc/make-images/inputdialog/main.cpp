#include <qapplication.h>
#include <qinputdialog.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QWidget *mw = new QWidget( 0 );
    app.setMainWidget( mw );

    mw->setCaption( "Input Dialog" );
    mw->show();
    QString text = QInputDialog::getText(
                    "QInputDialog",
                    "QInputDialog::getText()\nEnter a String",
                    QLineEdit::Normal, "http://doc.trolltech.com", 0, mw );
    int i = QInputDialog::getInteger(
                    "QInputDialog",
                    "QInputDialog::getInteger()\nEnter an Integer",
                    55, 0, 99, 5, 0, mw );
    double j = QInputDialog::getDouble(
                    "QInputDialog",
                    "QInputDialog::getDouble()\nEnter a Double",
                    922.87, 0, 999, 5, 0, mw );
    QStringList list;
    list << "Three Colors Red" << "Three Colors Green" << "Three Colors Blue";
    text = QInputDialog::getItem(
                    "QInputDialog",
                    "QInputDialog::getItem()\nChoose an Item",
                    list, 1, TRUE, 0, mw );

    return app.exec();
}
