#include "demowidget.h"

#include <qpainter.h>
#include <qdatetime.h>

void DemoWidget::paintEvent (QPaintEvent *paintevent)
{
	INT32 x,y,w,h;

	QTime tm;

	tm.start();

	QPainter *painter = new QPainter (this);

	painter->setPen (QPen (QColor (128,128,128),1,QPen::SolidLine));
	
	w = width();
	h = height();

	for (x=0;x<w;x++)
	{
		painter->moveTo (x,0);
		painter->lineTo (w-1-x,h-1);
	}

	for (y=h;y>0;y--)
	{
		painter->moveTo (0,y);
		painter->lineTo (w-1,h-1-y);
	}

	painter->end();

	QString bench;

	bench.sprintf ("%d ms for line drawing",tm.elapsed());

	_label->setText (bench);

	delete painter;
}

DemoWidget::DemoWidget (QWidget *parent,QLabel *label):QWidget (parent)
{
	_label = label;
}

DemoWidget::~DemoWidget()
{
}
