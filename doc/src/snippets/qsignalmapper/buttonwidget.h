#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include <qwidget.h>

class QSignalMapper;
class QString;
class QStringList;

class ButtonWidget : public QWidget
{
    Q_OBJECT

public:
    ButtonWidget(QStringList texts, QWidget *parent = 0);

signals:
    void clicked(const QString &text);

private:
    QSignalMapper *signalMapper;
};

#endif
