/****************************************************************************
**
** Implementation of QSqlEditorFactory class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "qsqleditorfactory.h"

#ifndef QT_NO_SQL

#include "qsqlfield.h"
#include "qcleanuphandler.h"

/*!
  \class QSqlEditorFactory qsqleditorfactory.h
    \ingroup database

  \brief The QSqlEditorFactory class is used to create the editors
  used by QSqlDataTable and QSqlForm.

  \module sql

  QSqlEditorFactory is used by QDataTable and QSqlForm to automatically
  create appropriate editors for a given QSqlField. For example if the
  field is a QVariant::String a QLineEdit would be the default editor,
  whereas a QVariant::Int's default editor would be a QSpinBox.

  If you want to create different editors for fields with the same
  data type, subclass QSqlEditorFactory and reimplement the createEditor()
  function.

  \sa QDataTable, QSqlForm
 */


/*!  Constructs a SQL editor factory with parent \a parent and name \a
 name.

*/

QSqlEditorFactory::QSqlEditorFactory ( QObject * parent, const char * name )
    : QEditorFactory( parent, name )
{

}

/*! Destroys the object and frees any allocated resources.

*/

QSqlEditorFactory::~QSqlEditorFactory()
{

}

static QSqlEditorFactory * defaultfactory = 0;
static QCleanupHandler< QSqlEditorFactory > qsql_cleanup_editor_factory;

/*! Returns an instance of a default editor factory.

*/

QSqlEditorFactory * QSqlEditorFactory::defaultFactory()
{
    if( defaultfactory == 0 ){
	defaultfactory = new QSqlEditorFactory();
	qsql_cleanup_editor_factory.add( &defaultfactory );
    }

    return defaultfactory;
}

/*!

  Replaces the default editor factory with \a factory. All QDataTable
  and QSqlForm instantiations will use this new factory for creating
  field editors. <em>QSqlEditorFactory takes ownership of factory,
  and destroys it when it is no longer needed. </em>
*/

void QSqlEditorFactory::installDefaultFactory( QSqlEditorFactory * factory )
{
    if( factory == 0 ) return;

    if( defaultfactory != 0 ){
	qsql_cleanup_editor_factory.remove( &defaultfactory );
	delete defaultfactory;
    }
    defaultfactory = factory;
    qsql_cleanup_editor_factory.add( &defaultfactory );
}

/*!

  Creates and returns the appropriate editor widget for the QVariant
  \a variant.

  The widget that is returned has the parent \a parent (which may be
  zero).  If \a variant is invalid, 0 is returned.


*/

QWidget * QSqlEditorFactory::createEditor( QWidget * parent,
					   const QVariant & variant )
{
    return QEditorFactory::createEditor( parent, variant );
}

/*! \overload

  Creates and returns the appropriate editor for the QSqlField \a field.

*/

QWidget * QSqlEditorFactory::createEditor( QWidget * parent,
					   const QSqlField * field )
{
    QVariant v = field->value();
    return createEditor( parent, v );
}

#endif // QT_NO_SQL
