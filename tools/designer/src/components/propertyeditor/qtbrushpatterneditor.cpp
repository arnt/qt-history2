#include "qtbrushpatterneditor.h"
#include "ui_qtbrushpatterneditor.h"

#include "qdebug.h"

class QtBrushPatternEditorPrivate
{
    QtBrushPatternEditor *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushPatternEditor)
public:
    void slotHsvClicked();
    void slotRgbClicked();
    void slotPatternChanged(int pattern);
    void slotChangeColor(const QColor &color);
    void slotChangeHue(const QColor &color);
    void slotChangeSaturation(const QColor &color);
    void slotChangeValue(const QColor &color);
    void slotChangeAlpha(const QColor &color);

    QBrush m_brush;

    Ui::QtBrushPatternEditor m_ui;
};

void QtBrushPatternEditorPrivate::slotHsvClicked()
{
    m_ui.hueLabel->setText(q_ptr->tr("Hue"));
    m_ui.saturationLabel->setText(q_ptr->tr("Saturation"));
    m_ui.valueLabel->setText(q_ptr->tr("Value"));
    m_ui.hueColorLine->setColorComponent(QtColorLine::Hue);
    m_ui.saturationColorLine->setColorComponent(QtColorLine::Saturation);
    m_ui.valueColorLine->setColorComponent(QtColorLine::Value);
}

void QtBrushPatternEditorPrivate::slotRgbClicked()
{
    m_ui.hueLabel->setText(q_ptr->tr("Red"));
    m_ui.saturationLabel->setText(q_ptr->tr("Green"));
    m_ui.valueLabel->setText(q_ptr->tr("Blue"));
    m_ui.hueColorLine->setColorComponent(QtColorLine::Red);
    m_ui.saturationColorLine->setColorComponent(QtColorLine::Green);
    m_ui.valueColorLine->setColorComponent(QtColorLine::Blue);
}

void QtBrushPatternEditorPrivate::slotPatternChanged(int pattern)
{
    QBrush brush = m_brush;
    brush.setStyle((Qt::BrushStyle)pattern);
    q_ptr->setBrush(brush);
}

void QtBrushPatternEditorPrivate::slotChangeColor(const QColor &color)
{
    QBrush brush = m_brush;
    brush.setColor(color);
    q_ptr->setBrush(brush);
}

void QtBrushPatternEditorPrivate::slotChangeHue(const QColor &color)
{
    QBrush brush = m_brush;
    brush.setColor(color);
    q_ptr->setBrush(brush);
}

void QtBrushPatternEditorPrivate::slotChangeSaturation(const QColor &color)
{
    QBrush brush = m_brush;
    brush.setColor(color);
    q_ptr->setBrush(brush);
}

void QtBrushPatternEditorPrivate::slotChangeValue(const QColor &color)
{
    QBrush brush = m_brush;
    brush.setColor(color);
    q_ptr->setBrush(brush);
}

void QtBrushPatternEditorPrivate::slotChangeAlpha(const QColor &color)
{
    QBrush brush = m_brush;
    brush.setColor(color);
    q_ptr->setBrush(brush);
}

QtBrushPatternEditor::QtBrushPatternEditor(QWidget *parent)
    : QWidget(parent)
{
    d_ptr = new QtBrushPatternEditorPrivate();
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);

    d_ptr->m_ui.hueColorLine->setColorComponent(QtColorLine::Hue);
    d_ptr->m_ui.saturationColorLine->setColorComponent(QtColorLine::Saturation);
    d_ptr->m_ui.valueColorLine->setColorComponent(QtColorLine::Value);
    d_ptr->m_ui.alphaColorLine->setColorComponent(QtColorLine::Alpha);

    QStringList patterns;
    patterns << tr("No Brush") << tr("Solid") << tr("Dense 1") << tr("Dense 2") << tr("Dense 3") << tr("Dense 4")
            << tr("Dense 5") << tr("Dense 6") << tr("Dense 7") << tr("Horizontal") << tr("Vertical")
            << tr("Cross") << tr("Backward Diagonal") << tr("Forward Diagonal") << tr("Crossing Diagonal");
    d_ptr->m_ui.patternComboBox->addItems(patterns);
    d_ptr->m_ui.patternComboBox->setCurrentIndex(1);

    connect(d_ptr->m_ui.patternComboBox, SIGNAL(activated(int)),
                this, SLOT(slotPatternChanged(int)));

    connect(d_ptr->m_ui.hueColorLine, SIGNAL(colorChanged(const QColor &)),
                this, SLOT(slotChangeHue(const QColor &)));
    connect(d_ptr->m_ui.saturationColorLine, SIGNAL(colorChanged(const QColor &)),
                this, SLOT(slotChangeSaturation(const QColor &)));
    connect(d_ptr->m_ui.valueColorLine, SIGNAL(colorChanged(const QColor &)),
                this, SLOT(slotChangeValue(const QColor &)));
    connect(d_ptr->m_ui.alphaColorLine, SIGNAL(colorChanged(const QColor &)),
                this, SLOT(slotChangeAlpha(const QColor &)));
    connect(d_ptr->m_ui.colorButton, SIGNAL(colorChanged(const QColor &)),
                this, SLOT(slotChangeColor(const QColor &)));

    connect(d_ptr->m_ui.hsvRadioButton, SIGNAL(clicked()),
                this, SLOT(slotHsvClicked()));
    connect(d_ptr->m_ui.rgbRadioButton, SIGNAL(clicked()),
                this, SLOT(slotRgbClicked()));

    QBrush brush(Qt::white);
    setBrush(brush);
}

QtBrushPatternEditor::~QtBrushPatternEditor()
{
    delete d_ptr;
}

void QtBrushPatternEditor::setBrush(const QBrush &brush)
{
    if (d_ptr->m_brush == brush)
        return;

    if (brush.style() == Qt::LinearGradientPattern ||
            brush.style() == Qt::RadialGradientPattern ||
            brush.style() == Qt::ConicalGradientPattern ||
            brush.style() == Qt::TexturePattern)
        return;

    d_ptr->m_brush = brush;
    d_ptr->m_ui.brushWidget->setBrush(brush);

    d_ptr->m_ui.patternComboBox->setCurrentIndex((int)d_ptr->m_brush.style());
    d_ptr->m_ui.colorButton->setColor(d_ptr->m_brush.color());
    d_ptr->m_ui.hueColorLine->setColor(d_ptr->m_brush.color());
    d_ptr->m_ui.saturationColorLine->setColor(d_ptr->m_brush.color());
    d_ptr->m_ui.valueColorLine->setColor(d_ptr->m_brush.color());
    d_ptr->m_ui.alphaColorLine->setColor(d_ptr->m_brush.color());
}

QBrush QtBrushPatternEditor::brush() const
{
    return d_ptr->m_brush;
}

#include "moc_qtbrushpatterneditor.cpp"
