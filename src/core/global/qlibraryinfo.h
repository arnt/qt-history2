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

#ifndef __QLIBRARYINFO_H__
#define __QLIBRARYINFO_H__

#include <QtCore/qstring.h>

class Q_CORE_EXPORT QLibraryInfo
{
public:
    static QString licensee();
    static QString licensedProducts();

    static QString buildKey();

    enum LibraryLocation
    {
        PrefixPath, 
        DocumentationPath, 
        HeadersPath, 
        LibrariesPath, 
        BinariesPath, 
        PluginsPath,
        DataPath,
        TranslationsPath,
        SettingsPath
    };
    static QString location(LibraryLocation);

    static QString configuration();

private:
    QLibraryInfo();
};


#endif /* __QLIBRARYINFO_H__ */
