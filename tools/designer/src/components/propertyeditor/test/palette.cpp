#include <qapplication.h>
#include <qmap.h>
#include "../paletteeditor.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QMap<int, QString> snrMap;

	PaletteEditor pe(0,0,&snrMap);
	app.setMainWidget(&pe);
	pe.show();

	app.exec();
	return 0;
}