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
#include <qstring.h>
#include <qdir.h>

#include "kdevdlg2ui.h"

class KDevDlgFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    KDevDlgFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;
};

KDevDlgFilter::KDevDlgFilter()
{
}

QRESULT KDevDlgFilter::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList KDevDlgFilter::featureList() const
{
    QStringList list;
    list << "KDevelop Dialog Files (*.kdevdlg)" ;
    return list;
}

QStringList KDevDlgFilter::import( const QString &, const QString& filename )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	qWarning( "uic: Could not open file '%s' ", filename.latin1() );
    QTextStream in;
    in.setDevice( &file );

    QString name = filename.right( filename.length() - filename.findRev( QDir::separator() ) - 1 ).section( ".", 0, 0 );
    KDEVDLG2UI c( &in, name );
    QStringList files;
    c.parse();
    return c.targetFiles;
}

bool KDevDlgFilter::init()
{
    return TRUE;
}

void KDevDlgFilter::cleanup()
{
}

bool KDevDlgFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( KDevDlgFilter )
}
