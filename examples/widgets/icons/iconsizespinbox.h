#ifndef ICONSIZESPINBOX_H
#define ICONSIZESPINBOX_H

#include <QSpinBox>

class IconSizeSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    IconSizeSpinBox(QWidget *parent = 0);

    int valueFromText(const QString &text) const;
    QString textFromValue(int value) const;
};

#endif
