#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qplatinumstyle.h>

class PlatinumStyleInterface : public QStyleInterface
{
public:
    PlatinumStyleInterface( QUnknownInterface *parent, const char *name = 0 );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

PlatinumStyleInterface::PlatinumStyleInterface( QUnknownInterface *parent, const char *name )
: QStyleInterface( parent, name )
{
}

QStringList PlatinumStyleInterface::featureList() const
{
    QStringList list;
    list << "Platinum";
    return list;
}

QStyle* PlatinumStyleInterface::create( const QString& style )
{
    if ( style == "Platinum" )
	return new QPlatinumStyle();
    return 0;
}

class PlugInInterface : public QComponentInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
{
    new PlatinumStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
