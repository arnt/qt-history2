#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qcompactstyle.h>

class CompactStyleInterface : public QStyleInterface
{
public:
    CompactStyleInterface( QUnknownInterface *parent, const char *name = 0 );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

CompactStyleInterface::CompactStyleInterface( QUnknownInterface *parent, const char *name )
: QStyleInterface( parent, name )
{
}

QStringList CompactStyleInterface::featureList() const
{
    QStringList list;
    list << "Compact";
    return list;
}

QStyle* CompactStyleInterface::create( const QString& style )
{
    if ( style == "Compact" )
	return new QCompactStyle();
    return 0;
}

class PlugInInterface : public QComponentInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
{
    new CompactStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
