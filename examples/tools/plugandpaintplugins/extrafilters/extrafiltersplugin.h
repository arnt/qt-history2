#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H

#include <QObject>

#include "interfaces.h"

class ExtraFiltersPlugin : public QObject, public FilterInterface
{
    Q_OBJECT
    Q_INTERFACES(FilterInterface)

public:
    QStringList filters() const;
    QImage filterImage(const QString &filter, const QImage &image,
                       QWidget *parent);
};

#endif
