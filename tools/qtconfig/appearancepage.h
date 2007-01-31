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

#ifndef APPEARANCEPAGE_H
#define APPEARANCEPAGE_H

#include <QFrame>
#include <QAbstractTableModel>
#include <QPalette>
#include <QObject>
#include <QModelIndex>
#include <QVariant>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QColor>
#include <QLabel>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>
#include <QPainter>
#include <QSize>
#include <QUndoStack>
#include "ui_appearancepage.h"


class PaletteModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QPalette::ColorRole colorRole READ colorRole)
public:
    PaletteModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QPalette getPalette() const;
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

    QPalette::ColorRole colorRole() const { return QPalette::NoRole; }
    void setCompute(bool on) { m_compute = on; }
    QPalette::ColorGroup columnToGroup(int index) const;
    int groupToColumn(QPalette::ColorGroup group) const;
signals:
    void paletteChanged(const QPalette &palette);
private:
    QPalette m_palette;
    QPalette m_parentPalette;
    QMap<QPalette::ColorRole, QString> m_roleNames;
    bool m_compute;
};

class RoleEditor : public QWidget
{
    Q_OBJECT
public:
    RoleEditor(QWidget *parent = 0);

    void setLabel(const QString &label);
    void setEdited(bool on);
    bool edited() const;
signals:
    void changed(QWidget *widget);
private slots:
    void emitResetProperty();
private:
    QLabel *m_label;
    bool m_edited;
};

class ColorDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ColorDelegate(QObject *parent = 0) : QItemDelegate(parent) { }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *ed, const QModelIndex &index) const;
    void setModelData(QWidget *ed, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *ed,
                              const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};


class AppearancePage : public QFrame, public Ui_AppearancePage
{
    Q_OBJECT
public:
    AppearancePage(QWidget *parent = 0);
    void load();
    void save();
    QPalette palette() const;
    void setPalette(const QPalette &palette);
    void setPalette(const QPalette &palette, const QPalette &parentPalette);
    void setUndoStack(QUndoStack *stack);
public slots:
    void onStyleComboChanged(const QString &stylename);
    void onColorGroupActivated(const QString &colorGroup);
    void onDetailsToggled(bool showDetails);
    void paletteChanged(const QPalette &palette);
    void buildPalette();
    void onDoubleClicked(const QModelIndex &index);
signals:
    void changed();
private:

    void updatePreviewPalette();
    void updateStyledButton();

    QPalette::ColorGroup currentColorGroup() const
    { return m_currentColorGroup; }

    QPalette m_editPalette;
    QPalette m_parentPalette;
    QPalette::ColorGroup m_currentColorGroup;
    PaletteModel *m_paletteModel;
    bool m_modelUpdated;
    bool m_paletteUpdated;
    bool m_compute;
    QUndoStack *stack;
};


#endif // PALETTEEDITOR_H
