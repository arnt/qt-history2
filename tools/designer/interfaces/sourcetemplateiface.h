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

#ifndef SOURCETEMPLATEIFACE_H
#define SOURCETEMPLATEIFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <qstring.h>

// {1b3446a4-1c71-424b-8789-1f34eb5697d8}
#ifndef IID_SourceTemplate
#define IID_SourceTemplate QUuid( 0x1b3446a4, 0x1c71, 0x424b, 0x87, 0x89, 0x1f, 0x34, 0xeb, 0x56, 0x97, 0xd8 )
#endif

struct SourceTemplateInterface : public QFeatureListInterface
{
    struct Source
    {
	QString code;
	enum Type { FileName, Unnamed, Invalid } type;
	QString filename;
	QString extension;
    };
    virtual Source create( const QString &templ, QUnknownInterface *appIface ) = 0;
    virtual QString language( const QString &templ ) const = 0;

};

#endif
