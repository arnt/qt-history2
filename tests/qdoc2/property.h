/*
  property.h
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#include <qstring.h>

/*
  The Property class represents a Qt property.
*/
class Property
{
public:
    Property();
    Property( const QString& type, const QString& name );
    Property( const Property& p );

    Property& operator=( const Property& p );

    void setType( const QString& type ) { t = type; }
    void setName( const QString& name ) { n = name; }
    void setReadFunction( const QString& getter ) { read = getter; }
    void setWriteFunction( const QString& setter ) { write = setter; }
    void setStored( bool stored ) { store = toTrool( stored ); }
    void setDesignable( bool designable ) { design = toTrool( designable ); }
    void setResetFunction( const QString& resetter ) { reset = resetter; }

    const QString& type() const { return t; }
    const QString& name() const { return n; }
    const QString& readFunction() const { return read; }
    const QString& writeFunction() const { return write; }
    bool stored() const { return fromTrool( store, storedDefault() ); }
    bool storedDefault() const { return true; }
    bool designable() const { return fromTrool( design, designableDefault() ); }
    bool designableDefault() const { return !read.isEmpty(); }
    const QString& resetFunction() const { return reset; }

private:
    /*
      A Trool is a bit like a bool, except that it admits three truth values.
    */
    enum Trool { Ttrue, Tfalse, Tdef };

    static Trool toTrool( bool b );
    static bool fromTrool( Trool tr, bool def );

    QString t;
    QString n;
    QString read;
    QString write;
    Trool store;
    Trool design;
    QString reset;
};

#endif
