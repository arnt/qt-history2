#define Q_UUIDIMPL
#include <qstyleinterface.h>
#include <qmotifplusstyle.h>

class MotifPlusStyle : public QStyleInterface
{
public:
    MotifPlusStyle();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

MotifPlusStyle::MotifPlusStyle()
: ref( 0 )
{
}

QUnknownInterface *MotifPlusStyle::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QStyleInterface )
	iface = (QStyleInterface*)this;

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

QStyle* MotifPlusStyle::create( const QString& style )
{
    if ( style.lower() == "motifplus" )
	return new QMotifPlusStyle();
    return 0;
}

Q_EXPORT_INTERFACE(MotifPlusStyle)
