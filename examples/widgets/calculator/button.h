#ifndef BUTTON_H
#define BUTTON_H

#include <QToolButton>

class Button : public QToolButton
{
    Q_OBJECT

public:
    Button(const QString &text, const QColor &color, QWidget *parent = 0);

    QSize sizeHint() const;
};

#endif
