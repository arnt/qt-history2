/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaxobject.h"
#include <activeqt/qaxfactory.h>

#include <qt_windows.h>

QAxBase *qax_create_object_wrapper(QObject *object)
{
    IDispatch *dispatch = 0;
    QAxObject *wrapper = 0;
    qAxFactory()->createObjectWrapper(object, &dispatch);
    if (dispatch) {
        wrapper = new QAxObject(dispatch, object);
        wrapper->setObjectName(object->objectName() + "_ScriptWrapper");
        dispatch->Release();
    }
    return wrapper;
}
