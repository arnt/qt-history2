#ifndef LOCALEWIDGET_H
#define LOCALEWIDGET_H

#include <QtGui/QWidget>

class LocaleModel;
class QTableView;

class LocaleWidget : public QWidget
{
    Q_OBJECT
public:
    LocaleWidget(QWidget *parent = 0);
private:
    LocaleModel *m_model;
    QTableView *m_view;
};

#endif // LOCALEWIDGET_H
