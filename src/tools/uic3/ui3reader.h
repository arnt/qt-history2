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

#ifndef UI3READER_H
#define UI3READER_H

#include <qnamespace.h>
#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qvariant.h>
#include <qbytearray.h>
#include <qpair.h>

class DomUI;
class DomWidget;
class DomProperty;
class DomLayout;
class DomLayoutItem;
class DomActionGroup;
class Porting;
struct Color;

typedef QList<QPair<int, Color> > ColorGroup;

class Ui3Reader
{
public:
    Ui3Reader(QTextStream &stream);
    ~Ui3Reader();

    void generateUi4(const QString &fn, const QString &outputFn, QDomDocument doc);

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

    void createColorGroupImpl(const QString& cg, const QDomElement& e);
    ColorGroup loadColorGroup(const QDomElement &e);

    QDomElement getObjectProperty(const QDomElement& e, const QString& name);
    QString getPixmapLoaderFunction(const QDomElement& e);
    QString getFormClassName(const QDomElement& e);
    QString getClassName(const QDomElement& e);
    QString getObjectName(const QDomElement& e);
    QString getLayoutName(const QDomElement& e);

    QString registerObject(const QString& name);
    QString registeredName(const QString& name);
    bool isObjectRegistered(const QString& name);
    QStringList unique(const QStringList&);

    QString trcall(const QString& sourceText, const QString& comment = QString());

    QDomElement parse(const QDomDocument &doc);

private:
    void init();

    DomUI *generateUi4(const QDomElement &e);
    DomWidget *createWidget(const QDomElement &w, const QString &widgetClass = QString::null);
    void createProperties(const QDomElement &e, QList<DomProperty*> *properties, const QString &className);
    void createAttributes(const QDomElement &e, QList<DomProperty*> *properties, const QString &className);
    DomLayout *createLayout(const QDomElement &e);
    DomLayoutItem *createLayoutItem(const QDomElement &e);
    DomProperty *readProperty(const QDomElement &e);
    void fixActionGroup(DomActionGroup *g);
    QString fixActionProperties(QList<DomProperty*> &properties, bool isActionGroup = false);

    QString fixHeaderName(const QString &headerName) const;
    QString fixClassName(const QString &className) const;
    QString fixDeclaration(const QString &declaration) const;
    QString fixType(const QString &type) const;

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
    QMap<QString, QStringList> dbCursors;
    QMap<QString, QStringList> dbForms;

    static bool isMainWindow;
    static QString mkBool(bool b);
    static QString mkBool(const QString& s);
    bool toBool(const QString& s);
    static QString fixString(const QString &str, bool encode = false);
    static bool onlyAscii;
    static QString mkStdSet(const QString& prop);
    static QString getComment(const QDomNode& n);
    QVariant defSpacing, defMargin;
    QString fileName;
    QString uiHeaderFile;
    bool writeFunctImpl;

    QDomElement root;
    QDomElement widget;

    QMap<QString, bool> candidateCustomWidgets;
    Porting *m_porting;
};

#endif
