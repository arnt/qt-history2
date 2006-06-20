#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QWhatsThis>
#include "ui_standard.h"
#include "ui_advanced.h"
#include "ui_system.h"

class StyleWidget : public QWidget
{
    Q_OBJECT

public:
    StyleWidget(QWidget *parent = 0, Qt::WFlags f = 0);
    ~StyleWidget();

public slots:
    void onWhatsThis() { QWhatsThis::enterWhatsThisMode(); }

private:
    void addComboBoxItems();
    void addListItems();
    void addTextEdit();
    void setupOtherWidgets();
    void setupButtons();
    void addTreeItems();
    void addTreeListItems();

    Ui::Standard m_staWidget;
    Ui::Advanced m_advWidget;
    Ui::System m_sysWidget;

    QIcon m_small1;
    QIcon m_small2;
    QIcon m_big;
};

#endif //MAINWINDOW_H

