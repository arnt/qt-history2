/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "feature.h"
#include <QTextStream>
#include <QRegExp>
#include <QLibraryInfo>
#include <QFileInfo>

QMap<QString, Feature*> Feature::instances;

Feature* Feature::getInstance(const QString &key)
{
    QString ukey = key.toUpper();
    if (!instances.contains(ukey))
        instances[ukey] = new Feature(ukey);
    return instances[ukey];
}

Feature::~Feature()
{
    delete d;
}

void Feature::clear()
{
    foreach (Feature *f, instances.values())
        delete f;
    instances.clear();
}

static QString listToHtml(const QString &title, const QStringList &list)
{
    if (list.isEmpty())
	return QString();

    QString str;
    QTextStream stream(&str);

    stream << "<h3>" << title << ":</h3>";
    stream << "<ul>";
    foreach (QString l, list)
	stream << "<li>" << l << "</li>";
    stream << "</ul>";

    return str;
}

static QString listToHtml(const QString &title, const QList<Feature*> &list)
{
    QStringList stringlist;
    foreach (Feature *f, list) {
        QString s("[%3] <a href=\"feature://%1\">%2</a>");
        s = s.arg(f->key()).arg(f->key());
        s = s.arg(f->selectable() && f->enabled() ? "On" : "Off");
        stringlist << s;
    }
    return listToHtml(title, stringlist);
}

static QString linkify(const QString &src)
{
    static QRegExp classRegexp("\\b(Q[\\w]+)");
    QString docRoot = QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    QString result = src;
    int pos = 0;
    while ((pos = classRegexp.indexIn(result, pos)) != -1) {
        QString className = classRegexp.cap(1);
        QString file = docRoot + "/html/" + className.toLower() + ".html";
        QFileInfo info(file);
        if (info.isFile()) {
            QString link = QString("<a href=\"file://%1\">%2</a>")
                           .arg(file).arg(className);
            result.replace(pos, className.length(), link);
            pos += link.length();
        } else {
            pos += className.length();
        }
    }

    return result;
}

QString Feature::toHtml() const
{
    QString str;
    QTextStream stream(&str);

    stream << "<h2><font size=\"+2\" color=\"darkBlue\">"
           << key() << "</font></h2>"
	   << "<h2><font size=\"+2\">" << title() << "</font></h2>";
    if (!description().isEmpty())
        stream << "<p>" << description() << "</p>";
    stream << listToHtml("Section", QStringList(section()))
	   << listToHtml("Requires", dependencies())
	   << listToHtml("Required for", supports())
	   << listToHtml("See also", relations());

    return linkify(str);
}

Feature::Feature(const QString &key) : d(new FeaturePrivate(key)) {}

void Feature::setTitle(const QString &title)
{
    d->title = title;
}

void Feature::setSection(const QString &section)
{
    d->section = section;
}

void Feature::setDescription(const QString &description)
{
    d->description = description;
}

void Feature::addRelation(const QString &key)
{
    d->relations.insert(getInstance(key));
}

void Feature::setRelations(const QStringList &keys)
{
    foreach(QString key, keys)
        if (key != "???")
            addRelation(key);
}

QList<Feature*> Feature::relations() const
{
    return d->relations.toList();
}

void Feature::addDependency(const QString &key)
{
    Feature *f = getInstance(key);
    d->dependencies.insert(f);
    f->d->supports.insert(this);
}

void Feature::setDependencies(const QStringList &keys)
{
    foreach(QString key, keys)
        addDependency(key);
}

QList<Feature*> Feature::dependencies() const
{
    return d->dependencies.toList();
}

QList<Feature*> Feature::supports() const
{
    return d->supports.toList();
}

/*
    Returns a html formatted detailed description of this Feature.
*/
QString Feature::getDocumentation() const
{
    return QString() + "<h2>" + d->title + "</h2>";

}

void Feature::setEnabled(bool on)
{
    if (on == d->enabled)
        return;

    d->enabled = on;
    foreach (Feature *f, supports())
	f->updateSelectable();
    emit changed();
}

/*
  Update whether this feature should be selectable.
  A feature is selectable if all its dependencies are enabled.
*/
void Feature::updateSelectable()
{
    bool selectable = true;
    foreach (Feature *f, dependencies())
        if (!f->selectable() || !f->enabled())
            selectable = false;
    if (selectable != d->selectable) {
        d->selectable = selectable;
        foreach (Feature *f, supports())
            f->updateSelectable();
        emit changed();
    }
}

