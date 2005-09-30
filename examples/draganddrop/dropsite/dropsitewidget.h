#ifndef DROPSITEWIDGET_H
#define DROPSITEWIDGET_H

#include <QLabel>
#include <QMimeData>

class QDropEvent;

class DropSiteWidget : public QLabel
{
    Q_OBJECT

public:
    DropSiteWidget(QWidget *parent = 0);
    QPixmap createPixmap(QByteArray data, QString format);
    QString createPlainText(QByteArray data, QString format);

public slots:
    void clear();

signals:
    void changed(const QMimeData *mimeData = 0);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QLabel *label;
};

#endif
