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
    void strokeGradientChanged();

private:
    void addItem(QComboBox *comboBox, const QString &text, int id);
    void addColor(QComboBox *comboBox, const QString &text,
                  const QColor &color);
    void populateWithColors(QComboBox *comboBox);

    enum { NumRenderAreas = 9 };

    RenderArea *renderAreas[NumRenderAreas];
    QLabel *fillRuleLabel;
    QLabel *fillGradientLabel;
    QLabel *fillToLabel;
    QLabel *strokeWidthLabel;
    QLabel *strokeGradientLabel;
    QLabel *strokeToLabel;
    QLabel *rotationAngleLabel;
    QComboBox *fillRuleComboBox;
    QComboBox *fillColor1ComboBox;
    QComboBox *fillColor2ComboBox;
    QSpinBox *strokeWidthSpinBox;
    QComboBox *strokeColor1ComboBox;
    QComboBox *strokeColor2ComboBox;
    QSpinBox *rotationAngleSpinBox;
};

#endif
