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

#ifndef UI3READER_H
#define UI3READER_H

#include <qnamespace.h>
#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qcorevariant.h>
#include <qbytearray.h>
#include <qpair.h>

class DomUI;
class DomWidget;
class DomProperty;
class DomLayout;
class DomLayoutItem;

struct Color;

typedef QList<QPair<int, Color> > ColorGroup;

class Ui3Reader : public Qt
{
public:
    Ui3Reader(QTextStream &stream);

    void generateUi4(const QString &fn, const QString &outputFn,
         QDomDocument doc);

    void generate(const QString &uiHeaderFile, const QString &fn, const QString &outputFn,
         QDomDocument doc, bool decl, bool subcl, const QString &trm,
         const QString& subclname, bool omitForwardDecls);

    void embed(const char *project, const QStringList &images);

    void setTrMacro(const QString &trmacro);
    void setForwardDeclarationsEnabled(bool b);
    void setOutputFileName(const QString &fileName);

    void createFormDecl(const QDomElement &e);
    void createFormImpl(const QDomElement &e);

    void createSubDecl(const QDomElement &e, const QString& subclname);
    void createSubImpl(const QDomElement &e, const QString& subclname);

    void createObjectDecl(const QDomElement &e);
    void createSpacerDecl(const QDomElement &e);
    void createActionDecl(const QDomElement &e);
    void createToolbarDecl(const QDomElement &e);
    void createMenuBarDecl(const QDomElement &e);
    void createPopupMenuDecl(const QDomElement &e);
    void createActionImpl(const QDomElement &e, const QString &parent);
    void createToolbarImpl(const QDomElement &e, const QString &parentClass, const QString &parent);
    void createMenuBarImpl(const QDomElement &e, const QString &parentClass, const QString &parent);
    void createPopupMenuImpl(const QDomElement &e, const QString &parentClass, const QString &parent);
    QString createObjectImpl(const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout = QString::null);
    QString createLayoutImpl(const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout = QString::null);
    QString createObjectInstance(const QString& objClass, const QString& parent, const QString& objName);
    QString createSpacerImpl(const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout = QString::null);
    void createExclusiveProperty(const QDomElement & e, const QString& exclusiveProp);
    QString createListBoxItemImpl(const QDomElement &e, const QString &parent, QString *value = 0);
    QString createIconViewItemImpl(const QDomElement &e, const QString &parent);
    QString createListViewColumnImpl(const QDomElement &e, const QString &parent, QString *value = 0);
    QString createTableRowColumnImpl(const QDomElement &e, const QString &parent, QString *value = 0);
    QString createListViewItemImpl(const QDomElement &e, const QString &parent,
                                    const QString &parentItem);
    void createColorGroupImpl(const QString& cg, const QDomElement& e);
    ColorGroup loadColorGroup(const QDomElement &e);

    QDomElement getObjectProperty(const QDomElement& e, const QString& name);
    QString getPixmapLoaderFunction(const QDomElement& e);
    QString getFormClassName(const QDomElement& e);
    QString getClassName(const QDomElement& e);
    QString getObjectName(const QDomElement& e);
    QString getLayoutName(const QDomElement& e);
    QString getInclude(const QString& className);

    QString setObjectProperty(const QString& objClass, const QString& obj, const QString &prop, const QDomElement &e, bool stdset);

    QString registerObject(const QString& name);
    QString registeredName(const QString& name);
    bool isObjectRegistered(const QString& name);
    QStringList unique(const QStringList&);

    QString trcall(const QString& sourceText, const QString& comment = "");

    QDomElement parse(const QDomDocument &doc);

private:
    void registerLayouts (const QDomElement& e);
    void init();

    DomUI *generateUi4( const QDomElement &e); // ### rename me in createUI4()
    DomWidget *createWidget(const QDomElement &w, const QString &widgetClass = QString::null);
    void createProperties(const QDomElement &e, QList<DomProperty*> *properties);
    void createAttributes(const QDomElement &e, QList<DomProperty*> *properties);
    DomLayout *createLayout(const QDomElement &e);
    DomLayoutItem *createLayoutItem(const QDomElement &e);
    DomProperty *readProperty(const QDomElement &e);


    QTextStream &out;
    QTextOStream trout;
    QString languageChangeBody;
    QString outputFileName;
    QStringList objectNames;
    QMap<QString,QString> objectMapper;
    QString indent;
    QStringList tags;
    QStringList layouts;
    QString formName;
    QString lastItem;
    QString trmacro;
    bool nofwd;

    struct Buddy
    {
        Buddy(const QString& k, const QString& b)
            : key(k), buddy(b) {}
        Buddy(){} // for valuelist
        QString key;
        QString buddy;
        bool operator==(const Buddy& other) const
            { return (key == other.key); }
    };
    struct CustomInclude
    {
        QString header;
        QString location;
        Q_DUMMY_COMPARISON_OPERATOR(CustomInclude)
    };
    QList<Buddy> buddies;

    QStringList layoutObjects;
    bool isLayout(const QString& name) const;

    uint item_used : 1;
    uint cg_used : 1;
    uint pal_used : 1;
    uint stdsetdef : 1;
    uint externPixmaps : 1;

    QString uiFileVersion;
    QString nameOfClass;
    QStringList namespaces;
    QString bareNameOfClass;
    QString pixmapLoaderFunction;

    void registerDatabases(const QDomElement& e);
    bool isWidgetInTable(const QDomElement& e, const QString& connection, const QString& table);
    bool isFrameworkCodeGenerated(const QDomElement& e);
    QString getDatabaseInfo(const QDomElement& e, const QString& tag);
    void createFormImpl(const QDomElement& e, const QString& form, const QString& connection, const QString& table);
    void writeFunctionsDecl(const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst);
    void writeFunctionsSubDecl(const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst);
    void writeFunctionsSubImpl(const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst,
                                const QString &subClass, const QString &descr);
    QStringList dbConnections;
    QMap< QString, QStringList > dbCursors;
    QMap< QString, QStringList > dbForms;

    static bool isMainWindow;
    static QString mkBool(bool b);
    static QString mkBool(const QString& s);
    bool toBool(const QString& s);
    static QString fixString(const QString &str, bool encode = FALSE);
    static bool onlyAscii;
    static QString mkStdSet(const QString& prop);
    static QString getComment(const QDomNode& n);
    QCoreVariant defSpacing, defMargin;
    QString fileName;
    QString uiHeaderFile;
    bool writeFunctImpl;
};

#endif
