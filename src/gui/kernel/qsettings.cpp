#include "qsettings.h"
#include "private/qcoresettings_p.h"

#include "qcoreapplication.h"
#include "qrect.h"
#include "qpoint.h"
#include "qcolor.h"
#include "qsize.h"


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

QString QSettings::variantToStringGuiImpl(const QCoreVariant &v)
{
    QVariant v2 = v;
    QString result;

    switch (v.type()) {
        case QCoreVariant::Rect: {
            QRect r = v2.toRect();
            result += "@Rect("
                        + QByteArray::number(r.x()) + ", "
                        + QByteArray::number(r.y()) + ", "
                        + QByteArray::number(r.width()) + ", "
                        + QByteArray::number(r.height()) + ")";
            break;
        }

        case QCoreVariant::Size: {
            QSize s = v2.toSize();
            result += "@Size("
                        + QByteArray::number(s.width()) + ", "
                        + QByteArray::number(s.height()) + ")";
            break;
        }

        case QCoreVariant::Color: {
            QColor c = v2.toColor();
            result += "@Color("
                        + QByteArray::number(c.red()) + ", "
                        + QByteArray::number(c.green()) + ", "
                        + QByteArray::number(c.blue()) + ")";
            break;
        }

        case QCoreVariant::Point: {
            QPoint p = v2.toPoint();
            result += "@Point("
                        + QByteArray::number(p.x()) + ", "
                        + QByteArray::number(p.y()) + ")";
            break;
        }

        default:
            result = variantToStringCoreImpl(v);
            break;
    }

    return result;
}

static QStringList splitArgs(const QString &s, int idx)
{
    int l = s.length();
    Q_ASSERT(l > 0);
    Q_ASSERT(s.at(idx) == '(');
    Q_ASSERT(s.at(l - 1) == ')');

    QStringList result;
    QString item;

    for (++idx; idx < l; ++idx) {
        QChar c = s.at(idx);
        if (c == QLatin1Char(')')) {
            Q_ASSERT(idx == l - 1);
            result.append(item);
        } else if (c == QLatin1Char(',')) {
            result.append(item);
            item.clear();
        } else {
            item.append(c);
        }
    }

    return result;
}

QCoreVariant QSettings::stringToVariantGuiImpl(const QString &s)
{
    if (s.length() > 3
            && s.at(0) == QLatin1Char('@')
            && s.at(s.length() - 1) == QLatin1Char(')')) {

        if (s.startsWith(QLatin1String("@Rect("))) {
            QStringList args = splitArgs(s, 5);
            if (args.size() == 4) {
                return QVariant(QRect(args[0].toInt(), args[1].toInt(),
                                        args[2].toInt(), args[3].toInt()));
            }
        } else if (s.startsWith(QLatin1String("@Size("))) {
            QStringList args = splitArgs(s, 5);
            if (args.size() == 2) {
                return QVariant(QSize(args[0].toInt(), args[1].toInt()));
            }
        } else if (s.startsWith(QLatin1String("@Color("))) {
            QStringList args = splitArgs(s, 6);
            if (args.size() == 3) {
                return QVariant(QColor(args[0].toInt(), args[1].toInt(),
                                        args[2].toInt()));
            }
        } else if (s.startsWith(QLatin1String("@Point("))) {
            QStringList args = splitArgs(s, 6);
            if (args.size() == 2) {
                return QVariant(QPoint(args[0].toInt(), args[1].toInt()));
            }
        }
    }

    return stringToVariantCoreImpl(s);
}

