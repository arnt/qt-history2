/****************************************************************************
**
** Implementation of QSqlRecord class
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

#include "qsqlrecord.h"

#ifndef QT_NO_SQL

#include "qregexp.h"
#include "qmap.h"
#include "qshared.h"
#include "qnamespace.h"

class QSqlRecordPrivate
{
public:
    class info {
    public:
	info() : nogen(FALSE),align(0),label(),visible(TRUE){}
	~info() {}
	info( const info& other )
	    : field( other.field ), nogen( other.nogen ), align( other.align ), label( other.label ),
	      visible( other.visible )
	{
	}
	info& operator=(const info& other)
	{
	    field = other.field;
	    nogen = other.nogen;
	    align = other.align;
	    label = other.label;
	    visible = other.visible;
	    return *this;
	}
	QSqlField field;
	bool    nogen;
	int     align;
	QString label;
	bool    visible;
    };

    QSqlRecordPrivate() { }
    QSqlRecordPrivate( const QSqlRecordPrivate& other )
    {
	*this = other;
    }
    ~QSqlRecordPrivate() {};
    QSqlRecordPrivate& operator=( const QSqlRecordPrivate& other )
    {
	fi = other.fi;
	return *this;
    }
    void append( const QSqlField& field )
    {
	info i;
	i.field = field;
	fi.insert( (int)fi.count(), i );
    }
    void insert( int pos, const QSqlField& field )
    {
	info i;
	i.field = field;
	fi.insert( pos, i );
    }
    void remove( int i )
    {
	fi.remove( i );
    }
    void clear()
    {
	fi.clear();
    }
    bool isEmpty()
    {
	return fi.isEmpty();
    }
    info* fieldInfo( int i )
    {
	return &fi[i];
    }
    uint count()
    {
	return fi.count();
    }
    bool contains( int i )
    {
	return fi.contains( i );
    }
private:
    QMap< int, info > fi;
};

QSqlRecordShared::~QSqlRecordShared()
{
    if ( d )
	delete d;
}

/*!
    \class QSqlRecord qsqlfield.h
    \brief A set of database fields.

    \module sql

    The QSqlRecord class encapsulates the functionality and
    characteristics of a database record (usually a table or view
    within the database), such as adding or removing fields, setting
    and retrieving field values, etc.  In addition, there are several
    functions which alter other characteristics of the record, for
    example, changing the display label associated with a particular
    field when displaying that field on screen.

    QSqlRecord is implicitly shared. This means you can make copies of
    the record in time O(1). If multiple QSqlRecord instances share
    the same data and one is modifying the record's data then this
    modifying instance makes a copy and modifies its private copy -
    thus it does not affect other instances.

*/


/*!
  Constructs an empty record.

*/

QSqlRecord::QSqlRecord()
{
    sh = new QSqlRecordShared( new QSqlRecordPrivate() );
}

/*!  Constructs a copy of \a other.

*/

QSqlRecord::QSqlRecord( const QSqlRecord& other )
    : sh( other.sh )
{
    sh->ref();
}

/*! Sets the record equal to \a other.
*/

QSqlRecord& QSqlRecord::operator=( const QSqlRecord& other )
{
    other.sh->ref();
    deref();
    sh = other.sh;
    return *this;
}

/*! \internal
*/

void QSqlRecord::deref()
{
    if ( sh->deref() ) {
	delete sh;
	sh = 0;
    }
}

/*! \internal
*/

bool QSqlRecord::checkDetach()
{
    if ( sh->count > 1 ) {
	sh->deref();
	sh = new QSqlRecordShared( new QSqlRecordPrivate( *sh->d ) );
	return TRUE;
    }
    return FALSE;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlRecord::~QSqlRecord()
{
    deref();
}

/*!  Returns the value of the field located at position \a i in the
  record.  It is up to you to check wether this item really exists.

*/

QVariant QSqlRecord::value( int i ) const
{
    return field(i)->value();
}

/*!  Returns the value of the field named \a name in the record.  It
  is up to you to check wether this item really exists.

*/

QVariant  QSqlRecord::value( const QString& name ) const
{
    return field( name )->value();
}

/*! Returns the name of the field at position \a i.  If the field does
  not exist, QString::null is returned.
*/

QString QSqlRecord::fieldName( int i ) const
{
    const QSqlField* f = field( i );
    if ( f )
	return f->name();
    return QString::null;
}

/*!  Returns the position of the field named \a name within the
  record, or -1 if it cannot be found.  Field names are not
  case-sensitive.

*/

int QSqlRecord::position( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( fieldName(i).upper() == name.upper() )
	    return i;
    }
#ifdef QT_CHECK_RANGE
    qWarning( "QSqlRecord::position: unable to find field " + name );
#endif
    return -1;
}

/*!  Returns a pointer to the field at position \a pos within the
  record, or 0 if it cannot be found.

*/

QSqlField* QSqlRecord::field( int i )
{
    checkDetach();
    if ( !sh->d->contains( i ) ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSqlRecord::field: index out of range: " + QString::number( i ) );
#endif
	return 0;
    }
    return &sh->d->fieldInfo( i )->field;
}

/*!  Returns a pointer to the field with name \a name within the
  record, or 0 if it cannot be found.  Field names are not
  case-sensitive.

*/

QSqlField* QSqlRecord::field( const QString& name )
{
    checkDetach();
    if ( !sh->d->contains( position( name ) ) )
	return 0;
    return &sh->d->fieldInfo( position( name ) )->field;
}


/*!  \overload

*/

const QSqlField* QSqlRecord::field( int i ) const
{
#ifdef QT_CHECK_RANGE
    if( (unsigned int) i > sh->d->count() ){
	qWarning( "QSqlRecord::field: index out of range: " + QString::number( i ) );
	return 0;
    }
#endif // QT_CHECK_RANGE
    return &sh->d->fieldInfo( i )->field;
}

/*!  \overload

*/

const QSqlField* QSqlRecord::field( const QString& name ) const
{
    if( (unsigned int) position( name ) > sh->d->count() )
	return 0;
    return &sh->d->fieldInfo( position( name ) )->field;
}

/*!  Appends a copy of the field \a field to the end of the record.

*/

void QSqlRecord::append( const QSqlField& field )
{
    checkDetach();
    sh->d->append( field );
}

/*!  Inserts a copy of \a field at position \a pos.  If a field
  already exists at \a pos, it is removed.
*/

void QSqlRecord::insert( int pos, const QSqlField& field )
{
    sh->d->insert( pos, field );
}

/*!  Removes the field at \a pos.  If \a pos does not exist, nothing
  happens.

*/

void QSqlRecord::remove( int pos )
{
    checkDetach();
    sh->d->remove( pos );
}

/*!  Removes all fields from the record.

  \sa clearValues()
*/

void QSqlRecord::clear()
{
    checkDetach();
    sh->d->clear();
}

/*!  Returns TRUE if there are no fields in the record, otherwise
  FALSE is returned.

*/

bool QSqlRecord::isEmpty() const
{
    return sh->d->isEmpty();
}


/*!  Returns TRUE if there is a field in the record with the field
  name \a name, otherwise FALSE is returned.

*/

bool QSqlRecord::contains( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( fieldName(i).upper() == name.upper() )
	    return TRUE;
    }
    return FALSE;
}

/*!  Clears the value of all fields in the record.  If \a nullify is
  TRUE, each field is set to null.

*/

void QSqlRecord::clearValues( bool nullify )
{
    checkDetach();
    for ( uint i = 0; i < count(); ++i ) {
	QVariant v;
	v.cast( field( i )->type() );
	field( i )->setValue( v );
	if ( nullify )
	    field( i )->setNull( TRUE );
    }
}

/*! Sets the generated flag for field \a name to \a generated.  If the
  field does not exist, nothing happens.

  \sa isGenerated()
*/

void QSqlRecord::setGenerated( const QString& name, bool generated )
{
    checkDetach();
    if ( !field( name ) )
	return;
    sh->d->fieldInfo( position( name ) )->nogen = !generated;
}

/*! Returns TRUE if the field \a name is to be generated (the
  default), otherwise FALSE is returned.  If the field does not exist,
  FALSE is returned.

  \sa setGenerated()
*/

bool QSqlRecord::isGenerated( const QString& name ) const
{
    if ( !field( name ) )
	return FALSE;
    return !sh->d->fieldInfo( position( name ) )->nogen;
}

/*! Sets the alignment of field \a name to \a align (which is of type
  Qt::AlignmentFlags).  If the field does not exist, nothing happens.

  \sa alignment()

*/
void QSqlRecord::setAlignment( const QString& name, int align )
{
    checkDetach();
    if ( !field( name ) )
	return;
    sh->d->fieldInfo( position( name ) )->align = align;
}

/*! Returns the alignment associated with the field \a name.  If the
   field does not exist, \c Qt::AlignLeft is returned.  If the field
   \a name has not been assigned an alignment (using setAlignment()),
   then the following rules are used:

   <ul>
   <li> If the field is a string data type, \c Qt::AlignLeft is returned.

   <li> Otherwise, \c Qt::AlignRight is returned.

   </ul>

   \sa setAlignment()
*/

int QSqlRecord::alignment( const QString& name ) const
{
    if ( !field( name ) )
	return Qt::AlignLeft;

    if ( !sh->d->fieldInfo( position( name ) ) ) {
	 int af = 0;
	 switch( field( name )->type() ) {
	 case QVariant::String:
	 case QVariant::CString:
	     af = Qt::AlignLeft;
	     break;
	 default:
	     af = Qt::AlignRight;
	     break;
	 }
	 return af;
    }
    return sh->d->fieldInfo( position( name ) )->align;
}

/*! Sets the display label of field \a name to \a label.  If the field
  does not exist, nothing happens.

  \sa displayLabel()
*/

void QSqlRecord::setDisplayLabel( const QString& name, const QString& label )
{
    checkDetach();
    if ( !field( name ) )
	return;
    sh->d->fieldInfo( position( name ) )->label = label;
}

/*! Returns the display label associated with the field \a name.  If
   the field does not exist, \a name is returned.

   \sa setDisplayLabel()
*/

QString QSqlRecord::displayLabel( const QString& name ) const
{
    if ( !field( name ) ||  !sh->d->fieldInfo( position( name ) ) )
	return name;
    QString ret = sh->d->fieldInfo( position( name ) )->label;
    if ( ret.isNull() )
	ret = name;
    return ret;
}

/*! Sets the visible flag of field \a name to \a visible.  If the
  field does not exist, nothing happens.

  \sa isVisible()
*/

void QSqlRecord::setVisible( const QString& name, bool visible )
{
    checkDetach();
    if ( !field( name ) )
	return;
    sh->d->fieldInfo( position( name ) )->visible = visible;
}

/*! Returns TRUE if the field \a name is visible (the default),
 otherwise FALSE is returned.  If the field does not exist, FALSE is
 returned.

 \sa setVisible()
*/

bool QSqlRecord::isVisible( const QString& name ) const
{
    if ( !field( name ) )
	return FALSE;
    if ( !sh->d->fieldInfo( position( name ) ) )
	return TRUE;
    return sh->d->fieldInfo( position( name ) )->visible;
}

/*!  Returns a comma-separated list of all field names as a string.
  Only generated fields are included in the list (see isGenerated() ).
  This string is suitable, for example, for generating a SQL SELECT
  statement.  If a \a prefix is specified, all fields are prefixed in
  the form:

  "\a prefix.\a fieldname"
*/

QString QSqlRecord::toString( const QString& prefix, const QString& sep ) const
{
    QString pflist;
    bool comma = FALSE;
    for ( uint i = 0; i < count(); ++i ){
	if ( isGenerated( field(i)->name() ) ) {
	    if( comma )
		pflist += sep + " ";
	    pflist += createField( i, prefix );
	    comma = TRUE;
	}
    }
    return pflist;
}

/*!

  Returns a list of all field names used in the Record.  Only
  generated fields are included in the list (see isGenerated() ). If
  \a prefix is supplied, all fields are prefixed in the form:

  "\a prefix.\a fieldname"

*/

QStringList QSqlRecord::toStringList( const QString& prefix ) const
{
    QStringList s;
    for ( uint i = 0; i < count(); ++i ) {
	if ( isGenerated( field(i)->name() ) )
	    s += createField( i, prefix );
    }
    return s;
}

/*! \internal
*/

QString QSqlRecord::createField( int i, const QString& prefix ) const
{
    QString f;
    if ( !prefix.isEmpty() )
	f = prefix + ".";
    f += field( i )->name();
    return f;
}

/*!  Returns the number of fields in the record.

*/

uint QSqlRecord::count() const
{
    return sh->d->count();
}

/*! Sets the value of the field at position \a i to \a val.  If the
  field does not exist, nothing happens.

*/

void QSqlRecord::setValue( int i, const QVariant& val )
{
    checkDetach();
    QSqlField* f = field( i );
    if ( f ) {
	f->setValue( val );
    }
}


/*!  Sets the value of field \a name to \a val.  If the field does not
  exist, nothing happens.
*/

void QSqlRecord::setValue( const QString& name, const QVariant& val )
{
    checkDetach();
    QSqlField* f = field( name );
    if ( f )
	f->setValue( val );
}

#endif
