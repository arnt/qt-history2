#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qsgistyle.h>

class SGIStyleInterface : public QStyleInterface
{
public:
    SGIStyleInterface( QUnknownInterface *parent );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

SGIStyleInterface::SGIStyleInterface( QUnknownInterface *parent )
: QStyleInterface( parent)
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

class PlugInInterface : public QUnknownInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
: QUnknownInterface()
{
    new SGIStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
