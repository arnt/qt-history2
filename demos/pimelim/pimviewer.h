#ifndef PIMVIEW_H
#define PIMVIEW_H

#include <qwidget.h>

class PimModel;
class QModelIndex;

class PimViewer : public QWidget
{
    Q_OBJECT
public:
    PimViewer(QWidget *parent = 0);
    ~PimViewer();

    void setModel(const PimModel *model);
    const PimModel *model() const;

public slots:
    void view(const QModelIndex &index);

signals:
    void done();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    const PimModel *m;
    int entry;
};

#endif // PIMVIEW_H
