#ifndef CONFIG_H
#define CONFIG_H

#include "profile.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qmap.h>

class DocuParser;
class Profile;

class Config
{
public:

    Config( Profile *p );

    void load();
    void save();
    Profile *profile() const { return profil; }
    QString profileName() const { return profil->props["name"]; }
    bool isDefaultProfile() const { return profil->isDefaultProfile(); }

    // From profile, read only
    QString title() const;
    QString aboutApplicationMenuText() const;
    QString aboutURL() const;
    QStringList docFiles() const;
    QString docTitle( const QString & ) const;
    QString docCategory( const QString & ) const;
    QString docContentsURL( const QString & ) const;
    QString docBasePath() const;
    QPixmap docIcon( const QString & ) const;
    QPixmap applicationIcon() const;

    // From QSettings, read / write
    QStringList selectedCategories() const { return selCat; }
    void setSelectedCategories( const QStringList &lst ) { selCat = lst; }

    QString webBrowser() const { return webBrows; }
    void setWebBrowser( const QString &cmd ) { webBrows = cmd; }

    QString homePage() const { return home; }
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

    QString source() const { return src; }
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

    bool isDifferentProfile() const { return profDiffer; }

    static Config *configuration();

private:
    Config( const Config &c );
    Config& operator=( const Config &c );

    DocuParser *parser( const QString & ) const;

private:
    Profile *profil;
    QMap<QString,DocuParser*> parserCache;

    QStringList selCat;
    QString webBrows;
    QString home;
    QString pdfApp;
    QString fontFam;
    QString fontFix;
    QString linkCol;
    QString src;
    QString mainWinLayout;
    QRect geom;
    int sideBar;
    int fontSiz;
    bool maximized;
    bool linkUnder;
    bool profDiffer;
};

#endif
