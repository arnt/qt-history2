#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "ui_previewwidget.h"

namespace qdesigner_internal {

class PreviewWidget: public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent);
    virtual ~PreviewWidget();

private:
    Ui::PreviewWidget ui;
};

}  // namespace qdesigner_internal

#endif // PREVIEWWIDGET_H
