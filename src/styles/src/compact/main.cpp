#define Q_GUIDIMPL
#include <qstyleinterface.h>
#include <qcompactstyle.h>

class CompactStyle : public QStyleInterface
{
public:
    CompactStyle();

    QUnknownInterface *queryInterface( const QGuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

CompactStyle::CompactStyle()
: ref( 0 )
{
}

QUnknownInterface *CompactStyle::queryInterface( const QGuid &guid )
{
    QUnknownInterface *iface = 0;
    if ( guid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( guid == IID_QStyleInterface )
	iface = (QStyleInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long CompactStyle::addRef()
{
    return ref++;
}

unsigned long CompactStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList CompactStyle::featureList() const
{
    QStringList list;
    list << "Compact";
    return list;
}

QStyle* CompactStyle::create( const QString& style )
{
    if ( style.lower() == "compact" )
        return new QCompactStyle();
    return 0;
}

Q_EXPORT_INTERFACE(CompactStyle)
