#include <qinitguid.h>
#include <qstyleinterface.h>
#include <qplatinumstyle.h>

class PlatinumStyle : public QStyleInterface
{
public:
    PlatinumStyle();

    QUnknownInterface *queryInterface( const QGuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

PlatinumStyle::PlatinumStyle()
: ref( 0 )
{
}

QUnknownInterface *PlatinumStyle::queryInterface( const QGuid &guid )
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

unsigned long PlatinumStyle::addRef()
{
    return ref++;
}

unsigned long PlatinumStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList PlatinumStyle::featureList() const
{
    QStringList list;
    list << "Platinum";
    return list;
}

QStyle* PlatinumStyle::create( const QString& style )
{
    if ( style == "Platinum" )
        return new QPlatinumStyle();
    return 0;
}

Q_EXPORT_INTERFACE(PlatinumStyle)
