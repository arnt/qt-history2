/*
  main.cpp
*/

#include <filterinterface.h>
#include <qapplication.h>

#include "glade2ui.h"

class GladeFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    GladeFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    unsigned long ref;
};

GladeFilter::GladeFilter()
: ref( 0 )
{
}

QRESULT GladeFilter::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ImportFilterInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ImportFilter )
	*iface = (ImportFilterInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;

    if ( *iface )
	(*iface)->addRef();
}

unsigned long GladeFilter::addRef()
{
    return ref++;
}

unsigned long GladeFilter::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QStringList GladeFilter::featureList() const
{
    QStringList list;
    list << "GNOME Glade Files (*.glade)" ;
    return list;
}

QStringList GladeFilter::import( const QString &, const QString& filename )
{
    Glade2Ui g;
    return g.convertGladeFile( filename );
}

bool GladeFilter::init()
{
    return TRUE;
}

void GladeFilter::cleanup()
{
}

bool GladeFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( GladeFilter )
}
