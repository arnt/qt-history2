#include "qsettings.h"
#include "private/qcoresettings_p.h"

#include "qcoreapplication.h"
#include "qrect.h"
#include "qpoint.h"
#include "qcolor.h"
#include "qsize.h"

QSETTINGS_DEFINE_GUI_PARSER_FUNCTIONS

/*!
    \class QSettings
    \brief The QSettings class provides persistent platform-indendent GUI application settings.

    \ingroup io
    \ingroup misc
    \mainclass

    Users normally expect an application to remember its settings
    (window sizes and positions, options, etc.) across sessions. This
    information is often stored in the system registry on Windows,
    and in XML preferences files on Mac OS X. On X11, in the absense
    of a standard, many applications (including the KDE applications)
    use INI text files.

    QSettings is an abstraction around these technologies, enabling
    you to save and restore application settings in a portable
    manner.

    QSettings inherits most of its functionality from QCoreSettings.
    If your application links against the QtGui library, you can use
    QSettings rather than QCoreSettings. QSettings's API is based on
    QVariant instead of QCoreVariant, which allows you to save
    GUI-related types such as QRect, QSize, and QColor.

    See the \l QCoreSettings documentation for a detailed description
    of how to use this class.
*/

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    Example:
    \code
        QSettings settings("www.technopro.co.jp", "Facturo-Pro");
    \endcode

    The scope is Qt::UserScope and the format is Qt::NativeFormat.
*/
QSettings::QSettings(const QString &organization, const QString &application,
                     QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                                 organization, application,
                                                 variantToStringGuiImpl,
                                                 stringToVariantGuiImpl), parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If \a scope is Qt::UserScope, the QSettings object searches
    user-specific settings first, before it seaches system-wide
    settings as a fallback. If \a scope is Qt::SystemScope, the
    QSettings object ignores user-specific settings and provides
    access to system-wide settings.

    The storage format is always Qt::NativeFormat.

    If no application name is given, the QSettings object will only
    access the organization-wide locations.
*/
QSettings::QSettings(Qt::SettingsScope scope, const QString &organization,
                     const QString &application, QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(Qt::NativeFormat, scope,
                                                 organization, application,
                                                 variantToStringGuiImpl,
                                                 stringToVariantGuiImpl), parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If \a scope is Qt::UserScope, the QSettings object searches
    user-specific settings first, before it seaches system-wide
    settings as a fallback. If \a scope is Qt::SystemScope, the
    QSettings object ignores user-specific settings and provides
    access to system-wide settings.

    If \a format is Qt::NativeFormat, the native API is used for
    storing settings. If \a format is Qt::IniFormat, the INI format
    is used.

    If no application name is given, the QSettings object will only
    access the organization-wide locations.
*/
QSettings::QSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                     const QString &organization, const QString &application, QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(format, scope, organization, application,
                                                 variantToStringGuiImpl,
                                                 stringToVariantGuiImpl), parent)

{
}

/*!
    Constructs a QSettings object for accessing the settings stored
    in the file called \a fileName, with parent \a parent. If the
    file doesn't already exist, it is created.

    If \a format is Qt::NativeFormat, the meaning of \a fileName
    depends on the platform. On Unix/X11, \a fileName is the name of
    an INI file. On Mac OS X, \a fileName is the name of a .plist
    file. On Windows, \a fileName is a path in the system registry.

    If \a format is Qt::IniFormat, \a fileName is the name of an INI
    file.
*/
QSettings::QSettings(const QString &fileName, Qt::SettingsFormat format, QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(fileName, format,
                                                 variantToStringGuiImpl,
                                                 stringToVariantGuiImpl), parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application and organization set previously with a call to
    QCoreApplication::setProductInfo().

    The scope is Qt::UserScope and the format is Qt::NativeFormat.

    The code

    \code
        QSettings settings("www.technopro.co.jp", "Facturo-Pro");
    \endcode

    is equivalent to

    \code
        qApp->setProductInfo("www.technopro.co.jp", "Facturo-Pro");
        QSettings settings;
    \endcode

    If QApplication::setProductInfo() has not been previously called,
    the QSettings object will not be able to read or write any settings,
    and status() will return \c AccessError.
*/
QSettings::QSettings(QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                                 QCoreApplication::instance()->organization(),
                                                 QCoreApplication::instance()->application(),
                                                 variantToStringGuiImpl,
                                                 stringToVariantGuiImpl), parent)
{
}

/*!
    Destroys the QSettings object. Any unsaved changes will be
    written to permanent storage at that point.
*/
QSettings::~QSettings()
{
}

/*! \fn void QSettings::setValue(const QString &key, const QVariant &value)

    Sets the value of setting \a key to \a value.

    If the key already exists, the previous value is overwritten.

    This function is the same as QCoreSettings::setValue(), except
    that the second parameter is of type QVariant instead of
    QCoreVariant.

    \sa value()
*/

/*! \fn QVariant QSettings::value(const QString &key, const QVariant &defaultValue) const

    Returns the value for setting \a key. If the setting doesn't
    exist, returns \a defaultValue.

    If no default value is specified, a default QVariant is returned.

    This function is the same as QCoreSettings::setValue(), except
    that the second parameter and the return value are of type
    QVariant instead of QCoreVariant.

    \sa setValue()
*/

/*! \fn bool QSettings::writeEntry(const QString &key, bool value)

    Sets the value of setting \a key to \a value.

    Use setValue() instead.
*/

/*! \fn bool QSettings::writeEntry(const QString &key, double value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, int value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const char *value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const QString &value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const QStringList &value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const QStringList &value, QChar separator)

    \overload

    Use setValue(\a key, \a value) instead. You don't need \a separator.
*/

/*! \fn QStringList QSettings::readListEntry(const QString &key, bool *ok = 0)

    Returns the value of setting \a key converted to a QStringList.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        QStringList list = settings.readListEntry("recentFiles", &ok);
    \newcode
        bool ok = settings.contains("recentFiles");
        QStringList list = settings.value("recentFiles").toStringList();
    \endcode
*/

/*! \fn QStringList QSettings::readListEntry(const QString &key, QChar separator, bool *ok)

    Returns the value of setting \a key converted to a QStringList.
    \a separator is ignored.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        QStringList list = settings.readListEntry("recentFiles", ":", &ok);
    \newcode
        bool ok = settings.contains("recentFiles");
        QStringList list = settings.value("recentFiles").toStringList();
    \endcode
*/

/*! \fn QString QSettings::readEntry(const QString &key, const QString &defaultValue, bool *ok)

    Returns the value for setting \a key converted to a QString. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        QString str = settings.readEntry("userName", "administrator", &ok);
    \newcode
        bool ok = settings.contains("userName");
        QString str = settings.value("userName", "administrator").toString();
    \endcode
*/

/*! \fn int QSettings::readNumEntry(const QString &key, int defaultValue, bool *ok)

    Returns the value for setting \a key converted to an \c int. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        int max = settings.readNumEntry("maxConnections", 30, &ok);
    \newcode
        bool ok = settings.contains("maxConnections");
        int max = settings.value("maxConnections", 30).toInt();
    \endcode
*/

/*! \fn double QSettings::readDoubleEntry(const QString &key, double defaultValue, bool *ok)

    Returns the value for setting \a key converted to a \c double. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        double pi = settings.readDoubleEntry("pi", 3.141592, &ok);
    \newcode
        bool ok = settings.contains("pi");
        double pi = settings.value("pi", 3.141592).toDouble();
    \endcode
*/

/*! \fn bool QSettings::readBoolEntry(const QString &key, bool defaultValue, bool *ok)

    Returns the value for setting \a key converted to a \c bool. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        bool grid = settings.readBoolEntry("showGrid", true, &ok);
    \newcode
        bool ok = settings.contains("showGrid");
        bool grid = settings.value("showGrid", true).toBool();
    \endcode
*/

/*! \fn bool QSettings::removeEntry(const QString &key)

    Use remove() instead.
*/

/*! \enum QSettings::System
    \compat

    \value Unix Unix/X11 systems
    \value Windows Microsoft Windows systems
    \value Mac Mac OS X systems

    \sa insertSearchPath(), removeSearchPath()
*/

/*! \fn void QSettings::insertSearchPath(System system, const QString &path)

    This function is implemented as a no-op. It is provided for
    source compatibility with Qt 3. The new QSettings class has no
    concept of "search path".
*/

/*! \fn void QSettings::removeSearchPath(System system, const QString &path)

    This function is implemented as a no-op. It is provided for
    source compatibility with Qt 3. The new QSettings class has no
    concept of "search path".
*/

/*! \enum QSettings::Scope
    \compat

    \value User The settings are specific to the current user (same as Qt::UserScope).
    \value Global The settings are shared by all users on the current machine (same as Qt::SystemScope).

    \sa setPath()
*/

/*! \fn void QSettings::setPath(const QString &organization, const QString &application, \
                                Scope scope)

    Specifies the \a organization, \a application, and \a scope to
    use by the QSettings object.

    Use the appropriate constructor instead, with Qt::UserScope
    instead of QSettings::User and Qt::SystemScope instead of
    QSettings::Global.

    \oldcode
        QSettings settings;
        settings.setPath("twikimaster.com", "Kanooth", QSettings::Global);
    \newcode
        QSettings settings(Qt::SystemScope, "twikimaster.com", "Kanooth");
    \endcode
*/

/*! \fn void QSettings::resetGroup()

    Sets the current group to be the empty string.

    Use endGroup() instead (possibly multiple times).

    \oldcode
        QSettings settings;
        settings.beginGroup("mainWindow");
        settings.beginGroup("leftPanel");
        ...
        settings.resetGroup();
    \newcode
        QSettings settings;
        settings.beginGroup("mainWindow");
        settings.beginGroup("leftPanel");
        ...
        settings.endGroup();
        settings.endGroup();
    \endcode
*/

/*! \fn QStringList QSettings::entryList(const QString &key) const

    Returns a list of all sub-keys of \a key.

    Use childKeys() instead.

    \oldcode
        QSettings settings;
        QStringList keys = settings.entryList("cities");
        ...
    \newcode
        QSettings settings;
        settings.beginGroup("cities");
        QStringList keys = settings.childKeys();
        ...
        settings.endGroup();
    \endcode
*/

/*! \fn QStringList QSettings::subkeyList(const QString &key) const

    Returns a list of all sub-keys of \a key.

    Use childGroups() instead.

    \oldcode
        QSettings settings;
        QStringList groups = settings.entryList("cities");
        ...
    \newcode
        QSettings settings;
        settings.beginGroup("cities");
        QStringList groups = settings.childKeys();
        ...
        settings.endGroup();
    \endcode
*/
