#include <qwidget.h>
#include <qpainter.h>
#include <qapp.h>

class Main : public QWidget {
public:
    Main(QWidget* parent=0, const char* name=0, int f=0) :
	QWidget(parent, name, f)
    {
    }

    void resizeEvent(QResizeEvent*)
    {
    }

    void keyPressEvent(QKeyEvent*)
    {
    }

    void keyReleaseEvent(QKeyEvent*)
    {
    }

    void paintEvent(QPaintEvent* e)
    {
	QPainter p(this);
	p.setClipRect(e->rect());

	// ...
    }
};

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}
