#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qcompactstyle.h>

class CompactStyleInterface : public QStyleInterface
{
public:
    CompactStyleInterface( QUnknownInterface *parent );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

CompactStyleInterface::CompactStyleInterface( QUnknownInterface *parent )
: QStyleInterface( parent )
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

class PlugInInterface : public QUnknownInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
: QUnknownInterface()
{
    new CompactStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
