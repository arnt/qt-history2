#include <qstyleinterface.h>
#include <qplatinumstyle.h>
#include <qguardedptr.h>

class PlatinumStyle : public QStyleInterface, public QLibraryInterface
{
public:
    PlatinumStyle();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

    bool init();
    bool canUnload() const;

private:
    QGuardedPtr<QStyle> style;

    unsigned long ref;
};

PlatinumStyle::PlatinumStyle()
: ref( 0 ), style( 0 )
{
}

QUnknownInterface *PlatinumStyle::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)(QStyleInterface*)this;
    else if ( uuid == IID_QStyleInterface )
	iface = (QStyleInterface*)this;
    else if ( uuid == IID_QLibraryInterface )
	iface = (QLibraryInterface*)this;

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

QStyle* PlatinumStyle::create( const QString& s )
{
    if ( s.lower() == "platinum" )
        return style = new QPlatinumStyle();
    return 0;
}

bool PlatinumStyle::init()
{
    return TRUE;
}

bool PlatinumStyle::canUnload() const
{
    return style.isNull();
}

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(QStyleInterface*)new PlatinumStyle;
    iface->addRef();
    return iface;
}
