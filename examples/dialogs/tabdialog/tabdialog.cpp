#include <QtGui>

#include "tabdialog.h"

TabDialog::TabDialog(const QString &fileName, QWidget *parent)
    : QDialog(parent)
{
    QFileInfo fileInfo(fileName);

    tabWidget = new QTabWidget(this);
    tabWidget->addTab(new GeneralTab(fileInfo),
//                      QIcon(QPixmap("/Users/twschulz/troll/qt/main/pics/generic.png")),
                      tr("Tab 1"));
    tabWidget->addTab(new PermissionsTab(fileInfo),
           // QIcon(QPixmap("/Users/twschulz/troll/qt/main/pics/generic.png")),
            tr("Tab 2"));
    tabWidget->addTab(new ApplicationsTab(fileInfo),
            //QIcon(QPixmap("/Users/twschulz/troll/qt/main/pics/generic.png")),
            tr("Applications"));
    tabWidget->setTabPosition(QTabWidget::West);
//    tabWidget->setTabShape(QTabWidget::Triangular);

    QPushButton *okButton = new QPushButton(tr("OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Tab Dialog"));
}

GeneralTab::GeneralTab(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
{
    QLabel *fileNameLabel = new QLabel(tr("File Name:"), this);
    QLineEdit *fileNameEdit = new QLineEdit(fileInfo.fileName(), this);

    QLabel *pathLabel = new QLabel(tr("Path:"), this);
    QLabel *pathValueLabel = new QLabel(fileInfo.absoluteFilePath(), this);
    pathValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *sizeLabel = new QLabel(tr("Size:"), this);
    unsigned long size = fileInfo.size()/1024;
    QLabel *sizeValueLabel = new QLabel(tr("%1 K").arg(size), this);
    sizeValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *lastReadLabel = new QLabel(tr("Last Read:"), this);
    QLabel *lastReadValueLabel = new QLabel(fileInfo.lastRead().toString(),
                                            this);
    lastReadValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *lastModLabel = new QLabel(tr("Last Modified:"), this);
    QLabel *lastModValueLabel = new QLabel(fileInfo.lastModified().toString(),
                                           this);
    lastModValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(fileNameEdit);
    mainLayout->addWidget(pathLabel);
    mainLayout->addWidget(pathValueLabel);
    mainLayout->addWidget(sizeLabel);
    mainLayout->addWidget(sizeValueLabel);
    mainLayout->addWidget(lastReadLabel);
    mainLayout->addWidget(lastReadValueLabel);
    mainLayout->addWidget(lastModLabel);
    mainLayout->addWidget(lastModValueLabel);
    mainLayout->addStretch(1);
}

PermissionsTab::PermissionsTab(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *permissionsGroup = new QGroupBox(tr("Permissions"), this);

    QCheckBox *readable = new QCheckBox(tr("Readable"), permissionsGroup);
    if (fileInfo.isReadable())
        readable->setChecked(true);

    QCheckBox *writable = new QCheckBox(tr("Writable"), permissionsGroup);
    if ( fileInfo.isWritable() )
        writable->setChecked(true);

    QCheckBox *executable = new QCheckBox(tr("Executable"), permissionsGroup);
    if ( fileInfo.isExecutable() )
        executable->setChecked(true);

    QGroupBox *ownerGroup = new QGroupBox(tr("Ownership"), this);

    QLabel *ownerLabel = new QLabel(tr("Owner"), ownerGroup);
    QLabel *ownerValueLabel = new QLabel(fileInfo.owner(), ownerGroup);
    ownerValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *groupLabel = new QLabel(tr("Group"), ownerGroup);
    QLabel *groupValueLabel = new QLabel(fileInfo.group(), ownerGroup);
    groupValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QVBoxLayout *permissionsLayout = new QVBoxLayout(permissionsGroup);
    permissionsLayout->addWidget(readable);
    permissionsLayout->addWidget(writable);
    permissionsLayout->addWidget(executable);

    QVBoxLayout *ownerLayout = new QVBoxLayout(ownerGroup);
    ownerLayout->addWidget(ownerLabel);
    ownerLayout->addWidget(ownerValueLabel);
    ownerLayout->addWidget(groupLabel);
    ownerLayout->addWidget(groupValueLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(permissionsGroup);
    mainLayout->addWidget(ownerGroup);
    mainLayout->addStretch(1);
}

ApplicationsTab::ApplicationsTab(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
{
    QLabel *topLabel = new QLabel(tr("Open with:"), this);

    QListWidget *applicationsListBox = new QListWidget(this);
    QStringList applications;

    for (int i = 1; i <= 30; ++i)
        applications.append(tr("Application %1").arg(i));
    applicationsListBox->insertItems(0, applications);

    QCheckBox *alwaysCheckBox;

    if (fileInfo.suffix().isEmpty())
        alwaysCheckBox = new QCheckBox(tr("Always use this application to "
            "open this type of file"), this);
    else
        alwaysCheckBox = new QCheckBox(tr("Always use this application to "
            "open files with the extension '%1'").arg(fileInfo.suffix()), this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(topLabel);
    layout->addWidget(applicationsListBox);
    layout->addWidget(alwaysCheckBox);
}
