/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <filterinterface.h>
#include <qapplication.h>

#include "glade2ui.h"

class GladeFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    GladeFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;

};

GladeFilter::GladeFilter()
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
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList GladeFilter::featureList() const
{
    QStringList list;
    list << "Glade Files (*.glade)" ;
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( GladeFilter )
}
