/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <filterinterface.h>
#include <qapplication.h>

#include "dlg2ui.h"

class DlgFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    DlgFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;
};

DlgFilter::DlgFilter()
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
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList DlgFilter::featureList() const
{
    QStringList list;
    list << "Qt Architect Dialog Files (*.dlg)" ;
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( DlgFilter )
}
