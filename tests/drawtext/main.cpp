#include <qapplication.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qpainter.h>

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    
//     QLabel l(0);
//     l.setAlignment( Qt::WordBreak | Qt::AlignCenter );
//     l.setText("Item 35");
//     l.show();
    QString text = "R";
    QWidget w (0, 0, Qt::WType_Popup );
    w.resize( 300, 300 );
    w.show();
    QPainter p( &w );
    QRect r (p.boundingRect( w.rect(), Qt::WordBreak | Qt::AlignCenter , text ));
    p.drawRect( r );
    p.drawText( r, Qt::WordBreak | Qt::AlignCenter, text  );
    a.setMainWidget( &w );
    return a.exec();
}
