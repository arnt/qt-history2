/****************************************************************************
**
** Implementation of Q3WhatsThis class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3whatsthis.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qevent.h"

Q3WhatsThis::Q3WhatsThis( QWidget *w)
    : QObject(w)
{
    if (w)
	w->installEventFilter(this);
}

Q3WhatsThis::~Q3WhatsThis()
{
}

bool Q3WhatsThis::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::WhatsThis && o->isWidgetType()) {
	if (QString s = text(static_cast<QHelpEvent*>(e)->pos())) {
	    QWhatsThis::showText(static_cast<QHelpEvent*>(e)->globalPos(), s, static_cast<QWidget*>(o));
	    connect(QApplication::activePopupWidget(), SIGNAL(clicked(QString)),
		    this, SLOT(hyperLinkClicked(QString)));
	}

	return true;
    }
    return false;
}
void Q3WhatsThis::hyperLinkClicked(const QString &href)
{
    if (clicked(href))
	QWhatsThis::hideText();
}

QString Q3WhatsThis::text( const QPoint & )
{ return QString(); }

bool Q3WhatsThis::clicked( const QString &)
{ return true;}


