#ifndef STYLEWIDGET_H
#define STYLEWIDGET_H

#include <QWidget>
#include <QPainterPath>

class StyleWidget : public QWidget
{
    Q_OBJECT

public:
    StyleWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawShape(QPainter *painter, const QBrush &brush);
    void drawText(QPainter *painter, const QString &first,
                           const QString &second = 0, const QString &third = 0);

    QPainterPath path;
    QPainterPath pattern;
};
#endif
