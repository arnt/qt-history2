/****************************************************************************
**
** Implementation of TDS driver plugin
**
** Created : 001103
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define Q_UUIDIMPL
#include <qsqldriverplugin.h>
#include "../../../../src/sql/drivers/tds/qsql_tds.h"

class QTDSDriverPlugin : public QSqlDriverPlugin
{
public:
    QTDSDriverPlugin();

    QSqlDriver* create( const QString & );
    QStringList keys() const;
};

QTDSDriverPlugin::QTDSDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QTDSDriverPlugin::create( const QString &name )
{
    if ( name == "QTDS7" ) {
	QTDSDriver* driver = new QTDSDriver();
	return driver;
    }
    return 0;
}

QStringList QTDSDriverPlugin::keys() const
{
    QStringList l;
    l.append("QTDS7");
    return l;
}

Q_EXPORT_PLUGIN( QTDSDriverPlugin )
