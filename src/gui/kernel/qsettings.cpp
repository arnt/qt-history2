#include "qsettings.h"
#include "private/qcoresettings_p.h"

#include "qcoreapplication.h"
#include "qrect.h"
#include "qpoint.h"
#include "qcolor.h"
#include "qsize.h"

QSETTINGS_DEFINE_CORE_PARSER_FUNCTIONS
QSETTINGS_DEFINE_GUI_PARSER_FUNCTIONS

// ************************************************************************
// QSettings

QSettings::QSettings(const QString &organization, const QString &application,
                        QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                                organization, application,
                                                variantToStringGuiImpl,
                                                stringToVariantGuiImpl), parent)
{
}

QSettings::QSettings(Qt::SettingsScope scope, const QString &organization,
                        const QString &application, QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(Qt::NativeFormat, scope,
                                                    organization, application,
                                                    variantToStringGuiImpl,
                                                    stringToVariantGuiImpl), parent)
{
}

QSettings::QSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                        const QString &organization,
                        const QString &application,
                        QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(format, scope, organization, application,
                                                    variantToStringGuiImpl,
                                                    stringToVariantGuiImpl), parent)

{
}

QSettings::QSettings(const QString &fileName, Qt::SettingsFormat format,
                        QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(fileName, format,
                                                    variantToStringGuiImpl,
                                                    stringToVariantGuiImpl), parent)
{
}

QSettings::QSettings(QObject *parent)
    : QCoreSettings(QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                                QCoreApplication::instance()->organization(),
                                                QCoreApplication::instance()->application(),
                                                variantToStringGuiImpl,
                                                stringToVariantGuiImpl), parent)
{
}

QSettings::~QSettings()
{
}

