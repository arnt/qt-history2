#ifndef PREVIEWWIDGETBASE_H
#define PREVIEWWIDGETBASE_H

#include <qvariant.h>
#include "ui_previewwidgetbase.h"

class PreviewWidgetBase : public QWidget, public Ui::PreviewWidgetBase
{
    Q_OBJECT

public:
    PreviewWidgetBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~PreviewWidgetBase();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();


};

#endif // PREVIEWWIDGETBASE_H
