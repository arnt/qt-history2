/****************************************************************************
**
** Implementation of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsettings.h"

#include <qglobal.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qcleanuphandler.h>

#include <stdlib.h>

// for now
#define QSETTINGS_DEFAULT_PATH "/usr/local/etc/qt/settings"




/*!
  \class QSettings qsettings.h
  \brief The QSettings class provides persistent application settings.

  \ingroup misc

  QSettings provides a platform independent way to load and save settings
  for applications.

  On Unix systems, QSettings uses small files to store settings.  On Windows
  systems, QSettings uses the registry.

  Data is accessed through keys.  A key is a Unicode string, similar to UNIX
  file paths.  A key must begin with a slash, must not end with a slash, must
  not contain "//" and must contain at least two slashes (including the leading
  slash).
*/




static QStringList *searchPaths = 0;
QCleanupHandler<QStringList> qsettings_path_cleanup;


static void initSearchPaths()
{
    if (searchPaths)
	return;

    searchPaths = new QStringList;
    Q_CHECK_PTR(searchPaths);
    qsettings_path_cleanup.add(searchPaths);

    searchPaths->append(QString(QSETTINGS_DEFAULT_PATH));
    searchPaths->append(QString(getenv("HOME")) + "/.qt/");
}


// QSettingsGroup is a map of key/value pairs
class QSettingsGroup : public QMap<QString,QString>
{
public:
    QSettingsGroup();

    bool modified;
};


QSettingsGroup::QSettingsGroup()
    : modified(FALSE)
{
}


// QSettingsHeading is a map of heading/group pairs
class QSettingsHeading : public QMap<QString,QSettingsGroup>
{
public:
    QSettingsHeading::Iterator git;
    void read(const QString &);
    void parseLine(const QString &);
};


inline void QSettingsHeading::read(const QString &filename)
{
    if (! QFileInfo(filename).exists())
	return;

    QFile file(filename);
    if (! file.open(IO_ReadOnly)) {
	qWarning("QSettings: failed to open file '%s'", filename.latin1());
	return;
    }

    git = end();

    QString line;
    QTextStream stream(&file);
    while (! stream.atEnd())
	parseLine(stream.readLine());

    git = end();

    file.close();
}


inline void QSettingsHeading::parseLine(const QString &l)
{
    QString line = l.stripWhiteSpace();
    if (line.isEmpty())
	// empty line... we'll allow it
	return;

    if (line[0] == QChar('[')) {
	QString gname = line;

	gname = gname.remove(0, 1);
	if (gname[(int)gname.length() - 1] == QChar(']'))
	    gname = gname.remove(gname.length() - 1, 1);

	git = replace(gname, QSettingsGroup());
    } else {
	if (git == end()) {
	    qWarning("QSettings: line '%s' out of group", line.latin1());

	    return;
	}

	QStringList list = QStringList::split('=', line);
	if (list.count() != 2 && line.find('=') == -1) {
	    qWarning("QSettings: malformed line '%s' in group '%s'",
		     line.latin1(), git.key().latin1());

	    return;
	}

	(*git).insert(list[0].stripWhiteSpace(), list[1].stripWhiteSpace());
    }
}


class QSettingsPrivate
{
public:
    QSettingsPrivate();

    QSettingsGroup readGroup();
    void removeGroup(const QString &);
    void writeGroup(const QString &, const QString &);
    QDateTime modificationTime();

    QMap<QString,QSettingsHeading> headings;
    QString group;
    QString heading;
    bool modified;
};


QSettingsPrivate::QSettingsPrivate()
    : modified(FALSE)
{
}


inline QSettingsGroup QSettingsPrivate::readGroup()
{
    QSettingsHeading hd;
    QSettingsGroup grp;

    QMap<QString,QSettingsHeading>::Iterator headingsit = headings.find(heading);
    if (headingsit != headings.end())
	hd = *headingsit;

    QSettingsHeading::Iterator grpit = hd.find(group);
    if (grpit == hd.end()) {
	initSearchPaths();
      	QStringList::Iterator it = searchPaths->begin();
	while (it != searchPaths->end()) {
	    QString fn((*it++) + "/" + heading + "rc");
	    if (! hd.contains(fn + "cached")) {
		hd.read(fn);
		hd.insert(fn + "cached", QSettingsGroup());
	    }
	}

	headings.replace(heading, hd);

	grpit = hd.find(group);
	if (grpit != hd.end())
	    grp = *grpit;
    } else
	grp = *grpit;

    return grp;
}


inline void QSettingsPrivate::removeGroup(const QString &key) {
    QSettingsHeading hd;
    QSettingsGroup grp;
    bool found = FALSE;

    QMap<QString,QSettingsHeading>::Iterator headingsit = headings.find(heading);
    if (headingsit != headings.end())
	hd = *headingsit;

    QSettingsHeading::Iterator grpit = hd.find(group);
    if (grpit == hd.end()) {
	initSearchPaths();
      	QStringList::Iterator it = searchPaths->begin();
	while (it != searchPaths->end()) {
	    QString fn((*it++) + "/" + heading + "rc");
	    if (! hd.contains(fn + "cached")) {
		hd.read(fn);
		hd.insert(fn + "cached", QSettingsGroup());
	    }
	}

	headings.replace(heading, hd);

	grpit = hd.find(group);
	if (grpit != hd.end()) {
	    found = TRUE;
	    grp = *grpit;
	}
    } else {
	found = TRUE;
	grp = *grpit;
    }

    if (found) {
	grp.remove(key);
	hd.replace(group, grp);
	headings.replace(heading, hd);

	modified = TRUE;
    }
}


inline void QSettingsPrivate::writeGroup(const QString &key, const QString &value)
{
    QSettingsHeading hd;
    QSettingsGroup grp;

    QMap<QString,QSettingsHeading>::Iterator headingsit = headings.find(heading);
    if (headingsit != headings.end())
	hd = *headingsit;

    QSettingsHeading::Iterator grpit = hd.find(group);
    if (grpit == hd.end()) {
	initSearchPaths();
      	QStringList::Iterator it = searchPaths->begin();
	while (it != searchPaths->end()) {
	    QString fn((*it++) + "/" + heading + "rc");
	    if (! hd.contains(fn + "cached")) {
		hd.read(fn);
		hd.insert(fn + "cached", QSettingsGroup());
	    }
	}

	headings.replace(heading, hd);

	grpit = hd.find(group);
	if (grpit != hd.end())
	    grp = *grpit;
    } else
	grp = *grpit;

    grp.modified = TRUE;
    grp.replace(key, value);
    hd.replace(group, grp);
    headings.replace(heading, hd);

    modified = TRUE;
}


inline QDateTime QSettingsPrivate::modificationTime()
{
    QSettingsHeading hd = headings[heading];
    QSettingsGroup grp = hd[group];

    QDateTime datetime;

    initSearchPaths();
    QStringList::Iterator it = searchPaths->begin();
    while (it != searchPaths->end()) {
	QFileInfo fi((*it++) + "/" + heading + "rc");
	if (fi.exists() && fi.lastModified() > datetime)
	    datetime = fi.lastModified();
    }

    return datetime;
}


/*!
  Inserts \a path into the settings search path.  The search path list will
  be used when trying to determine a suitable filename for reading and
  writing settings files.  By default, there are two entries in the search
  path:

  <ul>
  <li> <i>\<prefix\></i>/lib/qt/settings/ - where <i>\<prefix\></i> is the
  directory where Qt was installed.
  <li> <i>\<home\></i>/.qt/ - where <i>\<home\></i> is the user's home
  directory.
  </ul>

  All insertions into the search path will go before <i>\<home\></i>/.qt/.  For
  example:

  \code
  ...
  QSettings::insertSearchPath("/opt/mysoft/share/cfg");
  QSettings::insertSearchPath("/opt/mysoft/share/myapp/cfg");
  ...
  \endcode

  Will result in a list of:

  <ul>
  <li><i>\<prefix\></i>/lib/qt/settings
  <li>/opt/mysoft/share/cfg
  <li>/opt/mysoft/share/myapp/cfg
  <li><i>\<home\></i>/.qt
  </ul>
*/
void QSettings::insertSearchPath(const QString &path)
{
    initSearchPaths();

    QStringList::Iterator it = searchPaths->find(searchPaths->last());
    if (it != searchPaths->end()) {
	searchPaths->insert(it, path);
    }
}


/*!
  Removes all occurrences or \a path from the settings search path.  Note
  that the two default search paths cannot be removed.

  \sa insertSearchPath()
*/
void QSettings::removeSearchPath(const QString &path)
{
    initSearchPaths();

    if (path == searchPaths->first() || path == searchPaths->last())
	return;

    searchPaths->remove(path);
}


/*!
  Creates a settings object.
*/
QSettings::QSettings()
{
    d = new QSettingsPrivate;
    Q_CHECK_PTR(d);
}

/*!
  Destroys the settings object.  All modifications made to the object
  will automatically be written to disk by calling sync().

  \sa sync()
*/
QSettings::~QSettings()
{
    sync();

    if (d)
	delete d;
}


/*!
  Writes all modifications to the settings to disk.  If any errors are
  encountered, this function returns FALSE, otherwise it will return TRUE.
*/
bool QSettings::sync()
{
    if (! d->modified)
	// fake success
	return TRUE;

    bool success = TRUE;
    QMap<QString,QSettingsHeading>::Iterator it = d->headings.begin();

    while (it != d->headings.end()) {
	// determine filename
	QSettingsHeading hd(*it);
	QSettingsHeading::Iterator hdit = hd.begin();
	QFile file;

	initSearchPaths();
	QStringList::Iterator pit = searchPaths->begin();
	while (pit != searchPaths->end()) {
	    QFileInfo di(*pit);
	    QFileInfo fi((*pit++) + "/" + it.key() + "rc");

	    if ((fi.exists() && fi.isFile() && fi.isWritable()) ||
		(di.isDir() && di.isWritable())) {
		file.setName(fi.filePath());
		break;
	    }
	}

	it++;

	if (file.name().isNull() || file.name().isEmpty()) {

#ifdef QT_CHECK_STATE
	    qWarning("QSettings::sync: filename is null/empty");
#endif // QT_CHECK_STATE

	    success = FALSE;
	    continue;
	}

	if (! file.open(IO_WriteOnly)) {

#ifdef QT_CHECK_STATE
	    qWarning("QSettings::sync: failed to open '%s' for writing",
		     file.name().latin1());
#endif // QT_CHECK_STATE

	    success = FALSE;
	    continue;
	}

	// spew to file
	QTextStream stream(&file);
	stream.setEncoding(QTextStream::UnicodeUTF8);

	while (hdit != hd.end()) {
	    stream << "[" << hdit.key() << "]" << endl;

	    QSettingsGroup grp(*hdit);
	    QSettingsGroup::Iterator grpit = grp.begin();

	    while (grpit != grp.end()) {
		stream << grpit.key() << "=" << grpit.data() << endl;
		grpit++;
	    }

	    stream << endl;

	    hdit++;
	}

	if (file.status() != IO_Ok) {

#ifdef QT_CHECK_STATE
	    qWarning("QSettings::sync: error at end of write");
#endif // QT_CHECK_STATE

	    success = FALSE;
	}

	file.close();
    }

    d->modified = FALSE;

    return success;
}


/*!
  Reads the entry specified by \a key, and returns a bool.
  If \a ok non-null, *ok is set to TRUE if there are no errors, and
  FALSE if the entry could not be read.

  \sa writeEntry(), removeEntry()
*/
bool QSettings::readBoolEntry(const QString &key, bool *ok )
{
    QString value = readEntry( key, ok );
 
    if (value.lower() == "true")
	return TRUE;
    else if (value.lower() == "false")
	return FALSE;

    qWarning("QString::readBoolEntry: '%s' is not 'true' or 'false'", value.latin1());
    if ( ok )
	*ok = FALSE;
    return FALSE;
}


/*!
  Reads the entry specified by \a key, and returns a double.
  If \a ok non-null, *ok is set to TRUE if there are no errors, and
  FALSE if the entry could not be read.

  \sa writeEntry(), removeEntry()
*/
double QSettings::readDoubleEntry(const QString &key, bool *ok )
{
    QString value = readEntry( key, ok );
    return value.toDouble();
}


/*!
  Reads the entry specified by \a key, and returns a integer.
  If \a ok non-null, *ok is set to TRUE if there are no errors, and
  FALSE if the entry could not be read.

  \sa writeEntry(), removeEntry()
*/
int QSettings::readNumEntry(const QString &key, bool *ok )
{
    QString value = readEntry( key, ok );
    return value.toInt();
}


/*!
  Reads the entry specified by \a key, and returns a QString.
  If \a ok non-null, *ok is set to TRUE if there are no errors, and
  FALSE if the entry could not be read.

  \sa writeEntry(), removeEntry()
*/
QString QSettings::readEntry(const QString &key, bool *ok )
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::readEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return QString::null;
    }
#endif // QT_CHECK_STATE

    QString realkey;

    if (key[0] == '/') {
	// parse our key
	QStringList list(QStringList::split('/', key));

#ifdef QT_CHECK_STATE
	if (list.count() < 2) {
	    qWarning("QSettings::readEntry: invalid key '%s'", key.latin1());
	    if ( ok )
		*ok = FALSE;
	    return QString::null;
	}
#endif // QT_CHECK_STATE

	if (list.count() == 2) {
	    d->heading = list[0];
	    d->group = "General";
	    realkey = list[1];
	} else {
	    d->heading = list[0];
	    d->group = list[1];

	    // remove the group from the list
	    list.remove(list.at(1));
	    // remove the heading from the list
	    list.remove(list.at(0));

	    realkey = list.join("/");
	}
    } else
    	realkey = key;

    QSettingsGroup grp = d->readGroup();
    QString retval = grp[realkey];

    if ( ok )
	*ok = TRUE;
    return retval;
}


/*!
  Reads the string entry specified by \a key.  The \a separator is used to
  create a QStringList by calling QStringList::split (\a separator, data).
  If \a ok non-null, *ok is set to TRUE if there are no errors, and
  FALSE if the entry could not be read.

  \sa writeEntry(), removeEntry(), QStringList::split()
*/
QStringList QSettings::readListEntry(const QString &key, const QChar &separator, bool *ok )
{
    QString value = readEntry( key, ok );
    if ( ok && !*ok )
	return QStringList();

    return QStringList::split(separator, value);
}


/*!
  Writes the entry specified by \a key with the boolean \a value, replacing
  any previous setting.

  If an error occurs, this functions returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, bool value)
{
    QString s(value ? "true" : "false");
    return writeEntry(key, s);
}


/*!
  Writes the entry specified by \a key with the double \a value, replacing
  any previous setting.

  If an error occurs, this functions returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, double value)
{
    QString s(QString::number(value));
    return writeEntry(key, s);
}


/*!
  Writes the entry specified by \a key with the integer \a value, replacing
  any previous setting.

  If an error occurs, this functions returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, int value)
{
    QString s(QString::number(value));
    return writeEntry(key, s);
}


/*!
  Writes the entry specified by \a key with the string-literal \a value,
  replacing any previous setting.  If \a value is zero-length or null, the
  entry is replaced by an empty setting.

  \e NOTE: This function is provided because some compilers use the
  writeEntry (const QString &, bool) overload for this code:
  writeEntry ("/foo/bar", "baz")

  If an error occurs, this functions returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const char *value)
{
    return writeEntry(key, QString(value));
}


/*!
  Writes the entry specified by \a key with the string \a value, replacing
  any previous setting.  If \a value is zero-length or null, the entry is
  replaced by an empty setting.

  If an error occurs, this functions returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QString &value)
{
    // NOTE: we *do* allow value to be a null/empty string

#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::writeEntry: invalid null/empty key.");

	return FALSE;
    }
#endif // QT_CHECK_STATE

    QString realkey;

    if (key[0] == '/') {
	// parse our key
	QStringList list(QStringList::split('/', key));

#ifdef QT_CHECK_STATE
	if (list.count() < 2) {
	    qWarning("QSettings::writeEntry: invalid key '%s'", key.latin1());

	    return FALSE;
	}
#endif // QT_CHECK_STATE

	if (list.count() == 2) {
	    d->heading = list[0];
	    d->group = "General";
	    realkey = list[1];
	} else {
	    d->heading = list[0];
	    d->group = list[1];

	    // remove the group from the list
	    list.remove(list.at(1));
	    // remove the heading from the list
	    list.remove(list.at(0));

	    realkey = list.join("/");
	}
    } else
	realkey = key;

    d->writeGroup(realkey, value);

    return TRUE;
}


/*!
  Writes the entry specified by \a key with the string-list \a value,
  replacing any previous setting.  If \a value is empty, the entry is
  replaced by an empty setting.

  If an error occurs, this functions returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QStringList &value,
			   const QChar &separator)
{
    QString s(value.join(separator));
    return writeEntry(key, s);
}


/*!
  Removes the entry specified by \a key.  This function returns FALSE if
  an error occured and returns TRUE otherwise.

  \sa readEntry(), writeEntry()
*/
bool QSettings::removeEntry(const QString &key)
{

#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::removeEntry: invalid null/empty key.");

	return FALSE;
    }
#endif // QT_CHECK_STATE

    QString realkey;

    if (key[0] == '/') {
	// parse our key
	QStringList list(QStringList::split('/', key));

#ifdef QT_CHECK_STATE
	if (list.count() < 2) {
	    qWarning("QSettings::removeEntry: invalid key '%s'", key.latin1());

	    return FALSE;
	}
#endif // QT_CHECK_STATE

	if (list.count() == 2) {
	    d->heading = list[0];
	    d->group = "General";
	    realkey = list[1];
	} else {
	    d->heading = list[0];
	    d->group = list[1];

	    // remove the group from the list
	    list.remove(list.at(1));
	    // remove the heading from the list
	    list.remove(list.at(0));

	    realkey = list.join("/");
	}
    } else
	realkey = key;

    d->removeGroup(realkey);

    return TRUE;
}


/*!
  This function returns the time of last modification for \a key.
*/
QDateTime QSettings::lastModficationTime(const QString &key)
{

#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::lastModficationTime: invalid null/empty key.");

	return QDateTime();
    }
#endif // QT_CHECK_STATE

    if (key[0] == '/') {
	// parse our key
	QStringList list(QStringList::split('/', key));

#ifdef QT_CHECK_STATE
	if (list.count() < 2) {
	    qWarning("QSettings::lastModficationTime: invalid key '%s'", key.latin1());

	    return QDateTime();
	}
#endif // QT_CHECK_STATE

	if (list.count() == 2) {
	    d->heading = list[0];
	    d->group = "General";
	} else {
	    d->heading = list[0];
	    d->group = list[1];
	}
    }

    return d->modificationTime();
}
