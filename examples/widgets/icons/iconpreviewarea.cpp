#include <QtGui>

#include "iconpreviewarea.h"

IconPreviewArea::IconPreviewArea(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout;
    setLayout(mainLayout);

    stateLabels[0] = createHeaderLabel(tr("On"));
    stateLabels[1] = createHeaderLabel(tr("Off"));
    Q_ASSERT(NumStates == 2);

    modeLabels[0] = createHeaderLabel(tr("Active"));
    modeLabels[1] = createHeaderLabel(tr("Normal"));
    modeLabels[2] = createHeaderLabel(tr("Disabled"));
    Q_ASSERT(NumModes == 3);

    for (int j = 0; j < NumStates; ++j)
        mainLayout->addWidget(stateLabels[j], j + 1, 0);

    for (int i = 0; i < NumModes; ++i) {
        mainLayout->addWidget(modeLabels[i], 0, i + 1);

        for (int j = 0; j < NumStates; ++j) {
            pixmapLabels[i][j] = createPixmapLabel();
            mainLayout->addWidget(pixmapLabels[i][j], j + 1, i + 1);
        }
    }
}

void IconPreviewArea::setIcon(const QIcon &icon)
{
    this->icon = icon;
    updatePixmaps();
}

void IconPreviewArea::setSize(const QSize &size)
{
    if (size != this->size) {
        this->size = size;
        updatePixmaps();
    }
}

QLabel *IconPreviewArea::createHeaderLabel(const QString &text)
{
    QLabel *label = new QLabel(tr("<b>%1</b>").arg(text));
    label->setAlignment(Qt::AlignCenter);
    return label;
}

QLabel *IconPreviewArea::createPixmapLabel()
{
    QLabel *label = new QLabel;
    label->setAlignment(Qt::AlignCenter);
    label->setFrameShape(QFrame::Box);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setBackgroundRole(QPalette::Base);
    label->setMinimumSize(100, 100);
    return label;
}

void IconPreviewArea::updatePixmaps()
{
    for (int i = 0; i < NumModes; ++i) {
        QIcon::Mode mode;
        if (i == 0) {
            mode = QIcon::Active;
        } else if (i == 1) {
            mode = QIcon::Normal;
        } else {
            mode = QIcon::Disabled;
        }

        for (int j = 0; j < NumStates; ++j) {
            QIcon::State state = (j == 0) ? QIcon::On : QIcon::Off;
            pixmapLabels[i][j]->setPixmap(icon.pixmap(size, mode, state));
        }
    }
}
