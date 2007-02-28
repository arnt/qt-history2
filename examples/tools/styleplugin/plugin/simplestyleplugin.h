#ifndef SIMPLESTYLEPLUGIN_H
#define SIMPLESTYLEPLUGIN_H

#include <QStylePlugin>

class QStringList;
class QStyle;

class SimpleStylePlugin : public QStylePlugin
{
    Q_OBJECT

public:
    SimpleStylePlugin() {};

    QStringList keys() const;
    QStyle *create(const QString &key);
};

#endif
