/****************************************************************************
** $Id$
**
** Implementation of a custom QMacControl
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
