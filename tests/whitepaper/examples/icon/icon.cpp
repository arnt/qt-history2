/*
  icon.cpp
*/

#include <qapplication.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpushbutton.h>

int main( int argc, char **argv )
{
    QWidget *parent = 0;
    QApplication app( argc, argv );
// quote
QPushButton *button = new QPushButton( "&Find Address", parent );
button->setIconSet( QIconSet(QImage("find.bmp")) );
// endquote
    button->setFocusPolicy( QWidget::NoFocus );
    app.setMainWidget( button );
    button->show();
    return app.exec();
}
