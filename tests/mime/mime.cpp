#include "mime.h"
#include <qmime.h>
#include <qdragobject.h>
#include <qlabel.h>
#include <qapplication.h>
#include <unistd.h>

void show(const QMimeSource* d)
{
    QPixmap pm;
    QString txt;
    if ( d && QImageDrag::decode(d, pm) ) {
	QLabel* l = new QLabel(0);
	l->setPixmap(pm);
	l->show();
    } else if ( d && QTextDrag::decode(d, txt) ) {
	QLabel* l = new QLabel(0);
	l->setText(txt);
	l->show();
    } else {
	debug("Cannot display data");
    }
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QMimeSourceFactory msf;

    // File-based data...
    QStringList fp;
    fp.append(QString(getcwd(0,0)));
    msf.setFilePath(fp);
    msf.setExtensionType("png","image/png");
    msf.setExtensionType("txt","text/plain");
    // Directly stored data...
    QImage im("test.png");
    QImageDrag* i = new QImageDrag(im);
    msf.setData("foo",i);


    const QMimeSource* d1 = msf.data("test.png");
    show(d1);
    const QMimeSource* d2 = msf.data("test.txt");
    show(d2);
    const QMimeSource* d3 = msf.data("foo");
    show(d3);

    // Absolute vs. relative:
    #define D(aa,bb) \
    { \
	QString a = aa; \
	QString b = bb; \
	debug("\"%s\" relative to \"%s\" is \"%s\"", \
	    b.latin1(), \
	    a.latin1(), \
	    msf.makeAbsolute(b,a).latin1()); \
    }
    D("/a/b/c", "d");
    D("/a/b/c", "../d");
    D("/a/b/c", "/d/e/f");

    QObject::connect(qApp, SIGNAL(lastWindowClosed()),
		     qApp, SLOT(quit()));

    return app.exec();
}
