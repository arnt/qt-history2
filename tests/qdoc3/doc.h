/*
  doc.h
*/

#ifndef DOC_H
#define DOC_H

#include <qmap.h>
#include <qstringlist.h>

#include "location.h"

class Atom;
class DocPrivate;
class Molecule;
class StringSet;

class Doc
{
public:
    Doc();
    Doc( const Location& location, const QString& text,
	 const StringSet& metaCommandSet );
    Doc( const Doc& doc );
    ~Doc();

    Doc& operator=( const Doc& doc );

    void addAlso( const QString& target, const QString& text = "" );
    QStringList extractMetaCommand( const QString& command );

    const Location& location() const;
    bool isEmpty() const;
    const StringSet *metaCommands() const;
    const Molecule& molecule() const;

    static Doc propertyFunctionDoc( const Doc& propertyDoc,
				    const QString& role, const QString& param );

private:
    DocPrivate *priv;
};

#endif
