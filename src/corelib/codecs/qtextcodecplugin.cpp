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

#include "qtextcodecplugin.h"
#include "qstringlist.h"

/*!
    \class QTextCodecPlugin
    \brief The QTextCodecPlugin class provides an abstract base for custom QTextCodec plugins.
    \reentrant
    \ingroup plugins

    The text codec plugin is a simple plugin interface that makes it
    easy to create custom text codecs that can be loaded dynamically
    into applications.

    Writing a text codec plugin is achieved by subclassing this base
    class, reimplementing the pure virtual functions names(),
    aliases(), createForName(), mibEnums() and createForMib(), and
    exporting the class with the Q_EXPORT_PLUGIN() macro. See \l{How
    to Create Qt Plugins} for details.

    See the \l{http://www.iana.org/assignments/character-sets}{IANA
    character-sets encoding file} for more information on mime
    names and mib enums.
*/

/*!
    \fn QStringList QTextCodecPlugin::names() const

    Returns the list of MIME names and aliases supported by this plugin.

    \sa createForName()
*/

/*!
    \fn QList<QByteArray> QTextCodecPlugin::aliases() const

    Returns the list of aliases supported by this plugin.
*/

/*!
    \fn QTextCodec *QTextCodecPlugin::createForName(const QByteArray &name)

    Creates a QTextCodec object for the codec called \a name.

    \sa names()
*/


/*!
    \fn QList<int> QTextCodecPlugin::mibEnums() const

    Returns the list of mib enums supported by this plugin.

    \sa createForMib()
*/

/*!
    \fn QTextCodec *QTextCodecPlugin::createForMib(int mib);

    Creates a QTextCodec object for the mib enum \a mib.

    (See \link
    ftp://ftp.isi.edu/in-notes/iana/assignments/character-sets the
    IANA character-sets encoding file\endlink for more information)

    \sa mibEnums()
*/

/*!
    Constructs a text codec plugin with the given \a parent. This is
    invoked automatically by the Q_EXPORT_PLUGIN() macro.
*/
QTextCodecPlugin::QTextCodecPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the text codec plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QTextCodecPlugin::~QTextCodecPlugin()
{
}

QStringList QTextCodecPlugin::keys() const
{
    QStringList keys;
    QList<QByteArray> list = names();
    list += aliases();
    for (int i = 0; i < list.size(); ++i)
        keys += QString::fromLatin1(list.at(i));
    QList<int> mibs = mibEnums();
    for (int i = 0; i < mibs.count(); ++i)
        keys += QLatin1String("MIB: ") + QString::number(mibs.at(i));
    return keys;
}

QTextCodec *QTextCodecPlugin::create(const QString &name)
{
    if (name.startsWith(QLatin1String("MIB: ")))
        return createForMib(name.mid(4).toInt());
    return createForName(name.toLatin1());
}
