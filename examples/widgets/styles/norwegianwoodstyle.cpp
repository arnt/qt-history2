#include <QtGui>

#include "norwegianwoodstyle.h"

static const int NumGroups = 3;
static const QPalette::ColorGroup groups[NumGroups] = {
    QPalette::Active, QPalette::Disabled, QPalette::Inactive
};

NorwegianWoodStyle::NorwegianWoodStyle()
{
}

void NorwegianWoodStyle::polish(QPalette &palette)
{
    QImage buttonImage(QString(":/images/woodbutton.xpm"));
    QImage lightImage = buttonImage;
    QImage darkImage = buttonImage;
    QImage midImage = buttonImage;
    QImage backgroundImage = buttonImage;

qDebug("[%d]", buttonImage.numColors());
    for (int i = 0; i < buttonImage.numColors(); ++i) {
        QRgb rgb = buttonImage.color(i);
        QColor color(rgb);

        lightImage.setColor(i, color.light().rgb());
        darkImage.setColor(i, color.dark(180).rgb());
        midImage.setColor(i, color.dark(120).rgb());
        backgroundImage.setColor(i, color.dark(180).rgb());
qDebug("%d: %d %d %d", i, rgb, color.light().rgb(), color.dark(180).rgb());
    }

    palette = QPalette(QColor(212, 140, 95));
    setBrush(palette, QPalette::BrightText, Qt::white);
    setBrush(palette, QPalette::Base, QColor(236, 182, 120));
    setBrushPixmap(palette, QPalette::Button, buttonImage);
    setBrushPixmap(palette, QPalette::Light, lightImage);
    setBrushPixmap(palette, QPalette::Dark, darkImage);
    setBrushPixmap(palette, QPalette::Mid, midImage);
    setBrushPixmap(palette, QPalette::Background, backgroundImage);
}

void NorwegianWoodStyle::polish(QWidget *widget)
{
    QWindowsStyle::polish(widget);
}

void NorwegianWoodStyle::unpolish(QWidget *widget)
{
    QWindowsStyle::unpolish(widget);
}

void NorwegianWoodStyle::setBrush(QPalette &palette, QPalette::ColorRole role,
                                  const QBrush &brush)
{
    for (int i = 0; i < 3; ++i)
        palette.setBrush(groups[i], role, brush);
}

void NorwegianWoodStyle::setBrushPixmap(QPalette &palette,
                                        QPalette::ColorRole role,
                                        const QPixmap &pixmap)
{
    for (int i = 0; i < 3; ++i) {
        QColor color = palette.brush(groups[i], role).color();
        palette.setBrush(groups[i], role, QBrush(color, pixmap));
    }
}
