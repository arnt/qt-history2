// Qt stuff
#include "qnp.h"
#include <qpainter.h>
#include <qtstream.h>
#include <qbuffer.h>
#include <qpixmap.h>
#include <qmenubar.h>
#include <qpushbt.h>
#include <qlist.h>
#include <qmsgbox.h>

class Trivial : public QNPWidget {
    Q_OBJECT
public:

    void paintEvent(QPaintEvent* event)
    {
	QPainter p(this);
	p.setClipRect(event->rect());
	int w = width();
	p.drawText(w/8, 0, w-w/4, height(), AlignCenter|WordBreak, "Trivial!");
    }
};

class TrivialInstance : public QNPInstance {
    Q_OBJECT
public:
    QNPWidget* newWindow()
    {
	return new Trivial;
    }
};

class TrivialPlugin : public QNPlugin {
public:
    QNPInstance* newInstance()
    {
	return new TrivialInstance;
    }

    const char* getMIMEDescription() const
    {
	return "trivial/very::Trivial and useless";
    }

    const char * getPluginNameString() const
    {
	return "Trivial Qt-based Plugin";
    }

    const char * getPluginDescriptionString() const
    {
	return "A Qt-based LiveConnected plug-in that does nothing";
    }

};

QNPlugin* QNPlugin::actual()
{
    return new TrivialPlugin;
}

#include "trivial.moc"
