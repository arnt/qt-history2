#ifndef WIGGLYWIDGET_H
#define WIGGLYWIDGET_H

#include <QtGui>

class WigglyWidget : public QWidget
{
    Q_OBJECT

public:
    WigglyWidget(QWidget *parent);

public slots:
    void setText(const QString &text) { _text = text; }

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);

private slots:
    void animate();

private:
    QString _text;
    int _0to15;
};

#endif
