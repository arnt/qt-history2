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

#include "qcoreapplication.h"
#include <private/qcore_mac_p.h>

/*****************************************************************************
  QCoreApplication utility functions
 *****************************************************************************/
QString qAppName()                                // get application name
{
    static QString appName;
    QCoreApplication *app;
    if (appName.isEmpty() && (app = QCoreApplication::instance())) {
        char *p = strrchr(app->argv()[0], '/');
        appName = p ? QString::fromUtf8(p + 1) : QString::fromUtf8(app->argv()[0]);
    }
    return appName;
}

QString qAppFileName()
{
    static QString appFileName;
    if (appFileName.isEmpty()) {
        QCFType<CFURLRef> bundleURL(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
        QCFString cfPath(CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle));
        appFileName = cfPath;
    }
    return appFileName;
}

