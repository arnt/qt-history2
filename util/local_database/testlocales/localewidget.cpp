#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

#include "localewidget.h"
#include "localemodel.h"

LocaleWidget::LocaleWidget(QWidget *parent)
    : QWidget(parent)
{
    m_model = new LocaleModel(this);
    m_view = new QTableView(this);
    m_view->setModel(m_model);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_view);
}
