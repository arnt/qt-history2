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

#include "qdecorationfactory_qws.h"
#include "qdecorationplugin_qws.h"
#include "private/qfactoryloader_p.h"
#include "qmutex.h"

#include "qapplication.h"
#include "qdecorationdefault_qws.h"
#include "qdecorationwindows_qws.h"
#include "qdecorationkde_qws.h"
#include "qdecorationkde2_qws.h"
#include "qdecorationbeos_qws.h"
#include "qdecorationhydro_qws.h"

#ifndef QT_NO_COMPONENT
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QDecorationFactoryInterface_iid,  QCoreApplication::libraryPaths(),  "/decorations",  Qt::CaseInsensitive))
#endif



/*!
    \class QDecorationFactory qdecorationfactory.h
    \brief The QDecorationFactory class creates QDecoration objects.

    \ingroup appearance

    The decoration factory creates a QDecoration object for a given
    key with QDecorationFactory::create(key).

    The decorations are either built-in or dynamically loaded from
    a decoration plugin (see \l QDecorationPlugin).

    QDecorationFactory::keys() returns a list of valid keys, typically
    including "default", "windows", "kde", "kde2", "beos", "hydro".
*/

/*!
    Creates a QDecoration object that matches \a key. This is either a
    built-in decoration, or a decoration from a decoration plugin.

    \sa keys()
*/

QDecoration *QDecorationFactory::create(const QString& key)
{
    QDecoration *ret = 0;
    QString decoration = key.toLower();
#ifndef QT_NO_QWS_DECORATION_DEFAULT
    if (decoration == "default")
        ret = new QDecorationDefault;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_WINDOWS
    if (decoration == "windows")
        ret = new QDecorationWindows;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_KDE
    if (decoration == "kde")
        ret = new QDecorationKDE;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_KDE2
    if (decoration == "kde2")
        ret = new QDecorationKDE2;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_BEOS
    if (decoration == "beos")
        ret = new QDecorationBeOS;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_HYDRO
    if (decoration == "hydro")
        ret = new QDecorationHydro;
    else
#endif
    { } // Keep these here - they make the #ifdefery above work
#ifndef QT_NO_COMPONENT
    if (!ret) {
        if (QDecorationFactoryInterface *factory = qt_cast<QDecorationFactoryInterface*>(loader()->instance(decoration)))
            ret = factory->create(decoration);
    }
#endif
    return ret;
}

/*!
    Returns the list of keys this factory can create decorations for.

    \sa create()
*/
QStringList QDecorationFactory::keys()
{
#ifndef QT_NO_COMPONENT
    QStringList list = loader()->keys();
#else
    QStringList list;
#endif
#ifndef QT_NO_QWS_DECORATION_DEFAULT
    if (!list.contains("Default"))
        list << "Default";
#endif
#ifndef QT_NO_QWS_DECORATION_WINDOWS
    if (!list.contains("Windows"))
        list << "Windows";
#endif
#ifndef QT_NO_QWS_DECORATION_KDE
    if (!list.contains("KDE"))
        list << "KDE";
#endif
#ifndef QT_NO_QWS_DECORATION_KDE2
    if (!list.contains("KDE2"))
        list << "KDE2";
#endif
#ifndef QT_NO_QWS_DECORATION_BEOS
    if (!list.contains("BeOS"))
        list << "BeOS";
#endif
#ifndef QT_NO_QWS_DECORATION_HYDRO
    if (!list.contains("Hydro"))
        list << "Hydro";
#endif

    return list;
}
