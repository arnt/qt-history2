/****************************************************************************
** $Id: $
**
** Declaration of the QAxBindable class
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
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

#ifndef QAXBINDABLE_H
#define QAXBINDABLE_H

#include <qwidget.h>
#include <private/qcom_p.h>

class QAxServerBase;

class QAxBindable
{
    friend class QAxServerBase;
public:
    QAxBindable();

    virtual QRESULT queryInterface( const QUuid&, void** );
    long addRef();
    long release();

    virtual bool stayTopLevel() const;
    virtual bool hasStockEvents() const;

protected:
    bool requestPropertyChange( const char *property );
    void propertyChanged( const char *property );

private:
    QAxServerBase *activex;
};

#endif // QAXBINDABLE_H
