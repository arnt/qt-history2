#ifndef QSTYLEFACTORY_H
#define QSTYLEFACTORY_H

class QString;
class QStyle;

class QStyleFactory
{
public:
    static QStyle *create( const QString& );
};

#endif //QSTYLEFACTORY_H
