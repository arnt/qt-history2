#include <QtGui>

#include "renderarea.h"
#include "window.h"

const int IdRole = Qt::UserRole;

Window::Window()
{
    renderArea = new RenderArea(this);

    shapeComboBox = new QComboBox(this);
    shapeComboBox->addItem(tr("Rectangle"), RenderArea::Rect);
    shapeComboBox->addItem(tr("Round Rectangle"), RenderArea::RoundRect);
    shapeComboBox->addItem(tr("Ellipse"), RenderArea::Ellipse);
    shapeComboBox->addItem(tr("Pie"), RenderArea::Pie);
    shapeComboBox->addItem(tr("Chord"), RenderArea::Chord);
    shapeComboBox->addItem(tr("Polygon"), RenderArea::Polygon);
    shapeComboBox->addItem(tr("Path"), RenderArea::Path);
    shapeComboBox->addItem(tr("Line"), RenderArea::Line);
    shapeComboBox->addItem(tr("Polyline"), RenderArea::Polyline);
    shapeComboBox->addItem(tr("Arc"), RenderArea::Arc);
    shapeComboBox->addItem(tr("Points"), RenderArea::Points);
    shapeComboBox->addItem(tr("Text"), RenderArea::Text);
    shapeComboBox->addItem(tr("Pixmap"), RenderArea::Pixmap);

    shapeLabel = new QLabel(tr("&Shape:"), this);
    shapeLabel->setBuddy(shapeComboBox);

    penWidthSpinBox = new QSpinBox(this);
    penWidthSpinBox->setRange(0, 20);

    penWidthLabel = new QLabel(tr("Pen &Width:"), this);
    penWidthLabel->setBuddy(penWidthSpinBox);

    penStyleComboBox = new QComboBox(this);
    penStyleComboBox->addItem(tr("Solid"), Qt::SolidLine);
    penStyleComboBox->addItem(tr("Dash"), Qt::DashLine);
    penStyleComboBox->addItem(tr("Dot"), Qt::DotLine);
    penStyleComboBox->addItem(tr("Dash Dot"), Qt::DashDotLine);
    penStyleComboBox->addItem(tr("Dash Dot Dot"), Qt::DashDotDotLine);
    penStyleComboBox->addItem(tr("None"), Qt::NoPen);

    penStyleLabel = new QLabel(tr("&Pen Style:"), this);
    penStyleLabel->setBuddy(penStyleComboBox);

    penCapComboBox = new QComboBox(this);
    penCapComboBox->addItem(tr("Flat"), Qt::FlatCap);
    penCapComboBox->addItem(tr("Square"), Qt::SquareCap);
    penCapComboBox->addItem(tr("Round"), Qt::RoundCap);

    penCapLabel = new QLabel(tr("Pen &Cap:"), this);
    penCapLabel->setBuddy(penCapComboBox);

    penJoinComboBox = new QComboBox(this);
    penJoinComboBox->addItem(tr("Miter"), Qt::MiterJoin);
    penJoinComboBox->addItem(tr("Bevel"), Qt::BevelJoin);
    penJoinComboBox->addItem(tr("Round"), Qt::RoundJoin);

    penJoinLabel = new QLabel(tr("Pen &Join:"), this);
    penJoinLabel->setBuddy(penJoinComboBox);

    brushStyleComboBox = new QComboBox(this);
    brushStyleComboBox->addItem(tr("Linear Gradient"),
            Qt::LinearGradientPattern);
    brushStyleComboBox->addItem(tr("Texture"), Qt::TexturePattern);
    brushStyleComboBox->addItem(tr("Solid"), Qt::SolidPattern);
    brushStyleComboBox->addItem(tr("Horizontal"), Qt::HorPattern);
    brushStyleComboBox->addItem(tr("Verical"), Qt::VerPattern);
    brushStyleComboBox->addItem(tr("Cross"), Qt::CrossPattern);
    brushStyleComboBox->addItem(tr("Backward Diagonal"), Qt::BDiagPattern);
    brushStyleComboBox->addItem(tr("Forward Diagonal"), Qt::FDiagPattern);
    brushStyleComboBox->addItem(tr("Diagonal Cross"), Qt::DiagCrossPattern);
    brushStyleComboBox->addItem(tr("Dense 1"), Qt::Dense1Pattern);
    brushStyleComboBox->addItem(tr("Dense 2"), Qt::Dense2Pattern);
    brushStyleComboBox->addItem(tr("Dense 3"), Qt::Dense3Pattern);
    brushStyleComboBox->addItem(tr("Dense 4"), Qt::Dense4Pattern);
    brushStyleComboBox->addItem(tr("Dense 5"), Qt::Dense5Pattern);
    brushStyleComboBox->addItem(tr("Dense 6"), Qt::Dense6Pattern);
    brushStyleComboBox->addItem(tr("Dense 7"), Qt::Dense7Pattern);
    brushStyleComboBox->addItem(tr("None"), Qt::NoBrush);

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
        lg.setColorAt(0, Qt::red);
        lg.setColorAt(1, Qt::green);
        renderArea->setBrush(lg);
    } else if (style == Qt::TexturePattern) {
        renderArea->setBrush(QBrush(QPixmap(":/images/brick.png")));
    } else {
        renderArea->setBrush(QBrush(Qt::red, style));
    }
}
