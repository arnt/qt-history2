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
#include <qpixmap.h>

//hack, but usefull
#include <qpainter.h>
class QMacPainter : public QPainter
{
public:
    QMacPainter(QPaintDevice *p) : QPainter(p) { }
    void setPort() { QPainter::initPaintDevice(TRUE); }
};
QCString p2qstring(const unsigned char *); //qglobal.cpp

struct QMacControlPrivate
{
    QMacControlPrivate();
    ~QMacControlPrivate();
    ControlRef ctrl;
    EventHandlerRef ctrlHandler;

    static EventHandlerUPP ctrlHandlerUPP;
};
EventHandlerUPP QMacControlPrivate::ctrlHandlerUPP = NULL;
QMacControlPrivate::QMacControlPrivate() : ctrl(NULL), ctrlHandler(NULL) { }
QMacControlPrivate::~QMacControlPrivate() 
{
    if(ctrlHandler)
	RemoveEventHandler(ctrlHandler);
    ctrlHandler = NULL;
    if(ctrl)
	DisposeControl(ctrl);
    ctrl = NULL;
}
static void cleanup_ctrlHandlerUPP()
{
    if(QMacControlPrivate::ctrlHandlerUPP)
	DisposeEventHandlerUPP(QMacControlPrivate::ctrlHandlerUPP);	
    QMacControlPrivate::ctrlHandlerUPP = NULL;
}

/*!
  \class QMacControl qmaccontrol_mac.h
  \brief The QMacControl class provides a bindings between a ControlRef and a QWidget

  \ingroup abstractwidgets
  \mainclass

  Using QMacControl you can wrap a ControlRef (a Carbon "widget") in a
  QWidget. With this binding you may use the QWidget API and internally the
  ControlRef will receive events.
*/


/*!
  Contructs a QMacControl binding. \e parent and \e ctrl must be none NULL,
  and f must not contain WType_TopLevel.

  QMacControl will assume the geometry of \e ctrl, additionally ownership
  of the \e ctrl will be transfered to the QMacControl.

  \sa QWidget::QWidget(), setControl()
*/
QMacControl::QMacControl(QWidget *parent, ControlRef ctrl, const char *name, WFlags f) : QWidget(parent, name, f)
{
    if(!parent || f & WType_TopLevel) 
	qFatal("QMacControl must not be toplevel!!!");
    d = new QMacControlPrivate;
    setControl(ctrl);
}


/*!
  Destructs the QMacControl, this will also destroy control() if it is non-NULL.

  \sa setControl()
*/
QMacControl::~QMacControl()
{
    delete d;
    d = NULL;
}

/*!
  Sets the control binding to \e ctrl. Setting this to NULL will cause the
  binding to be terminated.
*/
void
QMacControl::setControl(ControlRef ctrl)
{
    if(d->ctrl == ctrl)
	return;

    d->ctrl = NULL;
    if(ctrl) {
	//parent the control
	ControlRef root;
	if(GetRootControl((WindowPtr)handle(), &root) || !root)
	    CreateRootControl((WindowPtr)handle(), &root);
	EmbedControl(ctrl, root);
	//resize myself
	Rect rect;
	GetControlBounds(ctrl, &rect);
	QPoint pos = parentWidget()->mapTo(topLevelWidget(), QPoint(rect.left, rect.top));
	move(pos);
	resize(rect.right - rect.left, rect.bottom - rect.top);
	//set my caption
	Str255 str;
	GetControlTitle(ctrl, str);
	setCaption(p2qstring((unsigned char *)str));
	//mask
	QRegion rgn;
	GetControlRegion(ctrl, 0, rgn.handle(TRUE));
	if(!rgn.isEmpty()) {
	    qDebug("Set mask (this is untested)!!!");
	    QPoint p = mapTo(topLevelWidget(), QPoint(0, 0));
	    rgn.translate(-p.x(), -p.y());
	    setMask(rgn);
	}
	//update visibility
	if(IsControlVisible(ctrl))
	    show();
	else
	    hide();
	//enabled state
	if(IsControlEnabled(ctrl))
	    setEnabled(TRUE);
	else
	    setEnabled(FALSE);

	//setup a callback
	if(!QMacControlPrivate::ctrlHandlerUPP) {
	    QMacControlPrivate::ctrlHandlerUPP = NewEventHandlerUPP(QMacControl::ctrlEventProcessor);
	    qAddPostRoutine( cleanup_ctrlHandlerUPP );
	}
	static EventTypeSpec events[] = {
	    { kEventClassMouse, kEventMouseDown },
	    { kEventClassControl, kEventControlDraw }
	};
	InstallEventHandler( GetControlEventTarget(ctrl), QMacControlPrivate::ctrlHandlerUPP,
			     GetEventTypeCount(events), events, (void *)this, &d->ctrlHandler);
	//now set it and repaint
	d->ctrl = ctrl;
	update();
    } else {
	setMask(QRegion());
    }

}

/*!
  Retrieves the current ControlRef binding.
*/
ControlRef
QMacControl::control() const
{
    return d->ctrl;
}

/*! \reimp */
bool
QMacControl::event(QEvent *e)
{
    if(!d->ctrl)
	return QWidget::event(e);
    switch(e->type()) {
    case QEvent::Hide:
	HideControl(d->ctrl);
	break;
    case QEvent::Show:
	ShowControl(d->ctrl);
	break;
    case QEvent::FocusIn:
	SetKeyboardFocus((WindowPtr)handle(), d->ctrl, 0);
	break;
    case QEvent::FocusOut:
	ClearKeyboardFocus((WindowPtr)handle());
	break;
    case QEvent::CaptionChange: {
	QString c = caption();
	SetControlTitleWithCFString(d->ctrl, 
				    CFStringCreateWithCharacters(NULL, (UniChar *)c.unicode(), c.length()));
	break; }
    case QEvent::Paint: {
	QMacPainter p(this);
	p.setPort(); //make sure the clipped region is right
	Draw1Control(d->ctrl);
	break; }
    case QEvent::Move: {
	QPoint p = mapTo(topLevelWidget(), QPoint(0, 0));
	MoveControl(d->ctrl, p.x(), p.y());
	break; }
    case QEvent::Resize: 
	SizeControl(d->ctrl, width()-1, height()-1);
	break;
    case QEvent::Reparent: {
	ControlRef root;
	if(GetRootControl((WindowPtr)handle(), &root) || !root)
	    CreateRootControl((WindowPtr)handle(), &root);
	EmbedControl(d->ctrl, root);
	break; }
    default:
	break;
    }
    return QWidget::event(e);
}

void
QMacControl::enabledChange(bool b)
{
    QWidget::enabledChange(b);
    if(!d->ctrl)
	return;
    if(b)
	EnableControl(d->ctrl);
    else
	DisableControl(d->ctrl);
}

/* \internal */
QMAC_PASCAL OSStatus
QMacControl::ctrlEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
    QMacControl *ctrl = (QMacControl *)data;
    if(!ctrl->d->ctrl)
	return CallNextEventHandler(er, event);

    bool call_back = TRUE;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassControl:
	if(ekind == kEventControlDraw) {
	    static bool once = FALSE;
	    if(!once) {
		once = TRUE;
		call_back = FALSE;
		QMacPainter p(ctrl);
		p.setPort(); //make sure the clipped region is right
		DrawControlInCurrentPort(ctrl->d->ctrl);
		once = FALSE;
	    }
	}
    case kEventClassMouse:
	if(ekind == kEventMouseDown) {
	    call_back = FALSE;
	    Point where;
	    GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
			      sizeof(where), NULL, &where);
	    QPoint p = ctrl->mapFromGlobal(QPoint(where.h, where.v));
	    where.h = p.x();  where.v = p.y();
	    TrackControl(ctrl->d->ctrl, where, NULL);
	    ctrl->repaint();
	}
    }
    if(call_back)
	return CallNextEventHandler(er, event);
    return 0;
}

#endif
