/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef YYREG_H
#define YYREG_H

#include <qstringlist.h>
#include <qlist.h>

class CppFunction
{
public:
    CppFunction() : cnst( FALSE ), lineno1( 0 ), lineno2( 0 ) { }

    void setReturnType( const QString& r ) { ret = r; }
    void setScopedName( const QString& n ) { nam = n; }
    void setParameterList( const QStringList& p ) { params = p; }
    void setConst( bool c ) { cnst = c; }
    void setBody( const QString& b ) { bod = b; }
    void setDocumentation( const QString& d ) { doc = d; }
    void setLineNums( int functionStart, int openingBrace, int closingBrace ) {
	lineno0 = functionStart;
	lineno1 = openingBrace;
	lineno2 = closingBrace;
    }

    const QString& returnType() const { return ret; }
    const QString& scopedName() const { return nam; }
    const QStringList& parameterList() const { return params; }
    bool isConst() const { return cnst; }
    QString prototype() const;
    const QString& body() const { return bod; }
    const QString& documentation() const { return doc; }
    int functionStartLineNum() const { return lineno0; }
    int openingBraceLineNum() const { return lineno1; }
    int closingBraceLineNum() const { return lineno2; }

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const CppFunction& ) const { return FALSE; }
#endif

private:
    QString ret;
    QString nam;
    QStringList params;
    bool cnst;
    QString bod;
    QString doc;
    int lineno0;
    int lineno1;
    int lineno2;
};

void extractCppFunctions( const QString& code, QList<CppFunction> *flist );
QString canonicalCppProto( const QString& proto );

#endif
