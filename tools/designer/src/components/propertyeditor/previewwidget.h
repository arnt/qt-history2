#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "ui_previewwidget.h"

namespace qdesigner { namespace components { namespace propertyeditor {

class PreviewWidget: public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent);
    virtual ~PreviewWidget();

private:
    Ui::PreviewWidget ui;
};

} } } // namespace qdesigner::components::propertyeditor

#endif // PREVIEWWIDGET_H
