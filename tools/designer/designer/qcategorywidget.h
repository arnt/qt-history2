#ifndef QCATEGORYWIDGET_H
#define QCATEGORYWIDGET_H

#include <qwidget.h>
#include <qlayout.h>
#include <qptrdict.h>

class QCategoryButton;
class QWidgetList;

class QCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    QCategoryWidget( QWidget *parent = 0, const char *name = 0 );
    ~QCategoryWidget();

    void addCategory( const QString &name, QWidget *page );

private slots:
    void buttonClicked();

private:
    void updateTabs();

private:
    QPtrDict<QWidget> pages;
    QWidgetList *buttons;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QCategoryButton *lastTab;

};

#endif
