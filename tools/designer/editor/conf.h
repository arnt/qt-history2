/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONF_H
#define CONF_H

#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>

struct ConfigStyle
{
    QFont font;
    QColor color;

    Q_DUMMY_COMPARISON_OPERATOR( ConfigStyle )
};

struct Config
{
    QMap<QString, ConfigStyle> styles;
    bool hasCompletion, hasParenMatching, hasWordWrap;

    static QMap<QString, ConfigStyle> defaultStyles();
    static QMap<QString, ConfigStyle> readStyles( const QString &path );
    static void saveStyles( const QMap<QString, ConfigStyle> &styles, const QString &path );
    static bool completion( const QString &path );
    static bool wordWrap( const QString &path );
    static bool parenMatching( const QString &path );
    static int indentTabSize( const QString &path );
    static int indentIndentSize( const QString &path );
    static bool indentKeepTabs( const QString &path );
    static bool indentAutoIndent( const QString &path );

    static void setCompletion( bool b, const QString &path );
    static void setWordWrap( bool b, const QString &path );
    static void setParenMatching( bool b,const QString &path );
    static void setIndentTabSize( int s, const QString &path );
    static void setIndentIndentSize( int s, const QString &path );
    static void setIndentKeepTabs( bool b, const QString &path );
    static void setIndentAutoIndent( bool b, const QString &path );

};

#endif
