/****************************************************************************
**
** Implementation of OCI driver plugin
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

#include "../../qsqldriverinterface.h"
#include "qsql_oci.h"
#include <qstringlist.h>

class QOCIDriverInterface : public QSqlDriverInterface
{
public:
    QOCIDriverInterface( QUnknownInterface * parent = 0 )
	: QSqlDriverInterface( parent ){}

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;
};

QSqlDriver* QOCIDriverInterface::create( const QString &name )
{
    if ( name == "QOCI" ) {
	return new QOCIDriver();
    }
    return 0;
}

QStringList QOCIDriverInterface::featureList() const
{
    QStringList l;
    l.append("QOCI");
    return l;
}


class QOCIDriverPlugIn : public QUnknownInterface
{
public:
    QOCIDriverPlugIn();
};

QOCIDriverPlugIn::QOCIDriverPlugIn()
    : QUnknownInterface()
{
    new QOCIDriverInterface( this );
}


Q_EXPORT_INTERFACE(QOCIDriverPlugIn)
