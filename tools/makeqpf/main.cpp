// Only interesting for Qt/Embedded

#include <qapplication.h>
#include <qscrollview.h>
#include <qfile.h>
#include <qfont.h>
#include <qvbox.h>
#include <qlabel.h>
#ifdef _WS_QWS_
#include <qmemorymanager_qws.h>
#endif

#include <stdlib.h>

class MakeQPF : public QScrollView
{
    QVBox *vbox;
public:
    MakeQPF()
    {
	vbox = new QVBox(this);
	addChild(vbox);
	startTimer(0);
    }

    void timerEvent(QTimerEvent*)
    {
	killTimers();
	make();
    }

    void make()
    {
	QString fontdir = QString(getenv("QTDIR")) + "/etc/fonts/fontdir";
	QFile fd(fontdir);
	if ( !fd.open(IO_ReadOnly) ) {
	    qWarning("Cannot read %s",fontdir.local8Bit().data());
	    exit(1);
	}
	while ( !fd.atEnd() ) {
	    QString line;
	    fd.readLine(line,9999);
	    if ( line[0] != '#' ) {
		QStringList attr = QStringList::split(" ",line);
		if ( attr.count() >= 7 ) {
		    QString family = attr[0];
		    int weight = QString(attr[4]).toInt();
		    bool italic = QString(attr[3]) == 'y';
		    QStringList sizes = attr[5];
		    if ( sizes[0] == "0" )
			sizes = QStringList::split(',',attr[7]);
		    for (QStringList::Iterator it = sizes.begin(); it != sizes.end(); ++it) {
			int pointSize = (*it).toInt()/10;
			if ( pointSize ) {
			    QFont f(family,pointSize,weight,italic);
#ifdef _WS_QWS_
			    memorymanager->savePrerenderedFont((QMemoryManager::FontID)f.handle());
#endif
			    QString fontdesc;
			    fontdesc = QString::number(pointSize);
			    fontdesc += "pt ";
			    fontdesc += family;
			    if ( weight < QFont::Normal ) {
				fontdesc += " Light";
			    } else if ( weight >= QFont::Black ) {
				fontdesc += " Black";
			    } else if ( weight >= QFont::Bold ) {
				fontdesc += " Bold";
			    } else if ( weight >= QFont::DemiBold ) {
				fontdesc += " DemiBold";
			    }
			    if ( italic )
				fontdesc += " Italic";
			    QLabel * l = new QLabel(fontdesc,vbox);
			    l->setFont(f);
			    l->show();
			    qApp->processEvents();
			}
		    }
		}
	    }
	}
    }
};

int main()
{
    int argc2 = 2;
    char* argv2[] = { "makeqpf", "-qws", "-nokeyboard", "-savefonts", 0 };
    QApplication app(argc2,argv2);
    MakeQPF m;
    app.setMainWidget(&m);
    m.showMaximized();
    return app.exec();
}
