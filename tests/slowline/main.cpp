#include <qapplication.h>
#include "myvbox.h"

int main (int args,char **argv)
{
	QApplication qapp (args,argv);

	MyVBox *wid = new MyVBox (0);

	qapp.setMainWidget (wid);
	wid->show();

	return qapp.exec();
}
