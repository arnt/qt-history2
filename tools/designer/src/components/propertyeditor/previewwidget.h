#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "ui_previewwidget.h"

class PreviewWidget: public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent);
    virtual ~PreviewWidget();

private:
    Ui::PreviewWidget ui;
};

#endif // PREVIEWWIDGET_H
