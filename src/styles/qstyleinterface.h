#ifndef QSTYLEINTERFACE_H
#define QSTYLEINTERFACE_H

#ifndef QT_H
#include "qcomponentinterface.h"
#include <qstringlist.h>
#endif // QT_H

#ifndef QT_NO_STYLE
#ifndef QT_NO_COMPONENT

class QStyle;

// {FC1B6EBE-053C-49c1-A483-C377739AB9A5}
Q_UUID( IID_QStyleInterface, 
0xfc1b6ebe, 0x53c, 0x49c1, 0xa4, 0x83, 0xc3, 0x77, 0x73, 0x9a, 0xb9, 0xa5);

struct Q_EXPORT QStyleInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0; 
    virtual QStyle* create( const QString& style ) = 0;
};

#endif //QT_NO_COMPONENT
#endif //QT_NO_STYLE

#endif //QSTYLEINTERFACE_H
