/****************************************************************************
**
** Implementation of Interbase driver plugin
**
** Created : 001103
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
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

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/ibase/qsql_ibase.h"

class QIBaseDriverPlugin : public QSqlDriverPlugin
{
public:
    QIBaseDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QIBaseDriverPlugin::QIBaseDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QIBaseDriverPlugin::create(const QString &name)
{
    if (name == "QIBASE") {
        QIBaseDriver* driver = new QIBaseDriver();
        return driver;
    }
    return 0;
}

QStringList QIBaseDriverPlugin::keys() const
{
    QStringList l;
    l  << "QIBASE";
    return l;
}

Q_EXPORT_PLUGIN(QIBaseDriverPlugin)
