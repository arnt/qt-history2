#include "filedlg.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qpushbutton.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    QVBoxLayout *lay = new QVBoxLayout( this );
    lab = new QLabel( "Hello", this );
    lay->addWidget( lab );
    QPushButton *but = new QPushButton( "&Press me!", this );
    lay->addWidget( but );
    connect( but, SIGNAL(clicked()), this, SLOT(bang()) );
}

void Main::bang()
{
    QString s = QFileDialog::getOpenFileName();
    if ( s.isEmpty() )
	lab->setText( "No luck!" );
    else
	lab->setText( s );
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

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
