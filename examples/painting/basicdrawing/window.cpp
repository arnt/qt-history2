#include <QtGui>

#include "renderarea.h"
#include "window.h"

const int IdRole = Qt::UserRole;

Window::Window()
{
    renderArea = new RenderArea(this);

    shapeComboBox = new QComboBox(this);
    addItem(shapeComboBox, tr("Rectangle"), RenderArea::Rect);
    addItem(shapeComboBox, tr("Round Rectangle"), RenderArea::RoundRect);
    addItem(shapeComboBox, tr("Ellipse"), RenderArea::Ellipse);
    addItem(shapeComboBox, tr("Pie"), RenderArea::Pie);
    addItem(shapeComboBox, tr("Chord"), RenderArea::Chord);
    addItem(shapeComboBox, tr("Polygon"), RenderArea::Polygon);
    addItem(shapeComboBox, tr("Path"), RenderArea::Path);
    addItem(shapeComboBox, tr("Line"), RenderArea::Line);
    addItem(shapeComboBox, tr("Polyline"), RenderArea::Polyline);
    addItem(shapeComboBox, tr("Arc"), RenderArea::Arc);
    addItem(shapeComboBox, tr("Points"), RenderArea::Points);
    addItem(shapeComboBox, tr("Text"), RenderArea::Text);
    addItem(shapeComboBox, tr("Pixmap"), RenderArea::Pixmap);

    shapeLabel = new QLabel(tr("&Shape:"), this);
    shapeLabel->setBuddy(shapeComboBox);

    penWidthSpinBox = new QSpinBox(this);
    penWidthSpinBox->setRange(0, 20);

    penWidthLabel = new QLabel(tr("Pen &Width:"), this);
    penWidthLabel->setBuddy(penWidthSpinBox);

    penStyleComboBox = new QComboBox(this);
    addItem(penStyleComboBox, tr("Solid"), Qt::SolidLine);
    addItem(penStyleComboBox, tr("Dash"), Qt::DashLine);
    addItem(penStyleComboBox, tr("Dot"), Qt::DotLine);
    addItem(penStyleComboBox, tr("Dash Dot"), Qt::DashDotLine);
    addItem(penStyleComboBox, tr("Dash Dot Dot"), Qt::DashDotDotLine);
    addItem(penStyleComboBox, tr("None"), Qt::NoPen);

    penStyleLabel = new QLabel(tr("&Pen Style:"), this);
    penStyleLabel->setBuddy(penStyleComboBox);

    penCapComboBox = new QComboBox(this);
    addItem(penCapComboBox, tr("Flat"), Qt::FlatCap);
    addItem(penCapComboBox, tr("Square"), Qt::SquareCap);
    addItem(penCapComboBox, tr("Round"), Qt::RoundCap);

    penCapLabel = new QLabel(tr("Pen &Cap:"), this);
    penCapLabel->setBuddy(penCapComboBox);

    penJoinComboBox = new QComboBox(this);
    addItem(penJoinComboBox, tr("Miter"), Qt::MiterJoin);
    addItem(penJoinComboBox, tr("Bevel"), Qt::BevelJoin);
    addItem(penJoinComboBox, tr("Round"), Qt::RoundJoin);

    penJoinLabel = new QLabel(tr("Pen &Join:"), this);
    penJoinLabel->setBuddy(penJoinComboBox);

    brushStyleComboBox = new QComboBox(this);
    addItem(brushStyleComboBox, tr("Linear Gradient"),
            Qt::LinearGradientPattern);
    addItem(brushStyleComboBox, tr("Texture"), Qt::TexturePattern);
    addItem(brushStyleComboBox, tr("Solid"), Qt::SolidPattern);
    addItem(brushStyleComboBox, tr("Horizontal"), Qt::HorPattern);
    addItem(brushStyleComboBox, tr("Verical"), Qt::VerPattern);
    addItem(brushStyleComboBox, tr("Cross"), Qt::CrossPattern);
    addItem(brushStyleComboBox, tr("Backward Diagonal"), Qt::BDiagPattern);
    addItem(brushStyleComboBox, tr("Forward Diagonal"), Qt::FDiagPattern);
    addItem(brushStyleComboBox, tr("Diagonal Cross"), Qt::DiagCrossPattern);
    addItem(brushStyleComboBox, tr("Dense 1"), Qt::Dense1Pattern);
    addItem(brushStyleComboBox, tr("Dense 2"), Qt::Dense2Pattern);
    addItem(brushStyleComboBox, tr("Dense 3"), Qt::Dense3Pattern);
    addItem(brushStyleComboBox, tr("Dense 4"), Qt::Dense4Pattern);
    addItem(brushStyleComboBox, tr("Dense 5"), Qt::Dense5Pattern);
    addItem(brushStyleComboBox, tr("Dense 6"), Qt::Dense6Pattern);
    addItem(brushStyleComboBox, tr("Dense 7"), Qt::Dense7Pattern);
    addItem(brushStyleComboBox, tr("None"), Qt::NoBrush);

    brushStyleLabel = new QLabel(tr("&Brush Style:"), this);
    brushStyleLabel->setBuddy(brushStyleComboBox);

    antialiasingCheckBox = new QCheckBox(tr("&Antialiasing"), this);
    transformationsCheckBox = new QCheckBox(tr("&Transformations"), this);

    connect(shapeComboBox, SIGNAL(activated(int)),
            this, SLOT(shapeChanged()));
    connect(penWidthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(penChanged()));
    connect(penStyleComboBox, SIGNAL(activated(int)),
            this, SLOT(penChanged()));
    connect(penCapComboBox, SIGNAL(activated(int)),
            this, SLOT(penChanged()));
    connect(penJoinComboBox, SIGNAL(activated(int)),
            this, SLOT(penChanged()));
    connect(brushStyleComboBox, SIGNAL(activated(int)),
            this, SLOT(brushChanged()));
    connect(antialiasingCheckBox, SIGNAL(toggled(bool)),
            renderArea, SLOT(setAntialiased(bool)));
    connect(transformationsCheckBox, SIGNAL(toggled(bool)),
            renderArea, SLOT(setTransformed(bool)));

    QHBoxLayout *checkBoxLayout = new QHBoxLayout;
    checkBoxLayout->addWidget(antialiasingCheckBox);
    checkBoxLayout->addWidget(transformationsCheckBox);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(renderArea, 0, 0, 1, 2);
    mainLayout->addWidget(shapeLabel, 1, 0);
    mainLayout->addWidget(shapeComboBox, 1, 1);
    mainLayout->addWidget(penWidthLabel, 2, 0);
    mainLayout->addWidget(penWidthSpinBox, 2, 1);
    mainLayout->addWidget(penStyleLabel, 3, 0);
    mainLayout->addWidget(penStyleComboBox, 3, 1);
    mainLayout->addWidget(penCapLabel, 4, 0);
    mainLayout->addWidget(penCapComboBox, 4, 1);
    mainLayout->addWidget(penJoinLabel, 5, 0);
    mainLayout->addWidget(penJoinComboBox, 5, 1);
    mainLayout->addWidget(brushStyleLabel, 6, 0);
    mainLayout->addWidget(brushStyleComboBox, 6, 1);
    mainLayout->addLayout(checkBoxLayout, 7, 0, 1, 2);

    shapeChanged();
    penChanged();
    brushChanged();
    penChanged();
    renderArea->setAntialiased(false);
    renderArea->setTransformed(false);

    setWindowTitle(tr("Basic Drawing"));
}

void Window::shapeChanged()
{
    RenderArea::Shape shape =
        (RenderArea::Shape)shapeComboBox->itemData(shapeComboBox->currentIndex(), IdRole).toInt();
    renderArea->setShape(shape);
}

void Window::penChanged()
{
    int width = penWidthSpinBox->value();
    Qt::PenStyle style =
        (Qt::PenStyle)penStyleComboBox->itemData(penStyleComboBox->currentIndex(), IdRole).toInt();
    Qt::PenCapStyle cap =
        (Qt::PenCapStyle)penCapComboBox->itemData(penCapComboBox->currentIndex(), IdRole).toInt();
    Qt::PenJoinStyle join =
        (Qt::PenJoinStyle)penJoinComboBox->itemData(penJoinComboBox->currentIndex(), IdRole).toInt();

    renderArea->setPen(QPen(Qt::blue, width, style, cap, join));
}

void Window::brushChanged()
{
    Qt::BrushStyle style =
        (Qt::BrushStyle)brushStyleComboBox->itemData(brushStyleComboBox->currentIndex(), IdRole).toInt();

    if (style == Qt::LinearGradientPattern) {
        QLinearGradient lg(0, 0, 400, 200);
        lg.appendStop(0, Qt::red);
        lg.appendStop(1, Qt::green);
        renderArea->setBrush(lg);
    } else if (style == Qt::TexturePattern) {
        renderArea->setBrush(QBrush(QPixmap(":/images/brick.png")));
    } else {
        renderArea->setBrush(QBrush(Qt::red, style));
    }
}

void Window::addItem(QComboBox *comboBox, const QString &text, int id)
{
    int row = comboBox->count();
    comboBox->insertItem(row, text);
    comboBox->setItemData(row, id, IdRole);
}
