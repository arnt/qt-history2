#include <qstyleinterface.h>
#include <qaquastyle.h>

class AquaStyle : public QStyleInterface
{
public:
    AquaStyle();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

AquaStyle::AquaStyle()
: ref( 0 )
{
}

QUnknownInterface *AquaStyle::queryInterface( const QUuid &uuid )
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

unsigned long AquaStyle::addRef()
{
    return ref++;
}

unsigned long AquaStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList AquaStyle::featureList() const
{
    QStringList list;
    list << "Aqua";
    return list;
}

QStyle* AquaStyle::create( const QString& style )
{
    if ( style.lower() == "aqua" )
        return new QAquaStyle();
    return 0;
}

Q_EXPORT_INTERFACE(AquaStyle)
