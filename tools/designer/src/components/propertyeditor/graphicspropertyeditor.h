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

#ifndef GRAPHICSPROPERTYSEDITOR_H
#define GRAPHICSPROPERTYSEDITOR_H

#include <QtGui/QWidget>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

class QComboBox;
class QToolButton;
class QHBoxLayout;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

// This handles editing of pixmap and icon properties
class GraphicsPropertyEditor : public QWidget
{
    Q_OBJECT

public:
    GraphicsPropertyEditor(QDesignerFormEditorInterface *core, const QIcon &pm, QWidget *parent);
    GraphicsPropertyEditor(QDesignerFormEditorInterface *core, const QPixmap &pixmap, QWidget *parent);
    ~GraphicsPropertyEditor();

    void setIcon(const QIcon &pm);
    void setPixmap(const QPixmap &pm);
    QIcon icon() const { return m_mode == Icon ? m_icon : QIcon(); }
    QPixmap pixmap() const { return m_mode == Pixmap ? m_pixmap : QPixmap(); }

    void setSpacing(int spacing);

signals:
    void iconChanged(const QIcon &pm);
    void pixmapChanged(const QPixmap &pm);

private slots:
    void showDialog();
    void comboActivated(int idx);

private:
    void init();
    void populateCombo();
    int indexOfIcon(const QIcon &icon);
    int indexOfPixmap(const QPixmap &pixmap);

    enum Mode { Icon, Pixmap };
    const Mode m_mode;

    QDesignerFormEditorInterface *m_core;
    QComboBox *m_combo;
    QToolButton *m_button;
    QIcon m_icon;
    QPixmap m_pixmap;
    QHBoxLayout *m_layout;
};
}

#endif
