#ifndef CLASS_DEMOWIDGET
#define CLASS_DEMOWIDGET

#include <qwidget.h>
#include <qlabel.h>

class DemoWidget:public QWidget
{
private:
	QLabel *_label;

protected:
	virtual void paintEvent (QPaintEvent *paintevent);
    void mousePressEvent( QMouseEvent* ) { repaint(); }

public:
	DemoWidget (QWidget *parent,QLabel *label);
	~DemoWidget();
};

#endif
