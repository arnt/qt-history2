#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

/* ### Change QStackedBox to QStackedWidget before TP2 */
class QStackedBox;
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

    QStackedBox *pagesWidget;
    QWidget *contentsWidget;
};

#endif
