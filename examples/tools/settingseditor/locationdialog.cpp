#include <QtGui>

#include "locationdialog.h"

LocationDialog::LocationDialog(QWidget *parent)
    : QDialog(parent)
{
    formatComboBox = new QComboBox;
    formatComboBox->addItem(tr("Native"));
    formatComboBox->addItem(tr("INI"));

    scopeComboBox = new QComboBox;
    scopeComboBox->addItem(tr("User"));
    scopeComboBox->addItem(tr("System"));

    organizationComboBox = new QComboBox;
    organizationComboBox->addItem(tr("Trolltech"));
    organizationComboBox->addItem(tr("Qt Examples"));
    organizationComboBox->setEditable(true);

    applicationComboBox = new QComboBox;
    applicationComboBox->addItem(tr("Any"));
    applicationComboBox->addItem(tr("Assistant"));
    applicationComboBox->addItem(tr("Designer"));
    applicationComboBox->addItem(tr("Linguist"));
    applicationComboBox->addItem(tr("QMake"));
    applicationComboBox->setEditable(true);
    applicationComboBox->setCurrentIndex(2);

    formatLabel = new QLabel(tr("&Format:"));
    formatLabel->setBuddy(formatComboBox);

    scopeLabel = new QLabel(tr("&Scope:"));
    scopeLabel->setBuddy(scopeComboBox);

    organizationLabel = new QLabel(tr("&Organization:"));
    organizationLabel->setBuddy(organizationComboBox);

    applicationLabel = new QLabel(tr("&Application:"));
    applicationLabel->setBuddy(applicationComboBox);

    locationsGroupBox = new QGroupBox(tr("Locations"));

    QStringList labels;
    labels << tr("Path") << tr("Size") << tr("Access");

    locationsTable = new QTableWidget;
    locationsTable->setSelectionMode(QAbstractItemView::NoSelection);
    locationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    locationsTable->setColumnCount(3);
    locationsTable->setHorizontalHeaderLabels(labels);
    locationsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    locationsTable->horizontalHeader()->resizeSection(2, 180);

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    okButton->setDefault(true);

    connect(formatComboBox, SIGNAL(activated(int)),
            this, SLOT(updateLocationsTable()));
    connect(scopeComboBox, SIGNAL(activated(int)),
            this, SLOT(updateLocationsTable()));
    connect(organizationComboBox, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(updateLocationsTable()));
    connect(applicationComboBox, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(updateLocationsTable()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QVBoxLayout *locationsLayout = new QVBoxLayout;
    locationsLayout->addWidget(locationsTable);
    locationsGroupBox->setLayout(locationsLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(formatLabel, 0, 0);
    mainLayout->addWidget(formatComboBox, 0, 1);
    mainLayout->addWidget(scopeLabel, 1, 0);
    mainLayout->addWidget(scopeComboBox, 1, 1);
    mainLayout->addWidget(organizationLabel, 2, 0);
    mainLayout->addWidget(organizationComboBox, 2, 1);
    mainLayout->addWidget(applicationLabel, 3, 0);
    mainLayout->addWidget(applicationComboBox, 3, 1);
    mainLayout->addWidget(locationsGroupBox, 4, 0, 1, 2);
    mainLayout->addLayout(buttonLayout, 5, 0, 1, 2);
    setLayout(mainLayout);

    updateLocationsTable();

    setWindowTitle(tr("Open Application Settings"));
    resize(650, 400);
}

QSettings::Format LocationDialog::format() const
{
    if (formatComboBox->currentIndex() == 0)
        return QSettings::NativeFormat;
    else
        return QSettings::IniFormat;
}

QSettings::Scope LocationDialog::scope() const
{
    if (scopeComboBox->currentIndex() == 0)
        return QSettings::UserScope;
    else
        return QSettings::SystemScope;
}

QString LocationDialog::organization() const
{
    return organizationComboBox->currentText();
}

QString LocationDialog::application() const
{
    if (applicationComboBox->currentIndex() == 0)
        return "";
    else
        return applicationComboBox->currentText();
}

void LocationDialog::updateLocationsTable()
{
    locationsTable->setUpdatesEnabled(false);
    locationsTable->setRowCount(0);

    for (int i = 0; i < 2; ++i) {
        if (i == 0 && scope() == QSettings::SystemScope)
            continue;

        QSettings::Scope actualScope = (i == 0) ? QSettings::UserScope
                                                : QSettings::SystemScope;
        for (int j = 0; j < 2; ++j) {
            if (j == 0 && application().isEmpty())
                continue;

            QString actualApplication;
            if (j == 0)
                actualApplication = application();
            QSettings settings(format(), actualScope, organization(),
                               actualApplication);

            int row = locationsTable->rowCount();
            locationsTable->setRowCount(row + 1);

            QFileInfo fileInfo(settings.fileName());

            QTableWidgetItem *headerItem = new QTableWidgetItem;
            headerItem->setText(QString::number(row + 1));

            QTableWidgetItem *item0 = new QTableWidgetItem;
            item0->setText(fileInfo.absoluteFilePath());

            QTableWidgetItem *item1 = new QTableWidgetItem;
            item1->setText(tr("%1 KB").arg((fileInfo.size() + 1023) / 1024));

            QTableWidgetItem *item2 = new QTableWidgetItem;
            bool disable = (fileInfo.size() == 0);

            if (row == 0) {
                if (settings.isWritable()) {
                    item2->setText(tr("Read-write"));
                    disable = false;
                } else {
                    item2->setText(tr("Read-only"));
                }
                okButton->setDisabled(disable);
            } else {
                item2->setText(tr("Read-only fallback"));
            }

            if (disable) {
                item0->setFlags(item0->flags() & ~Qt::ItemIsEnabled);
                item1->setFlags(item1->flags() & ~Qt::ItemIsEnabled);
                item2->setFlags(item2->flags() & ~Qt::ItemIsEnabled);
            }

            locationsTable->setVerticalHeaderItem(row, headerItem);
            locationsTable->setItem(row, 0, item0);
            locationsTable->setItem(row, 1, item1);
            locationsTable->setItem(row, 2, item2);
        }
    }
    locationsTable->setUpdatesEnabled(true);
}
