/*
  doc.h
*/

#ifndef DOC_H
#define DOC_H

#include <qstring.h>

#include "location.h"
#include "set.h"

class Config;
class DocPrivate;
class Text;
class Atom;

class Doc
{
public:
    // the order is important
    enum SectioningUnit { Book = -2, Part, Chapter, Section1, Section2, Section3, Section4 };

    Doc() : priv(0) {}
    Doc(const Location &loc, const QString &source, const Set<QString> &metaCommandSet);
    Doc(const Doc &doc);
    ~Doc();

    Doc& operator=( const Doc& doc );

    const Location& location() const;
    bool isEmpty() const;
    const QString& source() const;
    const Text& body() const;
    Text briefText() const;
    Text trimmedBriefText(const QString &className) const;
    Text legaleseText() const;
    const QString& baseName() const;
    SectioningUnit granularity() const;
    const Set<QString> &parameterNames() const;
    const Set<QString> &enumItemNames() const;
    const Set<QString> &omitEnumItemNames() const;
    const Set<QString> &metaCommandsUsed() const;
    QStringList metaCommandArgs( const QString& metaCommand ) const;
    const QList<Text> &alsoList() const;
    bool hasTableOfContents() const;
    const QList<Atom *> &tableOfContents() const;

    static void initialize( const Config& config );
    static void terminate();
    static QString alias( const QString& english );
    static void trimCStyleComment( Location& location, QString& str );

private:
    DocPrivate *priv;
};

#endif
