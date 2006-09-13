#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <QList>
#include <QVector>
#include <QPointF>

QList< QVector<QPointF> > parseData(const QByteArray &contents);
QList< QVector<QPointF> > parseFile(const QString &fileName);

#endif
