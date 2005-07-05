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

/****************************************************************************
**
** Implementation of QInputContextFactory class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#include "qinputcontextfactory.h"

#ifndef QT_NO_IM

#include "qcoreapplication.h"
#include "qinputcontext.h"
#include "qinputcontextplugin.h"

#ifdef Q_WS_X11
#include "private/qt_x11_p.h"
#include "qximinputcontext_p.h"
#endif
#ifdef Q_WS_WIN
#include "qwininputcontext_p.h"
#endif
#ifdef Q_WS_MAC
#include "qmacinputcontext_p.h"
#endif

#include "private/qfactoryloader_p.h"
#include "qmutex.h"

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QInputContextFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/inputmethods")))
#endif


/*!
    This function generates the input context that has the identifier
    name which is in agreement with \a key. \a widget is the client
    widget of QInputContext. \a widget may be null.
*/
QInputContext *QInputContextFactory::create( const QString& key, QObject *parent )
{
    QInputContext *result = 0;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
    if (key == QLatin1String("xim")) {
        result = new QXIMInputContext;
    }
#endif
#if defined(Q_WS_WIN)
    if (key == QLatin1String("win")) {
        result = new QWinInputContext;
    }
#endif
#if defined(Q_WS_MAC)
    if (key == QLatin1String("mac")) {
        result = new QMacInputContext;
    }
#endif
#ifndef QT_NO_LIBRARY
    if (QInputContextFactoryInterface *factory =
        qobject_cast<QInputContextFactoryInterface*>(loader()->instance(key))) {
        result = factory->create(key);
    }
#endif
    if (result)
        result->setParent(parent);
    return result;
}


/*!
    This function returns the list of the names input methods.
    Only input methods included in default and placed under
    $QTDIR/plugins/inputmethods are listed.
*/
QStringList QInputContextFactory::keys()
{
    QStringList result;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
    result << QLatin1String("xim");
#endif
#if defined(Q_WS_WIN) && !defined(QT_NO_XIM)
    result << QLatin1String("win");
#endif
#if defined(Q_WS_MAC)
    result << QLatin1String("mac");
#endif
#ifndef QT_NO_LIBRARY
    result += loader()->keys();
#endif // QT_NO_LIBRARY
    return result;
}


QStringList QInputContextFactory::languages( const QString &key )
{
    QStringList result;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
    if (key == QLatin1String("xim"))
        return QStringList(QString());
#endif
#if defined(Q_WS_WIN)
    if (key == QLatin1String("win"))
        return QStringList(QString());
#endif
#if defined(Q_WS_MAC)
    if (key == QLatin1String("mac"))
        return QStringList(QString());
#endif
#ifndef QT_NO_LIBRARY
    if (QInputContextFactoryInterface *factory =
        qobject_cast<QInputContextFactoryInterface*>(loader()->instance(key)))
        result = factory->languages(key);
#endif // QT_NO_LIBRARY
    return result;
}


QString QInputContextFactory::displayName( const QString &key )
{
    QString result;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
    if (key == QLatin1String("xim"))
        return QInputContext::tr( "XIM" );
#endif
#ifndef QT_NO_LIBRARY
    if (QInputContextFactoryInterface *factory =
        qobject_cast<QInputContextFactoryInterface*>(loader()->instance(key)))
        return factory->displayName(key);
#endif // QT_NO_LIBRARY
    return QString();
}


QString QInputContextFactory::description( const QString &key )
{
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
    if (key == QLatin1String("xim"))
        return QInputContext::tr( "XIM input method" );
#endif
#if defined(Q_WS_WIN) && !defined(QT_NO_XIM)
    if (key == QLatin1String("win"))
        return QInputContext::tr( "Windows input method" );
#endif
#if defined(Q_WS_MAC)
    if (key == QLatin1String("mac"))
        return QInputContext::tr( "Mac OS X input method" );
#endif
#ifndef QT_NO_LIBRARY
    if (QInputContextFactoryInterface *factory =
        qobject_cast<QInputContextFactoryInterface*>(loader()->instance(key)))
        return factory->description(key);
#endif // QT_NO_LIBRARY
    return QString();
}

#endif // QT_NO_IM
