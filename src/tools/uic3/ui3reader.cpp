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
#include "ui4.h"
#include "widgetinfo.h"

#include <qfile.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <qdebug.h>

/// ### share these constants with qt/designer
enum { BOXLAYOUT_DEFAULT_MARGIN = 11 };
enum { BOXLAYOUT_DEFAULT_SPACING = 6 };


bool Ui3Reader::isMainWindow = false;

QString Ui3Reader::getComment(const QDomNode& n)
{
    QDomNode child = n.firstChild();
    while (!child.isNull()) {
        if (child.toElement().tagName() == "comment")
            return child.toElement().firstChild().toText().data();
        child = child.nextSibling();
    }
    return QString::null;
}

QString Ui3Reader::mkBool(bool b)
{
    return b? "true" : "false";
}

QString Ui3Reader::mkBool(const QString& s)
{
    return mkBool(s == "true" || s == "1");
}

bool Ui3Reader::toBool(const QString& s)
{
    return s == "true" || s.toInt() != 0;
}

QString Ui3Reader::fixString(const QString &str, bool encode)
{
    QString s;
    if (!encode) {
        s = str;
        s.replace("\\", "\\\\");
        s.replace("\"", "\\\"");
        s.replace("\r", "");
        s.replace("\n", "\\n\"\n\"");
    } else {
        QByteArray utf8 = str.utf8();
        const int l = utf8.length();
        for (int i = 0; i < l; ++i)
            s += "\\x" + QString::number((uchar)utf8[i], 16);
    }

    return "\"" + s + "\"";
}

QString Ui3Reader::trcall(const QString& sourceText, const QString& comment)
{
    if (sourceText.isEmpty() && comment.isEmpty())
        return "QString()";

    QString t = trmacro;
    bool encode = false;
    if (t.isNull()) {
        t = "tr";
        for (int i = 0; i < (int) sourceText.length(); i++) {
            if (sourceText[i].unicode() >= 0x80) {
                t = "trUtf8";
                encode = true;
                break;
            }
        }
    }

    if (comment.isEmpty()) {
        return t + "(" + fixString(sourceText, encode) + ")";
    } else {
        return t + "(" + fixString(sourceText, encode) + ", " +
               fixString(comment, encode) + ")";
    }
}

QString Ui3Reader::mkStdSet(const QString& prop)
{
    return QString("set") + prop[0].toUpper() + prop.mid(1);
}

void Ui3Reader::init()
{
    outputFileName = QString::null;
    trmacro = QString::null;
    nofwd = false;

    fileName = QString::null;
    writeFunctImpl = true;
    defMargin = BOXLAYOUT_DEFAULT_MARGIN;
    defSpacing = BOXLAYOUT_DEFAULT_SPACING;
    externPixmaps = false;
    indent = "    "; // default indent

    item_used = cg_used = pal_used = 0;

    layouts.clear();
    layouts << "hbox" << "vbox" << "grid";
    tags = layouts;
    tags << "widget";

    nameOfClass = QString::null;
    namespaces.clear();
    bareNameOfClass = QString::null;
}

QDomElement Ui3Reader::parse(const QDomDocument &doc)
{
    root = doc.firstChild().toElement();
    widget = QDomElement();

    pixmapLoaderFunction = getPixmapLoaderFunction(doc.firstChild().toElement());
    nameOfClass = getFormClassName(doc.firstChild().toElement());

    uiFileVersion = doc.firstChild().toElement().attribute("version");
    stdsetdef = toBool(doc.firstChild().toElement().attribute("stdsetdef"));

    if (doc.firstChild().isNull() || doc.firstChild().firstChild().isNull())
        return widget;

    QDomElement e = doc.firstChild().firstChild().toElement();
    while (!e.isNull()) {
        if (e.tagName() == QLatin1String("widget")) {
            widget = e;
        } else if (e.tagName() == QLatin1String("pixmapinproject")) {
            externPixmaps = true;
        } else if (e.tagName() == QLatin1String("layoutdefaults")) {
            defSpacing = e.attribute("spacing", defSpacing.toString());
            defMargin = e.attribute("margin", defMargin.toString());
        } else if (e.tagName() == QLatin1String("layoutfunctions")) {
            defSpacing = e.attribute("spacing", defSpacing.toString());
            bool ok;
            defSpacing.toInt(&ok);
            if (!ok) {
                QString buf = defSpacing.toString();
                defSpacing = buf.append("()");
            }
            defMargin = e.attribute("margin", defMargin.toString());
            defMargin.toInt(&ok);
            if (!ok) {
                QString buf = defMargin.toString();
                defMargin = buf.append("()");
            }
        }
        e = e.nextSibling().toElement();
    }

    return widget;
}

Ui3Reader::Ui3Reader(QTextStream &outStream)
   : out(outStream), trout(&languageChangeBody)
{
}

void Ui3Reader::generate(const QString &uiHeaderFn, const QString &fn, const QString &outputFn,
          QDomDocument doc, bool decl, bool subcl, const QString &trm,
          const QString& subClass, bool omitForwardDecls)
{
    init();

    uiHeaderFile = uiHeaderFn;
    fileName = fn;
    outputFileName = outputFn;
    trmacro = trm;
    nofwd = omitForwardDecls;

    QDomElement e = parse(doc);

    if (nameOfClass.isEmpty())
        nameOfClass = getObjectName(e);
    namespaces = nameOfClass.split("::");
    bareNameOfClass = namespaces.last();
    namespaces.removeLast();

    if (subcl) {
        if (decl)
            createSubDecl(e, subClass);
        else
            createSubImpl(e, subClass);
    } else {
        if (decl)
            createFormDecl(e);
        else
            createFormImpl(e);
    }

}

void Ui3Reader::generateUi4(const QString &fn, const QString &outputFn, QDomDocument doc)
{
    init();

    fileName = fn;
    outputFileName = outputFn;

    DomUI *ui = generateUi4(parse(doc));
    if (!ui)
        return;

    if (pixmapLoaderFunction.size())
        ui->setElementPixmapFunction(pixmapLoaderFunction);

    QDomDocument outputDoc;
    outputDoc.appendChild(ui->write(outputDoc));
    out << outputDoc.toString(2);

    delete ui;
}

void Ui3Reader::setTrMacro(const QString &trmacro)
{
    this->trmacro = trmacro;
}

void Ui3Reader::setForwardDeclarationsEnabled(bool b)
{
    nofwd = !b;
}

void Ui3Reader::setOutputFileName(const QString &fileName)
{
    outputFileName = fileName;
}

/*! Extracts a pixmap loader function from \a e
 */
QString Ui3Reader::getPixmapLoaderFunction(const QDomElement& e)
{
    QDomElement n;
    for (n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement()) {
        if (n.tagName() == "pixmapfunction")
            return n.firstChild().toText().data();
    }
    return QString::null;
}


/*! Extracts the forms class name from \a e
 */
QString Ui3Reader::getFormClassName(const QDomElement& e)
{
    QDomElement n;
    QString cn;
    for (n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement()) {
        if (n.tagName() == "class") {
            QString s = n.firstChild().toText().data();
            int i;
            while ((i = s.indexOf(' ')) != -1)
                s[i] = '_';
            cn = s;
        }
    }
    return cn;
}

/*! Extracts a class name from \a e.
 */
QString Ui3Reader::getClassName(const QDomElement& e)
{
    QString s = e.attribute("class");
    if (s.isEmpty() && e.tagName() == QLatin1String("toolbar"))
        s = QLatin1String("QToolBar");
    else if (s.isEmpty() && e.tagName() == QLatin1String("menubar"))
        s = QLatin1String("QMenuBar");

    return fixClassName(s);
}

/*! Returns true if database framework code is generated, else false.
*/

bool Ui3Reader::isFrameworkCodeGenerated(const QDomElement& e)
{
    QDomElement n = getObjectProperty(e, "frameworkCode");
    if (n.attribute("name") == "frameworkCode" &&
         !DomTool::elementToVariant(n.firstChild().toElement(), QCoreVariant(true)).toBool())
        return false;
    return true;
}

/*! Extracts an object name from \a e. It's stored in the 'name'
 property.
 */
QString Ui3Reader::getObjectName(const QDomElement& e)
{
    QDomElement n = getObjectProperty(e, "name");
    if (n.firstChild().toElement().tagName() == "cstring")
        return n.firstChild().toElement().firstChild().toText().data();
    return QString::null;
}

/*! Extracts an layout name from \a e. It's stored in the 'name'
 property of the preceeding sibling (the first child of a QLayoutWidget).
 */
QString Ui3Reader::getLayoutName(const QDomElement& e)
{
    QDomElement p = e.parentNode().toElement();
    QString name;

    if (getClassName(p) != "QLayoutWidget")
        name = "Layout";

    QDomElement n = getObjectProperty(p, "name");
    if (n.firstChild().toElement().tagName() == "cstring") {
        name.prepend(n.firstChild().toElement().firstChild().toText().data());
        return name.split("::").last();
    }
    return e.tagName();
}


QString Ui3Reader::getDatabaseInfo(const QDomElement& e, const QString& tag)
{
    QDomElement n;
    QDomElement n1;
    int child = 0;
    // database info is a stringlist stored in this order
    if (tag == "connection")
        child = 0;
    else if (tag == "table")
        child = 1;
    else if (tag == "field")
        child = 2;
    else
        return QString::null;
    n = getObjectProperty(e, "database");
    if (n.firstChild().toElement().tagName() == "stringlist") {
            // find correct stringlist entry
            QDomElement n1 = n.firstChild().firstChild().toElement();
            for (int i = 0; i < child && !n1.isNull(); ++i)
                n1 = n1.nextSibling().toElement();
            if (n1.isNull())
                return QString::null;
            return n1.firstChild().toText().data();
    }
    return QString::null;
}

static const char* const ColorRole[] = {
    "Foreground", "Button", "Light", "Midlight", "Dark", "Mid",
    "Text", "BrightText", "ButtonText", "Base", "Background", "Shadow",
    "Highlight", "HighlightedText", "Link", "LinkVisited", 0
};


/*!
  Creates a colorgroup with name \a name from the color group \a cg
 */
void Ui3Reader::createColorGroupImpl(const QString& name, const QDomElement& e)
{
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QString color;

    Color white;
    white.init(255, 255, 255);

    Color black;
    white.init(0, 0, 0);

    while (!n.isNull()) {
        if (n.tagName() == "color") {
            r++;
            Color col = DomTool::readColor(n);
            color = "QColor(%1, %2, %3)";
            color = color.arg(col.red).arg(col.green).arg(col.blue);
            if (col == white)
                color = "white";
            else if (col == black)
                color = "black";
            if (n.nextSibling().toElement().tagName() != "pixmap") {
                out << indent << name << ".setColor(QColorGroup::" << ColorRole[r] << ", " << color << ");" << endl;
            }
        } else if (n.tagName() == "pixmap") {
            QString pixmap = n.firstChild().toText().data();
            if (!pixmapLoaderFunction.isEmpty()) {
                pixmap.prepend(pixmapLoaderFunction + "(" + QString(externPixmaps ? "\"" : ""));
                pixmap.append(QString(externPixmaps ? "\"" : "") + ")");
            }
            out << indent << name << ".setBrush(QColorGroup::"
                << ColorRole[r] << ", QBrush(" << color << ", " << pixmap << "));" << endl;
        }
        n = n.nextSibling().toElement();
    }
}

/*!
  Auxiliary function to load a color group. The colorgroup must not
  contain pixmaps.
 */
ColorGroup Ui3Reader::loadColorGroup(const QDomElement &e)
{
    ColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    Color col;
    while (!n.isNull()) {
        if (n.tagName() == "color") {
            r++;
            col = DomTool::readColor(n);
            cg.append(qMakePair(r, col));
        }
        n = n.nextSibling().toElement();
    }
    return cg;
}

/*!  Returns true if the widget properties specify that it belongs to
  the database \a connection and \a table.
*/

bool Ui3Reader::isWidgetInTable(const QDomElement& e, const QString& connection, const QString& table)
{
    QString conn = getDatabaseInfo(e, "connection");
    QString tab = getDatabaseInfo(e, "table");
    if (conn == connection && tab == table)
        return true;
    return false;
}

/*!
  Registers all database connections, cursors and forms.
*/

void Ui3Reader::registerDatabases(const QDomElement& e)
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    nl = e.parentNode().toElement().elementsByTagName("widget");
    for (i = 0; i < (int) nl.length(); ++i) {
        n = nl.item(i).toElement();
        QString conn = getDatabaseInfo(n, "connection" );
        QString tab = getDatabaseInfo(n, "table" );
        QString fld = getDatabaseInfo(n, "field" );
        if (!conn.isNull()) {
            dbConnections += conn;
            if (!tab.isNull()) {
                dbCursors[conn] += tab;
                if (!fld.isNull())
                    dbForms[conn] += tab;
            }
        }
    }
}

/*!
  Registers an object with name \a name.

  The returned name is a valid variable identifier, as similar to \a
  name as possible and guaranteed to be unique within the form.

  \sa registeredName(), isObjectRegistered()
 */
QString Ui3Reader::registerObject(const QString& name)
{
    if (objectNames.isEmpty()) {
        // some temporary variables we need
        objectNames += "img";
        objectNames += "item";
        objectNames += "cg";
        objectNames += "pal";
    }

    QString result = name;
    int i;
    while ((i = result.indexOf(' ')) != -1 ) {
        result[i] = '_';
    }

    if (objectNames.contains(result)) {
        int i = 2;
        while (objectNames.contains(result + "_" + QString::number(i)))
            i++;
        result += "_";
        result += QString::number(i);
    }
    objectNames += result;
    objectMapper.insert(name, result);
    return result;
}

/*!
  Returns the registered name for the original name \a name
  or \a name if \a name  wasn't registered.

  \sa registerObject(), isObjectRegistered()
 */
QString Ui3Reader::registeredName(const QString& name)
{
    if (!objectMapper.contains(name))
        return name;
    return objectMapper[name];
}

/*!
  Returns whether the object \a name was registered yet or not.
 */
bool Ui3Reader::isObjectRegistered(const QString& name)
{
    return objectMapper.contains(name);
}

/*!
  Unifies the entries in stringlist \a list. Should really be a QStringList feature.
 */
QStringList Ui3Reader::unique(const QStringList& list)
{
    if (list.isEmpty())
        return list;

    QStringList result;
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        if (!result.contains(*it))
            result += *it;
    }
    return result;
}

bool Ui3Reader::isLayout(const QString& name) const
{
    return layoutObjects.contains(name);
}

