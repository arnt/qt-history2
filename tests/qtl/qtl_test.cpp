#include "qtl_test.h"
#include <qalgorithms.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qstring.h>

#include <qfile.h>
#include <stdio.h>

class MyClass
{
public:
  MyClass() { ++count; }
  MyClass( const QString& c) { count++; str = c; }
  ~MyClass() { count--; }
  MyClass( const MyClass& c ) { count++; str = c.str; }

  QString str;
  static int count;
};

int MyClass::count = 0;

typedef QMap<QString,MyClass> Group;
typedef QMap<QString,Group> Config;

class QConfig
{
public:
  QConfig() { }

  Group& group( const QString& g ) { return config[ g ]; }
  const Group& cgroup( const QString& g ) const { return config[ g ]; }
  void insert( const QString& n, const Group& g ) { config.insert( n, g ); }
  void insert( const QString& g, const QString& n, const MyClass& v ) { config[g].insert(n,v); }
  void remove( const QString&g ) { config.remove( g ); }

private:
  Config config;
};

int main(int, char** )
{
  QTextStream cout( stdout, IO_WriteOnly );


  {
    typedef QMap<QString,MyClass> Map;
    Map map;
    Map map2( map );
    Q_ASSERT( map.count() == 0 );
    Q_ASSERT( map2.count() == 0 );
    Q_ASSERT( MyClass::count == 1 );
    cout << "============ " << MyClass::count << endl;

    map2["Hallo"] = MyClass("Fritz");
    Q_ASSERT( map.count() == 0 );
    Q_ASSERT( map2.count() == 1 );
    Q_ASSERT( MyClass::count == 3 );
    cout << "============ " << MyClass::count << endl;
  }
  Q_ASSERT( MyClass::count == 0 );

  {
    typedef QMap<QString,MyClass> Map;
    Map map;
    map.insert( "Torben", MyClass("Weis") );
    map.insert( "Claudia", MyClass("Sorg" ) );
    map.insert( "Lars", MyClass("Linzbach" ) );
    map.insert( "Matthias", MyClass("Ettrich" ) );
    map.insert( "Sue", MyClass("Paludo" ) );
    map.insert( "Eirik", MyClass("Eng" ) );
    map.insert( "Haavard", MyClass("Nord" ) );
    map.insert( "Arnt", MyClass("Gulbrandsen" ) );
    map.insert( "Paul", MyClass("Tvete" ) );
    map.insert( "Paul", MyClass("Tvete 1" ) );
    map.insert( "Paul", MyClass("Tvete 2" ) );
    map.insert( "Paul", MyClass("Tvete 3" ) );
    map.insert( "Paul", MyClass("Tvete 4" ) );
    map.insert( "Paul", MyClass("Tvete 5" ) );
    map.insert( "Paul", MyClass("Tvete 6" ) );

    Map::Iterator it = map.begin();
    for( ; it != map.end(); ++it )
      cout << "Key=" << it.key() << "  Data=" << it.data().str << endl;
    
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 9 );
    Q_ASSERT( MyClass::count == 10 );

    Map map2( map );
    Q_ASSERT( map2.count() == 9 );
    Q_ASSERT( MyClass::count == 10 );
    cout << "============ " << MyClass::count << endl;

    map2.insert( "Kay", MyClass("Roemer") );
    Q_ASSERT( map2.count() == 10 );
    Q_ASSERT( MyClass::count == 21 );
    cout << "============ " << MyClass::count << endl;

    map2 = map;
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 9 );
    Q_ASSERT( map2.count() == 9 );
    Q_ASSERT( MyClass::count == 10 );

    map2.insert( "Kay", MyClass("Roemer") );
    Q_ASSERT( map2.count() == 10 );
    Q_ASSERT( MyClass::count == 21 );
    cout << "============ " << MyClass::count << endl;

    map2.clear();
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 9 );
    Q_ASSERT( map2.count() == 0 );
    Q_ASSERT( MyClass::count == 11 );

    it = map.begin();
    for( ; it != map.end(); ++it )
      cout << "Key=" << it.key() << "  Data=" << it.data().str << endl;

    map2 = map;
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 9 );
    Q_ASSERT( map2.count() == 9 );
    Q_ASSERT( MyClass::count == 10 );

    map2.clear();
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 9 );
    Q_ASSERT( map2.count() == 0 );
    Q_ASSERT( MyClass::count == 11 );

    map.remove( "Lars" );
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 8 );
    Q_ASSERT( map2.count() == 0 );
    Q_ASSERT( MyClass::count == 10 );

    map.remove( "Mist" );
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( map.count() == 8 );
    Q_ASSERT( map2.count() == 0 );
    Q_ASSERT( MyClass::count == 10 );
  }
  Q_ASSERT( MyClass::count == 0 );

  {
    typedef QValueList<MyClass> List;
    List list;
    list.append( MyClass("Weis") );
    list.append( MyClass("Sorg" ) );
    list.append( MyClass("Linzbach" ) );
    list.append( MyClass("Ettrich" ) );
    list.append( MyClass("Paludo" ) );
    list.append( MyClass("Eng" ) );
    list.append( MyClass("Nord" ) );
    list.append( MyClass("Gulbrandsen" ) );
    list.append( MyClass("Tvete" ) );
    list.append( MyClass("Tvete 1" ) );
    list.append( MyClass("Tvete 2" ) );
    list.append( MyClass("Tvete 3" ) );
    list.append( MyClass("Tvete 4" ) );
    list.append( MyClass("Tvete 5" ) );
    list.append( MyClass("Tvete 6" ) );

    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( MyClass::count == 16 );

    List l2( list );
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( l2.count() == 15 );
    Q_ASSERT( MyClass::count == 16 );

    l2.append( MyClass("Holger") );
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( l2.count() == 16 );
    Q_ASSERT( MyClass::count == 33 );

    l2.clear();
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( l2.count() == 0 );
    Q_ASSERT( MyClass::count == 17 );

    l2 = list;
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( l2.count() == 15 );
    Q_ASSERT( MyClass::count == 16 );

    List::Iterator it = l2.begin();
    for( ; it != l2.end(); ++it )
      cout << (*it).str << endl;

    l2 += list;
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( l2.count() == 30 );
    Q_ASSERT( MyClass::count == 47 );

    List l3 = l2 + list;
    cout << "============ " << MyClass::count << endl;
    Q_ASSERT( list.count() == 15 );
    Q_ASSERT( l2.count() == 30 );
    Q_ASSERT( l3.count() == 45 );
    Q_ASSERT( MyClass::count == 93 );

  }
  Q_ASSERT( MyClass::count == 0 );

  {
    QConfig cfg;
    Q_ASSERT( MyClass::count == 1 );
    {
      Group g;
      g.insert( "Torben", MyClass("Weis") );
      g.insert( "Claudia", MyClass("Sorg" ) );
      g.insert( "Lars", MyClass("Linzbach" ) );
      g.insert( "Matthias", MyClass("Ettrich" ) );
      g.insert( "Sue", MyClass("Paludo" ) );
      g.insert( "Eirik", MyClass("Eng" ) );
      g.insert( "Haavard", MyClass("Nord" ) );
      g.insert( "Arnt", MyClass("Gulbrandsen" ) );
      Q_ASSERT( MyClass::count == 10 );
      cout << "------------" << MyClass::count << endl;

      cfg.insert( "Gruppe", g );
      cout << "------------" << MyClass::count << endl;
      Q_ASSERT( MyClass::count == 10 );

      g.insert( "Lady", MyClass("Gulbrandsen" ) );
      cout << "------------" << MyClass::count << endl;
      Q_ASSERT( MyClass::count == 20 );
    }
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 10 );
    
    Group g2( cfg.group( "Gruppe" ) );
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 10 );

    Group::Iterator it = g2.begin();
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 19 );

    for( ; it != g2.end(); ++it )
      cout << "Key=" << it.key() << "  Data=" << it.data().str << endl;

    cfg.remove("Gruppe");
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 10 );

    Group g3( cfg.cgroup( "GruppeNixGibts" ) );
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 10 );

    g3["Hallo"] = MyClass("Fritz");
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 12 );
  }
  Q_ASSERT( MyClass::count == 0 );

  {
    typedef QMap<QString,MyClass> Map;
    Map map;
    map["Torben"] = MyClass("Weis");
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 2 );
    Q_ASSERT( map.count() == 1 );

    cout << map["Torben"].str << endl << '"' << map["Lars"].str << endl;
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 3 );
    Q_ASSERT( map.count() == 2 );

    Map::Iterator it = map.begin();
    for( ; it != map.end(); ++it )
      cout << "Key=" << it.key() << "  Data=" << it.data().str << endl;

    const Map& cmap = map;
    cout << '"' << cmap["Depp"].str << '"' << endl;
    cout << "------------" << MyClass::count << endl;
    Q_ASSERT( MyClass::count == 3 );
    Q_ASSERT( map.count() == 2 );
    Q_ASSERT( cmap.count() == 2 );
  }
  Q_ASSERT( MyClass::count == 0 );

  return 0;
}
