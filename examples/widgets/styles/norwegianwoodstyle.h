#ifndef NORWEGIANWOODSTYLE_H
#define NORWEGIANWOODSTYLE_H

#include <QPalette>
#include <QWindowsStyle>

class NorwegianWoodStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    NorwegianWoodStyle();

    void polish(QPalette &palette);
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

private:
    void setBrush(QPalette &palette, QPalette::ColorRole role,
                  const QBrush &brush);
    void setBrushPixmap(QPalette &palette, QPalette::ColorRole role,
                        const QPixmap &pixmap);
};

#endif
