#include <qstyleinterface.h>
#include <qaquastyle.h>
#include <qguardedptr.h>

class AquaStyle : public QStyleInterface, public QLibraryInterface
{
public:
    AquaStyle();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QGuardedPtr<QStyle> style;

    unsigned long ref;
};

AquaStyle::AquaStyle()
: ref( 0 ), style( 0 )
{
}

QUnknownInterface *AquaStyle::queryInterface( const QUuid &uuid )
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

QStyle* AquaStyle::create( const QString& s )
{
    if ( s.lower() == "aqua" )
        return style = new QAquaStyle();
    return 0;
}

bool AquaStyle::init()
{
    return TRUE;
}

void AquaStyle::cleanup()
{
    delete style;
}

bool AquaStyle::canUnload() const
{
    return style.isNull();
}

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(QStyleInterface*)new AquaStyle;
    iface->addRef();
    return iface;
}
