#ifndef PLASMAMODEL_H
#define PLASMAMODEL_H

#include <qabstractitemmodel.h>
#include <qvector.h>

class PlasmaModel : public QAbstractTableModel
{
public:
    PlasmaModel(int rows, int cols, QObject *parent = 0);
    ~PlasmaModel();

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;

protected:
    void timerEvent(QTimerEvent *e);

private:
    inline int rgb(unsigned char r, unsigned char g, unsigned char b) const
        { return (r << 16) + (g << 8) + b; }
    inline unsigned int value(int row, int column) const
        { return values.at((row * cols) + column); }
    inline void setValue(int row, int column, int val)
        { values[(row * cols) + column] = val; }

    int rows;
    int cols;
    int timer;

    QVector<unsigned int> values;
    QVector<unsigned int> colors;
    QVector<unsigned int> cosinus;
    QVector<unsigned char> waves;
};

#endif // PLASMAMODEL_H
