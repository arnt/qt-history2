#include <QtGui>

#include "stylewidget.h"

StyleWidget::StyleWidget(QWidget *parent)
        : QWidget(parent)
{
    path.addRect(0, 0, 130, 60);
    pattern.addRect(0, 0, 410, 60);

    setWindowTitle(tr("Brush Styles"));
    resize(430, 605);
}

void StyleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(10, 10);
    painter.save();
    drawShape(&painter, QBrush(Qt::SolidPattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::Dense1Pattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::Dense2Pattern));

    painter.restore();
    painter.translate(0, 75);
    painter.save();

    drawText(&painter,  "Qt::SolidPattern", "Qt::Dense1Pattern",
                    "Qt::Dense2Pattern");

    painter.restore();
    painter.translate(0, 10);
    painter.save();

    drawShape(&painter, QBrush(Qt::Dense3Pattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::Dense4Pattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::Dense5Pattern));

    painter.restore();
    painter.translate(0, 75);
    painter.save();

    drawText(&painter,  "Qt::Dense3Pattern", "Qt::Dense4Pattern",
                    "Qt::Dense5Pattern");

    painter.restore();
    painter.translate(0, 10);
    painter.save();

    drawShape(&painter, QBrush(Qt::Dense6Pattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::Dense7Pattern));
    painter.translate(130, 0);
    drawShape(&painter, QBrush(Qt::NoBrush));

    painter.restore();
    painter.translate(0, 75);
    painter.save();

    drawText(&painter,  "Qt::Dense6Pattern", "Qt::Dense7Pattern",
                    "Qt::NoBrush");

    painter.restore();
    painter.translate(0, 10);
    painter.save();

    drawShape(&painter, QBrush(Qt::HorPattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::VerPattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::CrossPattern));

    painter.restore();
    painter.translate(0, 75);
    painter.save();

    drawText(&painter, "Qt::HorPattern", "Qt::VerPattern",
                   "Qt::CrossPattern");

    painter.restore();
    painter.translate(0, 10);
    painter.save();

    drawShape(&painter, QBrush(Qt::BDiagPattern));
    painter.drawPath(path);
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::FDiagPattern));
    painter.translate(140, 0);
    drawShape(&painter, QBrush(Qt::DiagCrossPattern));

    painter.restore();
    painter.translate(0, 75);
    painter.save();

    drawText(&painter, "Qt::BDiagPattern", "Qt::FDiagPattern",
                    "Qt::DiagCrossPattern");

    painter.restore();
    painter.translate(0, 10);
    painter.save();

    drawShape(&painter, QBrush(QLinearGradient(0, 0, 120, 120)));
    painter.translate(140, 0);

    QRadialGradient radial(65, 30, 65, 65, 30);
    radial.setColorAt(0.0, Qt::white);
    radial.setColorAt(1.0, Qt::black);
    drawShape(&painter, QBrush(radial));

    painter.translate(140, 0);
    drawShape(&painter, QBrush(QConicalGradient(65, 30, 90)));

    painter.restore();
    painter.translate(0, 75);
    painter.save();

    drawText(&painter, "Qt::LinearGradientPattern",
                    "Qt::ConicalGradientPattern", "Qt::RadialGradientPattern");

    painter.restore();
    painter.translate(0, 10);
    painter.save();

    drawShape(&painter, QBrush(QPixmap("qt-logo.png")));

    painter.restore();
    painter.translate(0, 75);

    drawText(&painter, "Qt::TexturePattern");
}

void StyleWidget::drawShape(QPainter *painter, const QBrush &brush)
{
    painter->setBrush(brush);
    if (brush.style() == Qt::TexturePattern )
        painter->drawPath(pattern);
    else
        painter->drawPath(path);
}

void StyleWidget::drawText(QPainter *painter, const QString &first,
                                         const QString &second,
                                         const QString &third)
{
    painter->setPen(Qt::SolidLine);
    painter->drawText(0, 0, first);
    painter->drawText(140, 0, second);
    painter->drawText(280, 0, third);
    painter->setPen(Qt::NoPen);
}
