#include <qstyleinterface.h>
#include <qmotifstyle.h>

class MotifStyle : public QStyleInterface
{
public:
    MotifStyle();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

private:
    unsigned long ref;
};

MotifStyle::MotifStyle()
: ref( 0 )
{
}

QUnknownInterface *MotifStyle::queryInterface( const QUuid &uuid )
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

unsigned long MotifStyle::addRef()
{
    return ref++;
}

unsigned long MotifStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList MotifStyle::featureList() const
{
    QStringList list;
    list << "Motif";
    return list;
}

QStyle* MotifStyle::create( const QString& style )
{
    if ( style.lower() == "motif" )
	return new QMotifStyle();
    return 0;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( MotifStyle )
}
