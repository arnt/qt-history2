/*
  yyreg.h

  Header file accompanying the parser for Reginald Stadlbauer.
*/

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

