#include "myvbox.h"
#include "demowidget.h"

#include <qlayout.h>
#include <qlabel.h>

MyVBox::MyVBox (QWidget *parent)
{
	QVBoxLayout *vbox = new QVBoxLayout (this);
	QLabel      *lab  = new QLabel (QString ("dummy"),this);
	DemoWidget  *dwid = new DemoWidget (this,lab);

	vbox->addWidget (dwid,1);
	vbox->addWidget (lab,0);

	vbox->activate();
}

MyVBox::~MyVBox()
{
}
