#include "translate.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qmultilinedit.h>
#include <qlabel.h>

#include "qtranslatedialog.h"

Main::Main(QWidget* parent, const char* name) :
    QVBox(parent, name)
{
    connect( new QPushButton(  tr("Quit"), this ), SIGNAL(clicked()), qApp, SLOT(quit()) );

    (void)new QLabel( tr("Text"), this );
    (void)new QMultiLineEdit( this );
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*)
{
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
    QTranslateDialog trd;
    QObject::connect(qApp, SIGNAL(couldNotTranslate( const char*, const char*)), 
		     &trd, SLOT(add( const char*, const char*)));
    
    QMessageFile mf(0);
    mf.load("test.tr",".");
    app.installMessageFile(&mf);

    //    mf.unsqueeze();
    int hash = mf.hash("Main","Quit");
    QString s = mf.find( hash, "Main", "Quit" );
    debug( "mf: %d -> %s", hash, (const char*)s );

    Main m;

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    m.show();

    return app.exec();
}
