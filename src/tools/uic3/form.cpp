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

#include "ui3reader.h"
#include "parser.h"
#include "domtool.h"

// uic4
#include "ui4.h"
#include "driver.h"
#include "option.h"

#include <qstringlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <globaldefs.h>
#include <qdebug.h>

QByteArray combinePath(const char *infile, const char *outfile)
{
    QFileInfo inFileInfo(QDir::current(), QFile::decodeName(infile));
    QFileInfo outFileInfo(QDir::current(), QFile::decodeName(outfile));
    int numCommonComponents = 0;

    QStringList inSplitted = inFileInfo.dir().canonicalPath().split('/');
    QStringList outSplitted = outFileInfo.dir().canonicalPath().split('/');

    while (!inSplitted.isEmpty() && !outSplitted.isEmpty() &&
            inSplitted.first() == outSplitted.first()) {
        inSplitted.erase(inSplitted.begin());
        outSplitted.erase(outSplitted.begin());
        numCommonComponents++;
    }

    if (numCommonComponents < 2) {
        /*
          The paths don't have the same drive, or they don't have the
          same root directory. Use an absolute path.
        */
        return QFile::encodeName(inFileInfo.absoluteFilePath());
    } else {
        /*
          The paths have something in common. Use a path relative to
          the output file.
        */
        while (!outSplitted.isEmpty()) {
            outSplitted.erase(outSplitted.begin());
            inSplitted.prepend("..");
        }
        inSplitted.append(inFileInfo.fileName());
        return QFile::encodeName(inSplitted.join("/"));
    }
}

/*!
  Creates a declaration (header file) for the form given in \a e

  \sa createFormImpl()
*/
void Ui3Reader::createFormDecl(const QDomElement &e)
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QString objClass = getClassName(e);
    if (objClass.isEmpty())
        return;
    QString objName = getObjectName(e);

    QStringList typeDefs;

    QMap<QString, CustomInclude> customWidgetIncludes;

    /*
      We are generating a few QImage members that are not strictly
      necessary in some cases. Ideally, we would use requiredImage,
      which is computed elsewhere, to keep the generated .h and .cpp
      files synchronized.
    */

    // at first the images
    QMap<QString, int> customWidgets;
    QStringList forwardDecl;
    QStringList forwardDecl2;
    QString exportMacro;
    for (n = e; !n.isNull(); n = n.nextSibling().toElement()) {
        if (n.tagName().toLower() == QLatin1String("customwidgets")) {
            QDomElement n2 = n.firstChild().toElement();
            while (!n2.isNull()) {
                if (n2.tagName().toLower() == QLatin1String("customwidget")) {
                    QDomElement n3 = n2.firstChild().toElement();
                    QString cl;
                    // ### WidgetDatabaseRecord *r = new WidgetDatabaseRecord;
                    while (!n3.isNull()) {
                        QString tagName = n3.tagName().toLower();
                        if (tagName == QLatin1String("class")) {
                            cl = n3.firstChild().toText().data();
                            if (!nofwd)
                                forwardDecl << cl;
                            customWidgets.insert(cl, 0);
                            // ### r->name = cl;
                        } else if (tagName == QLatin1String("header")) {
                            CustomInclude ci;
                            ci.header = n3.firstChild().toText().data();
                            ci.location = n3.attribute("location", "global");
                            // ### r->includeFile = ci.header;
                            customWidgetIncludes.insert(cl, ci);
                        }
                        // ### WidgetDatabase::append(r);
                        n3 = n3.nextSibling().toElement();
                    }
                }
                n2 = n2.nextSibling().toElement();
            }
        }
    }

    // register the object and unify its name
    objName = registerObject(objName);
    QString protector = objName.toUpper() + "_H";
    protector.replace("::", "_");
    out << "#ifndef " << protector << endl;
    out << "#define " << protector << endl;
    out << endl;

    out << "#include <qvariant.h>" << endl; // for broken HP-UX compilers

    if (uiHeaderFile.size())
        out << "#include \"" << uiHeaderFile << "\"" << endl;

    QStringList globalIncludes, localIncludes;

    {
        QMap<QString, CustomInclude>::Iterator it = customWidgetIncludes.find(objClass);
        if (it != customWidgetIncludes.end()) {
            if ((*it).location == QLatin1String("global"))
                globalIncludes += (*it).header;
            else
                localIncludes += (*it).header;
        }
    }

    QStringList::Iterator it;

    globalIncludes = unique(globalIncludes);
    for (it = globalIncludes.begin(); it != globalIncludes.end(); ++it) {
        if (!(*it).isEmpty())
            out << "#include <" << *it << ">" << endl;
    }
    localIncludes = unique(localIncludes);
    for (it = localIncludes.begin(); it != localIncludes.end(); ++it) {
        if (!(*it).isEmpty())
            out << "#include \"" << *it << "\"" << endl;
    }
    out << endl;

    bool dbForm = FALSE;
    registerDatabases(e);
    dbConnections = unique(dbConnections);
    if (dbForms["(default)"].count())
        dbForm = TRUE;
    bool subDbForms = FALSE;
    for (it = dbConnections.begin(); it != dbConnections.end(); ++it) {
        if (!(*it).isEmpty() && (*it) != QLatin1String("(default)")) {
            if (dbForms[(*it)].count()) {
                subDbForms = TRUE;
                break;
            }
        }
    }

    // some typedefs, maybe
    typeDefs = unique(typeDefs);
    for (it = typeDefs.begin(); it != typeDefs.end(); ++it) {
        if (!(*it).isEmpty())
            out << "typedef " << *it << ";" << endl;
    }

    nl = e.parentNode().toElement().elementsByTagName("forward");
    for (i = 0; i < (int) nl.length(); i++)
        forwardDecl2 << nl.item(i).toElement().firstChild().toText().data();

    nl = e.parentNode().toElement().elementsByTagName("exportmacro");
    if (nl.length() == 1)
        exportMacro = nl.item(0).firstChild().toText().data();

    forwardDecl = unique(forwardDecl);
    for (it = forwardDecl.begin(); it != forwardDecl.end(); ++it) {
        if (!(*it).isEmpty() && (*it) != objClass) {
            QString forwardName = *it;
            QStringList forwardNamespaces = forwardName.split("::");
            forwardName = forwardNamespaces.last();
            forwardNamespaces.removeAt(forwardNamespaces.size()-1);

            QStringList::ConstIterator ns = forwardNamespaces.begin();
            while (ns != forwardNamespaces.end()) {
                out << "namespace " << *ns << " {" << endl;
                ++ns;
            }
            out << "class " << forwardName << ";" << endl;
            for (int i = 0; i < (int) forwardNamespaces.count(); i++)
                out << "}" << endl;
        }
    }

    for (it = forwardDecl2.begin(); it != forwardDecl2.end(); ++it) {
        QString fd = *it;
        fd = fd.trimmed();
        if (!fd.endsWith(";"))
            fd += ";";
        out << fd << endl;
    }

    out << endl;

    if (uiHeaderFile.isEmpty()) {
        Driver d;
        d.option().headerProtection = false;
        if (trmacro.size())
            d.option().translateFunction = trmacro;
        DomUI *ui = generateUi4(e);
        d.uic(fileName, ui, &out);
        delete ui;
    }

    QStringList::ConstIterator ns = namespaces.begin();
    while (ns != namespaces.end()) {
        out << "namespace " << *ns << " {" << endl;
        ++ns;
    }

    out << "class ";
    if (!exportMacro.isEmpty())
        out << exportMacro << " ";
    out << bareNameOfClass << " : public " << objClass << ", public Ui::" << objName << endl << "{" << endl;

    /* qmake ignore Q_OBJECT */
    out << "    Q_OBJECT" << endl;
    out << endl;
    out << "public:" << endl;

    // constructor
    if (objClass == QLatin1String("QDialog") || objClass == QLatin1String("QWizard")) {
        out << "    " << bareNameOfClass << "(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0);" << endl;
    } else if (objClass == QLatin1String("QWidget")) {
        out << "    " << bareNameOfClass << "(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);" << endl;
    } else if (objClass == QLatin1String("QMainWindow")) {
        out << "    " << bareNameOfClass << "(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = WType_TopLevel);" << endl;
        isMainWindow = TRUE;
    } else {
        out << "    " << bareNameOfClass << "(QWidget* parent = 0, const char* name = 0);" << endl;
    }

    // destructor
    out << "    ~" << bareNameOfClass << "();" << endl;
    out << endl;

    // database connections
    dbConnections = unique(dbConnections);
    bool hadOutput = FALSE;
    for (it = dbConnections.begin(); it != dbConnections.end(); ++it) {
        if (!(*it).isEmpty()) {
            // only need pointers to non-default connections
            if ((*it) != QLatin1String("(default)") && !(*it).isEmpty()) {
                out << indent << "QSqlDatabase* " << *it << "Connection;" << endl;
                hadOutput = TRUE;
            }
        }
    }
    if (hadOutput)
        out << endl;

    QStringList publicSlots, protectedSlots, privateSlots;
    QStringList publicSlotTypes, protectedSlotTypes, privateSlotTypes;
    QStringList publicSlotSpecifier, protectedSlotSpecifier, privateSlotSpecifier;

    nl = e.parentNode().toElement().elementsByTagName("slot");
    for (i = 0; i < (int) nl.length(); i++) {
        n = nl.item(i).toElement();
        if (n.parentNode().toElement().tagName() != QLatin1String("slots")
             && n.parentNode().toElement().tagName() != QLatin1String("connections"))
            continue;
        if (n.attribute("language", "C++") != QLatin1String("C++"))
            continue;
        QString returnType = n.attribute("returnType", "void");
        QString functionName = n.firstChild().toText().data().trimmed();
        if (functionName.endsWith(";"))
            functionName = functionName.left(functionName.length() - 1);
        QString specifier = n.attribute("specifier");
        QString access = n.attribute("access");
        if (access == QLatin1String("protected")) {
            protectedSlots += functionName;
            protectedSlotTypes += returnType;
            protectedSlotSpecifier += specifier;
        } else if (access == QLatin1String("private")) {
            privateSlots += functionName;
            privateSlotTypes += returnType;
            privateSlotSpecifier += specifier;
        } else {
            publicSlots += functionName;
            publicSlotTypes += returnType;
            publicSlotSpecifier += specifier;
        }
    }

    QStringList publicFuncts, protectedFuncts, privateFuncts;
    QStringList publicFunctRetTyp, protectedFunctRetTyp, privateFunctRetTyp;
    QStringList publicFunctSpec, protectedFunctSpec, privateFunctSpec;

    nl = e.parentNode().toElement().elementsByTagName("function");
    for (i = 0; i < (int) nl.length(); i++) {
        n = nl.item(i).toElement();
        if (n.parentNode().toElement().tagName() != QLatin1String("functions"))
            continue;
        if (n.attribute("language", "C++") != QLatin1String("C++"))
            continue;
        QString returnType = n.attribute("returnType", "void");
        QString functionName = n.firstChild().toText().data().trimmed();
        if (functionName.endsWith(";"))
            functionName = functionName.left(functionName.length() - 1);
        QString specifier = n.attribute("specifier");
        QString access = n.attribute("access");
        if (access == QLatin1String("protected")) {
            protectedFuncts += functionName;
            protectedFunctRetTyp += returnType;
            protectedFunctSpec += specifier;
        } else if (access == QLatin1String("private")) {
            privateFuncts += functionName;
            privateFunctRetTyp += returnType;
            privateFunctSpec += specifier;
        } else {
            publicFuncts += functionName;
            publicFunctRetTyp += returnType;
            publicFunctSpec += specifier;
        }
    }

    QStringList publicVars, protectedVars, privateVars;
    nl = e.parentNode().toElement().elementsByTagName("variable");
    for (i = 0; i < (int)nl.length(); i++) {
        n = nl.item(i).toElement();
        // Because of compatibility the next lines have to be commented out.
        // Someday it should be uncommented.
        //if (n.parentNode().toElement().tagName() != QLatin1String("variables"))
        //    continue;
        QString access = n.attribute("access", "protected");
        QString var = n.firstChild().toText().data().trimmed();
        if (!var.endsWith(";"))
            var += ";";
        if (access == QLatin1String("public"))
            publicVars += var;
        else if (access == QLatin1String("private"))
            privateVars += var;
        else
            protectedVars += var;
    }

    if (!publicVars.isEmpty()) {
        for (it = publicVars.begin(); it != publicVars.end(); ++it)
            out << indent << *it << endl;
        out << endl;
    }
    if (!publicFuncts.isEmpty())
        writeFunctionsDecl(publicFuncts, publicFunctRetTyp, publicFunctSpec);

    if (!publicSlots.isEmpty()) {
        out << "public slots:" << endl;
        if (!publicSlots.isEmpty())
            writeFunctionsDecl(publicSlots, publicSlotTypes, publicSlotSpecifier);
    }

    // find signals
    QStringList extraSignals;
    nl = e.parentNode().toElement().elementsByTagName("signal");
    for (i = 0; i < (int) nl.length(); i++) {
        n = nl.item(i).toElement();
        if (n.parentNode().toElement().tagName() != QLatin1String("signals")
             && n.parentNode().toElement().tagName() != QLatin1String("connections"))
            continue;
        if (n.attribute("language", "C++") != QLatin1String("C++"))
            continue;
        QString sigName = n.firstChild().toText().data().trimmed();
        if (sigName.endsWith(";"))
            sigName = sigName.left(sigName.length() - 1);
        extraSignals += sigName;
    }

    // create signals
    if (!extraSignals.isEmpty()) {
        out << "signals:" << endl;
        for (it = extraSignals.begin(); it != extraSignals.end(); ++it)
            out << "    void " << (*it) << ";" << endl;
        out << endl;
    }

    if (!protectedVars.isEmpty()) {
        out << "protected:" << endl;
        for (it = protectedVars.begin(); it != protectedVars.end(); ++it)
            out << indent << *it << endl;
        out << endl;
    }

    if (!protectedFuncts.isEmpty()) {
        if (protectedVars.isEmpty())
            out << "protected:" << endl;

        writeFunctionsDecl(protectedFuncts, protectedFunctRetTyp, protectedFunctSpec);
    }

    out << "protected slots:" << endl;
    out << "    virtual void languageChange();" << endl;

    if (!protectedSlots.isEmpty()) {
        out << endl;
        writeFunctionsDecl(protectedSlots, protectedSlotTypes, protectedSlotSpecifier);
    }
    out << endl;

    // create all private stuff
    if (!privateFuncts.isEmpty() || !privateVars.isEmpty()) {
        out << "private:" << endl;
        if (!privateVars.isEmpty()) {
            for (it = privateVars.begin(); it != privateVars.end(); ++it)
                out << indent << *it << endl;
            out << endl;
        }
        if (!privateFuncts.isEmpty())
            writeFunctionsDecl(privateFuncts, privateFunctRetTyp, privateFunctSpec);
    }

    if (!privateSlots.isEmpty()) {
        out << "private slots:" << endl;
        writeFunctionsDecl(privateSlots, privateSlotTypes, privateSlotSpecifier);
    }

    out << "};" << endl;
    for (i = 0; i < (int) namespaces.count(); i++)
        out << "}" << endl;

    out << endl;
    out << "#endif // " << protector << endl;
}

void Ui3Reader::writeFunctionsDecl(const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst)
{
    QStringList::ConstIterator it, it2, it3;
    for (it = fuLst.begin(), it2 = typLst.begin(), it3 = specLst.begin();
          it != fuLst.end(); ++it, ++it2, ++it3) {
        QString signature = *it;
        QString specifier;
        QString pure;
        QString type = *it2;
        if (type.isEmpty())
            type = QLatin1String("void");
        if (*it3 == QLatin1String("static")) {
            specifier = QLatin1String("static ");
        } else {
            if (*it3 != QLatin1String("non virtual") && *it3 != QLatin1String("nonVirtual"))
                specifier = QLatin1String("virtual ");
            if (*it3 == QLatin1String("pure virtual") || *it3 == QLatin1String("pureVirtual"))
                pure = QLatin1String(" = 0");
        }
        type.replace(">>", "> >");
        if (!signature.contains("operator"))
            signature.replace(">>", "> >");
        out << "    " << specifier << type << " " << signature << pure << ";" << endl;
    }
    out << endl;
}

/*!
  Creates an implementation (cpp-file) for the form given in \a e.

  \sa createFormDecl(), createObjectImpl()
 */
void Ui3Reader::createFormImpl(const QDomElement &e)
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QString objClass = getClassName(e);
    if (objClass.isEmpty())
        return;
    QString objName = getObjectName(e);

    // generate local and local includes required
    QStringList globalIncludes, localIncludes;
    QStringList::Iterator it;

    QMap<QString, CustomInclude> customWidgetIncludes;

    // find additional slots and functions
    QStringList extraFuncts;
    QStringList extraFunctTyp;
    QStringList extraFunctSpecifier;

    nl = e.parentNode().toElement().elementsByTagName("slot");
    for (i = 0; i < (int) nl.length(); i++) {
        n = nl.item(i).toElement();
        if (n.parentNode().toElement().tagName() != QLatin1String("slots")
             && n.parentNode().toElement().tagName() != QLatin1String("connections"))
            continue;
        if (n.attribute("language", "C++") != QLatin1String("C++"))
            continue;
        QString functionName = n.firstChild().toText().data().trimmed();
        if (functionName.endsWith(";"))
            functionName = functionName.left(functionName.length() - 1);
        extraFuncts += functionName;
        extraFunctTyp += n.attribute("returnType", "void");
        extraFunctSpecifier += n.attribute("specifier", "virtual");
    }

    nl = e.parentNode().toElement().elementsByTagName("function");
    for (i = 0; i < (int) nl.length(); i++) {
        n = nl.item(i).toElement();
        if (n.parentNode().toElement().tagName() != QLatin1String("functions"))
            continue;
        if (n.attribute("language", "C++") != QLatin1String("C++"))
            continue;
        QString functionName = n.firstChild().toText().data().trimmed();
        if (functionName.endsWith(";"))
            functionName = functionName.left(functionName.length() - 1);
        extraFuncts += functionName;
        extraFunctTyp += n.attribute("returnType", "void");
        extraFunctSpecifier += n.attribute("specifier", "virtual");
    }

    // additional includes (local or global) and forward declaractions
    nl = e.parentNode().toElement().elementsByTagName("include");
    for (i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();
        if (n2.attribute("location") != QLatin1String("local")) {
            if (s.right(5) == QLatin1String(".ui.h") && !QFile::exists(s))
                continue;
            if (n2.attribute("impldecl", "in implementation") != QLatin1String("in implementation"))
                continue;
            globalIncludes += s;
        }
    }

    registerDatabases(e);
    dbConnections = unique(dbConnections);
    bool dbForm = FALSE;
    if (dbForms["(default)"].count())
        dbForm = TRUE;
    bool subDbForms = FALSE;
    for (it = dbConnections.begin(); it != dbConnections.end(); ++it) {
        if (!(*it).isEmpty()  && (*it) != QLatin1String("(default)")) {
            if (dbForms[(*it)].count()) {
                subDbForms = TRUE;
                break;
            }
        }
    }

    // do the local includes afterwards, since global includes have priority on clashes
    for (i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();
        if (n2.attribute("location") == QLatin1String("local") && !globalIncludes.contains(s)) {
            if (s.right(5) == QLatin1String(".ui.h") && !QFile::exists(s))
                continue;
            if (n2.attribute("impldecl", "in implementation") != QLatin1String("in implementation"))
                continue;
            localIncludes += s;
        }
    }

    // additional custom widget headers
    nl = e.parentNode().toElement().elementsByTagName("header");
    for (i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();
        if (n2.attribute("location") != QLatin1String("local"))
            globalIncludes += s;
        else
            localIncludes += s;
    }

    out << "#include <qvariant.h>" << endl; // first for gcc 2.7.2

    globalIncludes = unique(globalIncludes);
    for (it = globalIncludes.begin(); it != globalIncludes.end(); ++it) {
        if (!(*it).isEmpty())
            out << "#include <" << *it << ">" << endl;
    }

    if (externPixmaps) {
        out << "#include <qimage.h>" << endl;
        out << "#include <qpixmap.h>" << endl << endl;
    }

    /*
      Put local includes after all global includes
    */
    localIncludes = unique(localIncludes);
    for (it = localIncludes.begin(); it != localIncludes.end(); ++it) {
        if (!(*it).isEmpty() && *it != QFileInfo(fileName + ".h").fileName())
            out << "#include \"" << *it << "\"" << endl;
    }

    QString uiDotH = fileName + ".h";
    if (QFile::exists(uiDotH)) {
        if (!outputFileName.isEmpty())
            uiDotH = combinePath(uiDotH.latin1(), outputFileName);
        out << "#include \"" << uiDotH << "\"" << endl;
        writeFunctImpl = FALSE;
    }

    // register the object and unify its name
    objName = registerObject(objName);

    if (externPixmaps) {
        pixmapLoaderFunction = QLatin1String("QPixmap::fromMimeSource");
    }

    // constructor
    if (objClass == QLatin1String("QDialog") || objClass == QLatin1String("QWizard")) {
        out << "/*" << endl;
        out << " *  Constructs a " << nameOfClass << " as a child of 'parent', with the" << endl;
        out << " *  name 'name' and widget flags set to 'f'." << endl;
        out << " *" << endl;
        out << " *  The " << objClass.mid(1).toLower() << " will by default be modeless, unless you set 'modal' to" << endl;
        out << " *  TRUE to construct a modal " << objClass.mid(1).toLower() << "." << endl;
        out << " */" << endl;
        out << nameOfClass << "::" << bareNameOfClass << "(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)" << endl;
        out << "    : " << objClass << "(parent, name, modal, fl)";
    } else if (objClass == QLatin1String("QWidget"))  {
        out << "/*" << endl;
        out << " *  Constructs a " << nameOfClass << " as a child of 'parent', with the" << endl;
        out << " *  name 'name' and widget flags set to 'f'." << endl;
        out << " */" << endl;
        out << nameOfClass << "::" << bareNameOfClass << "(QWidget* parent, const char* name, Qt::WFlags fl)" << endl;
        out << "    : " << objClass << "(parent, name, fl)";
    } else if (objClass == QLatin1String("QMainWindow")) {
        out << "/*" << endl;
        out << " *  Constructs a " << nameOfClass << " as a child of 'parent', with the" << endl;
        out << " *  name 'name' and widget flags set to 'f'." << endl;
        out << " *" << endl;
        out << " */" << endl;
        out << nameOfClass << "::" << bareNameOfClass << "(QWidget* parent, const char* name, Qt::WFlags fl)" << endl;
        out << "    : " << objClass << "(parent, name, fl)";
        isMainWindow = TRUE;
    } else {
        out << "/*" << endl;
        out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the" << endl;
        out << " *  name 'name'.' " << endl;
        out << " */" << endl;
        out << nameOfClass << "::" << bareNameOfClass << "(QWidget* parent,  const char* name)" << endl;
        out << "    : " << objClass << "(parent, name)";
    }

    out << endl;

    out << "{" << endl;

//
// setup the gui
//
    out << indent << "setupUi(this);" << endl << endl;


    if (isMainWindow)
        out << indent << "(void)statusBar();" << endl;

    // database support
    dbConnections = unique(dbConnections);
    if (dbConnections.count())
        out << endl;
    for (it = dbConnections.begin(); it != dbConnections.end(); ++it) {
        if (!(*it).isEmpty() && (*it) != QLatin1String("(default)")) {
            out << indent << (*it) << "Connection = QSqlDatabase::database(\"" <<(*it) << "\");" << endl;
        }
    }

    nl = e.parentNode().toElement().elementsByTagName("widget");
    for (i = 1; i < (int) nl.length(); i++) { // start at 1, 0 is the toplevel widget
        n = nl.item(i).toElement();
        QString s = getClassName(n);
        if ((dbForm || subDbForms) && (s == QLatin1String("QDataBrowser") || s == QLatin1String("QDataView"))) {
            QString objName = getObjectName(n);
            QString tab = getDatabaseInfo(n, "table");
            QString con = getDatabaseInfo(n, "connection");
            out << indent << "QSqlForm* " << objName << "Form = new QSqlForm(this);" << endl;
            out << indent << objName << "Form->setObjectName(\"" << objName << "Form\");" << endl;
            QDomElement n2;
            for (n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement())
                createFormImpl(n2, objName, con, tab);
            out << indent << objName << "->setForm(" << objName << "Form);" << endl;
        }
    }

    for (n = e; !n.isNull(); n = n.nextSibling().toElement()) {
        if (n.tagName() == QLatin1String("connections")) {
            // setup signals and slots connections
            out << endl << indent << "// signals and slots connections" << endl;
            nl = n.elementsByTagName("connection");
            for (i = 0; i < (int) nl.length(); i++) {
                QString sender, receiver, signal, slot;
                for (QDomElement n2 = nl.item(i).firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement()) {
                    if (n2.tagName() == QLatin1String("sender"))
                        sender = n2.firstChild().toText().data();
                    else if (n2.tagName() == QLatin1String("receiver"))
                        receiver = n2.firstChild().toText().data();
                    else if (n2.tagName() == QLatin1String("signal"))
                        signal = n2.firstChild().toText().data();
                    else if (n2.tagName() == QLatin1String("slot"))
                        slot = n2.firstChild().toText().data();
                }
                if (sender.isEmpty() ||
                     receiver.isEmpty() ||
                     signal.isEmpty() ||
                     slot.isEmpty())
                    continue;
                if (sender[0] == '<' ||
                     receiver[0] == '<' ||
                     signal[0] == '<' ||
                     slot[0] == '<')
                    continue;

                sender = registeredName(sender);
                receiver = registeredName(receiver);

                 // translate formwindow name to "this"
                if (sender == objName)
                    sender = QLatin1String("this");
                if (receiver == objName)
                    receiver = QLatin1String("this");

                out << indent << "connect(" << sender << ", SIGNAL(" << signal << "), "
                    << receiver << ", SLOT(" << slot << "));" << endl;
            }
        }
    }

    if (extraFuncts.contains("init()"))
        out << indent << "init();" << endl;

    // end of constructor
    out << "}" << endl;
    out << endl;

    // destructor
    out << "/*" << endl;
    out << " *  Destroys the object and frees any allocated resources" << endl;
    out << " */" << endl;
    out << nameOfClass << "::~" << bareNameOfClass << "()" << endl;
    out << "{" << endl;
    if (extraFuncts.contains("destroy()"))
        out << indent << "destroy();" << endl;
    out << indent << "// no need to delete child widgets, Qt does it all for us" << endl;
    out << "}" << endl;
    out << endl;

    // handle application events if required
    bool needFontEventHandler = FALSE;
    bool needSqlTableEventHandler = FALSE;
    bool needSqlDataBrowserEventHandler = FALSE;
    nl = e.elementsByTagName("widget");
    for (i = 0; i < (int) nl.length(); i++) {
        if (!DomTool::propertiesOfType(nl.item(i).toElement() , "font").isEmpty())
            needFontEventHandler = TRUE;
        QString s = getClassName(nl.item(i).toElement());
        if (s == QLatin1String("QDataTable") || s == QLatin1String("QDataBrowser")) {
            if (!isFrameworkCodeGenerated(nl.item(i).toElement()))
                 continue;
            if (s == QLatin1String("QDataTable"))
                needSqlTableEventHandler = TRUE;
            if (s == QLatin1String("QDataBrowser"))
                needSqlDataBrowserEventHandler = TRUE;
        }
        if (needFontEventHandler && needSqlTableEventHandler && needSqlDataBrowserEventHandler)
            break;
    }

    out << "/*" << endl;
    out << " *  Sets the strings of the subwidgets using the current" << endl;
    out << " *  language." << endl;
    out << " */" << endl;
    out << "void " << nameOfClass << "::languageChange()" << endl;
    out << "{" << endl;
    out << "    refreshUi(this);" << endl;
    out << "}" << endl;
    out << endl;

    // create stubs for additional slots if necessary
    if (!extraFuncts.isEmpty() && writeFunctImpl) {
        it = extraFuncts.begin();
        QStringList::Iterator it2 = extraFunctTyp.begin();
        QStringList::Iterator it3 = extraFunctSpecifier.begin();
        while (it != extraFuncts.end()) {
            QString type = *it2;
            if (type.isEmpty())
                type = QLatin1String("void");
            type = type.simplified();
            QString fname = Parser::cleanArgs(*it);
            if (!(*it3).startsWith("pure")) { // "pure virtual" or "pureVirtual"
                out << type << " " << nameOfClass << "::" << fname << endl;
                out << "{" << endl;
                if (*it != QLatin1String("init()") && *it != QLatin1String("destroy()")) {
                    QRegExp numeric("^(?:signed|unsigned|u?char|u?short|u?int"
                                     "|u?long|Q_U?INT(?:8|16|32)|Q_U?LONG|float"
                                     "|double)$");
                    QString retVal;

                    /*
                      We return some kind of dummy value to shut the
                      compiler up.

                      1.  If the type is 'void', we return nothing.

                      2.  If the type is 'bool', we return 'FALSE'.

                      3.  If the type is 'unsigned long' or
                          'Q_UINT16' or 'double' or similar, we
                          return '0'.

                      4.  If the type is 'Foo *', we return '0'.

                      5.  If the type is 'Foo &', we create a static
                          variable of type 'Foo' and return it.

                      6.  If the type is 'Foo', we assume there's a
                          default constructor and use it.
                    */
                    if (type != QLatin1String("void")) {
                        QStringList toks = type.split(" ");
                        bool isBasicNumericType =
                                (toks.find(numeric).count() == toks.count());

                        if (type == QLatin1String("bool")) {
                            retVal = QLatin1String("FALSE");
                        } else if (isBasicNumericType || type.endsWith("*")) {
                            retVal = QLatin1String("0");
                        } else if (type.endsWith("&")) {
                            do {
                                type.truncate(type.length() - 1);
                            } while (type.endsWith(" "));
                            retVal = QLatin1String("uic_temp_var");
                            out << indent << "static " << type << " " << retVal << ";" << endl;
                        } else {
                            retVal = type + "()";
                        }
                    }

                    out << indent << "qWarning(\"" << nameOfClass << "::" << fname << ": Not implemented yet\");" << endl;
                    if (!retVal.isEmpty())
                        out << indent << "return " << retVal << ";" << endl;
                }
                out << "}" << endl;
                out << endl;
            }
            ++it;
            ++it2;
            ++it3;
        }
    }
}


/*! Creates form support implementation code for the widgets given
  in \a e.

  Traverses recursively over all children.
 */

void Ui3Reader::createFormImpl(const QDomElement& e, const QString& form, const QString& connection, const QString& table)
{
    if (e.tagName() == QLatin1String("widget") &&
         e.attribute("class") != QLatin1String("QDataTable")) {
        QString field = getDatabaseInfo(e, "field");
        if (!field.isEmpty()) {
            if (isWidgetInTable(e, connection, table))
                out << indent << form << "Form->insert(" << getObjectName(e) << ", " << fixString(field) << ");" << endl;
        }
    }
    QDomElement n;
    for (n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement()) {
        createFormImpl(n, form, connection, table);
    }
}
