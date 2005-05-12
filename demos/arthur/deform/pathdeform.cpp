#include "pathdeform.h"

#include <qapplication.h>
#include <qdebug.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qevent.h>

#include <math.h>


PathDeformWidget::PathDeformWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Vector Deformation");

    m_renderer = new PathDeformRenderer(this);
    m_renderer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ArthurGroupBox *mainGroup = new ArthurGroupBox(this);
    mainGroup->setTitle("Vector Deformation");

    ArthurGroupBox *radiusGroup = new ArthurGroupBox(mainGroup);
    radiusGroup->setAttribute(Qt::WA_ContentsPropagated);
    radiusGroup->setTitle("Lens radius");
    QSlider *radiusSlider = new QSlider(Qt::Horizontal, radiusGroup);
    radiusSlider->setRange(50, 150);
    radiusSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    ArthurGroupBox *deformGroup = new ArthurGroupBox(mainGroup);
    deformGroup->setAttribute(Qt::WA_ContentsPropagated);
    deformGroup->setTitle("Deformation");
    QSlider *deformSlider = new QSlider(Qt::Horizontal, deformGroup);
    deformSlider->setRange(-100, 100);
    deformSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    ArthurGroupBox *fontSizeGroup = new ArthurGroupBox(mainGroup);
    fontSizeGroup->setAttribute(Qt::WA_ContentsPropagated);
    fontSizeGroup->setTitle("Font Size");
    QSlider *fontSizeSlider = new QSlider(Qt::Horizontal, fontSizeGroup);
    fontSizeSlider->setRange(16, 200);
    fontSizeSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    ArthurGroupBox *textGroup = new ArthurGroupBox(mainGroup);
    textGroup->setAttribute(Qt::WA_ContentsPropagated);
    textGroup->setTitle("Text");
    QLineEdit *textInput = new QLineEdit(textGroup);

    QPushButton *animateButton = new QPushButton(mainGroup);
    animateButton->setText("Animated");
    animateButton->setCheckable(true);

    QPushButton *showSourceButton = new QPushButton(mainGroup);
    showSourceButton->setText("Show Source");
//     showSourceButton->setCheckable(true);

    QPushButton *whatsThisButton = new QPushButton(mainGroup);
    whatsThisButton->setText("What's This?");
    whatsThisButton->setCheckable(true);

    // Layouts
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_renderer);
    mainLayout->addWidget(mainGroup);
    mainGroup->setFixedWidth(180);

    QVBoxLayout *mainGroupLayout = new QVBoxLayout(mainGroup);
    mainGroupLayout->addWidget(radiusGroup);
    mainGroupLayout->addWidget(deformGroup);
    mainGroupLayout->addWidget(fontSizeGroup);
    mainGroupLayout->addWidget(textGroup);
    mainGroupLayout->addWidget(animateButton);
    mainGroupLayout->addStretch(1);
    mainGroupLayout->addWidget(showSourceButton);
    mainGroupLayout->addWidget(whatsThisButton);

    QVBoxLayout *radiusGroupLayout = new QVBoxLayout(radiusGroup);
    radiusGroupLayout->addWidget(radiusSlider);

    QVBoxLayout *deformGroupLayout = new QVBoxLayout(deformGroup);
    deformGroupLayout->addWidget(deformSlider);

    QVBoxLayout *fontSizeGroupLayout = new QVBoxLayout(fontSizeGroup);
    fontSizeGroupLayout->addWidget(fontSizeSlider);

    QVBoxLayout *textGroupLayout = new QVBoxLayout(textGroup);
    textGroupLayout->addWidget(textInput);

    connect(textInput, SIGNAL(textChanged(QString)), m_renderer, SLOT(setText(QString)));
    connect(radiusSlider, SIGNAL(valueChanged(int)), m_renderer, SLOT(setRadius(int)));
    connect(deformSlider, SIGNAL(valueChanged(int)), m_renderer, SLOT(setIntensity(int)));
    connect(fontSizeSlider, SIGNAL(valueChanged(int)), m_renderer, SLOT(setFontSize(int)));
    connect(animateButton, SIGNAL(clicked(bool)), m_renderer, SLOT(setAnimated(bool)));
    connect(whatsThisButton, SIGNAL(clicked(bool)), m_renderer, SLOT(setDescriptionEnabled(bool)));
    connect(showSourceButton, SIGNAL(clicked()), m_renderer, SLOT(showSource()));
    connect(m_renderer, SIGNAL(descriptionEnabledChanged(bool)),
            whatsThisButton, SLOT(setChecked(bool)));

    animateButton->animateClick();
    deformSlider->setValue(80);
    radiusSlider->setValue(100);
    fontSizeSlider->setValue(120);
    textInput->setText("Qt");

    m_renderer->loadSourceFile(":res/pathdeform.cpp");
    m_renderer->loadDescription(":res/pathdeform.html");
    m_renderer->setDescriptionEnabled(false);
}


static inline QRect circle_bounds(const QPointF &center, double radius, double compensation)
{
    return QRect(qRound(center.x() - radius - compensation),
                 qRound(center.y() - radius - compensation),
                 qRound((radius + compensation) * 2),
                 qRound((radius + compensation) * 2));

}

const int LENS_EXTENT = 10;

PathDeformRenderer::PathDeformRenderer(QWidget *widget)
    : ArthurFrame(widget)
{
    m_radius = 100;
    m_pos = QPointF(m_radius, m_radius);
    m_direction = QPointF(1, 1);
    m_fontSize = 24;
    m_animated = true;
    m_repaintTimer.start(25, this);
    m_repaintTracker.start();
    m_intensity = 1;

//     m_fpsTimer.start(1000, this);
//     m_fpsCounter = 0;

    generateLensPixmap();
}

void PathDeformRenderer::setText(const QString &text)
{
    m_text = text;

    QFont f("times new roman,utopia");
    f.setStyleStrategy(QFont::ForceOutline);
    f.setPointSize(m_fontSize);
    f.setStyleHint(QFont::Times);

    QFontMetrics fm(f);

    m_paths.clear();
    m_pathBounds = QRect();

    QPointF advance(0, 0);

    bool do_quick = true;
    for (int i=0; i<text.size(); ++i) {
        if (text.at(i).unicode() >= 0x4ff && text.at(i).unicode() <= 0x1e00) {
            do_quick = false;
            break;
        }
    }

    if (do_quick) {
        for (int i=0; i<text.size(); ++i) {
            QPainterPath path;
            path.addText(advance, f, text.mid(i, 1));
            m_pathBounds |= path.boundingRect();
            m_paths << path;
            advance += QPointF(fm.width(text.mid(i, 1)), 0);
        }
    } else {
        QPainterPath path;
        path.addText(advance, f, text);
        m_pathBounds |= path.boundingRect();
        m_paths << path;
    }

    for (int i=0; i<m_paths.size(); ++i)
        m_paths[i] = m_paths[i] * QMatrix(1, 0, 0, 1, -m_pathBounds.x(), -m_pathBounds.y());

    update();
}


void PathDeformRenderer::generateLensPixmap()
{
    double rad = m_radius + LENS_EXTENT;

    QRect bounds = circle_bounds(QPointF(), rad, 0);

    m_pixmap = QPixmap(bounds.size());
    m_pixmap.fill(QColor(0, 0, 0, 0));

    QPainter painter(&m_pixmap);

    QRadialGradient gr(rad, rad, rad, 3 * rad / 5, 3 * rad / 5);
    gr.setColorAt(0.0, QColor(255, 255, 255, 191));
    gr.setColorAt(0.2, QColor(255, 255, 127, 191));
    gr.setColorAt(0.9, QColor(150, 150, 200, 63));
    gr.setColorAt(0.95, QColor(0, 0, 0, 127));
    gr.setColorAt(1, QColor(0, 0, 0, 0));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(gr);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, bounds.width(), bounds.height());
}


void PathDeformRenderer::setAnimated(bool animated)
{
    m_animated = animated;

    if (m_animated) {
//         m_fpsTimer.start(1000, this);
//         m_fpsCounter = 0;
        m_repaintTimer.start(25, this);
        m_repaintTracker.start();
    } else {
//         m_fpsTimer.stop();
        m_repaintTimer.stop();
    }
}

void PathDeformRenderer::timerEvent(QTimerEvent *e)
{

    if (e->timerId() == m_repaintTimer.timerId()) {

        if (QLineF(QPointF(0,0), m_direction).length() > 1)
            m_direction *= 0.995;
        double time = m_repaintTracker.restart();

        QRect rectBefore = circle_bounds(m_pos, m_radius, m_fontSize);

        double dx = m_direction.x();
        double dy = m_direction.y();
        if (time > 0) {
            dx = dx * time * .1;
            dy = dy * time * .1;
        }

        m_pos += QPointF(dx, dy);



        if (m_pos.x() - m_radius < 0) {
            m_direction.setX(-m_direction.x());
            m_pos.setX(m_radius);
        } else if (m_pos.x() + m_radius > width()) {
            m_direction.setX(-m_direction.x());
            m_pos.setX(width() - m_radius);
        }

        if (m_pos.y() - m_radius < 0) {
            m_direction.setY(-m_direction.y());
            m_pos.setY(m_radius);
        } else if (m_pos.y() + m_radius > height()) {
            m_direction.setY(-m_direction.y());
            m_pos.setY(height() - m_radius);
        }

        QRect rectAfter = circle_bounds(m_pos, m_radius, m_fontSize);
        update(QRect(rectBefore | rectAfter));
        QApplication::syncX();
    }
//     else if (e->timerId() == m_fpsTimer.timerId()) {
//         printf("fps: %d\n", m_fpsCounter);
//         emit frameRate(m_fpsCounter);
//         m_fpsCounter = 0;

//     }
}

void PathDeformRenderer::mousePressEvent(QMouseEvent *e)
{
    setDescriptionEnabled(false);

    m_repaintTimer.stop();
    m_offset = QPointF();
    if (QLineF(m_pos, e->pos()).length() <= m_radius)
        m_offset = m_pos - e->pos();

    mouseMoveEvent(e);
}

void PathDeformRenderer::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::NoButton && m_animated) {
        m_repaintTimer.start(10, this);
        m_repaintTracker.start();
    }
}

void PathDeformRenderer::mouseMoveEvent(QMouseEvent *e)
{
    QRect rectBefore = circle_bounds(m_pos, m_radius, m_fontSize);
    if (e->type() == QEvent::MouseMove) {
        QLineF line(m_pos, e->pos() + m_offset);
        line.setLength(line.length() * .1);
        QPointF dir(line.dx(), line.dy());
        m_direction = (m_direction + dir) / 2;
    }
    m_pos = e->pos() + m_offset;
    QRect rectAfter = circle_bounds(m_pos, m_radius, m_fontSize);
    update(rectBefore | rectAfter);
}

QPainterPath PathDeformRenderer::lensDeform(const QPainterPath &source, const QPointF &offset)
{
    QPainterPath path;
    path.addPath(source);

    double flip = m_intensity;

    for (int i=0; i<path.elementCount(); ++i) {
        QPainterPath::Element &e = const_cast<QPainterPath::Element &>(path.elementAt(i));

        double x = e.x + offset.x();
        double y = e.y + offset.y();

        double dx = x - m_pos.x();
        double dy = y - m_pos.y();
        double len = m_radius - sqrt(dx * dx + dy * dy);

        if (len > 0) {
            e.x = x + flip * dx * len / m_radius;
            e.y = y + flip * dy * len / m_radius;
        } else {
            e.x = x;
            e.y = y;
        }

    }

    return path;
}


void PathDeformRenderer::paint(QPainter *painter)
{
    int pad_x = 5;
    int pad_y = 5;

    int skip_x = qRound(m_pathBounds.width() + pad_x + m_fontSize/2);
    int skip_y = qRound(m_pathBounds.height() + pad_y);

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);

    QRectF clip(painter->clipPath().boundingRect());

    int overlap = pad_x / 2;

    for (int start_y=0; start_y < height(); start_y += skip_y) {

        if (start_y > clip.bottom())
            break;

        int start_x = -overlap;
        for (; start_x < width(); start_x += skip_x) {

            if (start_y + skip_y >= clip.top() &&
                start_x + skip_x >= clip.left() &&
                start_x <= clip.right()) {
                for (int i=0; i<m_paths.size(); ++i) {
                    QPainterPath path = lensDeform(m_paths[i], QPointF(start_x, start_y));
                    painter->drawPath(path);
                }
            }
        }
        overlap = skip_x - (start_x - width());

    }

    painter->drawPixmap(m_pos - QPointF(m_radius + LENS_EXTENT, m_radius + LENS_EXTENT), m_pixmap);

//     ++m_fpsCounter;
}



void PathDeformRenderer::setRadius(int radius)
{
    double max = qMax(m_radius, (double)radius);
    m_radius = radius;
    generateLensPixmap();
    if (!m_animated || m_radius < max)
        update(circle_bounds(m_pos, max, m_fontSize));
}

void PathDeformRenderer::setIntensity(int intensity)
{
    m_intensity = intensity / 100.0;
    if (!m_animated)
        update(circle_bounds(m_pos, m_radius, m_fontSize));
}
