#ifndef QEDITORFACTORY_H
#define QEDITORFACTORY_H

#ifndef QT_H
#include "qobject.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QEditorFactory : public QObject
{
public:
    QEditorFactory ( QObject * parent=0, const char * name=0 );
    ~QEditorFactory();
    virtual QWidget * createEditor( QWidget * parent, const QVariant & v );
    
    static QEditorFactory * defaultFactory();
};

#endif // QT_NO_SQL
#endif // QEDITORFACTORY_H
