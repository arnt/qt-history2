/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mainwindow.h"
#include <QSystemTrayIcon>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
	    QMessageBox::warning(0, tr("System tray is unavailable"), 
                                   tr("System tray unavailable"));

    // Create the menu that will be used for the context menu
    menu = new QMenu(this);
    QObject::connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));
    toggleVisibilityAction = menu->addAction("Show/Hide", this, SLOT(toggleVisibility()));
    menu->addAction("Minimize", this, SLOT(showMinimized()));
    menu->addAction("Maximize", this, SLOT(showMaximized()));
    menu->addSeparator();
    menu->addAction("&Quit", qApp, SLOT(quit()));

    // Create the tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("System trayIcon example");
    trayIcon->setContextMenu(menu);
    QObject::connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(activated(QSystemTrayIcon::ActivationReason)));
    QObject::connect(trayIcon, SIGNAL(messageClicked()), 
                     this, SLOT(balloonClicked()));
    changeIcon(0); // set the first icon
    trayIcon->show();

    QLabel *titleLabel = new QLabel(tr("Message Title"));
    titleEdit = new QLineEdit(tr("Message Title"));
    QLabel *msgLabel = new QLabel(tr("Message Contents"));
    msgEdit = new QTextEdit(tr("Man is more ape than many of the apes"));
    msgEdit->setAcceptRichText(false);
    QLabel *typeLabel = new QLabel(tr("Message Type"));
    typeCombo = new QComboBox;
    QStringList types;
    types << "NoIcon" << "Information" << "Warning" << "Critical";
    typeCombo->addItems(types);
    typeCombo->setCurrentIndex(2);
    QPushButton *balloonButton = new QPushButton(tr("Balloon message"));
    balloonButton->setToolTip(tr("Click here to balloon the message"));
    QObject::connect(balloonButton, SIGNAL(clicked()), this, SLOT(showMessage()));
    info = new QTextEdit(tr("Status messages will be visible here"));
    info->setMaximumHeight(100);
    info->setReadOnly(true);
    QCheckBox *toggleIconCheckBox = new QCheckBox(tr("Show system tray icon"));
    toggleIconCheckBox->setChecked(true);
    QObject::connect(toggleIconCheckBox, SIGNAL(clicked(bool)), trayIcon, SLOT(setVisible(bool)));

    QLabel *iconLabel = new QLabel("Select icon");
    iconPicker = new QComboBox;
    QStringList icons;
    icons << "16x16 icon" << "22x22 icon" << "32x32 icon";
    iconPicker->addItems(icons);
    QObject::connect(iconPicker, SIGNAL(activated(int)),
                     this, SLOT(changeIcon(int)));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(titleLabel, 0, 0); layout->addWidget(titleEdit, 0, 1);
    layout->addWidget(msgLabel, 1, 0);   layout->addWidget(msgEdit, 1, 1);
    layout->addWidget(typeLabel, 2, 0);  layout->addWidget(typeCombo, 2, 1);
                                           layout->addWidget(balloonButton, 4, 1);
    layout->addWidget(info, 5, 0, 1, 2);
    layout->addWidget(toggleIconCheckBox, 6, 0);
    layout->addWidget(iconLabel, 7, 0);  layout->addWidget(iconPicker, 7, 1);
    setLayout(layout);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (QSystemTrayIcon::isSystemTrayAvailable() && trayIcon->isVisible()) {
        QMessageBox::information(this, tr("System tray example"),
                                 tr("Application will continue running. Quit using"
                                     " the context menu in the system tray"));
        hide();
        e->ignore();
    }
}

void MainWindow::updateMenu()
{
    toggleVisibilityAction->setText(isVisible() ? tr("Hide") : tr("Show"));
}

void MainWindow::toggleVisibility()
{
    if (isVisible())
        hide();
    else
        show();
}

void MainWindow::showMessage()
{
    if (!QSystemTrayIcon::supportsMessages()) {
        QMessageBox::information(this, tr("System tray example"),
                                 tr("Balloon tips are not supported on this platform"));
    } else {
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(typeCombo->currentIndex());
        trayIcon->showMessage(titleEdit->text(), msgEdit->toPlainText(), icon, 10000);
        trayIcon->setToolTip(titleEdit->text());
    }
}

void MainWindow::balloonClicked()
{
    info->append(tr("Balloon message was clicked"));
}

void MainWindow::activated(QSystemTrayIcon::ActivationReason reason)
{
    QString r;
    switch (reason) {
        case QSystemTrayIcon::Unknown:
            r = tr("Unknown");
            break;
        case QSystemTrayIcon::Context:
            r = tr("Context");
            break;
        case QSystemTrayIcon::DoubleClick:
            r = tr("DoubleClick");
            break;
        case QSystemTrayIcon::Trigger:
            r = tr("Trigger");
            break;
        case QSystemTrayIcon::MiddleClick:
            r = tr("MiddleClick");
            break;
    }
    info->append(QString("Activated - Reason %1").arg(r));
}

void MainWindow::changeIcon(int index)
{
    QString iconname;
    switch (index) {
    default:
    case 0:
        iconname = QLatin1String(":/resources/icon_16x16.png");
        break;
    case 1:
        iconname = QLatin1String(":/resources/icon_22x22.png");
        break;
    case 2:
        iconname = QLatin1String(":/resources/icon_32x32.png");
        break;
    }

    QPixmap pix(iconname);
    trayIcon->setIcon(QIcon(pix));
}

