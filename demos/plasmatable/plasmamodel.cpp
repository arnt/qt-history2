/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "plasmamodel.h"
#include <qcoreevent.h>
#include <math.h>

PlasmaModel::PlasmaModel(int rows, int cols, QObject *parent)
    : QAbstractTableModel(parent), rows(rows), cols(cols)
{

    waves.resize(4);

    cosinus.resize(256);
    float pi_over_half = 3.14159f / 128;
    for (int t = 0; t < 256; ++t)
        cosinus[t] = (int)(64 * cos(t * pi_over_half));

    colors.resize(256);
    for (int c = 0; c < 32; ++c) {
        int z = 63;
        int d = c << 1;
        colors[c] = rgb( z, z, z + d );
        colors[c + 32] = rgb(z + d, z, z + 63);
        colors[c + 64] = rgb(z + 63, z + d, z + 63);
        colors[c + 96] = rgb(z + 63, z + 63, z + 63);
        int e = 64 - d;
        colors[c + 128] = rgb(z + 63, z + 63, z + 63);
        colors[c + 160] = rgb(z + 63, z + 63, z + e);
        colors[c + 192] = rgb(z + 63, z + e, z);
        colors[c + 224] = rgb(z + e, z , z);
    }

    values.resize(rows * cols);
    timer = startTimer(100);
}

PlasmaModel::~PlasmaModel()
{
    killTimer(timer);
}

int PlasmaModel::rowCount(const QModelIndex &) const
{
    return rows;
}

int PlasmaModel::columnCount(const QModelIndex &) const
{
    return cols;
}

QVariant PlasmaModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole)
        return value(index.row(), index.column());
    return QVariant();
}

void PlasmaModel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timer)
        return;

    unsigned char a = waves.at(0);
    unsigned char b = waves.at(1);
    for (int y = 0; y < rows; ++y) {
        unsigned char c = waves.at(2);
        unsigned char d = waves.at(3);
        for (int x = 0; x < cols; ++x) {
            unsigned char color = cosinus.at(a) + cosinus.at(b) + cosinus.at(c) + cosinus.at(d);
            setValue(y, x, colors.at(color));
            c += 1;
            d += 2;
        }
        a += 2;
        b += 1;
    }
    waves[0] += 1;
    waves[1] -= 2;
    waves[2] -= 1;
    waves[3] += 3;

    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rows - 1, cols - 1);
    emit dataChanged(topLeft, bottomRight);
}
