#ifndef QUCOMLIBRARY_H
#define QUCOMLIBRARY_H

#ifndef QT_H
#include "qcom_p.h"
#include "qlibrary.h"
#endif // QT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//
#ifndef QT_NO_COMPONENT
class Q_EXPORT QComLibrary : public QLibrary
{
public:
    QComLibrary( const QString &filename );
    ~QComLibrary();

    bool unload();
    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );

private:
    void createInstanceInternal();

    QUnknownInterface *entry;
    QLibraryInterface *libiface;

};
#endif //QT_NO_COMPONENT
#endif // QUCOMLIBRARY_H
