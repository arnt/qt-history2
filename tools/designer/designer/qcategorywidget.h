#ifndef QCATEGORYWIDGET_H
#define QCATEGORYWIDGET_H

#include <qwidget.h>
#include <qlayout.h>
#include <qptrdict.h>
#include <qptrlist.h>

class QCategoryButton;

class QCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    QCategoryWidget( QWidget *parent = 0, const char *name = 0 );

    void addCategory( const QString &name, QWidget *page );

private slots:
    void buttonClicked();

private:
    void updateTabs();

private:
    QPtrDict<QWidget> pages;
    QPtrList<QWidget> buttons;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QCategoryButton *lastTab;

};

#endif
