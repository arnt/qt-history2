/****************************************************************************
**
** Implementation of QStyleFactory class
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qstyleinterface.h" // up here for GCC 2.7.* compatibility
#include "qstylefactory.h"

#ifndef QT_NO_STYLE

#include "qinterfacemanager.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qcdestyle.h"
#include "qmotifplusstyle.h"
#include "qplatinumstyle.h"
#include "qsgistyle.h"
#include "qcompactstyle.h"
#include "qapplication.h"
#include "qaquastyle.h"
#include <stdlib.h>

#ifndef QT_NO_COMPONENT
static QInterfaceManager<QStyleInterface> *manager = 0;

static void ensureManager()
{
    if ( !manager ) {
	manager = new QInterfaceManager<QStyleInterface>( IID_QStyleInterface, QString::null, "*.dll; *.so", QLibrary::Delayed, FALSE );

        QString defpath(getenv("QTDIR"));
        if (! defpath.isNull() && ! defpath.isEmpty()) {
            manager->addLibraryPath(defpath + "/plugins");
        }

        QStringList paths(QApplication::libraryPaths());
        QStringList::Iterator it = paths.begin();
        while (it != paths.end()) {
            manager->addLibraryPath(*it);
            it++;
        }
    }
}

#endif //QT_NO_COMPONENT

QStyle *QStyleFactory::create( const QString& s )
{
#ifndef QT_NO_COMPONENT
    ensureManager();

    QStyleInterface *iface = manager->queryInterface( s );

    if ( iface )
        return iface->create( s );
#endif

    QString style = s.lower();
#ifndef QT_NO_STYLE_WINDOWS
    if ( style == "windows" )
        return new QWindowsStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIF
    if ( style == "motif" )
        return new QMotifStyle;
    else
#endif
#ifndef QT_NO_STYLE_CDE
    if ( style == "cde" )
        return new QCDEStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    if ( style == "motifplus" )
        return new QMotifPlusStyle;
    else
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if ( style == "platinum" )
        return new QPlatinumStyle;
    else
#endif
#ifndef QT_NO_STYLE_SGI
    if ( style == "sgi")
        return new QSGIStyle;
    else
#endif
#ifndef QT_NO_STYLE_COMPACT
    if ( style == "compact" )
        return new QCompactStyle;
    else
#endif
#ifndef QT_NO_STYLE_AQUA
    if ( style == "aqua" )
        return new QAquaStyle;
#endif

    return 0;
}


QStringList QStyleFactory::styles()
{
#ifndef QT_NO_COMPONENT
    ensureManager();

    QStringList list = manager->featureList();
#else
    QStringList list;
#endif //QT_NO_COMPONENT

#ifndef QT_NO_STYLE_WINDOWS
    list << "Windows";
#endif
#ifndef QT_NO_STYLE_MOTIF
    list << "Motif";
#endif
#ifndef QT_NO_STYLE_CDE
    list << "CDE";
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    list << "MotifPlus";
#endif
#ifndef QT_NO_STYLE_PLATINUM
    list << "Platinum";
#endif
#ifndef QT_NO_STYLE_SGI
    list << "SGI";
#endif
#ifndef QT_NO_STYLE_COMPACT
    list << "Compact";
#endif
#ifndef QT_NO_STYLE_AQUA
    list << "Aqua";
#endif

    return list;
}

#endif // QT_NO_STYLE
