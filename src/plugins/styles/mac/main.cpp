#include <qmacstyle_mac.h>
#include <qstyleplugin.h>

class MacStyle : public QStylePlugin
{
public:
    MacStyle();

    QStringList keys() const;
    QStyle *create( const QString& );
};

MacStyle::MacStyle() : QStylePlugin()
{
}

QStringList MacStyle::keys() const
{
    QString mstyle = "Macintosh";
    if(Collection c=NewCollection()) {
	GetTheme(c);
	Str255 str;
	long int s = 256;
	if(!GetCollectionItem(c, kThemeNameTag, 0, &s, &str)) 
	    mstyle += " (" + p2qstring(str) + ")";
    }
    if ( !list.contains( mstyle ) )
	list << mstyle;

    QStringList list;
    list << mstyle;
    return list;
}

QStyle* AquaStyle::create( const QString& s )
{
    if ( s.lower().left(9) == "macintosh" )
        return new QMacStyle();

    return 0;
}

Q_EXPORT_PLUGIN( MacStyle )
