#include "genportrules.h"

#include <stdio.h>
#include <iostream>
#include <sstream>

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <qglobal.h>
#include <QByteArray>
#include <QTextStream>
#include <QProcess>
#include <QLibraryInfo>
#include <qalgorithms.h>

#include <lexer.h>
#include <parser.h>
#include <treedump.h>
#include <genqtsymbols.h>
#include <genclassnames.h>
#include <gendocrules.h>
#include <qtsimplexml.h>

using std::cout;
using std::endl;

int ruleIndex=0;
QtSimpleXml xml;

/*
    Paths to .qdocinc files
*/
QString baseDocDir;
QString renamedClassesFileName;
QString renamedEnumvaluesFileName;
QString renamedTypesFileName;
QString renamedStaticMethods;
QString modifiedVirtualFileName;
QString removedEnumvaluesFileName;
QString removedTypesFileName;
QString removedVirtualFileName;

/*
    Paths to qt include dirs
*/
QString qt3IncludeDir;
QString qt4Qt3SupportIncludeDir;
QString qt4IncludeDir;
QString qt4QtIncludeDir;

/*
    Paths to cpp files that includes various parts of qt3 and qt4
*/
QString includeQtNamespace="sourcefiles/includeqtnamespace.cpp";    //#include <qtnamespace.h>
QString includeAllQt3=     "sourcefiles/allqt3.cpp";                //#include <qt.h>
QString includeAllQt4Compat="sourcefiles/includeallqt4compat.cpp";  //#include <Qt3Compat>

/*
    Qt3 Qt class and Qt4 Qt namespace parsers
*/
QtClassParser     *qtClassParser;
QtNamespaceParser *qtNamespaceParser;

/*
    Rule Types:
    RenamedClass
    RenamedEnumvalue
    RenamedType
    RenamedHeader
    RenamedToken
    RenamedQtSymbol
    NeedHeader
    ModifiedVirtual
    RemovedEnumvalue
    RemovedType
    RemovedVirtual
    Qt4Class
    InheritsQt

    xml file format:
    <rules>
        <item Type = insert_replacement_type_here> <Qt3> xxx </Qt3> <Qt4> yyy </Qt4> </item>
    </rules>
*/

//forward declares
bool overrideClassRename(QString className);
bool overrideHeadersRename(QString headerName);

/*
    holds a name of a class plus the library in which it is placed.
*/
struct ClassNameLibrary
{
    QString className;
    QString libraryName;
};

QString getBaseDir()
{
    QString basedir = QLibraryInfo::location(QLibraryInfo::PrefixPath);
    basedir = QFileInfo(basedir + "/..").canonicalFilePath();
    return basedir;
}

QString getQtDir()
{
    return QLibraryInfo::location(QLibraryInfo::PrefixPath);
}

/*
    Preprocess a file using cpp
*/
inline QByteArray gccPreprocess(const QString &fileName, QStringList args = QStringList())
{
    args += fileName;
/*
    cout << "starting process" << endl;
    cout << "FileName " << fileName.toLatin1().constData() << endl;
    cout << "Args" << endl;
    foreach(QString arg, args)
        cout << arg.toLatin1().constData() << endl;
*/
    QProcess process;
    process.start("cpp", args);
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    cout << process.readAllStandardError().constData() << endl;

    return output;
}

/*
    returns a list of all the files in a directory
*/
QStringList getFileNames(QString path, QString filter)
{
    QStringList list;
    QDir dir(path);
    dir.setNameFilters(QStringList()<<filter);
    dir.setFilter(QDir::Files);
    list=dir.entryList();
    return list;
}

/*
    returns all new-style Qxxx header in a path, including sub-dirs
*/
QStringList getHeadersRecursive(QString path)
{
    QDir dir(path);
    dir.setFilter(QDir::Files);
    QStringList filesList = dir.entryList();
    QStringList qtHeaderList;
    foreach(QString file, filesList) {
        if (file[0] == 'Q' && file[1] != '_' && file[1] != 't') //don't want Q_LONG etc., nor QtXml etc.
            qtHeaderList += file;
    }

    dir.setFilter(QDir::Dirs);
    QStringList dirList=dir.entryList();
    foreach(QString dir, dirList) {
        if(dir != "private" && dir != ".." && dir != ".")
            qtHeaderList += getHeadersRecursive(path + dir);
    }
    qSort(qtHeaderList);
    return qtHeaderList;
 }

/*
    returns a list of class names and libraries based on
    parsing the Qt4 include dir
*/
QList<ClassNameLibrary> getClassNamesAndLibrary(QString path)
{
    QList<ClassNameLibrary> classList;
    QString currentLib;

    QDir dir(path);
    dir.setFilter(QDir::Dirs);
    QStringList dirList=dir.entryList();
    foreach(QString dir, dirList) {
        if(dir != "private" && dir!="Qt" && dir != ".." && dir != ".") {
            currentLib = dir;
            //cout<<path.latin1()<< currentLib.latin1()<<"/"<<endl;
            QDir libdir(path+dir+"/");
            libdir.setFilter(QDir::Files);
            QStringList filesList = libdir.entryList();
            foreach(QString file, filesList) {
                ClassNameLibrary className;
                if (file[0] == 'Q' && file[1] != '_' && file[1] != 't') { //don't want Q_LONG etc., nor QtXml etc.
              //      cout<<"Adding class "<< file.latin1() <<"  in library "<< dir.latin1() <<endl;
                    className.className = file;
                    className.libraryName = dir;
                    classList.push_back(className);
                }
            }

        }
    }
    return classList;
}


struct FileMatch
{
    QString qt3;
    QString qt4Compat;
};

/*
    returns true if the given pair of headers matches the pattern Q3XxxYyyy qxxxyyy.h
*/
bool matchCompatHeadersNewStyle(QString qt3header, QString qt4CompatHeader)
{
    int threeIndex=qt4CompatHeader.indexOf("3");
    if (threeIndex != -1) {
         qt4CompatHeader.remove(threeIndex,1);
         if(qt4CompatHeader.contains(".h"))
             return false;
         int dotIndex = qt3header.indexOf(".h");
         if(dotIndex==-1)
             return false;
         qt3header.remove(dotIndex,2);

        return (qt4CompatHeader.toLower() == qt3header.toLower());
    }

    return false;
}

/*
    returns true if the given pair of headers matches the pattern q3xxx.h qxxx.h
*/
bool matchCompatHeaders(QString qt3header, QString qt4CompatHeader)
{
    int threeIndex=qt4CompatHeader.indexOf("3");
    if (threeIndex != -1) {
        qt4CompatHeader.remove(threeIndex,1);
        return (qt4CompatHeader == qt3header);
    }

    return false;
}


/*
    Returns a list of all header file name matches, given a list of
    qt3 headers and a list of qt4 compat headers
*/
QList<FileMatch> getFileMatches(QStringList qt3Headers, QStringList qt4CompatHeaders)
{
    QList<FileMatch> matches;
    foreach(QString qt4compatHeader, qt4CompatHeaders) {
        foreach(QString qt3Header, qt3Headers) {
            if (matchCompatHeaders(qt3Header, qt4compatHeader)) {
                FileMatch m;
                m.qt3 = qt3Header;
                m.qt4Compat = qt4compatHeader;
                matches.append(m);
            }
        }
    }

    return matches;
}

/*
    prints header file name matches
*/
void printFileMatches(QList<FileMatch> matches)
{
    int i;
    for(i=0; i<matches.size(); ++i) {
        FileMatch match = matches.at(i);
        printf("Got a match: qt3 file %s qt4Compat file %s\n", match.qt3.latin1(), match.qt4Compat.latin1());
    }
}

void processHeaderFileNames()
{
//    printf("Reading directory structure\n");
    QStringList qt3Headers = getFileNames(qt3IncludeDir, "*.h");
    QStringList qt4CompatHeaders = getFileNames(qt4Qt3SupportIncludeDir, "*.h");
    qt4CompatHeaders += getFileNames(qt4Qt3SupportIncludeDir, "*.h");
    QStringList qt4Headers = getHeadersRecursive(qt4IncludeDir);
    /*
        generate headerRename rule for all qxxx -> Q3Xxx renames
    */
    //printf("Searching for file matches\n");
    QList<FileMatch> matches = getFileMatches(qt3Headers, qt4CompatHeaders);
    foreach(FileMatch match, matches)
    {
        if(!overrideHeadersRename(match.qt3)) {
            xml["Rules"][ruleIndex].setAttribute("Type", "RenamedHeader");
            xml["Rules"][ruleIndex]["Qt3"]=match.qt3;
            xml["Rules"][ruleIndex]["Qt4"]=match.qt4Compat;
            ++ruleIndex;
        }
    }

    QList<ClassNameLibrary> classNames = getClassNamesAndLibrary(qt4IncludeDir);
    foreach(ClassNameLibrary className, classNames) {
        xml["Rules"][ruleIndex].setAttribute("Type", "Qt4Class");
        xml["Rules"][ruleIndex]["Name"]=className.className;
        xml["Rules"][ruleIndex]["Library"]=className.libraryName;
        ++ruleIndex;
    }

    /*
        generate a list of all public Qt3 headers
    */
    foreach(QString headerName, qt3Headers)
    {
        xml["Rules"][ruleIndex].setAttribute("Type", "qt3Header");
        xml["Rules"][ruleIndex]=headerName;
        ++ruleIndex;
    }

    /*
        generate a list of all public Qt4 headers
    */
    foreach(QString headerName, qt4Headers)
    {
        xml["Rules"][ruleIndex].setAttribute("Type", "qt4Header");
        xml["Rules"][ruleIndex]=headerName;
        ++ruleIndex;
    }

    xml["Rules"]["Count"] = QString("%1").arg(ruleIndex);
}

void printFileSymbolData(FileSymbol *sym)
{
    TreeDump dump(sym->tokenStream);
    dump.dumpNode(sym->ast);

}

/*
    Find Qt3 classes that now has a Qt compat class by parsing
    the QT3 and Qt4compat header files.
*/
QList<SymbolRename> getCompatClassRenames()
{
    QStringList qt3ClassNames;
    QStringList qt4CompatClassNames;
    QList<SymbolRename> renamedClasses;

    /*
        Get all public Qt3 class names
    */
    {
        Lexer lexer;
        Parser parser;
        pool p;

        FileSymbol *sym = new FileSymbol();
        sym->contents= gccPreprocess(includeAllQt3, QStringList() << ("-I" + qt3IncludeDir));
        sym->tokenStream = lexer.tokenize(sym);
        sym->ast = parser.parse(sym, &p);
        ClassNamesParser classNameParser(sym);
        qt3ClassNames = classNameParser.getClassNames();
    }
    /*
        Get all public Qt4 compat class names
    */
    {

        Lexer lexer;
        Parser parser;
        pool p;

        FileSymbol *sym = new FileSymbol();
        sym->contents=gccPreprocess(includeAllQt4Compat, QStringList()
                                                       <<  ("-I" +  qt4Qt3SupportIncludeDir)
                                                       <<  ("-I" + qt4IncludeDir)
                                                       <<  ("-I" + qt4QtIncludeDir));
        sym->tokenStream = lexer.tokenize(sym);
        sym->ast = parser.parse(sym, &p);

        ClassNamesParser classNameParser(sym);
        qt4CompatClassNames = classNameParser.getClassNames();
    }

//     printf("%-30s%s\n", "Qt3", "Qt4Compat");
     foreach(QString qt3Name, qt3ClassNames)
     {
//         printf("%-30s", qt3Name.latin1());
         foreach(QString qt4CompatName, qt4CompatClassNames)
         {
            QString modQt3Name = qt3Name;
            QString modQt4CompatName = qt4CompatName;
            if (modQt4CompatName[1]=='3') {
                modQt4CompatName.remove(1,1);

                if(qt3Name == modQt4CompatName)
                {
     //               printf("Found Match: %s -> %s\n", qt3Name.latin1(), qt4CompatName.latin1());
                    SymbolRename ren;
                    ren.from = qt3Name;
                    ren.to = qt4CompatName;
                    renamedClasses.append(ren);
                }
            }
         }
    }
    return renamedClasses;
}

/*
    Returns a list of all qt3 classes that inherits the Qt class
*/
QStringList getQtClassAncestors()
{
    Lexer lexer;
    Parser parser;
    pool p;

    FileSymbol *sym = new FileSymbol();
    sym->contents= gccPreprocess(includeAllQt3, QStringList() << ("-I" + qt3IncludeDir));
    sym->tokenStream = lexer.tokenize(sym);
    sym->ast = parser.parse(sym, &p);
    return QtClassAncestors(sym).getClassNames();
}

/*
    adds a InheritsQt rule for each qt 3 class that inherits the
    Qt class;
*/
void generateQtClassAncestorRules()
{
    QStringList classNames = getQtClassAncestors();
    foreach(QString className, classNames) {
        xml["Rules"][ruleIndex].setAttribute("Type", "InheritsQt");
        xml["Rules"][ruleIndex] = className;
        ++ruleIndex;
    }
    xml["Rules"]["Count"] = QString("%1").arg(ruleIndex);
}

/*
    Generates a list of SymbolRenames for symbols that have moved
    from the Qt class to the Qt namespace
*/
QList<SymbolRename> generateSymbolRenames(QStringList qt3Symbols, QStringList qt4Symbols)
{
    QList<SymbolRename> matchList;
    foreach(QString qt3Symbol, qt3Symbols) {
        foreach(QString qt4Symbol, qt4Symbols) {
            if(qt3Symbol==qt4Symbol) {
                SymbolRename rename;
           //     printf("Found Qt class/namespace symbol: %s\n", qt3Symbol.latin1());
                rename.from = "Qt::" + qt3Symbol;
                rename.to = "Qt::" + qt4Symbol;
                matchList.append(rename);
            }
        }
    }
    return matchList;
}
/*
    Output rename rules in renames to the xml file
*/
void writeRenameRules(QString ruleName, QList<SymbolRename> renames)
{
    foreach(SymbolRename ren, renames) {
        xml["Rules"][ruleIndex].setAttribute("Type", ruleName);
        xml["Rules"][ruleIndex]["Qt3"] = ren.from;
        xml["Rules"][ruleIndex]["Qt4"] = ren.to;
        ++ruleIndex;
    }
    xml["Rules"]["Count"] = QString("%1").arg(ruleIndex);

}
/*
    Output remove rules in renames to the xml file
*/
void writeRemoveRules(QString ruleName, QStringList removes)
{
    foreach(QString item, removes) {
        xml["Rules"][ruleIndex].setAttribute("Type", ruleName);
        xml["Rules"][ruleIndex] = item;;
        ++ruleIndex;
    }
    xml["Rules"]["Count"] = QString("%1").arg(ruleIndex);
}
/*
    Merges two SymbolRename lists, removes duplicates.
*/
QList<SymbolRename> mergeRemoveDuplicates(QList<SymbolRename> list1, QList<SymbolRename> list2)
{
    foreach(SymbolRename r, list2) {
        if(!list1.contains(r))
            list1.append(r);
    }
    return list1;
}

/*
    Generate rules for renamed and compat classes
*/
void generateClassRenameRules()
{
    QList<SymbolRename> docClassRenames =  GenDocRules(renamedClassesFileName).getSymbolRenames();
    QList<SymbolRename> autoClassRenames = getCompatClassRenames();
    QList<SymbolRename> allClassRenames = mergeRemoveDuplicates(docClassRenames, autoClassRenames);

    //remove manually overridden renames
    QMutableListIterator<SymbolRename> i(allClassRenames);
    while (i.hasNext()) {
        if(overrideClassRename(i.next().from))
            i.remove();
    }

    writeRenameRules("RenamedClass", allClassRenames);
}

/*
    Generate rules for renamed enum values, types and symbols defined in the Qt class / namespace
*/
void generateQtClassRenameRules()
{
    /*
       Renamed Enumerators
    */
    QList<SymbolRename> docEnumvalueRenames =  GenDocRules(renamedEnumvaluesFileName).getSymbolRenames();
    QList<SymbolRename> autoEnumvalueRenames = generateSymbolRenames(qtClassParser->getEnumerators(), qtNamespaceParser->getEnumerators());
    QList<SymbolRename> allEnumvalueRenames = mergeRemoveDuplicates(docEnumvalueRenames, autoEnumvalueRenames);

    writeRenameRules("RenamedEnumvalue", allEnumvalueRenames);

    /*
        Renamed Types
        Renamed Enum specifiers
    */
    QList<SymbolRename> docTypeRenames =  GenDocRules(renamedTypesFileName).getSymbolRenames();
    QList<SymbolRename> autoTypeRenames  = generateSymbolRenames(qtClassParser->getEnumSpecifiers(), qtNamespaceParser->getEnumSpecifiers());
                        autoTypeRenames += generateSymbolRenames(qtClassParser->getDeclarators(), qtNamespaceParser->getDeclarators());
    QList<SymbolRename> allTypeRenames = mergeRemoveDuplicates(docTypeRenames, autoTypeRenames);

    writeRenameRules("RenamedType", allTypeRenames);

    /*
        The rest of the renamed Qt class / namespace symbols
    */

    QList<SymbolRename> autoSymbolRenames  = generateSymbolRenames(qtClassParser->getQtSymbols(), qtNamespaceParser->getQtSymbols());
    autoSymbolRenames += GenDocRules(renamedStaticMethods).getSymbolRenames();

    QList<SymbolRename> finalSymbolRenames;
    foreach(SymbolRename qtSymbolRename, autoSymbolRenames) {
        if(!(allTypeRenames.contains(qtSymbolRename) || allEnumvalueRenames.contains(qtSymbolRename) )) {
            finalSymbolRenames.append(qtSymbolRename);
      //      printf("Found Qt symbol Rename: %s\n", qtSymbolRename.from.latin1());
        }
    }
     writeRenameRules("RenamedQtSymbol", finalSymbolRenames);
}

/*
    Generate rules for modified virtual functions
*/
void generateVirtualModifyRules()
{
    QList<SymbolRename> docRenames =  GenDocRules(modifiedVirtualFileName).getSymbolRenames();
    writeRenameRules("ModifiedVirtual", docRenames);
}
/*
    Generate rules for removed enum values
*/
void generateRemovedEnumvalueRules()
{
    QStringList docRemoves = GenDocRules(removedEnumvaluesFileName).getSymbolRemoves();
    writeRemoveRules("RemovedEnum", docRemoves);
}

/*
    Generate rules for removed types
*/
void generateRemovedTypeRules()
{
    QStringList docRemoves = GenDocRules(removedTypesFileName).getSymbolRemoves();
    writeRemoveRules("RemovedType", docRemoves);
}

/*
    Generate rules for removed virtual  functions
*/
void generateRemovedVirtualRules()
{
    QList<SymbolRename> docRemoves = GenDocRules(removedVirtualFileName).getSymbolRenames();
    writeRenameRules("RemovedVirtual", docRemoves);
}

/*
    Convinience function for adding a renamed header rule.
*/
void addRenamedHeaderRule(QString qt3header, QString qt4header)
{
    xml["Rules"][ruleIndex].setAttribute("Type", "RenamedHeader");
    xml["Rules"][ruleIndex]["Qt4"]=qt4header;
    xml["Rules"][ruleIndex]["Qt3"]=qt3header;
    ++ruleIndex;
}

/*
    Convinience function for adding a renamed class rule.
*/
void addRenamedClassRule(QString qt3class, QString qt4class)
{
    xml["Rules"][ruleIndex].setAttribute("Type", "RenamedClass");
    xml["Rules"][ruleIndex]["Qt4"]=qt4class;
    xml["Rules"][ruleIndex]["Qt3"]=qt3class;
    ++ruleIndex;
}

/*
    Convinience function for adding a need header rule.
*/
void addNeedHeaderRule(QString klass, QString header)
{
    xml["Rules"][ruleIndex].setAttribute("Type", "NeedHeader");
    xml["Rules"][ruleIndex]["Class"]=klass;
    xml["Rules"][ruleIndex]["Header"]=header;
    ++ruleIndex;
}

/*
    Convinience function for adding a need header rule.
*/
void addNeedHeaderRule(QString name)
{
    addNeedHeaderRule(name, name);
}

/*
    Add tests here to override renaming of classes.
    Return true if class should NOT be renamed
*/
bool overrideClassRename(QString className)
{
    if(className == "QPainter")
        return true;
    /*
        Uic/uic3 must generate code that uses QAction, not Q3Action.
        This is because of classes like QMenu and QMenuBar, which
        takes QAction arguments, and does not have a corresponding compat class
    */
    else if(className == "QAction")
        return true;

    return false;
}

/*
    Add tests here to override renaming of headers.
    Return true if class should NOT be renamed
*/
bool overrideHeadersRename(QString headerName)
{
    if(headerName == "QPainter" ||  headerName == "qpainter.h")
        return true;
    else if(headerName == "QAction" ||  headerName == "qaction.h")
        return true;
    return false;
}

/*
    Manual rules. Use this section to add rules manually to the q3porting.xml
    file. If you do so, don't forget to add a test to manualtests/qt3t4/qt3to4/.
*/
void generateManualRules()
{
    addRenamedClassRule("QGuardedPtr", "QPointer");

    //The headers for event classes (and some other classes )now needs
    //to be specifically included, so we add rules for that.
    addNeedHeaderRule("QCloseEvent");
    addNeedHeaderRule("QHelpEvent");
    addNeedHeaderRule("QDragMoveEvent");
    addNeedHeaderRule("QDropEvent");
    addNeedHeaderRule("QKeyEvent");
    addNeedHeaderRule("QClipboardEvent");
    addNeedHeaderRule("QHideEvent");
    addNeedHeaderRule("QDragLeaveEvent");
    addNeedHeaderRule("QWhatsThisClickedEvent");
    addNeedHeaderRule("QMoveEvent");
    addNeedHeaderRule("QFileOpenEvent");
    addNeedHeaderRule("QResizeEvent");
    addNeedHeaderRule("QContextMenuEvent");
    addNeedHeaderRule("QActionEvent");
    addNeedHeaderRule("QWSUpdateEvent");
    addNeedHeaderRule("QInputMethodEvent");
    addNeedHeaderRule("QShortcutEvent");
    addNeedHeaderRule("QDragEnterEvent");
    addNeedHeaderRule("QDragResponseEvent");
    addNeedHeaderRule("QToolBarChangeEvent");
    addNeedHeaderRule("QShowEvent");
    addNeedHeaderRule("QWSEvent");
    addNeedHeaderRule("QFocusEvent");
    addNeedHeaderRule("QInputEvent");
    addNeedHeaderRule("QStatusTipEvent");
    addNeedHeaderRule("QWheelEvent");
    addNeedHeaderRule("QMouseEvent");
    addNeedHeaderRule("QTabletEvent");
    addNeedHeaderRule("QPaintEvent");
    addNeedHeaderRule("QIconDragEvent");
    addNeedHeaderRule("QChildEvent");
    addNeedHeaderRule("QEvent");
    addNeedHeaderRule("QWinEventNotifier");
    addNeedHeaderRule("QTimerEvent");
    addNeedHeaderRule("QEventLoop");
    addNeedHeaderRule("QCustomEvent");

    addNeedHeaderRule("Q3PtrList");
    addNeedHeaderRule("Q3ValueList");
    addNeedHeaderRule("Q3MemArray");
    addNeedHeaderRule("Q3StrList");
    addNeedHeaderRule("Q3Painter");
    addNeedHeaderRule("QTextStream");
    addNeedHeaderRule("Q3Frame");
    addNeedHeaderRule("QBoxLayout");
    addNeedHeaderRule("QVBoxLayout");
    addNeedHeaderRule("QHBoxLayout");
    addNeedHeaderRule("QGridLayout");
    addNeedHeaderRule("Q3CString");
    addNeedHeaderRule("QPixmap");
    addNeedHeaderRule("QTranslator");
    addNeedHeaderRule("QLabel");
    addNeedHeaderRule("QImageIO");
    addNeedHeaderRule("QListBoxItem");
    addNeedHeaderRule("Q3Action");
    addNeedHeaderRule("qPixmapFromMimeSource", "q3mimefactory.h");

    // some old obsolete headers
    addRenamedHeaderRule("qkeycode.h", "qnamespace.h");
    addRenamedHeaderRule("qobjectlist.h", "qobject.h");
    addRenamedHeaderRule("qwidgetlist.h", "qwidget.h");

    // corresponds to the renamed classes
    addRenamedHeaderRule("qgrid.h", "qgridwidget.h");
    addRenamedHeaderRule("qvbox.h", "qvboxwidget.h");
    addRenamedHeaderRule("qhbox.h", "qhboxwidget.h");
    addRenamedHeaderRule("qiconset.h", "qicon.h");
    addRenamedHeaderRule("qwmatrix.h", "qmatrix.h");
    addRenamedHeaderRule("qguardedptr.h", "qpointer.h");

    xml["Rules"]["Count"] = QString("%1").arg(ruleIndex);
}

void writeLicense(QString &str)
{
    std::stringstream license;
    license << endl;
    license <<"<!--************************************************************************" << endl;
    license << "**" << endl;
    license << "** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved." << endl;
    license << "**" << endl;
    license << "** This file is part of the $MODULE$ of the Qt Toolkit." << endl;
    license << "**" << endl;
    license << "** $LICENSE$" << endl;
    license << "**" << endl;
    license << "** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE" << endl;
    license << "** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE." << endl;
    license << "**" << endl;
    license << "**************************************************************************-->";

    //insert past the first line (<?xml version="1.0" encoding="UTF-8"?>)
    str.insert(38, QString::fromAscii(license.str().c_str()));
}

void writeXmlFile()
{
    QString filename = "q3porting.xml";
    cout << "Writing xml to " << qPrintable(filename) << endl;
    QDomDocument doc = xml.toDomDocument();
    QString str = doc.toString(4);
    writeLicense(str);
    QFile xmlFile(filename);
    xmlFile.open(QIODevice::WriteOnly);
    QTextStream xmlStream(&xmlFile);
    xmlStream << str;
    xmlFile.close();
}


int main()
{
    QString baseDir = getBaseDir();
    QString qtDir = getQtDir();

    cout << "basedir: " << qPrintable(baseDir) << endl;
    cout << "qtdir:   " << qPrintable(qtDir) << endl;

    // Paths to qdoc files
    baseDocDir = qtDir + "/doc/src/";
    renamedClassesFileName =     baseDocDir + "porting4-renamedclasses.qdocinc";
    renamedEnumvaluesFileName =  baseDocDir + "porting4-renamedenumvalues.qdocinc";
    renamedTypesFileName =       baseDocDir + "porting4-renamedtypes.qdocinc";
    renamedStaticMethods =       baseDocDir + "porting4-renamedstatic.qdocinc";
    modifiedVirtualFileName =    baseDocDir + "porting4-modifiedvirtual.qdocinc";
    removedEnumvaluesFileName =  baseDocDir + "porting4-removedenumvalues.qdocinc";
    removedTypesFileName =       baseDocDir + "porting4-removedtypes.qdocinc";
    removedVirtualFileName =     baseDocDir + "porting4-removedvirtual.qdocinc";

    // Paths to qt include dirs
    qt3IncludeDir =       baseDir + "/qt-3/include";
    qt4Qt3SupportIncludeDir = qtDir + "/include/Qt3Support/";
    qt4IncludeDir =       qtDir + "/include/";
    qt4QtIncludeDir =     qtDir + "/include/Qt/";

    // Look at headers in qt3/include qnd qt4/include/qt-3support/,
    // genereate header rename rules
    cout << "Generating header rename rules" << endl;
    processHeaderFileNames();

    // Parse the qt3 headers and qt4 compat headers, get the names for classes that
    // has been renamed.
    cout << "Parsing qt3 headers and qt4 compat headers. Generating class rename rules." << endl;
    generateClassRenameRules();

    //  Parse the Qt3 Qt class and the Qt4 Qt namespace.
    cout << "Parsing Qt class and Qt namespace" << endl;
    QByteArray qtClassTranslationUnit = gccPreprocess(includeQtNamespace, QStringList() << ("-I" + qt3IncludeDir));
    qtClassParser = new QtClassParser(qtClassTranslationUnit);
    QByteArray qtNamespaceTranslationUnit = gccPreprocess(includeQtNamespace, QStringList()
                                                                            << ("-I" +  qt4Qt3SupportIncludeDir)
                                                                            << ("-I" + qt4IncludeDir)
                                                                            << ("-I" + qt4QtIncludeDir));
    qtNamespaceParser = new QtNamespaceParser(qtNamespaceTranslationUnit);

    // generate list of all clases that inherits qt
    generateQtClassAncestorRules();

    // Generate rename rules for stuff in the old qt class / new qt namespace
    generateQtClassRenameRules();

// These are not in use
//    generateVirtualModifyRules();
//    generateRemovedEnumvalueRules();
//    generateRemovedTypeRules();
//    generateRemovedVirtualRules();

    generateManualRules();

   // Write xml file;
    writeXmlFile();

    delete qtClassParser;
    delete qtNamespaceParser;
    return 0;
}
