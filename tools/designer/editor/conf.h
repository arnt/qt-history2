#ifndef CONFIG_H
#define CONFIG_H

#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>
#include "dlldefs.h"

struct EDITOR_EXPORT ConfigStyle
{
    QFont font;
    QColor color;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class EDITOR_EXPORT QMap<QString, ConfigStyle>;
// MOC_SKIP_END
#endif

struct EDITOR_EXPORT Config
{
    QMap<QString, ConfigStyle> styles;
    bool hasCompletion, hasParenMatching, hasWordWrap;

    static QMap<QString, ConfigStyle> defaultStyles();
    static QMap<QString, ConfigStyle> readStyles( const QString &path );
    static void saveStyles( const QMap<QString, ConfigStyle> &styles, const QString &path );
    static bool completion( const QString &path );
    static bool wordWrap( const QString &path );
    static bool parenMatching( const QString &path );
    static void setCompletion( bool b, const QString &path );
    static void setWordWrap( bool b, const QString &path );
    static void setParenMatching( bool b,const QString &path );

};

#endif
