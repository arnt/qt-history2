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
static QString *appName=0;                        // application name
static QString *appFileName=0;                       // application file name

/*****************************************************************************
  QCoreApplication utility functions
 *****************************************************************************/
QString qAppName()                                // get application name
{
    if(!appName) {
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr) {
            QCFString cfstr;
            CopyProcessName(&psn, &cfstr);
            appName = new QString(static_cast<QString>(cfstr));
        } else if(QCoreApplication *app = QCoreApplication::instance()) {
            char *p = strrchr(app->argv()[0], '/');
            appName = new QString(p ? p + 1 : app->argv()[0]);
        }
    }
    return *appName;
}

QString qAppFileName()
{
    if(!appFileName) {
        QCFType<CFURLRef> bundleURL(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
        QCFString cfPath(CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle));
        appFileName = new QString(static_cast<QString>(cfPath));
    }
    return *appFileName;
}

