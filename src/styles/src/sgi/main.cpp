#define Q_GUIDIMPL
#include <qstyleinterface.h>
#include <qsgistyle.h>

class SGIStyle : public QStyleInterface
{
public:
    SGIStyle();

    QUnknownInterface *queryInterface( const QGuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

SGIStyle::SGIStyle()
: ref( 0 )
{
}

QUnknownInterface *SGIStyle::queryInterface( const QGuid &guid )
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

unsigned long SGIStyle::addRef()
{
    return ref++;
}

unsigned long SGIStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList SGIStyle::featureList() const
{
    QStringList list;
    list << "SGI";
    return list;
}

QStyle* SGIStyle::create( const QString& style )
{
    if ( style == "SGI" )
        return new QSGIStyle();
    return 0;
}

Q_EXPORT_INTERFACE(SGIStyle)
