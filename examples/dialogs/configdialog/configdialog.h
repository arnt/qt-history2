#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>

class QStackedWidget;
class QScrollArea;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigDialog();

public slots:
    void changePage();

private:
    void createIcons();

    QStackedWidget *pagesWidget;
    QWidget *contentsWidget;
};

#endif
