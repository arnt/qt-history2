/****************************************************************************
**
** Implementation of the qax_create_object_wrapper function.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
