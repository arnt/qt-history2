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
#include <qapplication.h>

const OSType qControlWidgetTag = 'qmct';
const OSType qControlWidgetCreator = 'CUTE';
void qt_mac_clear_mouse_state(); // qapplication_mac.cpp
QCString p2qstring(const unsigned char *); //qglobal.cpp

struct QMacControlPrivate
{
    QMacControlPrivate();
    ~QMacControlPrivate();
    ControlRef ctrl;
    EventHandlerRef ctrlHandler;
    ControlActionUPP oldCtrlActionUPP;

    static EventHandlerUPP ctrlHandlerUPP;
    static ControlActionUPP ctrlActionUPP;
};
EventHandlerUPP QMacControlPrivate::ctrlHandlerUPP = NULL;
ControlActionUPP QMacControlPrivate::ctrlActionUPP = NULL;
QMacControlPrivate::QMacControlPrivate() : ctrl(NULL), ctrlHandler(NULL), oldCtrlActionUPP(NULL) { }
QMacControlPrivate::~QMacControlPrivate() 
{
    if(oldCtrlActionUPP)
	DisposeControlActionUPP(oldCtrlActionUPP);
    oldCtrlActionUPP = NULL;
    if(ctrlHandler)
	RemoveEventHandler(ctrlHandler);
    ctrlHandler = NULL;
    if(ctrl)
	DisposeControl(ctrl);
    ctrl = NULL;
}
static void cleanup_qmaccontrol()
{
    if(QMacControlPrivate::ctrlHandlerUPP)
	DisposeEventHandlerUPP(QMacControlPrivate::ctrlHandlerUPP);	
    QMacControlPrivate::ctrlHandlerUPP = NULL;
    if(QMacControlPrivate::ctrlActionUPP)
	DisposeControlActionUPP(QMacControlPrivate::ctrlActionUPP);
    QMacControlPrivate::ctrlActionUPP = NULL;
}

const int QMacTrackEvent::teType = (QEvent::User+666);

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
  Contructs a QMacControl binding. \e parent must be none NULL, and f must
  not contain WType_TopLevel.

  \sa QWidget::QWidget()
*/
QMacControl::QMacControl(QWidget *parent, const char *name, WFlags f) : QWidget(parent, name, f)
{
    if(!parent || f & WType_TopLevel) 
	qFatal("QMacControl must not be toplevel!!!");
    d = new QMacControlPrivate;
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

    if(d->ctrl) 
	SetControlAction(d->ctrl, d->oldCtrlActionUPP);
    d->oldCtrlActionUPP = NULL;
    if(d->ctrl)
	RemoveControlProperty(ctrl, qControlWidgetCreator, qControlWidgetTag);
    d->ctrl = NULL;
    if(d->ctrlHandler)
	RemoveEventHandler(d->ctrlHandler);
    d->ctrlHandler = NULL;
    if(ctrl) {
	//Flags
	//sanity check
	UInt32 tmp = 0;
	if(!GetControlPropertySize(ctrl, qControlWidgetCreator, qControlWidgetTag, &tmp) || tmp) {
	    qWarning("You cannot bind to a ControlRef twice!!");
	    return;
	}
	//set a flag
	const QMacControl *me = this;
	SetControlProperty(ctrl, qControlWidgetCreator, qControlWidgetTag, sizeof(this), &me);

	//Properties
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
	//update visibility
	if(IsControlVisible(ctrl))
	    show();
	else
	    hide();
	//enabled state
	setEnabled(IsControlEnabled(ctrl));

	//Features
	UInt32 feat;
	GetControlFeatures(ctrl, &feat);
	//focus
	setFocusPolicy((feat & kControlSupportsFocus) ? StrongFocus : NoFocus);
	//mask
	if(feat & kControlSupportsGetRegion) {
	    QRegion rgn;
	    GetControlRegion(ctrl, 0, rgn.handle(TRUE));
	    if(!rgn.isEmpty()) {
		qDebug("Set mask (this is untested)!!!");
		QPoint p = mapTo(topLevelWidget(), QPoint(0, 0));
		rgn.translate(-p.x(), -p.y());
		setMask(rgn);
	    }
	}

	//Callbacks
	//setup event callback
	if(!QMacControlPrivate::ctrlHandlerUPP) {
	    QMacControlPrivate::ctrlHandlerUPP = NewEventHandlerUPP(QMacControl::ctrlEventProcessor);
	    qAddPostRoutine( cleanup_qmaccontrol );
	}
	static EventTypeSpec events[] = {
	    { kEventClassKeyboard, kEventRawKeyDown },
	    { kEventClassControl, kEventControlDraw },
	    { kEventClassMouse, kEventMouseDown }
	};
	InstallEventHandler( GetControlEventTarget(ctrl), QMacControlPrivate::ctrlHandlerUPP,
			     GetEventTypeCount(events), events, (void *)this, &d->ctrlHandler);
	//action callback
	if(GetControlAction(ctrl)) 
	    d->oldCtrlActionUPP = GetControlAction(ctrl);
	if(!QMacControlPrivate::ctrlActionUPP)
	    QMacControlPrivate::ctrlActionUPP = NewControlActionUPP(QMacControl::ctrlAction);
	SetControlAction(ctrl, QMacControlPrivate::ctrlActionUPP);
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
    case QMacTrackEvent::teType:
	trackControlEvent((QMacTrackEvent*)e);
	break;
    case QEvent::Hide:
	HideControl(d->ctrl);
	break;
    case QEvent::Show:
	ShowControl(d->ctrl);
	break;
    case QEvent::FocusIn:
	SetKeyboardFocus((WindowPtr)handle(), d->ctrl, kControlFocusNextPart);
	break;
    case QEvent::FocusOut:
	ClearKeyboardFocus((WindowPtr)handle());
	break;
    case QEvent::CaptionChange: {
	QString c = caption();
	CFStringRef s = CFStringCreateWithCharacters(NULL, (UniChar *)c.unicode(), 
						     c.length());
	SetControlTitleWithCFString(d->ctrl, s);
	break; }
    case QEvent::Paint: {
	//handle activation
	if(!topLevelWidget()->isActiveWindow()) 
	    DeactivateControl(d->ctrl);
	else
	    ActivateControl(d->ctrl);
	//now draw it
	QMacSavedPortInfo pi(this, TRUE);
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
    case QEvent::MouseButtonPress: {
	QPoint p = ((QMouseEvent*)e)->globalPos();
	QMacTrackEvent te(this, topLevelWidget()->mapFromGlobal(p));
	QApplication::sendEvent(this, &te); 
	qt_mac_clear_mouse_state();
	if(te.isAccepted())
	    repaint();
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
QMAC_PASCAL void
QMacControl::ctrlAction(ControlRef ctrl, ControlPartCode cpc)
{
    UInt32 len;
    QMacControl *qmc;
    GetControlProperty(ctrl, qControlWidgetCreator, qControlWidgetTag, sizeof(qmc), &len, &qmc);
    if(!qmc || len != sizeof(qmc)) 
	return;
    if(qmc->d->oldCtrlActionUPP) 
	InvokeControlActionUPP(ctrl, cpc, qmc->d->oldCtrlActionUPP);
    emit qmc->action(qmc, cpc);
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
		QMacSavedPortInfo pi(ctrl, TRUE);
		DrawControlInCurrentPort(ctrl->d->ctrl);
		once = FALSE;
	    }
	}
	break;
    case kEventClassKeyboard:
	if(ekind == kEventRawKeyDown) {
	    EventRecord erec;
	    if(ConvertEventRefToEventRecord(event, &erec)) { //lazy lazy lazy..
		HandleControlKey(ctrl->d->ctrl, (erec.message & keyCodeMask)>>16, 
				 erec.message & charCodeMask, erec.modifiers);
	    } else {
		qDebug("Untested case..");
		UInt32 keyc, modif, state;
		GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
		GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modif), NULL, &modif);
		char chr = KeyTranslate((void *)GetScriptManagerVariable(smUnicodeScript), 
					(modif & (shiftKey|rightShiftKey)) | keyc, &state);
		HandleControlKey(ctrl->d->ctrl, keyc, chr, 0);
	    }
	}
	break; 
    case kEventClassMouse: {
	Point where;
	GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
			  sizeof(where), NULL, &where);
	QWidget *ctrlTLW = ctrl->topLevelWidget();
	QPoint p = ctrlTLW->mapFromGlobal(QPoint(where.h, where.v));
	if(ctrl->clippedRegion(FALSE).contains(p)) {
	    call_back = FALSE;
	    if(ekind == kEventMouseDown) {
		if(ctrl->focusPolicy() & ClickFocus)
		    ctrl->setFocus(); //I must have the focus!

		QMacTrackEvent te(ctrl, p);
		QApplication::sendEvent(ctrl, &te); 
		if(te.isAccepted())
		    ctrl->repaint();
	    }
	}
	break; }
    default:
	break;
    }
    if(call_back)
	return CallNextEventHandler(er, event);
    return 0;
}

void
QMacControl::trackControlEvent(QMacTrackEvent *te)
{
    te->accept();
    Point where = { te->y(), te->y() };
    TrackControl(te->control()->control(), where, NULL);
}

#endif





\
