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

#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include "ui_paletteeditor.h"
#include <QItemDelegate>

class QListView;

namespace qdesigner_internal {

class PaletteEditor: public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QPalette editPalette READ editPalette WRITE setEditPalette)
public:
    virtual ~PaletteEditor();

    static QPalette getPalette(QWidget* parent, const QPalette &init = QPalette(), int *result = 0);

    QPalette editPalette() const;
    void setEditPalette(const QPalette&);

private slots:
    void on_buttonMainColor_changed();
    void on_buttonMainColor2_changed();
    void on_paletteCombo_activated(int);
    void colorChanged(QPalette::ColorRole role, const QColor &color);

    void showAdvanced(bool showIt);
protected:
    PaletteEditor(QWidget *parent);

private:
    void buildPalette();

    void buildActiveEffect();
    void updatePaletteEffect(QPalette::ColorGroup g);

    void updatePreviewPalette();
    void updateStyledButtons();

    QPalette::ColorGroup selectedColorGroup() const;

private:
    Ui::PaletteEditor ui;
    QPalette m_editPalette;
    class ColorRoleModel *m_colorRoleModel;
    QSize m_sizeWithoutExtension;
};

class ColorRoleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QPalette::ColorRole colorRole READ colorRole)
public:
    ColorRoleModel(QWidget *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QPalette getPalette() const;
    void setPalette(const QPalette &palette);

    QPalette::ColorRole colorRole() const { return QPalette::NoRole; }

signals:
    void paletteChanged(const QPalette &palette);
    void colorChanged(QPalette::ColorRole role, const QColor &color);
private:

    QPixmap generatePixmap(const QColor &color) const;

    QPalette m_palette;
    QMap<int, QString> m_roleNames;
    QMap<int, QPixmap> m_colorPixmaps;
};

class ColorEditor : public QWidget
{
    Q_OBJECT
public:
    ColorEditor(QWidget *parent = 0);

    void setColor(const QColor &color);
    void setLabel(const QString &text);
    QColor color() const;
signals:
    void changed(QWidget *widget);
private slots:
    void colorChanged();
private:
    StyledButton *button;
    QLabel       *label;
};

class ColorDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ColorDelegate(QObject *parent = 0) : QItemDelegate(parent) { }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
                const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

}  // namespace qdesigner_internal

#endif // PALETTEEDITOR_H
