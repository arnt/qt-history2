/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwsproperty_qws.h"

#ifndef QT_NO_QWS_PROPERTIES
#include "qwscommand_qws_p.h"
#include "qwindowsystem_qws.h"
#include "qhash.h"
#include "qalgorithms.h"
#include "qbytearray.h"

#include <stdio.h>

class QWSPropertyManager::Data {
public:
    QByteArray find(int winId, int property)
    {
        return properties.value(winId).value(property);
    }

    typedef QHash<int, QHash<int, QByteArray> > PropertyHash;
    PropertyHash properties;
};

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
    d = new Data;
}

QWSPropertyManager::~QWSPropertyManager()
{
    delete d;
}

bool QWSPropertyManager::setProperty(int winId, int property, int mode, const char *data, int len)
{
    QHash<int, QByteArray> props = d->properties.value(winId);
    QHash<int, QByteArray>::iterator it = props.find(property);
    if (it == props.end())
        return false;

    switch (mode) {
    case PropReplace:
        it.value() = QByteArray(data, len);
        break;
    case PropAppend:
        it.value().append(data);
        break;
    case PropPrepend:
        it.value().prepend(data);
        break;
    }
    return true;
}

bool QWSPropertyManager::hasProperty(int winId, int property)
{
    return d->properties.value(winId).contains(property);
}

bool QWSPropertyManager::removeProperty(int winId, int property)
{
    QWSPropertyManager::Data::PropertyHash::iterator it = d->properties.find(winId);
    if (it == d->properties.end())
        return false;
    return it.value().remove(property);
}

bool QWSPropertyManager::addProperty(int winId, int property)
{
    d->properties[winId][property] = QByteArray();
    return true;
}

bool QWSPropertyManager::getProperty(int winId, int property, char *&data, int &len)
{
    QHash<int, QByteArray> props = d->properties.value(winId);
    QHash<int, QByteArray>::iterator it = props.find(property);
    if (it == props.end()) {
        data = 0;
        len = -1;
        return false;
    }
    data = it.value().data();
    len = it.value().length();

    return true;
}

bool QWSPropertyManager::removeProperties(int winId)
{
    return d->properties.remove(winId);
}

#endif //QT_NO_QWS_PROPERTIES
