#include <qapplication.h>
#include <qwidget,h>

static void
initOleDnd( QWidget* w )
{
}

class QIDataObject : public IDataObject {
public:
    QIDataObject( const char* text )
    {
    }
};

class QIDropSource : public IDropSource {
    QWidget* src;
public:
    QIDropSource( QWidget* w ) :
	src(w)
    {
    }


};

static void
startDrag( QWidget* w, QPoint pos, const char* text )
{
    QIDataObject *obj = new QIDataObject(text);
    QIDropSource *src = new QIDataSource(w);
    DWORD effects = DROPEFFECT_COPY /* Need Qt API to allow this | DROPEFFECT_MOVE */;
    DWORD results;
    DoDragDrop(obj, src, effects, &results);
}

class OleDndSource : public QLabel {
public:
    OleDndSource( QWidget* parent ) :
	QLabel("SOURCE",parent)
    {
	initOleDnd(this);
    }

    void mousePressEvent(QMouseEvent* e)
    {
	startDrag(e->pos(), text());
    }
};

class Main : public QWidget {
    OleDndSource src;
    OleDndDestination dst;
public:
    Main() :
        src(this),
	dst(this)
    {
    }

    void resizeEvent(QResizeEvent*) 
    {
	src.setGeometry(0,0,width(),height()/2);
	dst.setGeometry(0,height()/2,width(),height()/2);
    }
};

main(int argc, char**argv)
{
    QApplication app(argc,argv);

    Main m;
    app.setMainWidget(&m);

    return m.exec();
}