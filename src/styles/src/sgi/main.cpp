#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qsgistyle.h>

class SGIStyleInterface : public QStyleInterface
{
public:
    SGIStyleInterface( QUnknownInterface *parent, const char *name = 0 );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

SGIStyleInterface::SGIStyleInterface( QUnknownInterface *parent, const char *name )
: QStyleInterface( parent, name )
{
}

QStringList SGIStyleInterface::featureList() const
{
    QStringList list;
    list << "SGI";
    return list;
}

QStyle* SGIStyleInterface::create( const QString& style )
{
    if ( style == "SGI" )
	return new QSGIStyle();
    return 0;
}

class PlugInInterface : public QComponentInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
{
    new SGIStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
