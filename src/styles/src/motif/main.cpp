#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qmotifstyle.h>

class MotifStyleInterface : public QStyleInterface
{
public:
    MotifStyleInterface( QUnknownInterface *parent, const char *name = 0 );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

MotifStyleInterface::MotifStyleInterface( QUnknownInterface *parent, const char *name )
: QStyleInterface( parent, name )
{
}

QStringList MotifStyleInterface::featureList() const
{
    QStringList list;
    list << "Motif";
    return list;
}

QStyle* MotifStyleInterface::create( const QString& style )
{
    if ( style == "Motif" )
	return new QMotifStyle();
    return 0;
}

class PlugInInterface : public QComponentInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
{
    new MotifStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
