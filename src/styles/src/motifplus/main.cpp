#include <qstyleinterface.h>
#include <qmotifplusstyle.h>
#include <qguardedptr.h>

class MotifPlusStyle : public QStyleInterface, public QLibraryInterface
{
public:
    MotifPlusStyle();

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

MotifPlusStyle::MotifPlusStyle()
: ref( 0 ), style( 0 )
{
}

QUnknownInterface *MotifPlusStyle::queryInterface( const QUuid &uuid )
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

unsigned long MotifPlusStyle::addRef()
{
    return ref++;
}

unsigned long MotifPlusStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList MotifPlusStyle::featureList() const
{
    QStringList list;
    list << "MotifPlus";
    return list;
}

QStyle* MotifPlusStyle::create( const QString& s )
{
    if ( s.lower() == "motifplus" )
	return style = new QMotifPlusStyle();
    return 0;
}

bool MotifPlusStyle::init()
{
    return TRUE;
}

bool MotifPlusStyle::canUnload() const
{
    return style.isNull();
}

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(QStyleInterface*)new MotifPlusStyle;
    iface->addRef();
    return iface;
}
