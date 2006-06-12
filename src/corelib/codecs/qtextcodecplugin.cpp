/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextcodecplugin.h"
#include "qstringlist.h"

#ifndef QT_NO_TEXTCODECPLUGIN

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
    exporting the class with the Q_EXPORT_PLUGIN2() macro. See \l{How
    to Create Qt Plugins} for details.

    See the \l{http://www.iana.org/assignments/character-sets}{IANA
    character-sets encoding file} for more information on mime
    names and mib enums.
*/

/*!
    \fn QStringList QTextCodecPlugin::names() const

    Returns the list of MIME names supported by this plugin.

    If a codec has several names, the extra names are returned by aliases().

    \sa createForName(), aliases()
*/

/*!
    \fn QList<QByteArray> QTextCodecPlugin::aliases() const

    Returns the list of aliases supported by this plugin.
*/

/*!
    \fn QTextCodec *QTextCodecPlugin::createForName(const QByteArray &name)

    Creates a QTextCodec object for the codec called \a name. The \a name
    must come from the list of encodings returned by names(). Encoding
    names are case sensitive.

    Example:

    \code
        QList<QByteArray> MyCodecPlugin::names() const
        {
            return QList<QByteArray> << "IBM01140" << "hp15-tw";
        }

        QTextCodec *MyCodecPlugin::createForName(const QByteArray &name)
        {
            if (name == "IBM01140") {
                return new Ibm01140Codec;
            } else if (name == "hp15-tw") {
                return new Hp15TwCodec;
            }
            return 0;
        }
    \endcode

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

    See \l{http://www.iana.org/assignments/character-sets}{the
    IANA character-sets encoding file} for more information.

    \sa mibEnums()
*/

/*!
    Constructs a text codec plugin with the given \a parent. This is
    invoked automatically by the Q_EXPORT_PLUGIN2() macro.
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

#endif // QT_NO_TEXTCODECPLUGIN
