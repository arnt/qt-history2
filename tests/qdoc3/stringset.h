/*
  stringset.h
*/

#ifndef STRINGSET_H
#define STRINGSET_H

#include <qmap.h>
#include <qstringlist.h>

/*
  The StringSet class is a set of QString objects.

  Apart from the set methods such as reunion(), intersection(), and
  difference(), StringSet objects can easily be used as a QStringList by calling
  stringList().
*/
class StringSet
{
    friend StringSet reunion( const StringSet& ss, const StringSet& tt );
    friend StringSet intersection( const StringSet& ss, const StringSet& tt );
    friend StringSet difference( const StringSet& ss, const StringSet& tt );

public:
    typedef QStringList::ConstIterator ConstIterator;

    StringSet();
    StringSet( const StringSet& ss );
    StringSet( const QStringList& sl );

    StringSet& operator=( const StringSet& ss );

    void clear();
    void insert( const QString& str );
    void remove( const QString& str );

    StringSet& operator<<( const QString& str );

    bool isEmpty() const { return map.isEmpty(); }
    uint count() const { return map.count(); }
    bool contains( const QString& str ) const;
    const QStringList& stringList() const;
    QStringList toIStringList() const;
    ConstIterator begin() const { return stringList().begin(); }
    ConstIterator end() const { return stringList().end(); }
    const QString& first() const { return stringList().first(); }
    const QString& last() const { return stringList().last(); }

private:
    QMap<QString, void *> map;
    QStringList stringl;
    bool dirty;
};

#endif
