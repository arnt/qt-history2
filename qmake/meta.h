#ifndef __META_H__
#define __META_H__

#include <qmap.h>
#include <qstringlist.h>
#include <qstring.h>

class QMakeMetaInfo 
{
    bool readLibtoolFile(const QString &f);
    bool readPkgCfgFile(const QString &f);
    QMap<QString, QStringList> vars;
    static QMap<QString, QMap<QString, QStringList> > cache_vars;
    void clear();
public:
    QMakeMetaInfo();

    bool readLib(const QString &lib);
    static QString findLib(const QString &lib);
    static bool libExists(const QString &lib);

    bool isEmpty(const QString &v);
    QStringList &values(const QString &v);
    QString first(const QString &v);
    QMap<QString, QStringList> &variables();
};

inline bool QMakeMetaInfo::isEmpty(const QString &v)
{ return !vars.contains(v) || vars[v].isEmpty(); }

inline QStringList &QMakeMetaInfo::values(const QString &v)
{ return vars[v]; }

inline QString QMakeMetaInfo::first(const QString &v)
{
#if defined(Q_CC_SUN) && (__SUNPRO_CC == 0x500) || defined(Q_CC_HP)
    // workaround for Sun WorkShop 5.0 bug fixed in Forte 6
    if (isEmpty(v))
	return QString("");
    else
	return vars[v].first();
#else
    return isEmpty(v) ? QString("") : vars[v].first();
#endif
}

inline QMap<QString, QStringList> &QMakeMetaInfo::variables()
{ return vars; }

inline bool QMakeMetaInfo::libExists(const QString &lib)
{ return !findLib(lib).isNull(); }

#endif /* __META_H__ */
