#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qaquastyle.h>

class AquaStyleInterface : public QStyleInterface
{
public:
    AquaStyleInterface( QUnknownInterface *parent );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

AquaStyleInterface::AquaStyleInterface( QUnknownInterface *parent )
: QStyleInterface( parent )
{
}

QStringList AquaStyleInterface::featureList() const
{
    QStringList list;
    list << "Aqua";
    return list;
}

QStyle* AquaStyleInterface::create( const QString& style )
{
    if ( style == "aqua" )
        return new QAquaStyle();
    return 0;
}

class PlugInInterface : public QUnknownInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
{
    new AquaStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
