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
    Atom *createAlsoAtomList() const;
    const Atom *atomList() const;

private:
    DocPrivate *priv;
};

#endif
