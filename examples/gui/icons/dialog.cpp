#include "dialog.h"

Dialog::Dialog(QWidget *parent = 0)
    : QDialog(parent)
{
    setIconFile(qApp->applicationDirPath() + "/fileopen.png");

    QPushButton *iconButton = new QPushButton("&Icon...", this);
    iconButton->move((100 - iconButton->width()) / 2,
                     (60 - iconButton->height()) / 2);
    connect(iconButton, SIGNAL(clicked()), this, SLOT(chooseIcon()));
}


void Dialog::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Helvetica", 12, QFont::Bold));
    QIconSet icon = QIconSet(QPixmap(m_filename));

    int h = 30;
    int h2 = h * 2;
    int ih = 50;

    painter.fillRect(0, 0, 400, 240, Qt::white);
    painter.fillRect(0, 0, 100, h2, Qt::lightGray);
    painter.fillRect(0, h2 + (ih * 3), 400, h, Qt::lightGray);
    painter.fillRect(101, 1, 149, 28, Qt::green);
    painter.fillRect(251, 1, 149, 28, Qt::green);
    painter.fillRect(101, h, 74, 28, Qt::green);
    painter.fillRect(176, h, 74, 28, Qt::green);
    painter.fillRect(251, h, 74, 28, Qt::green);
    painter.fillRect(326, h, 74, 28, Qt::green);
    painter.fillRect(0, h2 + (ih * 0), 100, ih - 1, Qt::green);
    painter.fillRect(0, h2 + (ih * 1), 100, ih - 1, Qt::green);
    painter.fillRect(0, h2 + (ih * 2), 100, ih - 1, Qt::green);

    painter.drawText(100, 0, 150, h, Qt::AlignCenter, "Small");
    painter.drawText(250, 0, 150, h, Qt::AlignCenter, "Large");
    painter.drawText(100, h,  75, h, Qt::AlignCenter, "On");
    painter.drawText(175, h,  75, h, Qt::AlignCenter, "Off");
    painter.drawText(250, h,  75, h, Qt::AlignCenter, "On");
    painter.drawText(325, h,  75, h, Qt::AlignCenter, "Off");
    painter.drawText(0, h2 + (ih * 0), 100, ih, Qt::AlignCenter, "Normal");
    painter.drawText(0, h2 + (ih * 1), 100, ih, Qt::AlignCenter, "Disabled");
    painter.drawText(0, h2 + (ih * 2), 100, ih, Qt::AlignCenter, "Active");
    painter.drawText(0, h2 + (ih * 3), 400, h, Qt::AlignCenter,
                     "Active == Normal unless explicitly set");

    painter.drawPixmap(100, h2 + (ih * 0),
            icon.pixmap(QIconSet::Small, QIconSet::Normal, QIconSet::On));
    painter.drawPixmap(175, h2 + (ih * 0),
            icon.pixmap(QIconSet::Small, QIconSet::Normal, QIconSet::Off));
    painter.drawPixmap(250, h2 + (ih * 0),
            icon.pixmap(QIconSet::Large, QIconSet::Normal, QIconSet::On));
    painter.drawPixmap(325, h2 + (ih * 0),
            icon.pixmap(QIconSet::Large, QIconSet::Normal, QIconSet::Off));

    painter.drawPixmap(100, h2 + (ih * 1),
            icon.pixmap(QIconSet::Small, QIconSet::Disabled, QIconSet::On));
    painter.drawPixmap(175, h2 + (ih * 1),
            icon.pixmap(QIconSet::Small, QIconSet::Disabled, QIconSet::Off));
    painter.drawPixmap(250, h2 + (ih * 1),
            icon.pixmap(QIconSet::Large, QIconSet::Disabled, QIconSet::On));
    painter.drawPixmap(325, h2 + (ih * 1),
            icon.pixmap(QIconSet::Large, QIconSet::Disabled, QIconSet::Off));

    painter.drawPixmap(100, h2 + (ih * 2),
            icon.pixmap(QIconSet::Small, QIconSet::Active, QIconSet::On));
    painter.drawPixmap(175, h2 + (ih * 2),
            icon.pixmap(QIconSet::Small, QIconSet::Active, QIconSet::Off));
    painter.drawPixmap(250, h2 + (ih * 2),
            icon.pixmap(QIconSet::Large, QIconSet::Active, QIconSet::On));
    painter.drawPixmap(325, h2 + (ih * 2),
            icon.pixmap(QIconSet::Large, QIconSet::Active, QIconSet::Off));
}


void Dialog::chooseIcon()
{
    QString filename = QFileDialog::getOpenFileName(
                                this, tr("QIconSet--Choose an Icon"),
                                iconFile(), "All (*)");
    if (!filename.isEmpty()) {
        setIconFile(filename);
        update();
    }
}
