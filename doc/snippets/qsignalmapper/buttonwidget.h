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
    ButtonWidget(QStringList captions, QWidget *parent=0, const char *name=0);

signals:
    void clicked(const QString &chosen);

private:
    QSignalMapper *signalMapper;
};

#endif
