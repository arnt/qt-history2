#include <qapplication.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qalgorithms.h>

class QString_LocalSort : public QString { };

static int ucstrcmp_localsort(const QString_LocalSort &as, const QString_LocalSort &bs)
{ return strcoll(as.local8Bit(),bs.local8Bit()); }

static bool operator<( const QString_LocalSort &s1, const QString_LocalSort &s2 )
{ return ucstrcmp_localsort(s1,s2) < 0; }
 
static bool operator<=( const QString_LocalSort &s1, const QString_LocalSort &s2 )
{ return ucstrcmp_localsort(s1,s2) <= 0; }
 
static bool operator>( const QString_LocalSort &s1, const QString_LocalSort &s2 )
{ return ucstrcmp_localsort(s1,s2) > 0; }
 
static bool operator>=( const QString_LocalSort &s1, const QString_LocalSort &s2 )
{ return ucstrcmp_localsort(s1,s2) >= 0; }


void collate(QStringList& list)
{
    qHeapSort( *((QValueList<QString_LocalSort> *)&list) );
}

void show(const QStringList& l)
{
    printf("( ");
    for (QStringList::ConstIterator i = l.begin(); i != l.end(); ++i) {
	printf("%s ", (*i).latin1());
    }
    printf(")\n");
}

main(int argc, char** argv)
{
    QApplication app(argc,argv);

    QStringList list;
    list << "Australien" << "Berlin" << "Ägypten";

    list.sort();
    show(list);

    collate(list);
    show(list);
}
