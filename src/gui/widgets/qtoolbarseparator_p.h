#ifndef QTOOLBARSEPARATOR_P_H
#define QTOOLBARSEPARATOR_P_H

#include <qwidget.h>

class QToolBar;

class QToolBarSeparator : public QWidget
{
    Q_OBJECT
    Qt::Orientation orient;

public:
    QToolBarSeparator(Qt::Orientation orientation, QToolBar *parent);

    QSize sizeHint() const;

    inline Qt::Orientation orientation() const
    { return orient; }
    inline void setOrientation(Qt::Orientation orientation)
    {
        orient = orientation;
        update();
    }

    void paintEvent(QPaintEvent *);
};

#endif // QTOOLBARSEPARATOR_P_H
