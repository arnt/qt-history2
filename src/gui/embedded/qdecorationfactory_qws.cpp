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

#include "qdecorationfactory_qws.h"
#include "qdecorationplugin_qws.h"
#include "private/qfactoryloader_p.h"
#include "qmutex.h"

#include "qapplication.h"
#include "qdecorationdefault_qws.h"
#include "qdecorationwindows_qws.h"
#include "qdecorationstyled_qws.h"

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QDecorationFactoryInterface_iid, QCoreApplication::libraryPaths(),
     QLatin1String("/decorations"), Qt::CaseInsensitive))
#endif



/*!
    \class QDecorationFactory
    \ingroup qws
    \ingroup appearance

    \brief The QDecorationFactory class creates QDecoration objects.

    Note that this class is only available in \l {Qtopia Core}.

    The QDecoration class is used to customize the appearance of top
    level windows. QDecorationFactory creates a QDecoration object
    using the create() function and a key identifying the
    decoration.

    The valid keys can be retrieved using the keys() function. The
    decorations are either built-in or dynamically loaded from a
    decoration plugin (see \l QDecorationPlugin).

    \sa QDecoration, QDecorationPlugin
*/

/*!
    Creates a QDecoration object for the decoration specified by the
    given \a key. The \a key specifies either a built-in decoration,
    or a decoration from a decoration plugin. Keys are case-insensitive.

    \sa keys()
*/

QDecoration *QDecorationFactory::create(const QString& key)
{
    QDecoration *ret = 0;
    QString decoration = key.toLower();
#ifndef QT_NO_QWS_DECORATION_DEFAULT
    if (decoration == QLatin1String("default"))
        ret = new QDecorationDefault;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_WINDOWS
    if (decoration == QLatin1String("windows"))
        ret = new QDecorationWindows;
    else
#endif
#ifndef QT_NO_QWS_DECORATION_STYLED
    if (decoration == QLatin1String("styled"))
        ret = new QDecorationStyled;
    else
#endif
    { } // Keep these here - they make the #ifdefery above work
#ifndef QT_NO_LIBRARY
    if (!ret) {
        if (QDecorationFactoryInterface *factory = qobject_cast<QDecorationFactoryInterface*>(loader()->instance(decoration))) {
            ret = factory->create(decoration);
        }
    }
#endif
    return ret;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create decorations for.

    \l {Qtopia Core} currently supports the following decorations by
    default: \c Default, \c Styled and \c Windows.

    \sa create()
*/
QStringList QDecorationFactory::keys()
{
    QStringList list;
#ifndef QT_NO_QWS_DECORATION_DEFAULT
    list << QLatin1String("Default");
#endif
#ifndef QT_NO_QWS_DECORATION_WINDOWS
    list << QLatin1String("Windows");
#endif
#ifndef QT_NO_QWS_DECORATION_STYLED
    list << QLatin1String("Styled");
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    QStringList plugins = loader()->keys();
    for (int i = 0; i < plugins.size(); ++i) {
        if (!list.contains(plugins.at(i)))
            list += plugins.at(i);
    }
#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

    return list;
}
