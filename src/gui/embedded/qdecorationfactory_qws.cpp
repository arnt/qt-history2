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
#include "qdecorationstyled_qws.h"

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QDecorationFactoryInterface_iid, QCoreApplication::libraryPaths(), "/decorations", Qt::CaseInsensitive))
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
    or a decoration from a decoration plugin. Keys are case sensitive.

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
#ifndef QT_NO_QWS_DECORATION_STYLED
    if (decoration == "styled")
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
#ifndef QT_NO_LIBRARY
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
#ifndef QT_NO_QWS_DECORATION_STYLED
    if (!list.contains("Styled"))
        list << "Styled";
#endif

    return list;
}
