#include <qaquastyle.h>
#include <qstyleplugin.h>

class AquaStyle : public QStylePlugin
{
public:
    AquaStyle();

    QStringList keys() const;
    QStyle *create( const QString& );
};

AquaStyle::AquaStyle()
: QStylePlugin()
{
}

QStringList AquaStyle::keys() const
{
    QStringList list;
    list << "Aqua";
    return list;
}

QStyle* AquaStyle::create( const QString& s )
{
    if ( s.lower() == "aqua" )
        return new QAquaStyle();

    return 0;
}

Q_EXPORT_PLUGIN( AquaStyle )
