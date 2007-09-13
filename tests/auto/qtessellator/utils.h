#ifndef UTILS_H
#define UTILS_H

#include <QVector>
#include <QPointF>
#include <QDebug>
#include "XrenderFake.h"

double compute_area(XTrapezoid *trap);
double compute_area_for_x(const QVector<XTrapezoid> &traps);

QT_BEGIN_NAMESPACE
Q_DECLARE_TYPEINFO(XTrapezoid, Q_PRIMITIVE_TYPE);
QT_END_NAMESPACE

#endif
