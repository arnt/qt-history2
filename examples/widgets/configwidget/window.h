#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QStackedWidget;
class QWidgetView;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void changePage();

private:
    void createIcons();

    QStackedWidget *pagesWidget;
    QWidget *contentsWidget;
};

#endif
