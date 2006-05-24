/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "composition.h"
#include <QBoxLayout>
#include <QRadioButton>
#include <QTimer>
#include <QDateTime>
#include <QSlider>
#include <QMouseEvent>
#include <math.h>

CompositionWidget::CompositionWidget(QWidget *parent)
    : QWidget(parent)
{
    CompositionRenderer *view = new CompositionRenderer(this);

    QGroupBox *mainGroup = new QGroupBox(parent);
    mainGroup->setAttribute(Qt::WA_ContentsPropagated);
    mainGroup->setTitle("Composition Modes");

    QGroupBox *modesGroup = new QGroupBox(mainGroup);
    modesGroup->setAttribute(Qt::WA_ContentsPropagated);
    modesGroup->setTitle("Mode");

    rbClear = new QRadioButton("Clear", modesGroup);
    connect(rbClear, SIGNAL(clicked()), view, SLOT(setClearMode()));
    rbSource = new QRadioButton("Source", modesGroup);
    connect(rbSource, SIGNAL(clicked()), view, SLOT(setSourceMode()));
    rbDest = new QRadioButton("Destination", modesGroup);
    connect(rbDest, SIGNAL(clicked()), view, SLOT(setDestMode()));
    rbSourceOver = new QRadioButton("Source Over", modesGroup);
    connect(rbSourceOver, SIGNAL(clicked()), view, SLOT(setSourceOverMode()));
    rbDestOver = new QRadioButton("Destination Over", modesGroup);
    connect(rbDestOver, SIGNAL(clicked()), view, SLOT(setDestOverMode()));
    rbSourceIn = new QRadioButton("Source In", modesGroup);
    connect(rbSourceIn, SIGNAL(clicked()), view, SLOT(setSourceInMode()));
    rbDestIn = new QRadioButton("Dest In", modesGroup);
    connect(rbDestIn, SIGNAL(clicked()), view, SLOT(setDestInMode()));
    rbSourceOut = new QRadioButton("Source Out", modesGroup);
    connect(rbSourceOut, SIGNAL(clicked()), view, SLOT(setSourceOutMode()));
    rbDestOut = new QRadioButton("Dest Out", modesGroup);
    connect(rbDestOut, SIGNAL(clicked()), view, SLOT(setDestOutMode()));
    rbSourceAtop = new QRadioButton("Source Atop", modesGroup);
    connect(rbSourceAtop, SIGNAL(clicked()), view, SLOT(setSourceAtopMode()));
    rbDestAtop = new QRadioButton("Dest Atop", modesGroup);
    connect(rbDestAtop, SIGNAL(clicked()), view, SLOT(setDestAtopMode()));
    rbXor = new QRadioButton("Xor", modesGroup);
    connect(rbXor, SIGNAL(clicked()), view, SLOT(setXorMode()));

    QGroupBox *circleColorGroup = new QGroupBox(mainGroup);
    circleColorGroup->setAttribute(Qt::WA_ContentsPropagated);
    circleColorGroup->setTitle("Circle color");
    QSlider *circleColorSlider = new QSlider(Qt::Horizontal, circleColorGroup);
    circleColorSlider->setRange(0, 359);
    circleColorSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(circleColorSlider, SIGNAL(valueChanged(int)), view, SLOT(setCircleColor(int)));

    QGroupBox *circleAlphaGroup = new QGroupBox(mainGroup);
    circleAlphaGroup->setAttribute(Qt::WA_ContentsPropagated);
    circleAlphaGroup->setTitle("Circle alpha");
    QSlider *circleAlphaSlider = new QSlider(Qt::Horizontal, circleAlphaGroup);
    circleAlphaSlider->setRange(0, 255);
    circleAlphaSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(circleAlphaSlider, SIGNAL(valueChanged(int)), view, SLOT(setCircleAlpha(int)));

    QPushButton *showSourceButton = new QPushButton(mainGroup);
    showSourceButton->setText("Show Source");

    QPushButton *whatsThisButton = new QPushButton(mainGroup);
    whatsThisButton->setText("What's This?");
    whatsThisButton->setCheckable(true);

    QPushButton *animateButton = new QPushButton(mainGroup);
    animateButton->setText("Animated");
    animateButton->setCheckable(true);
    animateButton->setChecked(true);

    QHBoxLayout *viewLayout = new QHBoxLayout(this);
    viewLayout->addWidget(view);
    viewLayout->addWidget(mainGroup);

    QVBoxLayout *mainGroupLayout = new QVBoxLayout(mainGroup);
    mainGroupLayout->addWidget(circleColorGroup);
    mainGroupLayout->addWidget(circleAlphaGroup);
    mainGroupLayout->addWidget(modesGroup);
    mainGroupLayout->addStretch();
    mainGroupLayout->addWidget(animateButton);
    mainGroupLayout->addWidget(whatsThisButton);
    mainGroupLayout->addWidget(showSourceButton);

    QVBoxLayout *modesLayout = new QVBoxLayout(modesGroup);
    modesLayout->addWidget(rbClear);
    modesLayout->addWidget(rbSource);
    modesLayout->addWidget(rbDest);
    modesLayout->addWidget(rbSourceOver);
    modesLayout->addWidget(rbDestOver);
    modesLayout->addWidget(rbSourceIn);
    modesLayout->addWidget(rbDestIn);
    modesLayout->addWidget(rbSourceOut);
    modesLayout->addWidget(rbDestOut);
    modesLayout->addWidget(rbSourceAtop);
    modesLayout->addWidget(rbDestAtop);
    modesLayout->addWidget(rbXor);

    QVBoxLayout *circleColorLayout = new QVBoxLayout(circleColorGroup);
    circleColorLayout->addWidget(circleColorSlider);

    QVBoxLayout *circleAlphaLayout = new QVBoxLayout(circleAlphaGroup);
    circleAlphaLayout->addWidget(circleAlphaSlider);

    view->loadDescription(":res/composition.html");
    view->loadSourceFile(":res/composition.cpp");

    connect(whatsThisButton, SIGNAL(clicked(bool)), view, SLOT(setDescriptionEnabled(bool)));
    connect(view, SIGNAL(descriptionEnabledChanged(bool)), whatsThisButton, SLOT(setChecked(bool)));
    connect(showSourceButton, SIGNAL(clicked()), view, SLOT(showSource()));
    connect(animateButton, SIGNAL(toggled(bool)), view, SLOT(setAnimationEnabled(bool)));

    circleColorSlider->setValue(270);
    circleAlphaSlider->setValue(200);
    rbSourceOut->animateClick();

    setWindowTitle(tr("Composition Modes"));
}


void CompositionWidget::nextMode()
{
    /*
      if (!m_animation_enabled)
      return;
      if (rbClear->isChecked()) rbSource->animateClick();
      if (rbSource->isChecked()) rbDest->animateClick();
      if (rbDest->isChecked()) rbSourceOver->animateClick();
      if (rbSourceOver->isChecked()) rbDestOver->animateClick();
      if (rbDestOver->isChecked()) rbSourceIn->animateClick();
      if (rbSourceIn->isChecked()) rbDestIn->animateClick();
      if (rbDestIn->isChecked()) rbSourceOut->animateClick();
      if (rbSourceOut->isChecked()) rbDestOut->animateClick();
      if (rbDestOut->isChecked()) rbSourceAtop->animateClick();
      if (rbSourceAtop->isChecked()) rbDestAtop->animateClick();
      if (rbDestAtop->isChecked()) rbXor->animateClick();
      if (rbXor->isChecked()) rbClear->animateClick();
    */
}

CompositionRenderer::CompositionRenderer(QWidget *parent)
    : ArthurFrame(parent)
{
    m_animation_enabled = true;
    m_image = QImage(":res/flower_2.png");
    m_circle_alpha = 127;
    m_circle_hue = 255;
    m_current_object = NoObject;
    m_composition_mode = QPainter::CompositionMode_SourceOut;

    m_circle_pos = QPoint(200, 100);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QRectF rectangle_around(const QPointF &p, const QSizeF &size = QSize(250, 200))
{
    QRectF rect(p, size);
    rect.translate(-size.width()/2, -size.height()/2);
    return rect;
}

void CompositionRenderer::updateCirclePos()
{
    if (m_current_object != NoObject)
        return;
    QDateTime dt = QDateTime::currentDateTime();
    qreal t = (dt.toTime_t() * 1000 + dt.time().msec()) / 1000.0;

    qreal x = width() / 2.0 + (cos(t) + sin(-t*2)) * width() / 2.0;
    qreal y = height() / 2.0 + (sin(t) + cos(t * 3)) * height() / 2.0;

    m_circle_pos = QLineF(m_circle_pos, QPointF(x, y)).pointAt(0.01);
}

void CompositionRenderer::paint(QPainter *painter)
{
    if (m_animation_enabled)
        updateCirclePos();

    if (m_buffer.size() != size()) {
        m_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        m_base_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

        m_base_buffer.fill(0);

        QPainter p(&m_base_buffer);
        p.setPen(Qt::NoPen);

        QLinearGradient rect_gradient(0, 0, 0, height());
        rect_gradient.setColorAt(0, Qt::red);
        rect_gradient.setColorAt(.17, Qt::yellow);
        rect_gradient.setColorAt(.33, Qt::green);
        rect_gradient.setColorAt(.50, Qt::cyan);
        rect_gradient.setColorAt(.66, Qt::blue);
        rect_gradient.setColorAt(.81, Qt::magenta);
        rect_gradient.setColorAt(1, Qt::red);
        p.setBrush(rect_gradient);
        p.drawRect(width() / 2, 0, width() / 2, height());

        QLinearGradient alpha_gradient(0, 0, width(), 0);
        alpha_gradient.setColorAt(0, Qt::white);
        alpha_gradient.setColorAt(0.2, Qt::white);
        alpha_gradient.setColorAt(0.5, Qt::transparent);
        alpha_gradient.setColorAt(0.8, Qt::white);
        alpha_gradient.setColorAt(1, Qt::white);

        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.setBrush(alpha_gradient);
        p.drawRect(0, 0, width(), height());

        p.setCompositionMode(QPainter::CompositionMode_DestinationOver);

        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.drawImage(rect(), m_image);
    }

    memcpy(m_buffer.bits(), m_base_buffer.bits(), m_buffer.numBytes());

    {
        QPainter p(&m_buffer);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(m_composition_mode);

        QRectF circle_rect = rectangle_around(m_circle_pos);
        QColor color = QColor::fromHsvF(m_circle_hue / 360.0, 1, 1, m_circle_alpha / 255.0);
        QLinearGradient circle_gradient(circle_rect.topLeft(), circle_rect.bottomRight());
        circle_gradient.setColorAt(0, color.light());
        circle_gradient.setColorAt(0.5, color);
        circle_gradient.setColorAt(1, color.dark());
        p.setBrush(circle_gradient);

        p.drawEllipse(circle_rect);
    }

    painter->drawImage(0, 0, m_buffer);

    if (m_animation_enabled)
        update();
}

void CompositionRenderer::mousePressEvent(QMouseEvent *e)
{
    setDescriptionEnabled(false);

    QRectF circle = rectangle_around(m_circle_pos);

    if (circle.contains(e->pos())) {
        m_current_object = Circle;
        m_offset = circle.center() - e->pos();
    } else {
        m_current_object = NoObject;
    }
}

void CompositionRenderer::mouseMoveEvent(QMouseEvent *e)
{
    if (m_current_object == Circle) setCirclePos(e->pos() + m_offset);
}

void CompositionRenderer::mouseReleaseEvent(QMouseEvent *)
{
    m_current_object = NoObject;
}
