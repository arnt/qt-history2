/****************************************************************************
** $Id: $
**
** Definition of QMacControl class
**
** Created : 931029
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __QMACCONTROL_MAC_H__
#define __QMACCONTROL_MAC_H__

#ifndef QT_H
#include <qwidget.h>
#endif

#ifdef Q_WS_MAC

class QMacControl;
class QMacControlPrivate;
typedef short ControlPartCode;
typedef long unsigned int OSType;
extern const OSType qControlWidgetTag;
extern const OSType qControlWidgetCreator;

class Q_EXPORT QMacTrackEvent : public QEvent
{
public:
    static const int teType;
    QMacTrackEvent(QMacControl *ctrl, const QPoint &pos) : QEvent((QEvent::Type)teType), 
	accpt(0), c(ctrl), p(pos) { }
    ~QMacTrackEvent() { }
    QPoint pos() const { return p; }
    int x() const { return p.x(); }
    int y() const { return p.y(); }
    QMacControl *control() const { return c; }
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
private:
    uint accpt : 1;
    QMacControl *c;
    QPoint p;
};

class Q_EXPORT QMacControl : public QWidget
{
    Q_OBJECT
public:
    QMacControl(QWidget *parent, ControlRef ctrl, const char *name=0, WFlags f=0 );
    QMacControl(QWidget *parent, const char *name=0, WFlags f=0 );
    ~QMacControl();

    void setControl(ControlRef ctrl);
    ControlRef control() const;

protected:
    bool event(QEvent *);
    void enabledChange(bool);
    virtual void trackControlEvent(QMacTrackEvent *);

private:
    static QMAC_PASCAL OSStatus ctrlEventProcessor(EventHandlerCallRef,  EventRef, void *);
    static QMAC_PASCAL void ctrlAction(ControlRef, ControlPartCode);
    QMacControlPrivate *d;

signals:
    void action(QMacControl *, ControlPartCode);
};

#endif //Q_WS_MAC
#endif /* __QMACCONTROL_MAC_H__ */
