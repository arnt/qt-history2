/****************************************************************************
**
** Implementation of QStyleFactory class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
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
#include "qmotifplusstyle.h"
#include "qplatinumstyle.h"
#include "qsgistyle.h"
#include "qcompactstyle.h"
#ifndef QT_NO_STYLE_WINDOWSXP
#include "qwindowsxpstyle.h"
#endif
#ifndef QT_NO_STYLE_POCKETPC
#include "qpocketpcstyle_wce.h"
#endif

#if !defined(QT_NO_STYLE_MAC) && defined(Q_WS_MAC)
QString pstring2qstring(const unsigned char *c); //qglobal.cpp
#include "qt_mac.h"
#include "qmacstyle_mac.h"
#endif

Q_GLOBAL_STATIC_LOCKED_WITH_ARGS(QFactoryLoader, loader,
                                 (QStyleFactoryInterface_iid, QCoreApplication::libraryPaths(), "/styles", Qt::CaseInsensitive))

/*!
    \class QStyleFactory qstylefactory.h
    \brief The QStyleFactory class creates QStyle objects.

    \ingroup appearance

    The style factory creates a QStyle object for a given key with
    QStyleFactory::create(key).

    The styles are either built-in or dynamically loaded from a style
    plugin (see \l QStylePlugin).

    QStyleFactory::keys() returns a list of valid keys, typically
    including "windows", "motif", "cde", "motifplus", "platinum",
    "sgi" and "compact". Depending on the platform, "windowsxp",
    "aqua" or "macintosh" may be available.
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
    if (style == "windows") {
        ret = new QWindowsStyle;
    }
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
#ifndef QT_NO_STYLE_MOTIFPLUS
    if (style == "motifplus")
        ret = new QMotifPlusStyle;
    else
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if (style == "platinum")
        ret = new QPlatinumStyle;
    else
#endif
#ifndef QT_NO_STYLE_SGI
    if (style == "sgi")
        ret = new QSGIStyle;
    else
#endif
#ifndef QT_NO_STYLE_COMPACT
    if (style == "compact")
        ret = new QCompactStyle;
    else
#endif
#ifndef QT_NO_STYLE_POCKETPC
    if (style == "pocketpc")
        ret = new QPocketPCStyle;
#endif
#if !defined(QT_NO_STYLE_MAC) && defined(Q_WS_MAC)
    if(style.left(9) == "macintosh")
        ret = new QMacStyle;
#endif
    { } // Keep these here - they make the #ifdefery above work

    if(!ret) {
        if (QStyleFactoryInterface *factory = qt_cast<QStyleFactoryInterface*>(loader()->instance(style)))
            ret = factory->create(style);
    }
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
    QStringList list = loader()->keys();

#ifndef QT_NO_STYLE_WINDOWS
    if (!list.contains("Windows"))
        list << "Windows";
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if (!list.contains("WindowsXP") && QWindowsXPStyle::resolveSymbols())
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
#ifndef QT_NO_STYLE_MOTIFPLUS
    if (!list.contains("MotifPlus"))
        list << "MotifPlus";
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if (!list.contains("Platinum"))
        list << "Platinum";
#endif
#ifndef QT_NO_STYLE_SGI
    if (!list.contains("SGI"))
        list << "SGI";
#endif
#ifndef QT_NO_STYLE_COMPACT
    if (!list.contains("Compact"))
        list << "Compact";
#endif
#if !defined(QT_NO_STYLE_MAC) && defined(Q_WS_MAC)
    QString mstyle = "Macintosh";
    Collection c = NewCollection();
    if (c) {
        GetTheme(c);
        Str255 str;
        long int s = 256;
        if(!GetCollectionItem(c, kThemeNameTag, 0, &s, &str))
            mstyle += " (" + pstring2qstring(str) + ")";
    }
    if (!list.contains(mstyle))
        list << mstyle;
    DisposeCollection(c);
#endif

    return list;
}
