/*
  yyvolk.h

  Header file accompanying the parser for Volker Hilsheimer.
*/

#ifndef CPPPARSER_H
#define CPPPARSER_H

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

class Interface
{
public:
    Interface() { }
    Interface( const Interface& i )
	: nam( i.nam ), uu( i.uu ), funcs( i.funcs ) { }

    Interface& operator=( const Interface& i ) {
	nam = i.nam;
	uu = i.uu;
	funcs = i.funcs;
	return *this;
    }

    void setName( const QString& n ) { nam = n; }
    void setUuid( const QString& u ) { uu = u; }
    void setFunctionList( const QStringList& f ) { funcs = f; }

    const QString& name() const { return nam; }
    const QString& uuid() const { return uu; }
    const QStringList& functionList() const { return funcs; }

private:
    QString nam;
    QString uu;
    QStringList funcs;
};

void parseHeaderFile( const QString& name,
		      QMap<QString, Interface> *interfaceMap );

#endif //CPPPARSER_H
