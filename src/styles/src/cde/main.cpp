#include <qstyleinterface.h>
#include <qcdestyle.h>

class CDEStyle : public QStyleInterface
{
public:
    CDEStyle();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

CDEStyle::CDEStyle()
: ref( 0 )
{
}

QUnknownInterface *CDEStyle::queryInterface( const QUuid &uuid )
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

unsigned long CDEStyle::addRef()
{
    return ref++;
}

unsigned long CDEStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList CDEStyle::featureList() const
{
    QStringList list;
    list << "CDE";
    return list;
}

QStyle* CDEStyle::create( const QString& style )
{
    if ( style.lower() == "cde" )
        return new QCDEStyle();
    return 0;
}

Q_EXPORT_INTERFACE(CDEStyle)
