/*
  LocalSQL

  Copyright (C) 2001 Trolltech AS

  Contact:
	 Dave Berton (db@trolltech.com)
	 Jasmin Blanchette (jasmin@trolltech.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <qfeatures.h>

#ifndef QT_NO_COMPONENT
#ifndef QT_NO_SQL

#include <qsqldriverinterface.h>
#include <localsql_qt.h>

class LocalSQLDriverPlugin : public QSqlDriverInterface
{
public:
    LocalSQLDriverPlugin();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;

private:
    unsigned long ref;
};

LocalSQLDriverPlugin::LocalSQLDriverPlugin()
: ref( 0 )
{
}

QUnknownInterface *LocalSQLDriverPlugin::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QSqlDriverInterface )
	iface = (QSqlDriverInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long LocalSQLDriverPlugin::addRef()
{
    return ref++;
}

unsigned long LocalSQLDriverPlugin::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QSqlDriver* LocalSQLDriverPlugin::create( const QString &name )
{
    if ( name == "LocalSQL" )
	return new LocalSQLDriver();
    return 0;
}

QStringList LocalSQLDriverPlugin::featureList() const
{
    QStringList l;
    l.append("LocalSQL");
    return l;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( LocalSQLDriverPlugin )
}

#endif
#endif
