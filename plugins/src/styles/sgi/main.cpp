#include <qstyleplugin.h>
#include <qsgistyle.h>

class SGIStyle : public QStylePlugin
{
public:
    SGIStyle();

    QStringList featureList() const;
    QStyle *create( const QString& );
};

SGIStyle::SGIStyle()
: QStylePlugin()
{
}

QStringList SGIStyle::featureList() const
{
    QStringList list;
    list << "SGI";
    return list;
}

QStyle* SGIStyle::create( const QString& s )
{
    if ( s.lower() == "sgi" )
        return new QSGIStyle();

    return 0;
}

Q_EXPORT_PLUGIN( SGIStyle )
