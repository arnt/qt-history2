/****************************************************************************
** $Id$
**
** Implementation of QWidget to ControlRef bindings
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#include "qmaccontrol_mac.h"
#ifdef Q_WS_MAC
#include "qt_mac.h"

//hack, but usefull
#include <qpainter.h>
class QMacPainter : public QPainter
{
public:
    QMacPainter(QPaintDevice *p) : QPainter(p) { }
    ~QMacPainter();
    void setPort() { QPainter::initPaintDevice(TRUE); }
};

struct QMacControlPrivate
{
    QMacControlPrivate();
    ~QMacControlPrivate();
    ControlRef ctrl;
};
QMacControlPrivate::QMacControlPrivate() : ctrl(NULL) { }
QMacControlPrivate::~QMacControlPrivate() 
{
    if(ctrl)
	DisposeControl(ctrl);
    ctrl = NULL;
}


QMacControl::QMacControl(QWidget *parent, ControlRef ctrl, const char *name, WFlags f) : QWidget(parent, name, f)
{
    if(!parent || f & WType_TopLevel) 
	qFatal("QMacControl must not be toplevel!!!");
    d = new QMacControlPrivate;
    setControl(ctrl);
}


QMacControl::~QMacControl()
{
    delete d;
    d = NULL;
}


void
QMacControl::setControl(ControlRef ctrl)
{
    if(ctrl) {
	ControlRef root;
	if(GetRootControl((WindowPtr)handle(), &root) || !root)
	    CreateRootControl((WindowPtr)handle(), &root);
	EmbedControl(ctrl, root);

	Rect rect;
	GetControlBounds(ctrl, &rect);
	resize(rect.right - rect.left, rect.bottom - rect.top);

	QRegion rgn;
	GetControlRegion(ctrl, 0, rgn.handle(TRUE));
	QPoint p = mapTo(topLevelWidget(), pos());
	rgn.translate(-p.x(), -p.y());
	setMask(rgn);
    } else {
	setMask(QRegion());
    }
    d->ctrl = ctrl;
}


ControlRef
QMacControl::control() const
{
    return d->ctrl;
}


bool
QMacControl::event(QEvent *e)
{
    if(!d->ctrl)
	return FALSE;
    switch(e->type()) {
    case QEvent::Paint: {
	QMacPainter p(this);
	p.setPort();
	DrawControlInCurrentPort(d->ctrl);
	break; }
    default:
	break;
    }
    return FALSE;
}

#endif
