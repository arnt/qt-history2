/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef YYREG_H
#define YYREG_H

#include <qstring.h>
#include <qvaluelist.h>

class CppFunction
{
public:
    CppFunction() : cnst( FALSE ) { }
    CppFunction( const CppFunction& f );

    CppFunction& operator=( const CppFunction& f );

    void setReturnType( const QString& r ) { ret = r; }
    void setScopedName( const QString& n ) { nam = n; }
    void setParameterList( const QStringList& p ) { params = p; }
    void setConst( bool c ) { cnst = c; }
    void setBody( const QString& b ) { bod = b; }
    void setDocumentation( const QString& d ) { doc = d; }

    const QString& returnType() const { return ret; }
    const QString& scopedName() const { return nam; }
    const QStringList& parameterList() const { return params; }
    bool isConst() const { return cnst; }
    QString prototype() const;
    const QString& body() const { return bod; }
    const QString& documentation() const { return doc; }

private:
    QString ret;
    QString nam;
    QStringList params;
    bool cnst;
    QString bod;
    QString doc;
};

void extractCppFunctions( const QString& code, QValueList<CppFunction> *flist );
QString canonicalCppProto( const QString& proto );

#endif
