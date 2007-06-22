/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::GraphicsPropertyEditor
*/

#include "graphicspropertyeditor.h"

#include <findicondialog_p.h>
#include <iconloader_p.h>

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerIconCacheInterface>

#include <QtGui/QComboBox>
#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>

namespace qdesigner_internal {

GraphicsPropertyEditor::~GraphicsPropertyEditor()
{
}

void GraphicsPropertyEditor::init()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_combo = new QComboBox(this);
    m_combo->setFrame(0);
    m_combo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    m_combo->setEditable(false);
    m_layout->addWidget(m_combo);
    m_button = new QToolButton(this);
    m_button->setIcon(createIconSet(QLatin1String("fileopen.png")));
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    m_layout->addWidget(m_button);
    connect(m_button, SIGNAL(clicked()), this, SLOT(showDialog()));
    connect(m_combo, SIGNAL(activated(int)), this, SLOT(comboActivated(int)));

    populateCombo();
}

void GraphicsPropertyEditor::setSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
}

void GraphicsPropertyEditor::comboActivated(int idx)
{
    if (m_mode == Icon) {
        setIcon(qvariant_cast<QIcon>(m_combo->itemData(idx)));
    } else {
        setPixmap(qvariant_cast<QPixmap>(m_combo->itemData(idx)));
    }
}

int GraphicsPropertyEditor::indexOfIcon(const QIcon &icon)
{
    if (m_mode == Pixmap)
        return -1;

    if (icon.isNull())
        return 0;

    for (int i = 1; i < m_combo->count(); ++i) {
        if (qvariant_cast<QIcon>(m_combo->itemData(i)).serialNumber() == icon.serialNumber())
            return i;
    }

    populateCombo();

    for (int i = 1; i < m_combo->count(); ++i) {
        if (qvariant_cast<QIcon>(m_combo->itemData(i)).serialNumber() == icon.serialNumber())
            return i;
    }

    return -1;
}

int GraphicsPropertyEditor::indexOfPixmap(const QPixmap &pixmap)
{
    if (m_mode == Icon)
        return -1;

    if (pixmap.isNull())
        return 0;

    for (int i = 1; i < m_combo->count(); ++i) {
        if (qvariant_cast<QPixmap>(m_combo->itemData(i)).serialNumber() == pixmap.serialNumber())
            return i;
    }

    populateCombo();

    for (int i = 1; i < m_combo->count(); ++i) {
        if (qvariant_cast<QPixmap>(m_combo->itemData(i)).serialNumber() == pixmap.serialNumber())
            return i;
    }

    return -1;
}

void GraphicsPropertyEditor::populateCombo()
{
    QDesignerFormWindowInterface *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    const QStringList qrc_list = form->resourceFiles();

    m_combo->clear();
    static const QString noIcon = tr("<no icon>");
    static const QString noPixmap = tr("<no pixmap>");

    QDesignerIconCacheInterface *cache = m_core->iconCache();
    if (m_mode == Icon) {
        m_combo->addItem(noIcon);
        const QList<QIcon> icon_list = cache->iconList();
        foreach (QIcon icon, icon_list) {
            const QString qrc_path = cache->iconToQrcPath(icon);
            if (!qrc_path.isEmpty() && !qrc_list.contains(qrc_path))
                continue;
            m_combo->addItem(icon, QFileInfo(cache->iconToFilePath(icon)).fileName(),
                                QVariant(icon));
        }
    } else {
        m_combo->addItem(noPixmap);
        const QList<QPixmap> pixmap_list = cache->pixmapList();
        foreach (QPixmap pixmap, pixmap_list) {
            const QString qrc_path = cache->iconToQrcPath(pixmap);
            if (!qrc_path.isEmpty() && !qrc_list.contains(qrc_path))
                continue;
            m_combo->addItem(QIcon(pixmap),
                                QFileInfo(cache->pixmapToFilePath(pixmap)).fileName(),
                                QVariant(pixmap));
        }
    }
    const bool blocked = m_combo->blockSignals(true);
    m_combo->setCurrentIndex(0);
    m_combo->blockSignals(blocked);
}

GraphicsPropertyEditor::GraphicsPropertyEditor(QDesignerFormEditorInterface *core, const QIcon &pm,
                                                QWidget *parent)
    : QWidget(parent),
      m_mode(Icon),
      m_core(core)
{
    init();
    setIcon(pm);
}

GraphicsPropertyEditor::GraphicsPropertyEditor(QDesignerFormEditorInterface *core, const QPixmap &pm,
                                                QWidget *parent)
    : QWidget(parent),
      m_mode(Pixmap),
      m_core(core)
{
    init();
    setPixmap(pm);
}

void GraphicsPropertyEditor::showDialog()
{
    QDesignerFormWindowInterface *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;

    QString file_path;
    QString qrc_path;

    if (m_mode == Icon && !m_icon.isNull()) {
        file_path = m_core->iconCache()->iconToFilePath(m_icon);
        qrc_path = m_core->iconCache()->iconToQrcPath(m_icon);
    } else if (!m_pixmap.isNull()) {
        file_path = m_core->iconCache()->pixmapToFilePath(m_pixmap);
        qrc_path = m_core->iconCache()->pixmapToQrcPath(m_pixmap);
    }

    FindIconDialog dialog(form, 0);
    dialog.setPaths(qrc_path, file_path);
    if (dialog.exec()) {
        file_path = dialog.filePath();
        qrc_path = dialog.qrcPath();
        if (!file_path.isEmpty()) {
            populateCombo();
            if (m_mode == Icon) {
                const QIcon icon = m_core->iconCache()->nameToIcon(file_path, qrc_path);
                populateCombo();
                setIcon(icon);
            } else {
                const QPixmap pixmap = m_core->iconCache()->nameToPixmap(file_path, qrc_path);
                populateCombo();
                setPixmap(pixmap);
            }
        }
    }
}

void GraphicsPropertyEditor::setIcon(const QIcon &pm)
{
    if (m_mode == Pixmap)
        return;

    if (pm.isNull() && m_icon.isNull())
        return;
    if (pm.serialNumber() == m_icon.serialNumber())
        return;

    m_icon = pm;

    const bool blocked = m_combo->blockSignals(true);
    m_combo->setCurrentIndex(indexOfIcon(m_icon));
    m_combo->blockSignals(blocked);

    emit iconChanged(m_icon);
}

void GraphicsPropertyEditor::setPixmap(const QPixmap &pm)
{
    if (m_mode == Icon)
        return;

    if (pm.isNull() && m_pixmap.isNull())
        return;
    if (pm.serialNumber() == m_pixmap.serialNumber())
        return;

    m_pixmap = pm;

    const bool blocked = m_combo->blockSignals(true);
    m_combo->setCurrentIndex(indexOfPixmap(m_pixmap));
    m_combo->blockSignals(blocked);

    emit pixmapChanged(m_pixmap);
}

}
