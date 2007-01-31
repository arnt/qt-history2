/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "appearancepage.h"
#include <QSettings>
#include <QStyleFactory>
#include <QStyle>
#include <QDebug>
#include <QWidget>
#include <QFrame>
#include <QPalette>
#include <QApplication>
#include <QAbstractItemView>
#include <QString>
#include <QLatin1String>
#include <QStringList>
#include <QApplication>
#include <QList>
#include <QDialog>
#include <QVBoxLayout>
#include <QObject>
#include <QHeaderView>
#include <QColor>
#include <QAbstractTableModel>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QModelIndex>
#include <QVariant>
#include <QLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QItemDelegate>
#include <QToolButton>
#include <QIcon>
#include <QSize>
#include <QSizePolicy>
#include <QFont>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>
#include <QPainter>
#include <QHeaderView>
#include <QColorDialog>

AppearancePage::AppearancePage(QWidget *parent)
    : QFrame(parent)
{
    setupUi(this);
    connect(paletteView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));
    m_modelUpdated = false;
    m_paletteUpdated = false;
    m_currentColorGroup = QPalette::Active;
    m_compute = true;
    updatePreviewPalette();
    updateStyledButton();
    m_paletteModel = 0;
    ColorDelegate *delegate = new ColorDelegate(this);
    paletteView->setItemDelegate(delegate);
    paletteView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    paletteView->header()->setStretchLastSection(false);
    connect(pbBuild, SIGNAL(changed()), this, SLOT(buildPalette()));
    connect(cmbColorGroup, SIGNAL(activated(QString)), this, SLOT(onColorGroupActivated(QString)));
    connect(cbDetails, SIGNAL(toggled(bool)), this, SLOT(onDetailsToggled(bool)));

    paletteView->setSelectionBehavior(QAbstractItemView::SelectRows);
    paletteView->setDragEnabled(true);
    paletteView->setDropIndicatorShown(true);
    paletteView->setRootIsDecorated(false);
    paletteView->setColumnHidden(2, true);
    paletteView->setColumnHidden(3, true);

    connect(cmbStyle, SIGNAL(currentIndexChanged(QString)), this, SLOT(onStyleComboChanged(QString)));
    load();
}
void AppearancePage::load()
{
    delete m_paletteModel;
    m_paletteModel = new PaletteModel(this);
    connect(m_paletteModel, SIGNAL(paletteChanged(const QPalette &)),
            this, SLOT(paletteChanged(const QPalette &)));
    paletteView->setModel(m_paletteModel);
    for (int i=0; i<paletteView->header()->count(); ++i) {
        paletteView->header()->setResizeMode(i, QHeaderView::Stretch);
    }
    updatePreviewPalette();

    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    cmbStyle->clear();
    QStringList styles = QStyleFactory::keys();
    QString currentstyle = settings.value("style").toString().toLower();
    if (currentstyle.isEmpty())
        currentstyle = QApplication::style()->objectName();
    int found = -1;
    for (int i=0; i<styles.size(); ++i) {
        const QString s = styles.at(i).toLower();
        if (s == currentstyle) {
            found = i;
            break;
        } else if (currentstyle.contains(s) || (currentstyle.left(9) == s.left(9))) {
            found = i;
        }
    }
    if (found == -1) {
        styles << QLatin1String("Unknown");
        found = styles.size() - 1;
    }
    cmbStyle->addItems(styles);
    cmbStyle->setCurrentIndex(found);
}

void AppearancePage::save()
{
    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    settings.setValue("style", cmbStyle->currentText());

    QStringList actcg, inactcg, discg;
    for (int i = 0; i < QPalette::NColorRoles; i++)
        actcg << m_editPalette.color(QPalette::Active,
                                     (QPalette::ColorRole) i).name();
    for (int i = 0; i < QPalette::NColorRoles; i++)
        inactcg << m_editPalette.color(QPalette::Inactive,
                                       (QPalette::ColorRole) i).name();
    for (int i = 0; i < QPalette::NColorRoles; i++)
        discg << m_editPalette.color(QPalette::Disabled,
                                     (QPalette::ColorRole) i).name();

    settings.setValue("Palette/active", actcg);
    settings.setValue("Palette/inactive", inactcg);
    settings.setValue("Palette/disabled", discg);


}
void AppearancePage::onStyleComboChanged(const QString &stylename)
{
    QStyle *style = QStyleFactory::create(stylename);
    if (style) {
        const QList<QWidget*> subWidgets = qFindChildren<QWidget*>(previewFrame);
        for (int i=0; i<subWidgets.size(); ++i) {
            subWidgets.at(i)->setStyle(style);
        }
    }
    emit changed();
}

QPalette AppearancePage::palette() const
{
    return m_editPalette;
}

void AppearancePage::setPalette(const QPalette &palette)
{
    m_editPalette = palette;
    uint mask = palette.resolve();
    for (int i = 0; i < (int)QPalette::NColorRoles; i++) {
        if (!(mask & (1 << i))) {
            m_editPalette.setBrush(QPalette::Active, (QPalette::ColorRole)i,
                                   m_parentPalette.brush(QPalette::Active, (QPalette::ColorRole)i));
            m_editPalette.setBrush(QPalette::Inactive, (QPalette::ColorRole)i,
                                   m_parentPalette.brush(QPalette::Inactive, (QPalette::ColorRole)i));
            m_editPalette.setBrush(QPalette::Disabled, (QPalette::ColorRole)i,
                                   m_parentPalette.brush(QPalette::Disabled, (QPalette::ColorRole)i));
        }
    }
    m_editPalette.resolve(mask);
    updatePreviewPalette();
    updateStyledButton();
    m_paletteUpdated = true;
    if (!m_modelUpdated)
        m_paletteModel->setPalette(m_editPalette, m_parentPalette);
    m_paletteUpdated = false;
}

void AppearancePage::setPalette(const QPalette &palette, const QPalette &parentPalette)
{
    m_parentPalette = parentPalette;
    setPalette(palette);
}

void AppearancePage::onColorGroupActivated(const QString &colorGroup)
{
    if (colorGroup == tr("Active")) {
        m_currentColorGroup = QPalette::Active;
    } else if (colorGroup == tr("Inactive")) {
        m_currentColorGroup = QPalette::Inactive;
    } else if (colorGroup == tr("Disabled")) {
        m_currentColorGroup = QPalette::Disabled;
    }

    updatePreviewPalette();
}

void AppearancePage::onDetailsToggled(bool showDetails)
{
    if (showDetails) {
        paletteView->setColumnHidden(2, false);
        paletteView->setColumnHidden(3, false);
        m_compute = false;
        m_paletteModel->setCompute(false);
    } else {
        paletteView->setColumnHidden(2, true);
        paletteView->setColumnHidden(3, true);
        m_compute = true;
        m_paletteModel->setCompute(true);
    }
}

void AppearancePage::paletteChanged(const QPalette &palette)
{
    m_modelUpdated = true;
    if (!m_paletteUpdated) {
        if (palette != m_editPalette)
            emit changed();
        setPalette(palette);
    }
    m_modelUpdated = false;
}

void AppearancePage::buildPalette()
{
    QColor btn = pbBuild->brush().color();
    QPalette temp = QPalette(btn);
    if (temp != m_editPalette)
        emit changed();
    setPalette(temp);
}

void AppearancePage::updatePreviewPalette()
{
    QPalette::ColorGroup g = currentColorGroup();

    // build the preview palette
    QPalette currentPalette = palette();
    QPalette previewPalette;
    for (int i = QPalette::Foreground; i < QPalette::NColorRoles; i++) {
        const QPalette::ColorRole r = (QPalette::ColorRole)i;
        const QColor c = currentPalette.color(g, r);
        previewPalette.setColor(QPalette::Active, r, c);
        previewPalette.setColor(QPalette::Inactive, r, c);
        previewPalette.setColor(QPalette::Disabled, r, c);
    }
    previewFrame->setPreviewPalette(previewPalette);

    bool enabled = true;
    if (g == QPalette::Disabled)
        enabled = false;
    previewFrame->setEnabled(enabled);
}

void AppearancePage::updateStyledButton()
{
    pbBuild->setBrush(palette().brush(QPalette::Active, QPalette::Button));
}

// PaletteModel

PaletteModel::PaletteModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    const QMetaObject *meta = metaObject();
    int index = meta->indexOfProperty("colorRole");
    QMetaProperty p = meta->property(index);
    QMetaEnum e = p.enumerator();
    for (int r = QPalette::Foreground; r < QPalette::NColorRoles; r++) {
        m_roleNames[(QPalette::ColorRole)r] = QLatin1String(e.key(r));
    }
    m_compute = true;
}

int PaletteModel::rowCount(const QModelIndex &) const
{
    return m_roleNames.count();
}

int PaletteModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= QPalette::NColorRoles)
        return QVariant();
    if (index.column() < 0 || index.column() >= 4)
        return QVariant();

    if (index.column() == 0) {
        if (role == Qt::DisplayRole)
            return m_roleNames[(QPalette::ColorRole)index.row()];
        if (role == Qt::EditRole) {
            uint mask = m_palette.resolve();
            if (mask & (1 << index.row()))
                return true;
            return false;
        }
        return QVariant();
    }
    if (role == Qt::BackgroundColorRole)
        return m_palette.color(columnToGroup(index.column()),
                               (QPalette::ColorRole)index.row());
    return QVariant();
}

bool PaletteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.column() != 0 && role == Qt::BackgroundColorRole) {
        QColor c = qVariantValue<QColor>(value);
        QPalette::ColorRole r = (QPalette::ColorRole)index.row();
        QPalette::ColorGroup g = columnToGroup(index.column());
        m_palette.setBrush(g, r, c);

        QModelIndex idxBegin = PaletteModel::index(r, 0);
        QModelIndex idxEnd = PaletteModel::index(r, 3);
        if (m_compute) {
            m_palette.setBrush(QPalette::Inactive, r, c);
            switch (r) {
            case QPalette::Foreground:
            case QPalette::Text:
            case QPalette::ButtonText:
            case QPalette::Base:
                break;
            case QPalette::Dark:
                m_palette.setBrush(QPalette::Disabled, QPalette::Foreground, c);
                m_palette.setBrush(QPalette::Disabled, QPalette::Dark, c);
                m_palette.setBrush(QPalette::Disabled, QPalette::Text, c);
                m_palette.setBrush(QPalette::Disabled, QPalette::ButtonText, c);
                idxBegin = PaletteModel::index(0, 0);
                idxEnd = PaletteModel::index(m_roleNames.count() - 1, 3);
                break;
            case QPalette::Background:
                m_palette.setBrush(QPalette::Disabled, QPalette::Base, c);
                m_palette.setBrush(QPalette::Disabled, QPalette::Background, c);
                idxBegin = PaletteModel::index(QPalette::Base, 0);
                break;
            case QPalette::Highlight:
                m_palette.setBrush(QPalette::Disabled, QPalette::Highlight, c.dark(120));
                break;
            default:
                m_palette.setBrush(QPalette::Disabled, r, c);
                break;
            }
        }
        emit paletteChanged(m_palette);
        emit dataChanged(idxBegin, idxEnd);
        return true;
    }
    if (index.column() == 0 && role == Qt::EditRole) {
        uint mask = m_palette.resolve();
        bool isMask = qVariantValue<bool>(value);
        int r = index.row();
        if (isMask)
            mask |= (1 << r);
        else {
            m_palette.setBrush(QPalette::Active, (QPalette::ColorRole)r,
                               m_parentPalette.brush(QPalette::Active, (QPalette::ColorRole)r));
            m_palette.setBrush(QPalette::Inactive, (QPalette::ColorRole)r,
                               m_parentPalette.brush(QPalette::Inactive, (QPalette::ColorRole)r));
            m_palette.setBrush(QPalette::Disabled, (QPalette::ColorRole)r,
                               m_parentPalette.brush(QPalette::Disabled, (QPalette::ColorRole)r));

            mask &= ~(1 << index.row());
        }
        m_palette.resolve(mask);
        emit paletteChanged(m_palette);
        QModelIndex idxEnd = PaletteModel::index(r, 3);
        emit dataChanged(index, idxEnd);
        return true;
    }
    return false;
}

Qt::ItemFlags PaletteModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

QVariant PaletteModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Color Role");
        if (section == groupToColumn(QPalette::Active))
            return tr("Active");
        if (section == groupToColumn(QPalette::Inactive))
            return tr("Inactive");
        if (section == groupToColumn(QPalette::Disabled))
            return tr("Disabled");
    }
    return QVariant();
}

QPalette PaletteModel::getPalette() const
{
    return m_palette;
}

void PaletteModel::setPalette(const QPalette &palette, const QPalette &parentPalette)
{
    m_parentPalette = parentPalette;
    m_palette = palette;
    QModelIndex idxBegin = index(0, 0);
    QModelIndex idxEnd = index(m_roleNames.count() - 1, 3);
    emit paletteChanged(m_palette);
    emit dataChanged(idxBegin, idxEnd);
}

QPalette::ColorGroup PaletteModel::columnToGroup(int index) const
{
    if (index == 1)
        return QPalette::Active;
    if (index == 2)
        return QPalette::Inactive;
    return QPalette::Disabled;
}

int PaletteModel::groupToColumn(QPalette::ColorGroup group) const
{
    if (group == QPalette::Active)
        return 1;
    if (group == QPalette::Inactive)
        return 2;
    return 3;
}

// RoleEditor

RoleEditor::RoleEditor(QWidget *parent)
    : QWidget(parent)
{
    m_edited = false;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_label = new QLabel(this);
    layout->addWidget(m_label);
    m_label->setAutoFillBackground(true);
    m_label->setIndent(3); // ### hardcode it should have the same value of textMargin in QItemDelegate
    setFocusProxy(m_label);

    QToolButton *button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIcon(QIcon(QLatin1String(":/images/resetproperty.png")));
    button->setIconSize(QSize(8,8));
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    layout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(emitResetProperty()));
}

void RoleEditor::setLabel(const QString &label)
{
    m_label->setText(label);
}

void RoleEditor::setEdited(bool on)
{
    QFont font;
    if (on == true) {
        font.setBold(on);
    }
    m_label->setFont(font);
    m_edited = on;
}

bool RoleEditor::edited() const
{
    return m_edited;
}

void RoleEditor::emitResetProperty()
{
    setEdited(false);
    emit changed(this);
}

QWidget *ColorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                     const QModelIndex &index) const
{
    QWidget *ed = 0;
    if (index.column() == 0) {
        RoleEditor *editor = new RoleEditor(parent);
        connect(editor, SIGNAL(changed(QWidget *)), this, SIGNAL(commitData(QWidget *)));
        ed = editor;
    }
    return ed;
}

void ColorDelegate::setEditorData(QWidget *ed, const QModelIndex &index) const
{
    if (index.column() == 0) {
        bool mask = qVariantValue<bool>(index.model()->data(index, Qt::EditRole));
        RoleEditor *editor = static_cast<RoleEditor *>(ed);
        editor->setEdited(mask);
        QString colorName = qVariantValue<QString>(index.model()->data(index, Qt::DisplayRole));
        editor->setLabel(colorName);
    }
}

void ColorDelegate::setModelData(QWidget *ed, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    if (index.column() == 0) {
        RoleEditor *editor = static_cast<RoleEditor *>(ed);
        bool mask = editor->edited();
        model->setData(index, mask, Qt::EditRole);
    }
}

void ColorDelegate::updateEditorGeometry(QWidget *ed,
                                         const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::updateEditorGeometry(ed, option, index);
    ed->setGeometry(option.rect.adjusted(0, 0, -1, -1));
}

void ColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
                          const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    bool mask = qVariantValue<bool>(index.model()->data(index, Qt::EditRole));
    if (index.column() == 0 && mask) {
        option.font.setBold(true);
    }
    QItemDelegate::paint(painter, option, index);
    painter->drawLine(option.rect.right(), option.rect.y(),
                      option.rect.right(), option.rect.bottom());
    painter->drawLine(option.rect.x(), option.rect.bottom(),
                      option.rect.right(), option.rect.bottom());
}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(opt, index) + QSize(4, 4);
}
void AppearancePage::onDoubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        const QColor c = QColorDialog::getColor(qVariantValue<QColor>(index.data(Qt::BackgroundColorRole)),
                                                this);
        if (c.isValid()) {
            m_paletteModel->setData(index, c, Qt::BackgroundColorRole);
        }
    }
}
