#ifndef QGPLUGINMANAGER_H
#define QPLUGINMANAGER_H

#ifndef QT_H
#include "qlibrary.h"
#include "qcom.h"
#include "qdict.h"
#include "qdir.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QDict<QLibrary>;
// MOC_SKIP_END
#endif

class Q_EXPORT QGPluginManager
{
public:
    QGPluginManager( const QUuid& id, const QString& path = QString::null, QLibrary::Policy pol = QLibrary::Delayed, bool cs = TRUE );
    virtual ~QGPluginManager();
    void addLibraryPath( const QString& path );
    void setDefaultPolicy( QLibrary::Policy pol );
    QLibrary::Policy defaultPolicy() const;
    QLibrary* library( const QString& feature ) const;
    QStringList featureList() const;

    virtual QLibrary* addLibrary( const QString& file ) = 0;
    virtual bool removeLibrary( const QString& file ) = 0;

protected:
    QUuid interfaceId;
    QDict<QLibrary> plugDict;	    // Dict to match feature with library
    QDict<QLibrary> libDict;	    // Dict to match library file with library
    QStringList libList;

    QLibrary::Policy defPol;
    uint casesens : 1;
};

#endif

#endif //QGPLUGINMANAGER_H
