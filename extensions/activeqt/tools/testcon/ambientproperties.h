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
    void on_buttonBackground_clicked();
    void on_buttonForeground_clicked();
    void on_buttonFont_clicked();
    void on_buttonEnabled_toggled(bool on);

private:
    QWidget *container;
};

#endif // AMBIENTPROPERTIES_H
