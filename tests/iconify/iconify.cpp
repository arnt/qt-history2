#include "iconify.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qtimer.h>

#include <qmessagebox.h>

#ifdef WIN_SOMETHING
#include <windows.h>
#endif

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
   QTimer *t = new QTimer;
   connect( t, SIGNAL(timeout()), SLOT(bang()) );
   t->start( 500 );
}

static int counter;

void Main::bang()
{

counter++;
#ifdef WIN_SOMETHING
   debug( " Windows thinks the widget is %svisible", IsWindowVisible(winId())?"":"not " );
#endif
   debug( " Qt thinks the widget is %svisible", isVisible()?"":"not " );
   if ( counter % 30 == 0 ) {
       show();
       debug( "                  ***showing" );
   }
   if ( counter % 8 == 0 ) {
       debug( "   ***MessageBox" );
       QMessageBox msg( "Hello", "Hi There!", QMessageBox::Information, 
			QMessageBox::Ok+QMessageBox::Default, 0, 0 );
       msg.exec();
   }
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*k)
{
    if ( k->ascii() == 'i' || k->ascii() == 'I' )
	iconify();
    else
	hide();

}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());

    // ...
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    m.show();

    //QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
