/**********************************************************************
** Copyright (C) 2000-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include "profile.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qmap.h>

class Profile;

class Config
{
public:

    Config( const QString &name );
    Config();

    void load( const QString &name );
    void save();
    Profile *profile() const { return profil; }
    QString profileName() const { return profil->props["name"]; }
    bool setCurrentProfile( const QString &name );
    bool startedWithProfile() const;
    bool validProfileName() const;
    void hideSideBar( bool b );
    bool sideBarHidden() const;
    QStringList mimePaths();

    // From profile, read only
    QStringList profiles() const;
    QString title() const;
    QString aboutApplicationMenuText() const;
    QString aboutURL() const;
    QStringList docFiles() const;
    QString docTitle( const QString & ) const;
    QString docImageDir( const QString & ) const;
    QString basePath() const;
    QPixmap docIcon( const QString & ) const;
    QPixmap applicationIcon() const;
    bool needsNewDoc() const;

    // From QSettings, read / write
    QString webBrowser() const { return webBrows; }
    void setWebBrowser( const QString &cmd ) { webBrows = cmd; }

    QString homePage() const;
    void setHomePage( const QString &hom ) { home = hom; }

    QString pdfReader() const { return pdfApp; }
    void setPdfReader( const QString &cmd ) { pdfApp = cmd; }

    int fontSize() const { return fontSiz; }
    void setFontSize( int size ) { fontSiz = size; }

    QString fontFamily() const { return fontFam; }
    void setFontFamily( const QString &fam ) { fontFam = fam; }

    QString fontFixedFamily() const { return fontFix; }
    void setFontFixedFamily( const QString &fn ) { fontFix = fn; }

    QString linkColor() const { return linkCol; }
    void setLinkColor( const QString &col ) { linkCol = col; }

    QString source() const;
    void setSource( const QString &s ) { src = s; }

    int sideBarPage() const { return sideBar; }
    void setSideBarPage( int sbp ) { sideBar = sbp; }

    QRect geometry() const { return geom; }
    void setGeometry( const QRect &geo ) { geom = geo; }

    bool isMaximized() const { return maximized; }
    void setMaximized( bool max ) { maximized = max; }

    bool isLinkUnderline() const { return linkUnder; }
    void setLinkUnderline( bool ul ) { linkUnder = ul; }

    QString mainWindowLayout() const { return mainWinLayout; }
    void setMainWindowLayout( const QString &layout ) { mainWinLayout = layout; }

    QString assistantDocPath() const { return assDocPath; }
    void setAssistantDocPath( const QString & adp ) { assDocPath = adp; }

    void saveProfile( Profile *profile, bool changed = FALSE );
    Profile* loadProfile( const QString &name );
    void removeProfile( const QString &name );
    void reloadProfiles();

    static Config *configuration();
    static bool addProfile( const QString &profileFileName,
			    const QString &path );

private:
    Config( const Config &c );
    Config& operator=( const Config &c );

    void saveSettings();

private:
    Profile *profil;

    QStringList profileNames;
    QString webBrows;
    QString home;
    QString pdfApp;
    QString fontFam;
    QString fontFix;
    QString linkCol;
    QString src;
    QString mainWinLayout;
    QString assDocPath;
    QRect geom;
    int sideBar;
    int fontSiz;
    bool maximized;
    bool linkUnder;
    bool startWithProfile;
    bool profileNameValid;
    bool hideSidebar;
};

#endif
