#include "iconloader.h"

static const char *fileopen[] = {
"    16    13        5            1",
". c #040404",
"# c #808304",
"a c None",
"b c #f3f704",
"c c #f3f7f3",
"aaaaaaaaa...aaaa",
"aaaaaaaa.aaa.a.a",
"aaaaaaaaaaaaa..a",
"a...aaaaaaaa...a",
".bcb.......aaaaa",
".cbcbcbcbc.aaaaa",
".bcbcbcbcb.aaaaa",
".cbcb...........",
".bcb.#########.a",
".cb.#########.aa",
".b.#########.aaa",
"..#########.aaaa",
"...........aaaaa"
};


IconLoader::IconLoader()
{
}
    
QIconSet IconLoader::iconSet( const QString &/*icon*/ )
{
    return QIconSet( QPixmap( fileopen ) );
}

QPixmap IconLoader::pixmap( const QString &/*pixmap*/ )
{
    return QPixmap( fileopen );
}
