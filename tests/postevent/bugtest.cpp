#include "bugtest.h"

int main(int argc, char ** argv)
{
	QApplication a(argc, argv);

	SimpleManager *sm = new SimpleManager;
	Q_CHECK_PTR(sm);

	MyButton *btn = new MyButton(sm, 0);
	Q_CHECK_PTR(btn);
	
	a.setMainWidget(btn);
	btn->show();

	return a.exec();
}
