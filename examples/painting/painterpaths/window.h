#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QComboBox;
class QLabel;
class QSpinBox;
class RenderArea;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private slots:
    void fillRuleChanged();
    void fillGradientChanged();
    void penColorChanged();

private:
    void populateWithColors(QComboBox *comboBox);
    QVariant currentItemData(QComboBox *comboBox);

    enum { NumRenderAreas = 9 };

    RenderArea *renderAreas[NumRenderAreas];
    QLabel *fillRuleLabel;
    QLabel *fillGradientLabel;
    QLabel *fillToLabel;
    QLabel *penWidthLabel;
    QLabel *penColorLabel;
    QLabel *rotationAngleLabel;
    QComboBox *fillRuleComboBox;
    QComboBox *fillColor1ComboBox;
    QComboBox *fillColor2ComboBox;
    QSpinBox *penWidthSpinBox;
    QComboBox *penColorComboBox;
    QSpinBox *rotationAngleSpinBox;
};

#endif
