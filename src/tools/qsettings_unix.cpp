/****************************************************************************
** $Id$
**
** Implementation of QSettings class
**
** Created: 2000.06.26
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsettings.h"
#ifndef QT_NO_SETTINGS

#include "qdir.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qmap.h"
#include "qtextstream.h"
#include "qcleanuphandler.h"
#include "qregexp.h"


// for now
#define QSETTINGS_DEFAULT_PATH_SUFFIX "/etc/settings"




/*!
  \class QSettings qsettings.h
  \brief The QSettings class provides persistent platform-independent application settings.

  \ingroup io
  \ingroup misc
  \mainclass

  On Unix systems, QSettings uses text files to store settings. On
  Windows systems, QSettings uses the system registry.

  Each setting comprises an identifying key and the data associated with
  the key. A key is a unicode string which consists of \e two or more
  subkeys. A subkey is a slash, '/', followed by one or more unicode
  characters (excluding slashes, newlines, carriage returns and equals,
  '=', signs). The associated data, called the entry or value, may be a
  boolean, an integer, a double, a string or a list of strings. Entry
  strings may contain any unicode characters.

  If you want to save and restore the entire desktop's settings, i.e.
  which applications are running, use QSettings to save the settings
  for each individual application and QSessionManager to save the
  desktop's session.

    Example settings:
    \code
    /MyCompany/MyApplication/background color
    /MyCompany/MyApplication/foreground color
    /MyCompany/MyApplication/geometry/x
    /MyCompany/MyApplication/geometry/y
    /MyCompany/MyApplication/geometry/width
    /MyCompany/MyApplication/geometry/height
    /MyCompany/MyApplication/recent files/1
    /MyCompany/MyApplication/recent files/2
    /MyCompany/MyApplication/recent files/3
    \endcode
    Each line above is a complete key, made up of subkeys.

    A typical usage pattern for application startup:
    \code
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/MyCompany" );
    // No search path needed for Unix; see notes further on.
    // Use default values if the keys don't exist
    QString bgColor = settings.readEntry( "/MyApplication/background color", "white" );
    int width = settings.readNumEntry( "/MyApplication/geometry/width", 640 );
    // ...
    \endcode

    A typical usage pattern for application exit or 'save preferences':
    \code
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/MyCompany" );
    // No search path needed for Unix; see notes further on.
    settings.writeEntry( "/MyApplication/background color", bgColor );
    settings.writeEntry( "/MyApplication/geometry/width", width );
    // ...
    \endcode

    You can get a list of entry-holding keys by calling entryList(), and
    a list of key-holding keys using subkeyList().

    \code
    QStringList keys = entryList( "/MyApplication" );
    // keys contains 'background color' and 'foreground color'.

    QStringList keys = entryList( "/MyApplication/recent files" );
    // keys contains '1', '2' and '3'.

    QStringList subkeys = subkeyList( "/MyApplication" );
    // subkeys contains 'geometry' and 'recent files'

    QStringList subkeys = subkeyList( "/MyApplication/recent files" );
    // subkeys is empty.
    \endcode

    If you wish to use a different search path call insertSearchPath()
    as often as necessary to add your preferred paths. Call
    removeSearchPath() to remove any unwanted paths.

    Since settings for Windows are stored in the registry there are size
    limits as follows:
    \list
    \i A subkey may not exceed 255 characters.
    \i An entry's value may not exceed 16,300 characters.
    \i All the values of a key (for example, all the 'recent files'
    subkeys values), may not exceed 64,535 characters.
    \endlist

    These limitations are not enforced on Unix.

    \section1 Notes for Unix Applications

    There is no universally accepted place for storing application
    settings under Unix. In the examples the settings file will be
    searched for in the following directories:
    \list 1
    \i $QTDIR/etc
    \i /opt/MyCompany/share/etc
    \i /opt/MyCompany/share/MyApplication/etc
    \i $HOME/.qt
    \endlist
    When reading settings the files are searched in the order shown
    above, with later settings overriding earlier settings. Files for
    which the user doesn't have read permission are ignored. When saving
    settings QSettings works forwards in the order shown above writing
    to the first settings file for which the user has write permission.
    ($QTDIR is the directory where Qt was installed.)

    If you want to put the settings in a particular place in the
    filesystem you could do this:
    \code
    settings.insertSearchPath( QSettings::Unix, "/opt/MyCompany/share" );
    \endcode

    But in practice you may prefer not to use a search path for Unix.
    For example the following code:
    \code
    settings.writeEntry( "/MyApplication/geometry/width", width );
    \endcode
    will end up writing the "geometry/width" setting to the file
    \c{$HOME/.qt/myapplicationrc} (assuming that the application is
    being run by an ordinary user, i.e. not by root).

    For cross-platform applications you should ensure that the Windows
    size limitations are not exceeded.
*/

/*!
    \enum QSettings::System

    \value Mac (reserved for future use)
    \value Unix Unix, Linux and Unix-like execution environments
    \value Windows Windows execution environments
*/


static QStringList *searchPaths = 0;
QCleanupHandler<QStringList> qsettings_path_cleanup;


static void initSearchPaths()
{
    if (searchPaths)
	return;

    searchPaths = new QStringList;
    Q_CHECK_PTR(searchPaths);
    qsettings_path_cleanup.add(&searchPaths);

    QDir dir(QDir::homeDirPath() + "/.qt/");
    if (! dir.exists()) {
	if (! dir.mkdir(dir.path()))
	    qWarning("QSettings: error creating %s", dir.path().latin1());
    }

#ifndef QT_INSTALL_PREFIX
#  define QT_INSTALL_PREFIX "/usr/local/qt"
#endif // QT_INSTALL_PREFIX

    QString defpath(QT_INSTALL_PREFIX);
    defpath += QSETTINGS_DEFAULT_PATH_SUFFIX;
    searchPaths->append(defpath);
    searchPaths->append(dir.path());
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
    void parseLine(QTextStream &);
};


void QSettingsHeading::read(const QString &filename)
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
    stream.setEncoding(QTextStream::UnicodeUTF8);
    while (! stream.atEnd())
	parseLine(stream);

    git = end();

    file.close();
}


void QSettingsHeading::parseLine(QTextStream &stream)
{
    QString line = stream.readLine();
    if (line.isEmpty())
	// empty line... we'll allow it
	return;

    if (line[0] == QChar('#'))
	// commented line
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

	int i = line.find('=');
       	if (i == -1) {
	    qWarning("QSettings: malformed line '%s' in group '%s'",
		     line.latin1(), git.key().latin1());
	    return;
	} else {
	    QString key, value;
	    key = line.left(i);
	    value = "";
	    bool esc=TRUE;
	    i++;
	    while (esc) {
		esc = FALSE;
		for ( ; i < (int)line.length(); i++ ) {
		    if ( esc ) {
			if ( line[i] == 'n' )
			    value.append('\n'); // escaped newline
			else if ( line[i] == '0' )
			    value = QString::null; // escaped empty string
			else
			    value.append(line[i]);
			esc = FALSE;
		    } else if ( line[i] == '\\' )
			esc = TRUE;
		    else
			value.append(line[i]);
		}
		if ( esc ) {
		    // Backwards-compatiblity...
		    // still escaped at EOL - manually escaped "newline"
		    if (stream.atEnd()) {
			qWarning("QSettings: reached end of file, expected continued line");
			break;
		    }
		    value.append('\n');
		    line = stream.readLine();
		    i = 0;
		}
	    }

	    (*git).insert(key, value);
	}
    }
}


class QSettingsPrivate
{
public:
    QSettingsPrivate();

    QSettingsGroup readGroup();
    bool removeGroup(const QString &);
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


QSettingsGroup QSettingsPrivate::readGroup()
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
	    QString filebase = heading.lower().replace(QRegExp("\\s+"), "_");
	    QString fn((*it++) + "/" + filebase + "rc");
	    if (! hd.contains(fn + "cached")) {
		hd.read(fn);
		hd.insert(fn + "cached", QSettingsGroup());
	    }
	}

	headings.replace(heading, hd);

	grpit = hd.find(group);
	if (grpit != hd.end())
	    grp = *grpit;
    } else if (hd.count() != 0)
	grp = *grpit;

    return grp;
}


bool QSettingsPrivate::removeGroup(const QString &key)
{
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
	    QString filebase = heading.lower().replace(QRegExp("\\s+"), "_");
	    QString fn((*it++) + "/" + filebase + "rc");
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
    } else if (hd.count() != 0) {
	found = TRUE;
	grp = *grpit;
    }

    if (found) {
	grp.remove(key);

	if (grp.count() > 0)
	    hd.replace(group, grp);
	else
	    hd.remove(group);

	if (hd.count() > 0)
	    headings.replace(heading, hd);
	else
	    headings.remove(heading);

	modified = TRUE;
    }

    return found;
}


void QSettingsPrivate::writeGroup(const QString &key, const QString &value)
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
	    QString filebase = heading.lower().replace(QRegExp("\\s+"), "_");
	    QString fn((*it++) + "/" + filebase + "rc");
	    if (! hd.contains(fn + "cached")) {
		hd.read(fn);
		hd.insert(fn + "cached", QSettingsGroup());
	    }
	}

	headings.replace(heading, hd);

	grpit = hd.find(group);
	if (grpit != hd.end())
	    grp = *grpit;
    } else if (hd.count() != 0)
	grp = *grpit;


    QString v = value;
    if ( v.isNull() ) {
	v = "\\0"; // escape null string
    } else {
	v.replace(QRegExp("\\"), "\\\\"); // escape backslash
	v.replace(QRegExp("\n"), "\\n"); // escape newlines
    }
    grp.modified = TRUE;
    grp.replace(key, v);
    hd.replace(group, grp);
    headings.replace(heading, hd);

    modified = TRUE;
}


QDateTime QSettingsPrivate::modificationTime()
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
  Inserts \a path into the settings search path. The semantics of \a
  path depends on the system \a s.

  When \a s is \e Windows and the execution environment is \e not
  Windows the function does nothing. Similarly when \a s is \e Unix and
  the execution environment is \e not Unix the function does nothing.

  When \a s is \e Windows, and the execution environment is Windows, the
  search path list will be used as the first subfolder of the "Software"
  folder in the registry.

    When reading settings the folders are searched forwards from the first
    folder (listed below) to the last, with later settings overriding
    settings found earlier, and ignoring any folders for which the user
    doesn't have read permission.
  \list 1
  \i HKEY_CURRENT_USER/Software/MyCompany/MyApplication
  \i HKEY_CURRENT_USER/Software/MyApplication
  \i HKEY_LOCAL_MACHINE/Software/MyCompany/MyApplication
  \i HKEY_LOCAL_MACHINE/Software/MyApplication
  \endlist

  \code
  QSettings settings;
  settings.insertSearchPath( QSettings::Windows, "/MyCompany" );
  settings.writeEntry( "/MyApplication/Tip of the day", TRUE );
  \endcode
    The code above will write the subkey "Tip of the day" into the \e first
    of the registry folders listed below that is found and for which the
    user has write permission.
  \list 1
  \i HKEY_LOCAL_MACHINE/Software/MyApplication
  \i HKEY_LOCAL_MACHINE/Software/MyCompany/MyApplication
  \i HKEY_CURRENT_USER/Software/MyApplication
  \i HKEY_CURRENT_USER/Software/MyCompany/MyApplication
  \endlist

  When \a s is \e Unix, and the execution environment is Unix, the
  search path list will be used when trying to determine a suitable
  filename for reading and writing settings files. By default, there are
  two entries in the search path:

  \list 1
  \i $QTDIR/etc - where $QTDIR is the directory where Qt was installed.
  \i $HOME/.qt/ - where $HOME is the user's home directory.
  \endlist

  All insertions into the search path will go before $HOME/.qt/.
  For example:
  \code
  QSettings settings;
  settings.insertSearchPath( QSettings::Unix, "/opt/MyCompany/share/etc" );
  settings.insertSearchPath( QSettings::Unix, "/opt/MyCompany/share/MyApplication/etc" );
  // ...
  \endcode
  Will result in a search path of:
  \list 1
  \i $QTDIR/etc
  \i /opt/MyCompany/share/etc
  \i /opt/MyCompany/share/MyApplication/etc
  \i $HOME/.qt
  \endlist
    When reading settings the files are searched in the order shown
    above, with later settings overriding earlier settings. Files for
    which the user doesn't have read permission are ignored. When saving
    settings QSettings works forwards in the order shown above writing
    to the first settings file for which the user has write permission.
    ($QTDIR is the directory where Qt was installed.)

  Settings under Unix are stored in files whose names are based on the
  first subkey of the key (not including the search path). The algorithm
  for creating names is essentially: lowercase the first subkey, replace
  spaces with underscores and add 'rc', e.g.
  <tt>/MyCompany/MyApplication/background color</tt> will be stored in
  <tt>myapplicationrc</tt> (assuming that <tt>/MyCompany</tt> is part of
  the search path).

  \sa removeSearchPath()

*/
void QSettings::insertSearchPath( System s, const QString &path)
{
    if ( s != Unix )
	return;
    initSearchPaths();

    QStringList::Iterator it = searchPaths->find(searchPaths->last());
    if (it != searchPaths->end()) {
	searchPaths->insert(it, path);
    }
}


/*!
  Removes all occurrences of \a path (using exact matching) from the
  settings search path for system \a s. Note that the default search
  paths cannot be removed.

  \sa insertSearchPath()
*/
void QSettings::removeSearchPath( System s, const QString &path)
{
    if ( s != Unix )
	return;

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
  Destroys the settings object.  All modifications made to the settings
  will automatically be saved.

*/
QSettings::~QSettings()
{
    sync();

    delete d;
}


/*! \internal
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
	    QString filebase = it.key().lower().replace(QRegExp("\\s+"), "_");
	    QFileInfo di(*pit);
	    QFileInfo fi((*pit++) + "/" + filebase + "rc");

	    if ((fi.exists() && fi.isFile() && fi.isWritable()) ||
		(! fi.exists() && di.isDir() && di.isWritable())) {
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
	    if ((*hdit).count() > 0) {
		stream << "[" << hdit.key() << "]" << endl;

		QSettingsGroup grp(*hdit);
		QSettingsGroup::Iterator grpit = grp.begin();

		while (grpit != grp.end()) {
		    stream << grpit.key() << "=" << grpit.data() << endl;
		    grpit++;
		}

		stream << endl;
	    }

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
  Reads the entry specified by \a key, and returns a bool, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to TRUE if the key was read, FALSE
  otherwise.

  \sa readEntry(), readNumEntry(), readDoubleEntry(), writeEntry(), removeEntry()
*/
bool QSettings::readBoolEntry(const QString &key, bool def, bool *ok )
{
    QString value = readEntry( key, QString::null, ok );

    if (value.lower() == "true")
	return TRUE;
    else if (value.lower() == "false")
	return FALSE;

    if (! value.isEmpty())
	qWarning("QString::readBoolEntry: '%s' is not 'true' or 'false'",
		 value.latin1());
    if ( ok )
	*ok = FALSE;
    return def;
}


/*!
  Reads the entry specified by \a key, and returns a double, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to TRUE if the key was read, FALSE
  otherwise.

  \sa readEntry(), readNumEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/
double QSettings::readDoubleEntry(const QString &key, double def, bool *ok )
{
    QString value = readEntry( key, QString::number(def), ok );
    return value.toDouble();
}


/*!
  Reads the entry specified by \a key, and returns an integer, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to TRUE if the key was read, FALSE
  otherwise.

  \sa readEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/
int QSettings::readNumEntry(const QString &key, int def, bool *ok )
{
    QString value = readEntry( key, QString::number( def ), ok );
    return value.toInt();
}


/*!
  Reads the entry specified by \a key, and returns a QString, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to TRUE if the key was read, FALSE
  otherwise.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/
QString QSettings::readEntry(const QString &key, const QString &def, bool *ok )
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::readEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
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
	    return def;
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
    if ( retval.isNull() )
	retval = def;

    if ( ok )
	*ok = TRUE;
    return retval;
}


#if !defined(Q_NO_BOOL_TYPE)
/*!
    Writes the boolean entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and FALSE is
    returned; otherwise TRUE is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, bool value)
{
    QString s(value ? "true" : "false");
    return writeEntry(key, s);
}
#endif


/*!
    \overload
    Writes the double entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and FALSE is
    returned; otherwise TRUE is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, double value)
{
    QString s(QString::number(value));
    return writeEntry(key, s);
}


/*!
    \overload
    Writes the integer entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and FALSE is
    returned; otherwise TRUE is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, int value)
{
    QString s(QString::number(value));
    return writeEntry(key, s);
}


/*!
    \internal

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
    \overload
    Writes the string entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value. If \a value is an empty string or a null string the key's
    value will be an empty string.

    If an error occurs the settings are left unchanged and FALSE is
    returned; otherwise TRUE is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
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
  Removes the entry specified by \a key.

    Returns TRUE if the entry existed and was removed, FALSE otherwise.

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

    return d->removeGroup(realkey);
}


/*!
  Returns a list of the keys which contain entries under \a key. Does \e
  not return any keys that contain keys.

    Example settings:
    \code
    /MyCompany/MyApplication/background color
    /MyCompany/MyApplication/foreground color
    /MyCompany/MyApplication/geometry/x
    /MyCompany/MyApplication/geometry/y
    /MyCompany/MyApplication/geometry/width
    /MyCompany/MyApplication/geometry/height
    \endcode
    \code
    QStringList keys = entryList( "/MyApplication" );
    \endcode
    \c keys contains 'background color' and 'foreground color'. It does
    not contain 'geometry' because this key contains keys not entries.

    To access the geometry values could either use subkeyList() to read
    the keys and then read each entry, or simply read each entry
    directly by specifying its full key, e.g.
    "/MyCompany/MyApplication/geometry/y".

  \sa subkeyList()
*/
QStringList QSettings::entryList(const QString &key) const
{

#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::listEntries: invalid null/empty key.");

	return QStringList();
    }
#endif // QT_CHECK_STATE

    QString realkey;
    if (key[0] == '/') {
	// parse our key
	QStringList list(QStringList::split('/', key));

#ifdef QT_CHECK_STATE
	if (list.count() < 1) {
	    qWarning("QSettings::listEntries: invalid key '%s'", key.latin1());

	    return QStringList();
	}
#endif // QT_CHECK_STATE

	if (list.count() == 1) {
	    d->heading = list[0];
	    d->group = "General";
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

    int start = realkey.length();

    QSettingsGroup grp = d->readGroup();
    QSettingsGroup::Iterator it = grp.begin();
    QStringList ret;
    QString itkey;
    while (it != grp.end()) {
	itkey = it.key();
	it++;

	if (start > 0 && itkey.left(start) != realkey)
	    continue;
	else if (itkey.find('/', start) != -1)
	    continue;

	ret << itkey;
    }

    return ret;
}


/*!
  Returns a list of the keys which contain keys under \a key. Does \e
  not return any keys that contain entries.

    Example settings:
    \code
    /MyCompany/MyApplication/background color
    /MyCompany/MyApplication/foreground color
    /MyCompany/MyApplication/geometry/x
    /MyCompany/MyApplication/geometry/y
    /MyCompany/MyApplication/geometry/width
    /MyCompany/MyApplication/geometry/height
    /MyCompany/MyApplication/recent files/1
    /MyCompany/MyApplication/recent files/2
    /MyCompany/MyApplication/recent files/3
    \endcode
    \code
    QStringList keys = subkeyList( "/MyApplication" );
    \endcode
    \c keys contains 'geometry' and 'recent files'. It does not contain
    'background color' or 'foreground color' because they are keys which
    contain entries not keys. To get a list of keys that have values
    rather than subkeys use entryList().

  \sa entryList()
*/
QStringList QSettings::subkeyList(const QString &key) const
{

#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::listSubkeys: invalid null/empty key.");

	return QStringList();
    }
#endif // QT_CHECK_STATE

    QString realkey;
    if (key[0] == '/') {
	// parse our key
	QStringList list(QStringList::split('/', key));

#ifdef QT_CHECK_STATE
	if (list.count() < 1) {
	    qWarning("QSettings::listSubkeys: invalid key '%s'", key.latin1());

	    return QStringList();
	}
#endif // QT_CHECK_STATE

	if (list.count() == 1) {
	    d->heading = list[0];
	    d->group = "General";
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

    int start = realkey.length();

    QSettingsGroup grp = d->readGroup();
    QSettingsGroup::Iterator it = grp.begin();
    QStringList ret;
    QString itkey;
    while (it != grp.end()) {
	itkey = it.key();
	it++;

	if (start > 0 && itkey.left(start) != realkey)
	    continue;
	else if (itkey.find('/', start) == -1)
	    continue;

	ret << itkey;
    }

    return ret;
}


/*!
    \internal

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

#endif //QT_NO_SETTINGS
