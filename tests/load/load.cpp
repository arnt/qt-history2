#include <qwidget.h>
#include <qurloperator.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qlabel.h>

#include "load.h"

#define MAXCHUNKS 8

class LoadData {
    QMemArray<int> ins,outs;
    struct {
	int start,end;
	int total;
    } chunk[MAXCHUNKS];
    Load* widget;
    QLabel* label;
    int current;
    int max;
    int nchunk;
    int chunk_start; // When did this chunk start?
    int chunk_total; // Total bytes in this chunk
    int chunk_baseline; // What is considered "silence"? (an average)

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
	nchunk = chunk_baseline = chunk_start = chunk_total = 0;
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

    QString numberAbbrev(int n) const
    {
	QString s;
	int f=1;
	QString suffix="";
	if ( n > 1024*1024 ) {
	    f=1024*1024;
	    suffix="M";
	} else if ( n > 1024 ) {
	    f=1024;
	    suffix="K";
	} else {
	    s.setNum(n);
	    return s;
	}
	int n10 = n/(f/10);
	s.setNum(n10/10);
	s += ".";
	s += QString::number(n10%10);
	s += suffix;
	return s;
    }

    void setLabel()
    {
	label->setText(numberAbbrev(max));
	QSize sh = label->sizeHint();
	label->setGeometry(widget->width()-sh.width()-10,
		0, sh.width(), sh.height());
    }

    QRect chunkRect(int i)
    {
	int x = (ins.size()-chunk[i].start+current)%ins.size();
	return QRect(x,widget->height()-30,
	    (chunk[i].end-chunk[i].start+ins.size())%ins.size(),20);
    }

    void addChunk(int start, int end, int total)
    {
	int i=0;
	QRegion r;
	if ( nchunk == MAXCHUNKS ) {
	    int smallest=0;
	    for (int j=0; j<nchunk; j++) {
		if ( chunk[j].total <= smallest ) {
		    i = j;
		    smallest = chunk[j].total;
		}
	    }
	    r |= chunkRect(i);
	} else {
	    i = ++nchunk;
	}
	chunk[i].start = start;
	chunk[i].end = end;
	chunk[i].total = total;
	r |= chunkRect(i);
	widget->repaint(r);
    }

    void addDataPoint(int in, int out)
    {
	ins[current]=in;
	outs[current]=out;
	current=(current+1)%ins.size();
	/*
	if ( in+out > chunk_baseline ) {
	    if ( !chunk_total )
		chunk_start = current;
	    chunk_total += in+out;
	} else {
	    if ( chunk_total > chunk_baseline*4 ) {
		addChunk(chunk_start,current,chunk_total);
	    }
	    chunk_total = 0;
	}
	chunk_baseline = (chunk_baseline+in+out)*15/16;
	*/

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
	p.setPen(Qt::black);
	for (int j=0; j<nchunk; j++) {
	    QRect r = chunkRect(j);
	    if ( r.intersects(QRect(QPoint(from,r.y()),QPoint(to,r.y()))) ) {
		p.drawText(r,Qt::AlignCenter,numberAbbrev(chunk[j].total));
	    }
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
