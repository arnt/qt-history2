#ifndef DROPSITEWINDOW_H
#define DROPSITEWINDOW_H

#include <QWidget>
#include "dropsitewidget.h"

class QLabel;
class QMimeData;
class QPushButton;
class QStringList;
class QTableWidget;
class QVBoxLayout;

class DropSiteWindow : public QWidget
{
    Q_OBJECT

public:
    DropSiteWindow(QWidget *parent = 0);

public slots:
    void updateSupportedFormats(const QMimeData *mimeData = 0);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QLabel *abstractLabel;
    DropSiteWidget *dropSiteWidget;
    QTableWidget *supportedFormats;

    QPushButton *clearButton;
    QPushButton *quitButton;
    QVBoxLayout *layout;
};

#endif
