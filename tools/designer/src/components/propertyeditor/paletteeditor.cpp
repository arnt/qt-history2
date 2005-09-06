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

#include <QtCore/qdebug.h>
#include <QtGui/QMessageBox>
#include <QMetaProperty>
#include <QPainter>

using namespace qdesigner_internal;

static const int pixmapSize = 32;

PaletteEditor::PaletteEditor(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    m_colorRoleModel = new ColorRoleModel(this);
    ColorDelegate *delegate = new ColorDelegate(this);
    ui.roleListView->setModel(m_colorRoleModel);
    ui.roleListView->setItemDelegate(delegate);
    ui.roleListView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.roleListView->setIconSize(QSize(pixmapSize, pixmapSize));
    ui.roleListView->setSpacing(1);
    connect(m_colorRoleModel, SIGNAL(colorChanged(QPalette::ColorRole, const QColor &)),
                this, SLOT(colorChanged(QPalette::ColorRole, const QColor &)));
    connect(ui.btnAdvanced, SIGNAL(toggled(bool)), this, SLOT(showAdvanced(bool)));
    ui.advancedBox->setVisible(false);
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::on_buttonMainColor_changed()
{
    buildPalette();
}

void PaletteEditor::on_buttonMainColor2_changed()
{
    buildPalette();
}

void PaletteEditor::on_paletteCombo_activated(int)
{
    updatePreviewPalette();
}

void PaletteEditor::colorChanged(QPalette::ColorRole role, const QColor &color)
{
    QPalette::ColorGroup g = selectedColorGroup();
    m_editPalette.setColor(g, role, color);
    updatePreviewPalette();
}

void PaletteEditor::showAdvanced(bool showIt)
{
    if (ui.advancedBox->isVisible() == showIt)
        return;
    if (showIt) {
        m_sizeWithoutExtension = size();
        ui.advancedBox->setVisible(true);
    }
    else {
        ui.advancedBox->setVisible(false);
        layout()->activate();
        resize(m_sizeWithoutExtension);
    }
}

void PaletteEditor::buildPalette()
{
    QColor btn = ui.buttonMainColor->brush().color();
    QColor back = ui.buttonMainColor2->brush().color();
    m_editPalette = QPalette(btn, back);
    buildActiveEffect();
    updatePaletteEffect(QPalette::Inactive);
    updatePaletteEffect(QPalette::Disabled);

    setEditPalette(m_editPalette);
}

void PaletteEditor::buildActiveEffect()
{
    QColor btn = m_editPalette.color(QPalette::Button);

    QPalette temp(btn, btn);
    temp.setCurrentColorGroup(QPalette::Active);

    m_editPalette.setCurrentColorGroup(QPalette::Active);

    m_editPalette.setBrush(QPalette::Light, temp.light());
    m_editPalette.setBrush(QPalette::Midlight, temp.midlight());
    m_editPalette.setBrush(QPalette::Mid, temp.mid());
    m_editPalette.setBrush(QPalette::Dark, temp.dark());
    m_editPalette.setBrush(QPalette::Shadow, temp.shadow());
}

void PaletteEditor::updatePaletteEffect(QPalette::ColorGroup g)
{
    QColor btn = m_editPalette.color(g, QPalette::Button);

    QColor light = btn.light(150);
    QColor midlight = btn.light(115);
    QColor mid = btn.dark(150);
    QColor dark = btn.dark();
    QColor shadow = Qt::black;

    m_editPalette.setColor(g, QPalette::Light, light);
    m_editPalette.setColor(g, QPalette::Midlight, midlight);
    m_editPalette.setColor(g, QPalette::Mid, mid);
    m_editPalette.setColor(g, QPalette::Dark, dark);
    m_editPalette.setColor(g, QPalette::Shadow, shadow);
}

QPalette::ColorGroup PaletteEditor::selectedColorGroup() const
{
    switch (ui.paletteCombo->currentIndex()) {
    default: return QPalette::Active;
    case 0: return QPalette::Active;
    case 1: return QPalette::Inactive;
    case 2: return QPalette::Disabled;
    }
}

void PaletteEditor::updatePreviewPalette()
{
    QPalette::ColorGroup g = selectedColorGroup();

    // build the preview palette
    QPalette previewPalette = m_editPalette;
    for (QPalette::ColorRole r = QPalette::Foreground; r < QPalette::NColorRoles; reinterpret_cast<int&>(r)++) {
        previewPalette.setColor(QPalette::Active, r, m_editPalette.color(g, r));
        previewPalette.setColor(QPalette::Inactive, r, m_editPalette.color(g, r));
        previewPalette.setColor(QPalette::Disabled, r, m_editPalette.color(g, r));
    }
    ui.previewFrame->setPreviewPalette(previewPalette);

    QPalette p = m_editPalette;
    p.setCurrentColorGroup(g);
    m_colorRoleModel->setPalette(p);
}

void PaletteEditor::updateStyledButtons()
{
    ui.buttonMainColor->setBrush(m_editPalette.color(QPalette::Active, QPalette::Button));
    ui.buttonMainColor2->setBrush(m_editPalette.color(QPalette::Active, QPalette::Background));
}

void PaletteEditor::setEditPalette(const QPalette& pal)
{
    m_editPalette = pal;
    updatePreviewPalette();
    updateStyledButtons();
}

QPalette PaletteEditor::editPalette() const
{
    return m_editPalette;
}

QPalette PaletteEditor::getPalette(QWidget* parent, const QPalette &init, int *ok)
{
    PaletteEditor dlg(parent);
    dlg.setEditPalette(init);

    int result = dlg.exec();
    if (ok) *ok = result;

    return result == QDialog::Accepted ? dlg.editPalette() : init;
}

//////////////////////

ColorRoleModel::ColorRoleModel(QWidget *parent)
    : QAbstractListModel(parent)
{
    const QMetaObject *meta = metaObject();
    int index = meta->indexOfProperty("colorRole");
    QMetaProperty p = meta->property(index);
    QMetaEnum e = p.enumerator();
    for (int i = 0; i < QPalette::NColorRoles; i++) {
        m_roleNames[i] = QLatin1String(e.key(i));
        m_colorPixmaps[i] = generatePixmap(m_palette.color((QPalette::ColorRole)i));
    }
}

int ColorRoleModel::rowCount(const QModelIndex &) const
{
    return m_colorPixmaps.count();
}

QVariant ColorRoleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= QPalette::NColorRoles)
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_roleNames[index.row()];
    if (role == Qt::DecorationRole)
        return m_colorPixmaps[index.row()];
    if (role == Qt::EditRole)
        return m_palette.color((QPalette::ColorRole)index.row());
    return QVariant();
}

bool ColorRoleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        QColor c = qVariantValue<QColor>(value);
        QPalette::ColorRole r = (QPalette::ColorRole)index.row();
        if (m_palette.color(r) == c)
            return true;
        m_palette.setColor(r, c);
        m_colorPixmaps[index.row()] = generatePixmap(c);
        emit colorChanged(r, c);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags ColorRoleModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QPalette ColorRoleModel::getPalette() const
{
    return m_palette;
}

void ColorRoleModel::setPalette(const QPalette &palette)
{
    for (int i = 0; i < QPalette::NColorRoles; i++) {
        QModelIndex idx = index(i);
        setData(idx, QVariant(palette.color((QPalette::ColorRole)i)), Qt::EditRole);
    }
}

QPixmap ColorRoleModel::generatePixmap(const QColor &color) const
{
    int w = pixmapSize;
    int h = pixmapSize;
    QPixmap pix(w, h);
    pix.fill(color);
    QPainter p(&pix);
    p.drawRect(0, 0, w - 1, h - 1);
    return pix;
}

//////////////////////////

ColorEditor::ColorEditor(QWidget *parent)
    : QWidget(parent)
{
    QLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(2);
    button = new StyledButton(this);
    label = new QLabel(this);
    layout->addWidget(button);
    layout->addWidget(label);
    button->setFixedWidth(pixmapSize + 2);
    connect(button, SIGNAL(changed()), this, SLOT(colorChanged()));
    setFocusProxy(button);
//    button->setFocusPolicy(Qt::NoFocus);
}

void ColorEditor::setColor(const QColor &color)
{
    button->setBrush(color);
}

void ColorEditor::setLabel(const QString &text)
{
    QString boldText = QString("<b>%1</b>").arg(text);
    label->setText(boldText);
}

QColor ColorEditor::color() const
{
    return button->brush().color();
}

void ColorEditor::colorChanged()
{
    emit changed(this);
}

//////////////////////////

QWidget *ColorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                const QModelIndex &) const
{
    ColorEditor *editor = new ColorEditor(parent);
    connect(editor, SIGNAL(changed(QWidget *)), this, SIGNAL(commitData(QWidget *)));
    editor->setFocusPolicy(Qt::NoFocus);
    editor->installEventFilter(const_cast<ColorDelegate*>(this));
    return editor;
}

void ColorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QColor c = qVariantValue<QColor>(index.model()->data(index, Qt::EditRole));
    QString colorName = qVariantValue<QString>(index.model()->data(index, Qt::DisplayRole));
    ColorEditor *button = static_cast<ColorEditor*>(editor);
    button->setColor(c);
    button->setLabel(colorName);
}

void ColorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                const QModelIndex &index) const
{
    ColorEditor *button = static_cast<ColorEditor*>(editor);
    QColor c = button->color();

    model->setData(index, c, Qt::EditRole);
}

void ColorDelegate::updateEditorGeometry(QWidget *editor,
                const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
