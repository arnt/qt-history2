/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LCDRANGE_H
#define LCDRANGE_H

#include <QWidget>

QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QSlider)

class LCDRange : public QWidget
{
    Q_OBJECT

public:
    LCDRange(QWidget *parent = 0);
    LCDRange(const QString &text, QWidget *parent = 0);

    int value() const;
    QString text() const;

public slots:
    void setValue(int value);
    void setRange(int minValue, int maxValue);
    void setText(const QString &text);

signals:
    void valueChanged(int newValue);

private:
    void init();

    QSlider *slider;
    QLabel *label;
};

#endif
