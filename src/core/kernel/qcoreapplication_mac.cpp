/****************************************************************************
**
** Implementation of QCoreApplication class for Mac OS X.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcoreapplication.h"
#include "qcore_mac.h"

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static char    *appName = 0;                        // application name
static char *appFileName = 0;                       // application file name


/*****************************************************************************
  External functions
 *****************************************************************************/
extern QString cfstring2qstring(CFStringRef); //qglobal.cpp

/*****************************************************************************
  QCoreApplication utility functions
 *****************************************************************************/
const char *qAppName()                                // get application name
{
    if(!appName) {
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr) {
            CFStringRef cfstr;
            CopyProcessName(&psn, &cfstr);
            appName = strdup(cfstring2qstring(cfstr).latin1());
            CFRelease(cfstr);
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
        CFURLRef bundleURL = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
        CFStringRef cfPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
        QString tmp = cfstring2qstring(cfPath);
        appFileName = strdup(tmp.latin1());
        CFRelease(bundleURL);
        CFRelease(cfPath);
    }
    return appFileName;
}

