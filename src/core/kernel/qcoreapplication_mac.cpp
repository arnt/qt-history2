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
  Internal variables and functions
 *****************************************************************************/
static char    *appName = 0;                        // application name
static char *appFileName = 0;                       // application file name

/*****************************************************************************
  QCoreApplication utility functions
 *****************************************************************************/
const char *qAppName()                                // get application name
{
    if(!appName) {
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr) {
            QCFString cfstr;
            CopyProcessName(&psn, &cfstr);
            appName = strdup(static_cast<QString>(cfstr).latin1());
        } else if(QCoreApplication *app = QCoreApplication::instance()) {
            char *p = strrchr(app->argv()[0], '/');
            appName = strdup(p ? p + 1 : app->argv()[0]);
        }
    }
    return appName;
}

const char *qAppFileName()
{
    if(!appFileName) {
        QCFType<CFURLRef> bundleURL(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
        QCFString cfPath(CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle));
        appFileName = strdup(static_cast<QString>(cfPath).latin1());
    }
    return appFileName;
}

