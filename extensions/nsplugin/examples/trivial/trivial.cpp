/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Qt stuff
#include "qnp.h"
#include <qpainter.h>
#include <qmessagebox.h>
#include <qevent.h>

class Trivial : public QNPWidget {
    Q_OBJECT
public:
    void mouseReleaseEvent(QMouseEvent* event)
    {
	QMessageBox::aboutQt(this);
    }

    void paintEvent(QPaintEvent* event)
    {
	QPainter p(this);
	p.setClipRect(event->rect());
	int w = width();
	p.drawRect(rect());
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

    void print(QPainter* p)
    {
	p->drawText(0,0,"Hello");
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
	return "trivial/very:xxx:Trivial and useless";
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

QNPlugin* QNPlugin::create()
{
    return new TrivialPlugin;
}

#include "trivial.moc"
