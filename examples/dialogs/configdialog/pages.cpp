#include <QtGui>

#include "pages.h"

ConfigurationPage::ConfigurationPage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *configGroup = new QGroupBox(tr("Server configuration"), this);

    QLabel *serverLabel = new QLabel(tr("Server:"), configGroup);
    QComboBox *serverCombo = new QComboBox(configGroup);
    serverCombo->addItem(tr("Trolltech (Australia)"));
    serverCombo->addItem(tr("Trolltech (Norway)"));
    serverCombo->addItem(tr("Trolltech (USA)"));

    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(serverCombo);

    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
    configLayout->addLayout(serverLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(configGroup);
    mainLayout->addStretch(1);
}

UpdatePage::UpdatePage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *updateGroup = new QGroupBox(tr("Package selection"), this);

    QCheckBox *systemCheckBox = new QCheckBox(tr("Update system"),
        updateGroup);

    QCheckBox *appsCheckBox = new QCheckBox(tr("Update applications"),
        updateGroup);

    QCheckBox *docsCheckBox = new QCheckBox(tr("Update documentation"),
        updateGroup);

    QGroupBox *packageGroup = new QGroupBox(tr("Existing packages"), this);

    QListWidget *packageList = new QListWidget(packageGroup);
    QListWidgetItem *qtItem = new QListWidgetItem(packageList);
    qtItem->setText("Qt");
    QListWidgetItem *qsaItem = new QListWidgetItem(packageList);
    qsaItem->setText("QSA");
    QListWidgetItem *teamBuilderItem = new QListWidgetItem(packageList);
    teamBuilderItem->setText("Teambuilder");

    QPushButton *startUpdateButton = new QPushButton(tr("Start update"), this);

    QVBoxLayout *updateLayout = new QVBoxLayout(updateGroup);
    updateLayout->addWidget(systemCheckBox);
    updateLayout->addWidget(appsCheckBox);
    updateLayout->addWidget(docsCheckBox);

    QVBoxLayout *packageLayout = new QVBoxLayout(packageGroup);
    packageLayout->addWidget(packageList);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(updateGroup);
    mainLayout->addWidget(packageGroup);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(startUpdateButton);
    mainLayout->addStretch(1);
}

QueryPage::QueryPage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *packagesGroup = new QGroupBox(tr("Look for packages"), this);

    QLabel *nameLabel = new QLabel(tr("Name:"), packagesGroup);
    QLineEdit *nameEdit = new QLineEdit(packagesGroup);

    QLabel *dateLabel = new QLabel(tr("Released after:"), packagesGroup);
    QDateTimeEdit *dateEdit = new QDateTimeEdit(QDate::currentDate());

    QCheckBox *releasesCheckBox = new QCheckBox(tr("Releases"), packagesGroup);
    QCheckBox *upgradesCheckBox = new QCheckBox(tr("Upgrades"), packagesGroup);

    QSpinBox *hitsSpinBox = new QSpinBox(packagesGroup);
    hitsSpinBox->setPrefix(tr("Return up to "));
    hitsSpinBox->setSuffix(tr(" results."));
    hitsSpinBox->setSpecialValueText(tr("Return only the first result."));
    hitsSpinBox->setMinimum(1);
    hitsSpinBox->setMaximum(100);
    hitsSpinBox->setSingleStep(10);

    QPushButton *startQueryButton = new QPushButton(tr("Start query"), this);

    QGridLayout *packagesLayout = new QGridLayout(packagesGroup);
    packagesLayout->addWidget(nameLabel, 0, 0);
    packagesLayout->addWidget(nameEdit, 0, 1);
    packagesLayout->addWidget(dateLabel, 1, 0);
    packagesLayout->addWidget(dateEdit, 1, 1);
    packagesLayout->addWidget(releasesCheckBox, 2, 0);
    packagesLayout->addWidget(upgradesCheckBox, 3, 0);
    packagesLayout->addWidget(hitsSpinBox, 4, 0, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(packagesGroup);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(startQueryButton);
    mainLayout->addStretch(1);
}
