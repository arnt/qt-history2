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
    // the order is important
    enum SectioningUnit { Book = -2, Part, Chapter, Section1, Section2,
			  Section3, Section4 };

    Doc();
    Doc( const Location& location, const QString& text,
	 const StringSet& metaCommandSet );
    Doc( const Doc& doc );
    ~Doc();

    Doc& operator=( const Doc& doc );

    // ### no non-const
    void addAlso( const Molecule& also );
    QStringList extractMetaCommand( const QString& command );

    const Location& location() const;
    bool isEmpty() const;
    const StringSet *metaCommands() const;
    const Molecule& body() const;
    const QString& baseName() const;
    SectioningUnit granularity() const;
    SectioningUnit sectioningUnit() const;
    const QValueList<Molecule> *alsoList() const;

    static Doc propertyFunctionDoc( const Doc& propertyDoc,
				    const QString& role, const QString& param );

private:
    DocPrivate *priv;
};

#endif
