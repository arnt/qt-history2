#include "dialog.h"
#include "widget.h"

Dialog::Dialog()
    : QDialog(0)
{
    QLabel *groupLabel = new QLabel("Color &Group", this);
    groupComboBox = new QComboBox(this);
    groupComboBox->insertItem("Active");
    groupComboBox->insertItem("Disabled");
    groupComboBox->insertItem("Inactive");
    groupLabel->setBuddy(groupComboBox);
    QLabel *roleLabel = new QLabel("Color &Role", this);
    roleComboBox = new QComboBox(this);
    roleComboBox->insertItem("Background");
    roleComboBox->insertItem("Foreground");
    roleComboBox->insertItem("Base");
    roleComboBox->insertItem("Text");
    roleComboBox->insertItem("Button");
    roleComboBox->insertItem("ButtonText");
    roleComboBox->insertItem("Light");
    roleComboBox->insertItem("Midlight");
    roleComboBox->insertItem("Dark");
    roleComboBox->insertItem("Mid");
    roleComboBox->insertItem("Shadow");
    roleComboBox->insertItem("Highlight");
    roleComboBox->insertItem("HighlightedText");
    roleComboBox->insertItem("BrightText");
    roleComboBox->insertItem("Link");
    roleComboBox->insertItem("LinkVisited");
    roleLabel->setBuddy(roleComboBox);
    QPushButton *colorButton = new QPushButton("&Color...", this);
    colorLabel = new QLabel(this);
    colorLabel->setFrameStyle(QFrame::Panel|QFrame::Raised);
    QPushButton *saveButton = new QPushButton("&Save...", this);
    QPushButton *quitButton = new QPushButton("&Quit", this);
    QWorkspace *mdi = new QWorkspace(this);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(groupLabel);
    hbox->addWidget(groupComboBox);
    hbox->addWidget(roleLabel);
    hbox->addWidget(roleComboBox);
    hbox->addWidget(colorButton);
    hbox->addWidget(colorLabel);
    hbox->addStretch(1);
    hbox->addWidget(saveButton);
    hbox->addWidget(quitButton);
    vbox->addLayout(hbox);
    vbox->addWidget(mdi);

    connect(groupComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(updatePalette()));
    connect(roleComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(updatePalette()));
    connect(colorButton, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    inactiveWidget = new Widget("Inactive", this);
    disabledWidget = new Widget("Disabled", this);
    disabledWidget->setEnabled(false);
    activeWidget = new Widget("Active", this);

    mdi->addWindow(inactiveWidget);
    mdi->addWindow(disabledWidget);
    mdi->addWindow(activeWidget);
    mdi->tile();

    userPalette = activeWidget->palette();
    updatePalette();

    setWindowTitle("Palette Editor");
}


void Dialog::updatePalette()
{
    QPalette::ColorGroup group = getColorGroup();
    QPalette::ColorRole role = getColorRole();
    color = userPalette.color(group, role);
    QPixmap pixmap(120, 25);
    pixmap.fill(color);
    colorLabel->setPixmap(pixmap);
    inactiveWidget->setPalette(userPalette);
    inactiveWidget->update();
    disabledWidget->setPalette(userPalette);
    disabledWidget->update();
    activeWidget->setPalette(userPalette);
    activeWidget->update();
    activeWidget->setActiveWindow();
}


QPalette::ColorGroup Dialog::getColorGroup()
{
    QString group = groupComboBox->currentText();
    if (group == "Active")
        return QPalette::Active;
    else if (group == "Disabled")
        return QPalette::Disabled;
    else
        return QPalette::Inactive;
}


QPalette::ColorRole Dialog::getColorRole()
{
    QString role = roleComboBox->currentText();
    if (role == "Background")
        return QPalette::Background;
    else if (role == "Foreground")
        return QPalette::Foreground;
    else if (role == "Base")
        return QPalette::Base;
    else if (role == "Text")
        return QPalette::Text;
    else if (role == "Button")
        return QPalette::Button;
    else if (role == "ButtonText")
        return QPalette::ButtonText;
    else if (role == "Light")
        return QPalette::Light;
    else if (role == "Midlight")
        return QPalette::Midlight;
    else if (role == "Dark")
        return QPalette::Dark;
    else if (role == "Mid")
        return QPalette::Mid;
    else if (role == "Shadow")
        return QPalette::Shadow;
    else if (role == "Highlight")
        return QPalette::Highlight;
    else if (role == "HighlightedText")
        return QPalette::HighlightedText;
    else if (role == "BrightText")
        return QPalette::BrightText;
    else if (role == "Link")
        return QPalette::Link;
    else if (role == "LinkVisited")
        return QPalette::LinkVisited;
    else
        return QPalette::Foreground;
}


void Dialog::changeColor()
{
    QPalette::ColorGroup group = getColorGroup();
    QPalette::ColorRole role = getColorRole();
    color = userPalette.color(group, role);
    QColor newColor = QColorDialog::getColor(color, this);
    if (newColor.isValid()) {
        color = newColor;
        userPalette.setColor(group, role, color);
        updatePalette();
    }
}

