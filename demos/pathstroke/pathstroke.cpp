
#include "arthurstyle.h"
#include "pathstroke.h"
#include "arthurwidgets.h"

extern void draw_round_rect(QPainter *p, const QRect &bounds, int radius);

PathStrokeWidget::PathStrokeWidget()
{
    setWindowTitle("Primitive Stroking");

    // Setting up palette.
    QPalette pal = palette();
//     pal.setBrush(QPalette::Background, Qt::white);
//     pal.setBrush(QPalette::Foreground, QColor("aquamarine"));
//     pal.setBrush(QPalette::Background, QPixmap("background.png"));
    setPalette(pal);

    // Widget construction and property setting
    m_renderer = new PathStrokeRenderer(this);

    QGroupBox *mainGroup = new ArthurGroupBox(this);
//     QWidget *mainGroup = new QWidget(this);
    mainGroup->setFixedWidth(180);
    mainGroup->setTitle("Path Stroking");

    QGroupBox *capGroup = new ArthurGroupBox(mainGroup);
    capGroup->setAttribute(Qt::WA_ContentsPropagated);
    QRadioButton *flatCap = new QRadioButton(capGroup);
    QRadioButton *squareCap = new QRadioButton(capGroup);
    QRadioButton *roundCap = new QRadioButton(capGroup);
    capGroup->setTitle("Cap Style");
    flatCap->setText("Flat Cap");
    squareCap->setText("Square Cap");
    roundCap->setText("Round Cap");

    QGroupBox *joinGroup = new ArthurGroupBox(mainGroup);
    joinGroup->setAttribute(Qt::WA_ContentsPropagated);
    QRadioButton *bevelJoin = new QRadioButton(joinGroup);
    QRadioButton *miterJoin = new QRadioButton(joinGroup);
    QRadioButton *roundJoin = new QRadioButton(joinGroup);
    joinGroup->setTitle("Join Style");
    bevelJoin->setText("Bevel Join");
    miterJoin->setText("Miter Join");
    roundJoin->setText("Round Join");

    QGroupBox *styleGroup = new ArthurGroupBox(mainGroup);
    styleGroup->setAttribute(Qt::WA_ContentsPropagated);
    QRadioButton *solidLine = new QRadioButton(styleGroup);
    QRadioButton *dashLine = new QRadioButton(styleGroup);
    QRadioButton *dotLine = new QRadioButton(styleGroup);
    QRadioButton *dashDotLine = new QRadioButton(styleGroup);
    QRadioButton *dashDotDotLine = new QRadioButton(styleGroup);
    QRadioButton *customDashLine = new QRadioButton(styleGroup);
    styleGroup->setTitle("Pen Style");
#if 0
    solidLine->setText("Solid Line");
    dashLine->setText("Dash Line");
    dotLine->setText("Dot Line");
    dashDotLine->setText("Dash Dot Line");
    dashDotDotLine->setText("Dash Dot Dot Line");
#else
    solidLine->setIcon(QPixmap(":res/images/line_solid.png"));
    dashLine->setIcon(QPixmap(":res/images/line_dashed.png"));
    dotLine->setIcon(QPixmap(":res/images/line_dotted.png"));
    dashDotLine->setIcon(QPixmap(":res/images/line_dash_dot.png"));
    dashDotDotLine->setIcon(QPixmap(":res/images/line_dash_dot_dot.png"));
    customDashLine->setText("Custom Style");

    int fixedHeight = bevelJoin->sizeHint().height();
    solidLine->setFixedHeight(fixedHeight);
    dashLine->setFixedHeight(fixedHeight);
    dotLine->setFixedHeight(fixedHeight);
    dashDotLine->setFixedHeight(fixedHeight);
    dashDotDotLine->setFixedHeight(fixedHeight);
#endif

    QGroupBox *pathModeGroup = new ArthurGroupBox(mainGroup);
    pathModeGroup->setAttribute(Qt::WA_ContentsPropagated);
    QRadioButton *curveMode = new QRadioButton(pathModeGroup);
    QRadioButton *lineMode = new QRadioButton(pathModeGroup);
    pathModeGroup->setTitle("Path composed of");
    curveMode->setText("Curves");
    lineMode->setText("Lines");

    QGroupBox *penWidthGroup = new ArthurGroupBox(mainGroup);
    penWidthGroup->setAttribute(Qt::WA_ContentsPropagated);
    QSlider *penWidth = new QSlider(Qt::Horizontal, penWidthGroup);
    penWidth->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    penWidthGroup->setTitle("Pen Width");
    penWidth->setRange(0, 500);

#if 0
    QCheckBox *animated = new QCheckBox(mainGroup);
    animated->setText("Animated");
#else
    QPushButton *animated = new QPushButton(mainGroup);
    animated->setText("Animate");
    animated->setCheckable(true);
#endif

    QPushButton *showSourceButton = new QPushButton(mainGroup);
    showSourceButton->setText("Show Source");

    QPushButton *whatsThisButton = new QPushButton(mainGroup);
    whatsThisButton->setText("What's This?");
    whatsThisButton->setCheckable(true);

    // Layouting
    QHBoxLayout *viewLayout = new QHBoxLayout(this);
    viewLayout->addWidget(m_renderer);
    viewLayout->addWidget(mainGroup);

    QVBoxLayout *mainGroupLayout = new QVBoxLayout(mainGroup);
    mainGroupLayout->setMargin(3);
    mainGroupLayout->addWidget(capGroup);
    mainGroupLayout->addWidget(joinGroup);
    mainGroupLayout->addWidget(styleGroup);
    mainGroupLayout->addWidget(penWidthGroup);
    mainGroupLayout->addWidget(pathModeGroup);
    mainGroupLayout->addWidget(animated);
    mainGroupLayout->addStretch(1);
    mainGroupLayout->addWidget(showSourceButton);
    mainGroupLayout->addWidget(whatsThisButton);

    QVBoxLayout *capGroupLayout = new QVBoxLayout(capGroup);
    capGroupLayout->addWidget(flatCap);
    capGroupLayout->addWidget(squareCap);
    capGroupLayout->addWidget(roundCap);

    QVBoxLayout *joinGroupLayout = new QVBoxLayout(joinGroup);
    joinGroupLayout->addWidget(bevelJoin);
    joinGroupLayout->addWidget(miterJoin);
    joinGroupLayout->addWidget(roundJoin);

    QVBoxLayout *styleGroupLayout = new QVBoxLayout(styleGroup);
    styleGroupLayout->addWidget(solidLine);
    styleGroupLayout->addWidget(dashLine);
    styleGroupLayout->addWidget(dotLine);
    styleGroupLayout->addWidget(dashDotLine);
    styleGroupLayout->addWidget(dashDotDotLine);
    styleGroupLayout->addWidget(customDashLine);

    QVBoxLayout *pathModeGroupLayout = new QVBoxLayout(pathModeGroup);
    pathModeGroupLayout->addWidget(curveMode);
    pathModeGroupLayout->addWidget(lineMode);

    QVBoxLayout *penWidthLayout = new QVBoxLayout(penWidthGroup);
    penWidthLayout->addWidget(penWidth);

    // Set up connections
    connect(penWidth, SIGNAL(valueChanged(int)),
            m_renderer, SLOT(setPenWidth(int)));
    connect(animated, SIGNAL(toggled(bool)),
            m_renderer, SLOT(setAnimation(bool)));

    connect(flatCap, SIGNAL(clicked()), m_renderer, SLOT(setFlatCap()));
    connect(squareCap, SIGNAL(clicked()), m_renderer, SLOT(setSquareCap()));
    connect(roundCap, SIGNAL(clicked()), m_renderer, SLOT(setRoundCap()));

    connect(bevelJoin, SIGNAL(clicked()), m_renderer, SLOT(setBevelJoin()));
    connect(miterJoin, SIGNAL(clicked()), m_renderer, SLOT(setMiterJoin()));
    connect(roundJoin, SIGNAL(clicked()), m_renderer, SLOT(setRoundJoin()));

    connect(curveMode, SIGNAL(clicked()), m_renderer, SLOT(setCurveMode()));
    connect(lineMode, SIGNAL(clicked()), m_renderer, SLOT(setLineMode()));

    connect(solidLine, SIGNAL(clicked()), m_renderer, SLOT(setSolidLine()));
    connect(dashLine, SIGNAL(clicked()), m_renderer, SLOT(setDashLine()));
    connect(dotLine, SIGNAL(clicked()), m_renderer, SLOT(setDotLine()));
    connect(dashDotLine, SIGNAL(clicked()), m_renderer, SLOT(setDashDotLine()));
    connect(dashDotDotLine, SIGNAL(clicked()), m_renderer, SLOT(setDashDotDotLine()));
    connect(customDashLine, SIGNAL(clicked()), m_renderer, SLOT(setCustomDashLine()));

    connect(showSourceButton, SIGNAL(clicked()), m_renderer, SLOT(showSource()));

    connect(whatsThisButton, SIGNAL(clicked(bool)), m_renderer, SLOT(setDescriptionEnabled(bool)));
    connect(m_renderer, SIGNAL(descriptionEnabledChanged(bool)),
            whatsThisButton, SLOT(setChecked(bool)));

    // Set the defaults
    animated->setChecked(true);
    flatCap->setChecked(true);
    bevelJoin->setChecked(true);
    penWidth->setValue(50);
    curveMode->setChecked(true);
    solidLine->setChecked(true);

    m_renderer->loadSourceFile(":res/pathstroke.cpp");
    m_renderer->loadDescription(":res/pathstroke.html");
}


PathStrokeRenderer::PathStrokeRenderer(QWidget *parent)
    : ArthurFrame(parent)
{
    m_pointSize = 10;
    m_activePoint = -1;
    m_capStyle = Qt::FlatCap;
    m_joinStyle = Qt::BevelJoin;
    m_pathMode = CurveMode;
    m_penWidth = 1;
    m_penStyle = Qt::SolidLine;
    m_wasAnimated = true;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PathStrokeRenderer::paint(QPainter *painter)
{
    if (m_points.isEmpty())
        initializePoints();

    painter->setRenderHint(QPainter::Antialiasing);

    QPalette pal = palette();
    painter->setPen(Qt::NoPen);

    // Construct the path
    QPainterPath path;
    path.moveTo(m_points.at(0));

    if (m_pathMode == LineMode) {
        for (int i=1; i<m_points.size(); ++i) {
            path.lineTo(m_points.at(i));
        }
    } else {
        int i=1;
        while (i + 2 < m_points.size()) {
            path.cubicTo(m_points.at(i), m_points.at(i+1), m_points.at(i+2));
            i += 3;
        }
        while (i < m_points.size()) {
            path.lineTo(m_points.at(i));
            ++i;
        }
    }

    // Draw the path
    {
        QColor lg = Qt::red;

        // The "custom" pen
        if (m_penStyle == Qt::NoPen) {
            QPainterPathStroker stroker;
            stroker.setWidth(m_penWidth);
            stroker.setJoinStyle(m_joinStyle);
            stroker.setCapStyle(m_capStyle);

            QVector<qreal> dashes;
            qreal space = 4;
            dashes << 1 << space
                   << 3 << space
                   << 9 << space
                   << 27 << space
                   << 9 << space
                   << 3 << space;
            stroker.setDashPattern(dashes);
            QPainterPath stroke = stroker.createStroke(path);
            painter->fillPath(stroke, lg);

        } else {
            QPen pen(lg, m_penWidth, m_penStyle, m_capStyle, m_joinStyle);
            painter->strokePath(path, pen);
        }
    }

    if (1) {
        // Draw the control points
        painter->setPen(QColor(50, 100, 120, 200));
        painter->setBrush(QColor(200, 200, 210, 120));
        for (int i=0; i<m_points.size(); ++i) {
            QPointF pos = m_points.at(i);
            painter->drawEllipse(QRectF(pos.x() - m_pointSize,
                                       pos.y() - m_pointSize,
                                       m_pointSize*2, m_pointSize*2));
        }
        painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolyline(m_points);
    }

}

void PathStrokeRenderer::initializePoints()
{
    const int count = 7;
    m_points.clear();
    m_vectors.clear();

    QMatrix m;
    double rot = 360 / count;
    QPointF center(width() / 2, height() / 2);
    QMatrix vm;
    vm.shear(2, -1);
    vm.scale(3, 3);

    for (int i=0; i<count; ++i) {
        m_vectors << QPointF(.1f, .25f) * (m * vm);
        m_points << QPointF(0, 100) * m + center;
        m.rotate(rot);
    }
}

void PathStrokeRenderer::updatePoints()
{
    double pad = 10;
    double left = pad;
    double right = width() - pad;
    double top = pad;
    double bottom = height() - pad;

    Q_ASSERT(m_points.size() == m_vectors.size());
    for (int i=0; i<m_points.size(); ++i) {

        if (i == m_activePoint)
            continue;

        QPointF pos = m_points.at(i);
        QPointF vec = m_vectors.at(i);
        pos += vec;
        if (pos.x() < left || pos.x() > right) {
            vec.setX(-vec.x());
            pos.setX(pos.x() < left ? left : right);
        } if (pos.y() < top || pos.y() > bottom) {
            vec.setY(-vec.y());
            pos.setY(pos.y() < top ? top : bottom);
        }
        m_points[i] = pos;
        m_vectors[i] = vec;
    }
    update();
}

void PathStrokeRenderer::mousePressEvent(QMouseEvent *e)
{
    setDescriptionEnabled(false);
    m_activePoint = -1;
    qreal distance = -1;
    for (int i=0; i<m_points.size(); ++i) {
        qreal d = QLineF(e->pos(), m_points.at(i)).length();
        if ((distance < 0 && d < 8 * m_pointSize) || d < distance) {
            distance = d;
            m_activePoint = i;
        }
    }

    if (m_activePoint != -1) {
        m_wasAnimated = m_timer.isActive();
        setAnimation(false);
        mouseMoveEvent(e);
    }
}

void PathStrokeRenderer::mouseMoveEvent(QMouseEvent *e)
{
    if (m_activePoint >= 0 && m_activePoint < m_points.size()) {
        m_points[m_activePoint] = e->pos();
        update();
    }
}

void PathStrokeRenderer::mouseReleaseEvent(QMouseEvent *)
{
    m_activePoint = -1;
    setAnimation(m_wasAnimated);
}

void PathStrokeRenderer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timer.timerId()) {
        updatePoints();
        QApplication::syncX();
    } // else if (e->timerId() == m_fpsTimer.timerId()) {
//         emit frameRate(m_frameCount);
//         m_frameCount = 0;
//     }
}

void PathStrokeRenderer::setAnimation(bool animation)
{
    m_timer.stop();
//     m_fpsTimer.stop();

    if (animation) {
        m_timer.start(25, this);
//         m_fpsTimer.start(1000, this);
//         m_frameCount = 0;
    }
}


