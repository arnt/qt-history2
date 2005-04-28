#include <QtGui>

#include "interfaces.h"
#include "plugindialog.h"

PluginDialog::PluginDialog(const QString &path, const QStringList &fileNames,
                           QWidget *parent)
    : QDialog(parent)
{
    label = new QLabel;
    label->setWordWrap(true);

    QStringList headerLabels;
    headerLabels << tr("Components");

    treeWidget = new QTreeWidget;
    treeWidget->setAlternatingRowColors(false);
    treeWidget->setHeaderLabels(headerLabels);
    treeWidget->header()->hide();

    if (fileNames.isEmpty()) {
        label->setText(tr("Plug & Paint couldn't detect any plugins."));
        treeWidget->hide();
    } else {
        label->setText(tr("Plug & Paint found the following plugins in the %1 "
                          "directory:")
                       .arg(QDir::convertSeparators(path)));

        QDir dir(path);

        foreach (QString fileName, fileNames) {
            QPluginLoader loader(dir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();

            QTreeWidgetItem *pluginItem = new QTreeWidgetItem(treeWidget);
            pluginItem->setText(0, fileName);
            treeWidget->setItemExpanded(pluginItem, true);

            QFont boldFont = pluginItem->font(0);
            boldFont.setBold(true);
            pluginItem->setFont(0, boldFont);

            if (plugin) {
                BrushInterface *iBrush = qobject_cast<BrushInterface *>(plugin);
                if (iBrush)
                    addItems(pluginItem, "BrushInterface", iBrush->brushes());

                ShapeInterface *iShape = qobject_cast<ShapeInterface *>(plugin);
                if (iShape)
                    addItems(pluginItem, "ShapeInterface", iShape->shapes());

                FilterInterface *iFilter =
                        qobject_cast<FilterInterface *>(plugin);
                if (iFilter)
                    addItems(pluginItem, "FilterInterface", iFilter->filters());
            }
        }
    }

    okButton = new QPushButton(tr("OK"));

    connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->addWidget(label, 0, 0, 1, 3);
    mainLayout->addWidget(treeWidget, 1, 0, 1, 3);
    mainLayout->addWidget(okButton, 2, 1);
    setLayout(mainLayout);

    setWindowTitle(tr("Plug & Paint"));
}

void PluginDialog::addItems(QTreeWidgetItem *pluginItem,
                            const char *interfaceName,
                            const QStringList &features)
{
    QTreeWidgetItem *interfaceItem = new QTreeWidgetItem(pluginItem);
    interfaceItem->setText(0, interfaceName);
    treeWidget->setItemExpanded(interfaceItem, true);

    foreach (QString feature, features) {
        QTreeWidgetItem *featureItem = new QTreeWidgetItem(interfaceItem);
        featureItem->setText(0, feature);
    }
}
