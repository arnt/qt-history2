/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "resourcemimedata_p.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtGui/QIcon>

namespace qdesigner_internal {
ResourceMimeData::ResourceMimeData(Type t) :
    m_type(t)
{
}

QStringList ResourceMimeData::formats() const
{
    return QStringList(QLatin1String("resourceditor/resource"));
}

// Convenience to return an icon normalized to form directory
QIcon ResourceMimeData::icon(QDesignerFormWindowInterface *fw) const
{
    if (m_type != Image)
        return QIcon();

    const QString normalizedQrcPath = fw->absoluteDir().absoluteFilePath(m_qrcPath);
    const QIcon rc =  fw->core()->iconCache()->nameToIcon(m_filePath, normalizedQrcPath);
    return rc;
}
}
