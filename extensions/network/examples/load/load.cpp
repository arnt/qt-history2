#include <qwidget.h>
#include <qurloperator.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qlabel.h>

#include "load.h"

class LoadData {
    QArray<int> ins,outs;
    Load* widget;
    QLabel* label;
    int current;
    int max;

private:
    void rescale(int need)
    {
	while ( max < need ) {
	    if ( max < 4096 )
		max *= 2;
	    else
		max += 4096;
	}
	setLabel();
	widget->repaint(FALSE);
    }

public:
    LoadData(Load* l) :
	widget(l),
	label(new QLabel(widget)),
	current(0),
	max(1)
    {
	label->setAutoMask(TRUE);
	setWidth(1);
	addDataPoint(0,0);
    }

    void setWidth(int w)
    {
	int ow = ins.size();
	ins.resize(w);
	outs.resize(w);
	current %= w;
	for (int i=ow; i<w; i++)
	    ins[i] = outs[i] = 0;
	setLabel();
    }

    void setLabel()
    {
	QString s;
	if ( max > 1024 ) {
	    s.setNum(max/1024);
	    s += "K";
	} else {
	    s.setNum(max);
	}
	label->setText(s);
	QSize sh = label->sizeHint();
	label->setGeometry(widget->width()-sh.width()-10,
		0, sh.width(), sh.height());
    }

    void addDataPoint(int in, int out)
    {
	ins[current]=in;
	outs[current]=out;
	current=(current+1)%ins.size();

	if (in+out > max) {
	    rescale(in+out);
	} else {
	    widget->scroll(-1,0,widget->rect()); // (not children)
	    paintRange(widget->width()-1,widget->width()-1);
	}
    }

    void paintRange(int from, int to)
    {
	QPainter p(widget);
	int h=widget->height();
	for (int x=from; x<=to; x++) {
	    int i = (current+x)%ins.size();
	    int in = ins[i];
	    int out = outs[i];
	    p.moveTo(x,h);
	    p.setPen(Qt::red);
	    p.lineTo(x,h-in*h/max);
	    p.setPen(Qt::green);
	    p.lineTo(x,h-(in+out)*h/max);
	    p.setPen(Qt::white);
	    p.lineTo(x,0);
	}
    }
};

Load::Load(const QString& url, QWidget* parent, const char* name, WFlags f) :
    QWidget(parent,name,f),
    d(new LoadData(this))
{
    setBackgroundMode(PaletteBase);
    QUrlOperator *op = new QUrlOperator(url);;
    connect(op,SIGNAL(data(const QByteArray&,QNetworkOperation*)),
	    this,SLOT(data(const QByteArray&)));
    op->get();
}

Load::~Load()
{
    delete d;
}

void Load::resizeEvent(QResizeEvent*)
{
    d->setWidth(width());
}

void Load::paintEvent(QPaintEvent* e)
{
    QRect r = e->rect();
    d->paintRange(r.left(), r.right());
}

void Load::data(const QByteArray& a)
{
    QTextIStream s(a);
    int in, out;
    s >> in >> out;
    d->addDataPoint(in,out);
}
