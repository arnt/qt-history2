#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qmotifplusstyle.h>

class MotifPlusStyleInterface : public QStyleInterface
{
public:
    MotifPlusStyleInterface( QUnknownInterface *parent );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

MotifPlusStyleInterface::MotifPlusStyleInterface( QUnknownInterface *parent )
: QStyleInterface( parent )
{
}

QStringList MotifPlusStyleInterface::featureList() const
{
    QStringList list;
    list << "MotifPlus";
    return list;
}

QStyle* MotifPlusStyleInterface::create( const QString& style )
{
    if ( style == "MotifPlus" )
	return new QMotifPlusStyle();
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
    new MotifPlusStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
