#include <QtGui>

#include "renderarea.h"
#include "window.h"

const int IdRole = QAbstractItemModel::UserRole;
const int ColorRole = QAbstractItemModel::UserRole + 1;

Window::Window()
{
    QPainterPath rectPath;
    rectPath.moveTo(20.0, 30.0);
    rectPath.lineTo(80.0, 30.0);
    rectPath.lineTo(80.0, 70.0);
    rectPath.lineTo(20.0, 70.0);
    rectPath.closeSubpath();

    QPainterPath roundRectPath;
    roundRectPath.moveTo(80.0, 35.0);
    roundRectPath.arcTo(70.0, 30.0, 10.0, 10.0, 0.0, 90.0);
    roundRectPath.lineTo(25.0, 30.0);
    roundRectPath.arcTo(20.0, 30.0, 10.0, 10.0, 90.0, 90.0);
    roundRectPath.lineTo(20.0, 65.0);
    roundRectPath.arcTo(20.0, 60.0, 10.0, 10.0, 180.0, 90.0);
    roundRectPath.lineTo(75.0, 70.0);
    roundRectPath.arcTo(70.0, 60.0, 10.0, 10.0, 270.0, 90.0);
    roundRectPath.closeSubpath();

    QPainterPath ellipsePath;
    ellipsePath.moveTo(80.0, 50.0);
    ellipsePath.arcTo(20.0, 30.0, 60.0, 40.0, 0.0, 360.0);

    QPainterPath piePath;
    piePath.moveTo(50.0, 50.0);
    piePath.lineTo(65.0, 32.679492950439453);
    piePath.arcTo(20.0, 30.0, 60.0, 40.0, 60.0, 240.0);
    piePath.closeSubpath();

    QPainterPath polylinePath;
    polylinePath.moveTo(10.0, 80.0);
    polylinePath.lineTo(20.0, 10.0);
    polylinePath.lineTo(80.0, 30.0);
    polylinePath.lineTo(90.0, 70.0);

    QPainterPath polygonPath = polylinePath;
    polygonPath.closeSubpath();

    QPainterPath textPath;
    QFont timesFont("Times", 60);
    timesFont.setStyleStrategy(QFont::ForceOutline);
    textPath.addText(10, 70, timesFont, tr("Qt"));

    QPainterPath bezierPath;
    bezierPath.moveTo(20, 30);
    bezierPath.curveTo(80, 0, 50, 50, 80, 80);

    QPainterPath compositionPath;

    renderAreas[0] = new RenderArea(rectPath, this);
    renderAreas[1] = new RenderArea(roundRectPath, this);
    renderAreas[2] = new RenderArea(ellipsePath, this);
    renderAreas[3] = new RenderArea(piePath, this);
    renderAreas[4] = new RenderArea(polylinePath, this);
    renderAreas[5] = new RenderArea(polygonPath, this);
    renderAreas[6] = new RenderArea(textPath, this);
    renderAreas[7] = new RenderArea(bezierPath, this);
    renderAreas[8] = new RenderArea(compositionPath, this);
    Q_ASSERT(NumRenderAreas == 9);

    fillRuleComboBox = new QComboBox(this);
    addItem(fillRuleComboBox, tr("Odd Even"), Qt::OddEvenFill);
    addItem(fillRuleComboBox, tr("Winding"), Qt::WindingFill);

    fillRuleLabel = new QLabel(tr("Fill &Rule:"), this);
    fillRuleLabel->setBuddy(fillRuleComboBox);

    fillColor1ComboBox = new QComboBox(this);
    populateWithColors(fillColor1ComboBox);

    fillColor2ComboBox = new QComboBox(this);
    populateWithColors(fillColor2ComboBox);

    fillGradientLabel = new QLabel(tr("&Fill Gradient:"), this);
    fillGradientLabel->setBuddy(fillColor1ComboBox);

    fillToLabel = new QLabel(tr("to"), this);
    fillToLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    strokeWidthSpinBox = new QSpinBox(this);
    strokeWidthSpinBox->setRange(0, 20);

    strokeWidthLabel = new QLabel(tr("&Stroke Width:"), this);
    strokeWidthLabel->setBuddy(strokeWidthSpinBox);

    strokeColor1ComboBox = new QComboBox(this);
    populateWithColors(strokeColor1ComboBox);

    strokeColor2ComboBox = new QComboBox(this);
    populateWithColors(strokeColor2ComboBox);

    strokeGradientLabel = new QLabel(tr("Stroke &Gradient:"), this);
    strokeGradientLabel->setBuddy(strokeColor1ComboBox);

    strokeToLabel = new QLabel(tr("to"), this);
    strokeToLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    rotationAngleSpinBox = new QSpinBox(this);
    rotationAngleSpinBox->setRange(0, 359);
    rotationAngleSpinBox->setWrapping(true);
    rotationAngleSpinBox->setSuffix("\xB0");   // degree sign

    rotationAngleLabel = new QLabel(tr("&Rotation Angle:"), this);
    rotationAngleLabel->setBuddy(rotationAngleSpinBox);

    connect(fillRuleComboBox, SIGNAL(activated(int)),
            this, SLOT(fillRuleChanged()));
    connect(fillColor1ComboBox, SIGNAL(activated(int)),
            this, SLOT(fillGradientChanged()));
    connect(fillColor2ComboBox, SIGNAL(activated(int)),
            this, SLOT(fillGradientChanged()));
    connect(strokeColor1ComboBox, SIGNAL(activated(int)),
            this, SLOT(strokeGradientChanged()));
    connect(strokeColor2ComboBox, SIGNAL(activated(int)),
            this, SLOT(strokeGradientChanged()));

    for (int i = 0; i < NumRenderAreas; ++i) {
        connect(strokeWidthSpinBox, SIGNAL(valueChanged(int)),
                renderAreas[i], SLOT(setStrokeWidth(int)));
        connect(rotationAngleSpinBox, SIGNAL(valueChanged(int)),
                renderAreas[i], SLOT(setRotationAngle(int)));
    }

    QGridLayout *topLayout = new QGridLayout;
    for (int i = 0; i < NumRenderAreas; ++i)
        topLayout->addWidget(renderAreas[i], i / 3, i % 3);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addLayout(topLayout, 0, 0, 1, 4);
    mainLayout->addWidget(fillRuleLabel, 1, 0);
    mainLayout->addWidget(fillRuleComboBox, 1, 1, 1, 3);
    mainLayout->addWidget(fillGradientLabel, 2, 0);
    mainLayout->addWidget(fillColor1ComboBox, 2, 1);
    mainLayout->addWidget(fillToLabel, 2, 2);
    mainLayout->addWidget(fillColor2ComboBox, 2, 3);
    mainLayout->addWidget(strokeWidthLabel, 3, 0);
    mainLayout->addWidget(strokeWidthSpinBox, 3, 1, 1, 3);
    mainLayout->addWidget(strokeGradientLabel, 4, 0);
    mainLayout->addWidget(strokeColor1ComboBox, 4, 1);
    mainLayout->addWidget(strokeToLabel, 4, 2);
    mainLayout->addWidget(strokeColor2ComboBox, 4, 3);
    mainLayout->addWidget(rotationAngleLabel, 5, 0);
    mainLayout->addWidget(rotationAngleSpinBox, 5, 1, 1, 3);

    fillRuleChanged();
    fillGradientChanged();
    strokeGradientChanged();
    strokeWidthSpinBox->setValue(2);

    setWindowTitle(tr("Painter Paths"));
}

void Window::fillRuleChanged()
{
    Qt::FillRule rule = (Qt::FillRule)fillRuleComboBox->data(IdRole,
            fillRuleComboBox->currentItem()).toInt();

    for (int i = 0; i < NumRenderAreas; ++i)
        renderAreas[i]->setFillRule(rule);
}

void Window::fillGradientChanged()
{
    QColor color1 = fillColor1ComboBox->data(ColorRole,
            fillColor1ComboBox->currentItem()).toColor();
    QColor color2 = fillColor2ComboBox->data(ColorRole,
            fillColor2ComboBox->currentItem()).toColor();

    for (int i = 0; i < NumRenderAreas; ++i)
        renderAreas[i]->setFillGradient(color1, color2);
}

void Window::strokeGradientChanged()
{
    QColor color1 = strokeColor1ComboBox->data(ColorRole,
            strokeColor1ComboBox->currentItem()).toColor();
    QColor color2 = strokeColor2ComboBox->data(ColorRole,
            strokeColor2ComboBox->currentItem()).toColor();

    for (int i = 0; i < NumRenderAreas; ++i)
        renderAreas[i]->setStrokeGradient(color1, color2);
}

void Window::addItem(QComboBox *comboBox, const QString &text, int id)
{
    int row = comboBox->count();
    comboBox->insertItem(text, row);
    comboBox->setItemData(IdRole, id, row);
}

void Window::addColor(QComboBox *comboBox, const QString &text,
                      const QColor &color)
{
    int row = comboBox->count();
    comboBox->insertItem(text, row);
    comboBox->setItemData(ColorRole, color, row);
}

void Window::populateWithColors(QComboBox *comboBox)
{
    addColor(comboBox, tr("Red"), Qt::red);
    addColor(comboBox, tr("Green"), Qt::green);
    addColor(comboBox, tr("Blue"), Qt::blue);
}
