#ifndef QSQLEDITOR_H
#define QSQLEDITOR_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QRadioButton;
class Q_EXPORT QSqlCustomEd : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( bool state READ state WRITE setState )

public:
    QSqlCustomEd ( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    
    bool state() const;
    void setState( bool st );

private:
    QRadioButton * s;
};

#endif	// QT_NO_SQL
#endif
