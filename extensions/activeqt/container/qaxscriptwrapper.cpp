/****************************************************************************
** $Id: $
**
** Implementation of the qax_create_object_wrapper function
**
** Copyright (C) 2002-2003 Trolltech AS.  All rights reserved.
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

#include <qaxobject.h>
#include <qaxfactory.h>

#include <qt_windows.h>

QAxBase *qax_create_object_wrapper(QObject *object)
{
    IDispatch *dispatch = 0;
    QAxBase *wrapper = 0;
    qAxFactory()->createObjectWrapper(object, &dispatch);
    if (dispatch) {
	wrapper = new QAxObject(dispatch, object, object->objectName());
	dispatch->Release();
    }
    return wrapper;
}
