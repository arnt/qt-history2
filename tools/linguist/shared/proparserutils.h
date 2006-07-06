/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROPARSERUTILS_H
#define PROPARSERUTILS_H

#include <QtCore/QLibraryInfo>

// Pre- and postcondition macros
#define PRE(cond) do {if(!(cond))qt_assert(#cond,__FILE__,__LINE__);} while (0)
#define POST(cond) do {if(!(cond))qt_assert(#cond,__FILE__,__LINE__);} while (0)

// This struct is from qmake, but we are not using everything.
struct Option
{
    //simply global convenience
    //static QString libtool_ext;
    //static QString pkgcfg_ext;
    //static QString prf_ext;
    //static QString prl_ext;
    //static QString ui_ext;
    //static QStringList h_ext;
    //static QStringList cpp_ext;
    //static QString h_moc_ext;
    //static QString cpp_moc_ext;
    //static QString obj_ext;
    //static QString lex_ext;
    //static QString yacc_ext;
    //static QString h_moc_mod;
    //static QString cpp_moc_mod;
    //static QString lex_mod;
    //static QString yacc_mod;
    static QString dir_sep;
    static QString dirlist_sep;
    //static QString sysenv_mod;
    //static QString pro_ext;
    //static QString res_ext;
    //static char field_sep;

    static void init()
    {
#ifdef Q_OS_WIN
    Option::dirlist_sep = ';';
    Option::dir_sep = QLatin1Char('\\');
#else
    Option::dirlist_sep = ':';
    Option::dir_sep = QLatin1Char('/');
#endif
    }
};


QString Option::dirlist_sep;
QString Option::dir_sep;

static void unquote(QString *string)
{
    PRE(string);
    if ( (string->startsWith(QLatin1Char('\"')) && string->endsWith(QLatin1Char('\"')))
        || (string->startsWith(QLatin1Char('\'')) && string->endsWith(QLatin1Char('\''))) )
    {
        string->remove(0,1);
        string->remove(string->length() - 1,1);
    }
}


static void insertUnique(QMap<QByteArray, QStringList> *map, const QByteArray &key, const QString &value, bool unique = true)
{
    QStringList &sl = (*map)[key];
    if (!unique || (unique && !sl.contains(value))) {
        sl.append(value);
    }
}

inline QStringList splitPathList(const QString paths) { return paths.split(Option::dirlist_sep); }

static QStringList split_arg_list(QString params)
{
    int quote = 0;
    QStringList args;

    const ushort LPAREN = '(';
    const ushort RPAREN = ')';
    const ushort SINGLEQUOTE = '\'';
    const ushort DOUBLEQUOTE = '"';
    const ushort COMMA = ',';
    const ushort SPACE = ' ';
    //const ushort TAB = '\t';

    ushort unicode;
    const QChar *params_data = params.data();
    const int params_len = params.length();
    int last = 0;
    while(last < params_len && ((params_data+last)->unicode() == SPACE
                                /*|| (params_data+last)->unicode() == TAB*/))
        ++last;
    for(int x = last, parens = 0; x <= params_len; x++) {
        unicode = (params_data+x)->unicode();
        if(x == params_len) {
            while(x && (params_data+(x-1))->unicode() == SPACE)
                --x;
            QString mid(params_data+last, x-last);
            if(quote) {
                if(mid[0] == quote && mid[(int)mid.length()-1] == quote)
                    mid = mid.mid(1, mid.length()-2);
                quote = 0;
            }
            args << mid;
            break;
        }
        if(unicode == LPAREN) {
            --parens;
        } else if(unicode == RPAREN) {
            ++parens;
        } else if(quote && unicode == quote) {
            quote = 0;
        } else if(!quote && (unicode == SINGLEQUOTE || unicode == DOUBLEQUOTE)) {
            quote = unicode;
        } else if(!parens && !quote && unicode == COMMA) {
            QString mid = params.mid(last, x - last).trimmed();
            args << mid;
            last = x+1;
            while(last < params_len && ((params_data+last)->unicode() == SPACE
                                        /*|| (params_data+last)->unicode() == TAB*/))
                ++last;
        }
    }
    for(int i = 0; i < args.count(); i++)
        unquote(&args[i]);
    return args;
}


static QStringList qmake_mkspec_paths()
{
    QStringList ret;
    const QString concat = QDir::separator() + QString("mkspecs");
    QByteArray qmakepath = qgetenv("QMAKEPATH");
    if (!qmakepath.isEmpty()) {
        const QStringList lst = splitPathList(QString::fromLocal8Bit(qmakepath));
        for(QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it)
            ret << ((*it) + concat);
    }
    ret << QLibraryInfo::location(QLibraryInfo::DataPath) + concat;

    return ret;
}


static QString getPropertyValue(const QString &v)
{
    if(v == "QT_INSTALL_PREFIX")
        return QLibraryInfo::location(QLibraryInfo::PrefixPath);
    else if(v == "QT_INSTALL_DATA")
        return QLibraryInfo::location(QLibraryInfo::DataPath);
    else if(v == "QT_INSTALL_DOCS")
        return QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    else if(v == "QT_INSTALL_HEADERS")
        return QLibraryInfo::location(QLibraryInfo::HeadersPath);
    else if(v == "QT_INSTALL_LIBS")
        return QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    else if(v == "QT_INSTALL_BINS")
        return QLibraryInfo::location(QLibraryInfo::BinariesPath);
    else if(v == "QT_INSTALL_PLUGINS")
        return QLibraryInfo::location(QLibraryInfo::PluginsPath);
    else if(v == "QT_INSTALL_TRANSLATIONS")
        return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    else if(v == "QT_INSTALL_CONFIGURATION")
        return QLibraryInfo::location(QLibraryInfo::SettingsPath);
    else if(v == "QT_INSTALL_EXAMPLES")
        return QLibraryInfo::location(QLibraryInfo::ExamplesPath);
    else if(v == "QT_INSTALL_DEMOS")
        return QLibraryInfo::location(QLibraryInfo::DemosPath);
    else if(v == "QMAKE_MKSPECS")
        return qmake_mkspec_paths().join(Option::dirlist_sep);
    else if(v == "QMAKE_VERSION")
        return QLatin1String("1.0");        //###
        //return qmake_version();
#ifdef QT_VERSION_STR
    else if(v == "QT_VERSION")
        return QT_VERSION_STR;
#endif
    return QLatin1String("UNKNOWN");        //###
}


#endif // PROPARSERUTILS_H

