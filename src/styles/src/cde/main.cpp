#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qcdestyle.h>

class CDEStyleInterface : public QStyleInterface
{
public:
    CDEStyleInterface( QUnknownInterface *parent );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

CDEStyleInterface::CDEStyleInterface( QUnknownInterface *parent )
: QStyleInterface( parent )
{
}

QStringList CDEStyleInterface::featureList() const
{
    QStringList list;
    list << "CDE";
    return list;
}

QStyle* CDEStyleInterface::create( const QString& style )
{
    if ( style == "CDE" )
	return new QCDEStyle();
    return 0;
}

class PlugInInterface : public QUnknownInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
: QUnknownInterface()
{
    new CDEStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
