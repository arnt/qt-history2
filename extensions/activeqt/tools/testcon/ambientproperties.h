#ifndef AMBIENTPROPERTIES_H
#define AMBIENTPROPERTIES_H

#include "ui_ambientproperties.h"

class AmbientProperties : public QDialog, Ui::AmbientProperties
{
    Q_OBJECT
public:
    AmbientProperties(QWidget *parent);

    void setControl(QWidget *widget);

public slots:
    void backColor();
    void foreColor();
    void pickFont();
    void toggleEnabled(bool on);

private:
    QWidget *container;
};

#endif // AMBIENTPROPERTIES_H
