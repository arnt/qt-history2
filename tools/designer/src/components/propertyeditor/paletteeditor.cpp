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

#include "paletteeditor.h"
#include <iconloader_p.h>

#include <QMetaProperty>
#include <QPainter>
#include <QToolButton>
#include <QLabel>
#include <QHeaderView>

using namespace qdesigner_internal;

PaletteEditor::PaletteEditor(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    m_modelUpdated = false;
    m_paletteUpdated = false;
    m_currentColorGroup = QPalette::Active;
    m_compute = true;
    updatePreviewPalette();
    updateStyledButton();
    m_paletteModel = new PaletteModel(this);
    ui.paletteView->setModel(m_paletteModel);
    ColorDelegate *delegate = new ColorDelegate(this);
    ui.paletteView->setItemDelegate(delegate);
    ui.paletteView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //ui.paletteView->setAlternatingRowColors(true);
    connect(m_paletteModel, SIGNAL(paletteChanged(const QPalette &)),
                this, SLOT(paletteChanged(const QPalette &)));
    ui.paletteView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.paletteView->setDragEnabled(true);
    ui.paletteView->setDropIndicatorShown(true);
    ui.paletteView->setRootIsDecorated(false);
    ui.paletteView->setColumnHidden(2, true);
    ui.paletteView->setColumnHidden(3, true);
}

PaletteEditor::~PaletteEditor()
{
}

QPalette PaletteEditor::palette() const
{
    return m_editPalette;
}

void PaletteEditor::setPalette(const QPalette &palette)
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

void PaletteEditor::setPalette(const QPalette &palette, const QPalette &parentPalette)
{
    m_parentPalette = parentPalette;
    setPalette(palette);
}

void PaletteEditor::on_buildButton_changed()
{
    buildPalette();
}

void PaletteEditor::on_activeRadio_clicked()
{
    m_currentColorGroup = QPalette::Active;
    updatePreviewPalette();
}

void PaletteEditor::on_inactiveRadio_clicked()
{
    m_currentColorGroup = QPalette::Inactive;
    updatePreviewPalette();
}

void PaletteEditor::on_disabledRadio_clicked()
{
    m_currentColorGroup = QPalette::Disabled;
    updatePreviewPalette();
}

void PaletteEditor::on_computeRadio_clicked()
{
    if (m_compute)
        return;
    ui.paletteView->setColumnHidden(2, true);
    ui.paletteView->setColumnHidden(3, true);
    m_compute = true;
    m_paletteModel->setCompute(true);
}

void PaletteEditor::on_detailsRadio_clicked()
{
    if (!m_compute)
        return;
    int w = ui.paletteView->columnWidth(1);
    ui.paletteView->setColumnHidden(2, false);
    ui.paletteView->setColumnHidden(3, false);
    QHeaderView *header = ui.paletteView->header();
    header->resizeSection(1, w / 3);
    header->resizeSection(2, w / 3);
    header->resizeSection(3, w / 3);
    m_compute = false;
    m_paletteModel->setCompute(false);
}

void PaletteEditor::paletteChanged(const QPalette &palette)
{
    m_modelUpdated = true;
    if (!m_paletteUpdated)
        setPalette(palette);
    m_modelUpdated = false;
}

void PaletteEditor::buildPalette()
{
    QColor btn = ui.buildButton->brush().color();
    QPalette temp = QPalette(btn);
    setPalette(temp);
}

void PaletteEditor::updatePreviewPalette()
{
    QPalette::ColorGroup g = currentColorGroup();

    // build the preview palette
    QPalette currentPalette = palette();
    QPalette previewPalette;
    for (QPalette::ColorRole r = QPalette::Foreground;
                r < QPalette::NColorRoles; reinterpret_cast<int&>(r)++) {
        QColor c = currentPalette.color(g, r);
        previewPalette.setColor(QPalette::Active, r, c);
        previewPalette.setColor(QPalette::Inactive, r, c);
        previewPalette.setColor(QPalette::Disabled, r, c);
    }
    ui.previewFrame->setPreviewPalette(previewPalette);

    bool enabled = true;
    if (g == QPalette::Disabled)
        enabled = false;
    ui.previewFrame->setEnabled(enabled);
}

void PaletteEditor::updateStyledButton()
{
    ui.buildButton->setBrush(palette().brush(QPalette::Active, QPalette::Button));
}

QPalette PaletteEditor::getPalette(QWidget* parent, const QPalette &init,
            const QPalette &parentPal, int *ok)
{
    PaletteEditor dlg(parent);
    QPalette parentPalette(parentPal);
    uint mask = init.resolve();
    for (int i = 0; i < (int)QPalette::NColorRoles; i++) {
        if (!(mask & (1 << i))) {
            parentPalette.setBrush(QPalette::Active, (QPalette::ColorRole)i,
                        init.brush(QPalette::Active, (QPalette::ColorRole)i));
            parentPalette.setBrush(QPalette::Inactive, (QPalette::ColorRole)i,
                        init.brush(QPalette::Inactive, (QPalette::ColorRole)i));
            parentPalette.setBrush(QPalette::Disabled, (QPalette::ColorRole)i,
                        init.brush(QPalette::Disabled, (QPalette::ColorRole)i));
        }
    }
    dlg.setPalette(init, parentPalette);

    int result = dlg.exec();
    if (ok) *ok = result;

    return result == QDialog::Accepted ? dlg.palette() : init;
}

//////////////////////

PaletteModel::PaletteModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    const QMetaObject *meta = metaObject();
    int index = meta->indexOfProperty("colorRole");
    QMetaProperty p = meta->property(index);
    QMetaEnum e = p.enumerator();
    for (QPalette::ColorRole r = QPalette::Foreground; r < QPalette::NColorRoles;
                reinterpret_cast<int&>(r)++) {
        m_roleNames[r] = QLatin1String(e.key(reinterpret_cast<int&>(r)));
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

//////////////////////////

ColorEditor::ColorEditor(QWidget *parent)
    : QWidget(parent)
{
    QLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    button = new StyledButton(this);
    layout->addWidget(button);
    connect(button, SIGNAL(changed()), this, SLOT(colorChanged()));
    setFocusProxy(button);
    m_changed = false;
}

void ColorEditor::setColor(const QColor &color)
{
    button->setBrush(color);
    m_changed = false;
}

QColor ColorEditor::color() const
{
    return button->brush().color();
}

void ColorEditor::colorChanged()
{
    m_changed = true;
    emit changed(this);
}

bool ColorEditor::changed() const
{
    return m_changed;
}

//////////////////////////

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
    button->setIcon(createIconSet(QLatin1String("resetproperty.png")));
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
        //editor->setFocusPolicy(Qt::NoFocus);
        //editor->installEventFilter(const_cast<ColorDelegate *>(this));
        ed = editor;
    } else {
        ColorEditor *editor = new ColorEditor(parent);
        connect(editor, SIGNAL(changed(QWidget *)), this, SIGNAL(commitData(QWidget *)));
        editor->setFocusPolicy(Qt::NoFocus);
        editor->installEventFilter(const_cast<ColorDelegate *>(this));
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
    } else {
        QColor c = qVariantValue<QColor>(index.model()->data(index, Qt::BackgroundColorRole));
        ColorEditor *editor = static_cast<ColorEditor *>(ed);
        editor->setColor(c);
    }
}

void ColorDelegate::setModelData(QWidget *ed, QAbstractItemModel *model,
                const QModelIndex &index) const
{
    if (index.column() == 0) {
        RoleEditor *editor = static_cast<RoleEditor *>(ed);
        bool mask = editor->edited();
        model->setData(index, mask, Qt::EditRole);
    } else {
        ColorEditor *editor = static_cast<ColorEditor *>(ed);
        if (editor->changed()) {
            QColor c = editor->color();
            model->setData(index, c, Qt::BackgroundColorRole);
        }
    }
}

void ColorDelegate::updateEditorGeometry(QWidget *ed,
                const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::updateEditorGeometry(ed, option, index);
    ed->setGeometry(ed->geometry().adjusted(0, 0, -1, -1));
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







