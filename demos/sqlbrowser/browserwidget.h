#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <qwidget.h>

class ConnectionWidget;
class QGenericTableView;
class QPushButton;
class QTextEdit;

class BrowserWidget: public QWidget
{
    Q_OBJECT
public:
    BrowserWidget(QWidget *parent = 0);
    virtual ~BrowserWidget();

public slots:
    void exec();
    void addConnection();

signals:
    void statusMessage(const QString &message);

private:
    QTextEdit *edit;
    QGenericTableView *view;
    QPushButton *submitButton;
    ConnectionWidget *dbc;
};

#endif

