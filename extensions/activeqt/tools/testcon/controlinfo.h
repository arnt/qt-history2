#ifndef CONTROLINFO_H
#define CONTROLINFO_H

#include "ui_controlinfo.h"

class ControlInfo : public QDialog, Ui::ControlInfo
{
    Q_OBJECT
public:
    ControlInfo(QWidget *parent);

    void setControl(QWidget *activex);
};

#endif // CONTROLINFO_H
