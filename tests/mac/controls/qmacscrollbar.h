/****************************************************************************
**
** Implementation of a custom QMacControl.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QMACSCROLLBAR_H__
#define __QMACSCROLLBAR_H__

#include "qmaccontrol_mac.h"
#include <Carbon/Carbon.h>

class Q_EXPORT QMacScrollBar : public QMacControl
{
    Q_OBJECT
public:
    QMacScrollBar(QWidget *, const char * =NULL);
    ~QMacScrollBar() { }

protected:
    virtual void trackControlEvent(QMacTrackEvent *);

private:
    static ControlActionUPP actionUPP;
    static QMAC_PASCAL void actionCallbk(ControlRef, ControlPartCode);
};

#endif /* __QMACSCROLLBAR_H__ */
