#ifndef CHARWIDGET_H
#define CHARWIDGET_H

#include <QChar>
#include <QCursor>
#include <QFont>
#include <QPoint>
#include <QSize>
#include <QWidget>
#include <QWidgetView>

class QMouseEvent;
class QPaintEvent;

class CharView : public QWidgetView
{
    Q_OBJECT

public:
    CharView(QWidget *parent = 0);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QCursor dragCursor;
    QPoint dragPosition;
    bool dragging;
};

class CharWidget : public QWidget
{
    Q_OBJECT

public:
    CharWidget(QWidget *parent = 0);
    QSize sizeHint() const;
    void showToolTip(const QPoint &position);

public slots:
    void updateFont(const QString &fontFamily);
    void updateStyle(const QString &fontStyle);

signals:
    void characterSelected(const QString &character);

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QFont displayFont;
    int currentKey;
};

#endif
