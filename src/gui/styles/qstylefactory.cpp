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

#include "qstylefactory.h"
#include "qstyleplugin.h"
#include "private/qfactoryloader_p.h"
#include "qmutex.h"

#include "qapplication.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qcdestyle.h"
#ifndef QT_NO_STYLE_PLASTIQUE
#include "qplastiquestyle.h"
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
#include "qwindowsxpstyle.h"
#endif

#if !defined(QT_NO_STYLE_MAC) && defined(Q_WS_MAC)
#  include <private/qt_mac_p.h>
#  include "qmacstyle_mac.h"
QString qt_mac_get_style_name()
{
    QString ret;
    Collection c = NewCollection();
    if (c) {
        GetTheme(c);
        Str255 str;
        long int s = 256;
        if(!GetCollectionItem(c, kThemeNameTag, 0, &s, &str)) {
            extern QString qt_mac_from_pascal_string(const Str255); //qglobal.cpp
            ret = qt_mac_from_pascal_string(str);
        }
    }
    DisposeCollection(c);
    return ret;
}
#endif

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QStyleFactoryInterface_iid, QCoreApplication::libraryPaths(), "/styles", Qt::CaseInsensitive))
#endif

/*!
    \class QStyleFactory qstylefactory.h
    \brief The QStyleFactory class creates QStyle objects.

    \ingroup appearance

    The style factory creates a QStyle object for a given key with
    QStyleFactory::create().

    The styles are either built-in or dynamically loaded from a style
    plugin (see \l QStylePlugin).

    QStyleFactory::keys() returns a list of valid keys, typically
    including "windows", "motif", "cde", and "plastique".
    Depending on the platform, "windowsxp" and "macintosh" may be
    available.

    \sa QStyle
*/

/*!
    Creates a QStyle object that matches \a key. This is either a
    built-in style, or a style from a style plugin.

    \sa keys()
*/
QStyle *QStyleFactory::create(const QString& key)
{
    QStyle *ret = 0;
    QString style = key.toLower();
#ifndef QT_NO_STYLE_WINDOWS
    if (style == "windows")
        ret = new QWindowsStyle;
    else
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if (style == "windowsxp")
        ret = new QWindowsXPStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIF
    if (style == "motif")
        ret = new QMotifStyle;
    else
#endif
#ifndef QT_NO_STYLE_CDE
    if (style == "cde")
        ret = new QCDEStyle;
    else
#endif
#ifndef QT_NO_STYLE_PLASTIQUE
    if (style == "plastique")
        ret = new QPlastiqueStyle;
    else
#endif
#ifndef QT_NO_STYLE_MAC
    if(style.left(9) == "macintosh") {
        ret = new QMacStyle;
#  ifdef Q_WS_MAC
        if(style == "macintosh")
            style +=" (" + qt_mac_get_style_name() + ")";
#  endif
    } else
#endif
    { } // Keep these here - they make the #ifdefery above work
#ifndef QT_NO_LIBRARY
    if(!ret) {
        if (QStyleFactoryInterface *factory = qobject_cast<QStyleFactoryInterface*>(loader()->instance(style)))
            ret = factory->create(style);
    }
#endif
    if(ret)
        ret->setObjectName(style);
    return ret;
}

/*!
    Returns the list of keys this factory can create styles for.

    \sa create()
*/
QStringList QStyleFactory::keys()
{
#ifndef QT_NO_LIBRARY
    QStringList list = loader()->keys();
#else
    QStringList list;
#endif
#ifndef QT_NO_STYLE_WINDOWS
    if (!list.contains("Windows"))
        list << "Windows";
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if (!list.contains("WindowsXP"))
        list << "WindowsXP";
#endif
#ifndef QT_NO_STYLE_MOTIF
    if (!list.contains("Motif"))
        list << "Motif";
#endif
#ifndef QT_NO_STYLE_CDE
    if (!list.contains("CDE"))
        list << "CDE";
#endif
#ifndef QT_NO_STYLE_PLASTIQUE
    if (!list.contains("Plastique"))
        list << "Plastique";
#endif
#ifndef QT_NO_STYLE_MAC
    QString mstyle = "Macintosh";
# ifdef Q_WS_MAC
    mstyle += " (" + qt_mac_get_style_name() + ")";
# endif
    if (!list.contains(mstyle))
        list << mstyle;
#endif
    return list;
}
