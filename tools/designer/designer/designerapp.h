/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DESIGNERAPP_H
#define DESIGNERAPP_H

class QSplashScreen;

#include <qapplication.h>

class DesignerApplication : public QApplication
{
public:
    const char *className() const { return "DesignerApplication"; }

    DesignerApplication( int &argc, char **argv );

    QSplashScreen *showSplash();
    static void closeSplash();

    static QString settingsKey();
    static QString oldSettingsKey();
    static void setSettingsKey( const QString &key );

protected:
    QDateTime lastMod;

#if defined(Q_WS_WIN)
    bool winEventFilter( MSG *msg );
    uint DESIGNER_OPENFILE;
#endif

};

#endif
