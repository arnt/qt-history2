#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QDateTimeEdit;
class QDoubleSpinBox;
class QGroupBox;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void setFormatString(const QString &formatString);
    void changePrecision(int precision);

private:
    void createSpinBoxes();
    void createDateTimeEdits();
    void createDoubleSpinBoxes();

    QDateTimeEdit *meetingEdit;
    QDoubleSpinBox *doubleSpinBox;
    QDoubleSpinBox *priceSpinBox;
    QDoubleSpinBox *scaleSpinBox;
    QGroupBox *spinBoxesGroup;
    QGroupBox *editsGroup;
    QGroupBox *doubleSpinBoxesGroup;
};

#endif
