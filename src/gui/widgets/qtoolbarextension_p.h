#ifndef QTOOLBAREXTENSION_P_H
#define QTOOLBAREXTENSION_P_H

#include <qtoolbutton.h>

class QToolBarExtension : public QToolButton
{
    Qt::Orientation orientation;

public:
    QToolBarExtension(QWidget *parent);

    void setOrientation(Qt::Orientation o);

    QSize sizeHint() const;
};

#endif // QTOOLBAREXTENSION_P_H
