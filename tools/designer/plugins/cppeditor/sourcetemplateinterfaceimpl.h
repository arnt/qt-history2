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

#ifndef SOURCETEMPLATEINTERFACEIMPL_H
#define SOURCETEMPLATEINTERFACEIMPL_H

#include <sourcetemplateiface.h>

class SourceTemplateInterfaceImpl : public SourceTemplateInterface
{
public:
    SourceTemplateInterfaceImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    QStringList featureList() const;
    Source create( const QString &templ, QUnknownInterface *appIface );
    QString language( const QString &templ ) const;

    Q_REFCOUNT
};


#endif
