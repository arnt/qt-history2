/*
  main.cpp
*/

#include <filterinterface.h>
#include <qapplication.h>

#include "dlg2ui.h"

class DlgFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    DlgFilter();

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

DlgFilter::DlgFilter()
: ref( 0 )
{
}

QRESULT DlgFilter::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

unsigned long DlgFilter::addRef()
{
    return ref++;
}

unsigned long DlgFilter::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QStringList DlgFilter::featureList() const
{
    QStringList list;
    list << "Qt Architect 2.1+ Dialog Files (*.dlg)" ;
    return list;
}

QStringList DlgFilter::import( const QString &, const QString& filename )
{
    Dlg2Ui d;
    return d.convertQtArchitectDlgFile( filename );
}

bool DlgFilter::init()
{
    return TRUE;
}

void DlgFilter::cleanup()
{
}

bool DlgFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( DlgFilter )
}
