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

#ifndef CONFIG_H
#define CONFIG_H

#include "profile.h"

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QMap>

#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

class Profile;

struct FontSettings
{
    FontSettings() : useWindowFont(false), useBrowserFont(false),
        windowWritingSystem(QFontDatabase::Latin), browserWritingSystem(QFontDatabase::Latin)
        { }

    QFont windowFont;
    QFont browserFont;

    bool useWindowFont;
    bool useBrowserFont;

    QFontDatabase::WritingSystem windowWritingSystem;
    QFontDatabase::WritingSystem browserWritingSystem;
};

class Config
{
public:

    Config();

    void load();
    void save();
    Profile *profile() const { return profil; }
    QString profileName() const { return profil->props[QLatin1String("name")]; }
    bool validProfileName() const;
    void hideSideBar( bool b );
    bool sideBarHidden() const;
    QStringList mimePaths();

    // From profile, read only
    QStringList docFiles() const;
    QStringList docTitles() const;
    QString indexPage( const QString &title ) const;
    QString docImageDir( const QString &title ) const;
    QPixmap docIcon( const QString &title ) const;

    QStringList profiles() const;
    QString title() const;
    QString aboutApplicationMenuText() const;
    QString aboutURL() const;
    QPixmap applicationIcon() const;

    // From QSettings, read / write
    QString homePage() const;
    void setHomePage( const QString &hom ) { home = hom; }

    QStringList source() const;
    void setSource( const QStringList &s ) { src = s; }

    int sideBarPage() const { return sideBar; }
    void setSideBarPage( int sbp ) { sideBar = sbp; }

    QByteArray windowGeometry() const { return winGeometry; }
    void setWindowGeometry( const QByteArray &geometry ) { winGeometry = geometry; }
    
    QByteArray mainWindowState() const { return mainWinState; }
    void setMainWindowState( const QByteArray &state ) { mainWinState = state; }

    qreal fontPointSize() const { return pointFntSize; }
    void setFontPointSize(qreal size)
    { 
        pointFntSize = size;
        m_fontSettings.useBrowserFont = true;
        m_fontSettings.browserFont.setPointSizeF(size); 
    }

    FontSettings fontSettings() { return m_fontSettings; }
    void setFontSettings(const FontSettings &settings) { m_fontSettings = settings; }

    QString assistantDocPath() const;

    bool docRebuild() const { return rebuildDocs; }
    void setDocRebuild( bool rb ) { rebuildDocs = rb; }

    void saveProfile( Profile *profile );
    void loadDefaultProfile();
    bool defaultProfileExists();

    static Config *configuration();
    static Config *loadConfig(const QString &profileFileName);

private:
    Config( const Config &c );
    Config& operator=( const Config &c );

    void saveSettings();
    
private:
    Profile *profil;

    QStringList profileNames;
    QString home;
    QStringList src;
    QByteArray mainWinState;
    QByteArray winGeometry;
    qreal pointFntSize;
    int sideBar;
    bool hideSidebar;
    bool rebuildDocs;
    FontSettings m_fontSettings;
};

#endif // CONFIG_H
