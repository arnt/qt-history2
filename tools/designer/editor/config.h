#ifndef CONFIG_H
#define CONFIG_H

#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>

struct ConfigStyle
{
	QFont font;
	QColor color;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMap<QString, ConfigStyle>;
// MOC_SKIP_END
#endif

struct Config
{
    QMap<QString, ConfigStyle> styles;

    static QMap<QString, ConfigStyle> defaultStyles();
    static QMap<QString, ConfigStyle> readStyles( const QString &path );
    static void saveStyles( const QMap<QString, ConfigStyle> &styles, const QString &path );

};

#endif
