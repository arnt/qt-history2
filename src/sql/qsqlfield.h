/****************************************************************************
**
** Definition of QSqlField class
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

#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlFieldPrivate;

class Q_EXPORT QSqlField
{
public:
    QSqlField( const QString& fieldName = QString::null, QVariant::Type type = QVariant::Invalid, const QString& displayLabel = QString::null );
    QSqlField( const QSqlField& other );
    QSqlField& operator=( const QSqlField& other );
    bool operator==(const QSqlField& other) const;
    ~QSqlField();

    QVariant           value() const;
    void               setValue( const QVariant& value );
    void               clear();

    void               setName( const QString& name ) { nm = name; }
    QString            name() const { return nm; }
    QVariant::Type     type() const { return val.type(); }

    void               setDisplayLabel( const QString& l ) { label = l; }
    QString            displayLabel() const { return label; }
    void               setReadOnly( bool readOnly ) { ro = readOnly; }
    bool               isReadOnly() const { return ro; }
    void               setNull( bool n ) { nul = n; }
    bool               isNull() const { return nul; }
    void               setPrimaryIndex( bool primaryIndex ) { pIdx = primaryIndex; }
    bool               isPrimaryIndex() const { return pIdx; }
    void               setVisible( bool visible ) { iv = visible; }
    bool               isVisible() const { return iv; }
    void               setCalculated( bool calculated ) { cf = calculated; }
    bool               isCalculated() const { return cf; }
    void               setAlignment( int align ) { af = align; }
    int                alignment() const { return af; }

private:
    QString       nm;
    QVariant      val;
    QString       label;
    bool          ro;
    bool          nul;
    bool          pIdx;
    bool          iv;
    bool          cf;
    int           af;
    QSqlFieldPrivate* d;
};

#endif	// QT_NO_SQL
#endif
