#ifndef CONFIG_H
#define CONFIG_H

#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>

struct Config
{
    struct Style
    {
	QFont font;
	QColor color;
    };
    QMap<QString, Style> styles;

    static QMap<QString, Style> defaultStyles();
    static QMap<QString, Style> readStyles( const QString &path );
    static void saveStyles( const QMap<QString, Style> &styles, const QString &path );

};

#endif
