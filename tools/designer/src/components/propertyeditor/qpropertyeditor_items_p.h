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

#ifndef QPROPERTYEDITOR_ITEMS_P_H
#define QPROPERTYEDITOR_ITEMS_P_H

#include "propertyeditor_global.h"

#include <QStringList>
#include <QVariant>
#include <QMap>
#include <QDateTime>
#include <QCursor>
#include <QPixmap>
#include <QPalette>
#include <QKeySequence>

class QWidget;
class QComboBox;

namespace I
{
    enum PropertyKind
    {
        Property_Normal,
        Property_Group

        // ### more
    };

    struct Property
    {
        inline Property()
            : m_parent(0), m_changed(0), m_dirty(0), m_fake(0) {}

        virtual ~Property() {}

        // ### pure
        bool changed() const { return m_changed; }
        void setChanged(bool b) { m_changed = b; m_dirty = b; }

        virtual bool dirty() const { return m_dirty; }
        virtual void setDirty(bool b) { m_dirty = b; }

        bool isFake() const { return m_fake; }
        void setFake(bool b) { m_fake = b; }

        virtual PropertyKind kind() const = 0;

        virtual I::Property *parent() const { return m_parent; }
        virtual void setParent(I::Property *parent) { m_parent = parent; }

        virtual QString propertyName() const = 0;

        virtual QVariant value() const = 0;
        virtual void setValue(const QVariant &value) = 0;

        virtual QString toString() const = 0;
        virtual QVariant decoration() const = 0;

        virtual bool hasEditor() const = 0;
        virtual QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver) = 0;

        virtual void updateEditorContents(QWidget *editor) { Q_UNUSED(editor); } // ### pure??
        virtual void updateValue(QWidget *editor) { Q_UNUSED(editor); }

        virtual bool hasExternalEditor() const = 0;
        virtual QWidget *createExternalEditor(QWidget *parent) = 0;

    protected:
        I::Property *m_parent;
        uint m_changed : 1;
        uint m_dirty : 1;
        uint m_fake : 1;
    };

    struct PropertyGroup: public Property
    {
        virtual int indexOf(I::Property *property) const = 0;
        virtual int propertyCount() const = 0;
        virtual Property *propertyAt(int index) const = 0;
    };
}

template <typename T>
class QT_PROPERTYEDITOR_EXPORT AbstractProperty: public I::Property
{
public:
    AbstractProperty(const T &value, const QString &name)
        : m_value(value), m_name(name) {}

    I::PropertyKind kind() const { return I::Property_Normal; }

//
// I::Property Interface
//
    QVariant decoration() const { return QVariant(); }
    QString propertyName() const { return m_name; }
    QVariant value() const { return QVariant(m_value); }

    bool hasEditor() const { return true; }
    bool hasExternalEditor() const { return false; }
    QWidget *createExternalEditor(QWidget *parent) { Q_UNUSED(parent); return 0; }

protected:
    T m_value;
    QString m_name;
};

class QT_PROPERTYEDITOR_EXPORT AbstractPropertyGroup: public I::PropertyGroup
{
public:
    AbstractPropertyGroup(const QString &name)
        : m_name(name) {}

    ~AbstractPropertyGroup()
    { qDeleteAll(m_properties); }

    I::PropertyKind kind() const { return I::Property_Group; }

//
// I::PropertyGroup Interface
//
    int indexOf(I::Property *property) const { return m_properties.indexOf(property); }
    int propertyCount() const { return m_properties.size(); }
    I::Property *propertyAt(int index) const { return m_properties.at(index); }

//
// I::Property Interface
//
    bool dirty() const;
    void setDirty(bool b);

    QString propertyName() const { return m_name; }

    QVariant decoration() const { return QVariant(); }
    QString toString() const
    {
        QString text = QLatin1String("[");
        for (int i=0; i<propertyCount(); ++i) {
            text += propertyAt(i)->toString();
            if (i+1 < propertyCount())
                text += QLatin1String(", ");
        }
        text += QLatin1String("]");
        return text;
    }

    bool hasEditor() const { return false; }
    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver)
    {
        Q_UNUSED(parent);
        Q_UNUSED(target);
        Q_UNUSED(receiver);
        return 0;
    }

    bool hasExternalEditor() const { return false; }
    QWidget *createExternalEditor(QWidget *parent) { Q_UNUSED(parent); return 0; }

protected:
    QString m_name;
    QList<I::Property*> m_properties;
};

class QT_PROPERTYEDITOR_EXPORT PropertyCollection: public I::PropertyGroup
{
public:
    PropertyCollection(const QString &name);
    ~PropertyCollection();

    I::PropertyKind kind() const { return I::Property_Group; }

    void addProperty(I::Property *property);
    void removeProperty(I::Property *property);

//
// I::PropertyGroup Interface
//
    int indexOf(I::Property *property) const;
    int propertyCount() const;
    I::Property *propertyAt(int index) const;

//
// I::Property Interface
//
    QString propertyName() const;

    QVariant value() const;
    void setValue(const QVariant &value);

    QVariant decoration() const { return QVariant(); }
    QString toString() const;

    bool hasEditor() const;
    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);

    bool hasExternalEditor() const;
    QWidget *createExternalEditor(QWidget *parent);

private:
    QString m_name;
    QList<I::Property*> m_properties;
};

class QT_PROPERTYEDITOR_EXPORT IntProperty: public AbstractProperty<int>
{
public:
    IntProperty(int value, const QString &name);

    QString specialValue() const;
    void setSpecialValue(const QString &specialValue);

    void setRange(int low, int hi);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);

private:
    QString m_specialValue;
    int m_low;
    int m_hi;
};

class QT_PROPERTYEDITOR_EXPORT BoolProperty: public AbstractProperty<bool>
{
public:
    BoolProperty(bool value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT StringProperty: public AbstractProperty<QString>
{
public:
    StringProperty(const QString &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT ListProperty: public AbstractProperty<int>
{
public:
    ListProperty(const QStringList &items, int value,
                 const QString &name);

    QStringList items() const;

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);

private:
    QStringList m_items;
};

class QT_PROPERTYEDITOR_EXPORT MapProperty: public AbstractProperty<QVariant>
{
public:
    MapProperty(const QMap<QString, QVariant> &items, const QVariant &value,
                const QString &name);

    QStringList keys() const;
    QMap<QString, QVariant> items() const;
    int indexOf(const QVariant &value) const;

    QVariant value() const;
    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);

private:
    QMap<QString, QVariant> m_items;
    QStringList m_keys;
};

class QT_PROPERTYEDITOR_EXPORT FlagsProperty: public MapProperty
{
public:
    FlagsProperty(const QMap<QString, QVariant> &items, int m_value,
                  const QString &name);

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT PointProperty: public AbstractPropertyGroup
{
public:
    PointProperty(const QPoint &value, const QString &name);

    QVariant value() const;
    void setValue(const QVariant &value);
};

class QT_PROPERTYEDITOR_EXPORT SizeProperty: public AbstractPropertyGroup
{
public:
    SizeProperty(const QSize &value, const QString &name);

    QVariant value() const;
    void setValue(const QVariant &value);
};

class QT_PROPERTYEDITOR_EXPORT RectProperty: public AbstractPropertyGroup
{
public:
    RectProperty(const QRect &value, const QString &name);

    QVariant value() const;
    void setValue(const QVariant &value);
};

class QT_PROPERTYEDITOR_EXPORT ColorProperty: public AbstractPropertyGroup
{
public:
    ColorProperty(const QColor &value, const QString &name);

    QVariant value() const;
    void setValue(const QVariant &value);
    QVariant decoration() const;

    QString toString() const { return QLatin1String("  ") + AbstractPropertyGroup::toString(); } // ### temp hack remove me!!
};

class QT_PROPERTYEDITOR_EXPORT FontProperty: public AbstractPropertyGroup
{
public:
    FontProperty(const QFont &value, const QString &name);

    QString toString() const;
    QVariant value() const;
    void setValue(const QVariant &value);
    QVariant decoration() const;
};

class QT_PROPERTYEDITOR_EXPORT SizePolicyProperty: public AbstractPropertyGroup
{
public:
    SizePolicyProperty(const QSizePolicy &value, const QString &name);

    QString toString() const;
    QVariant value() const;
    void setValue(const QVariant &value);
    QVariant decoration() const;
};

class QT_PROPERTYEDITOR_EXPORT DateTimeProperty: public AbstractProperty<QDateTime>
{
public:
    DateTimeProperty(const QDateTime &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT DateProperty: public AbstractProperty<QDate>
{
public:
    DateProperty(const QDate &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT TimeProperty: public AbstractProperty<QTime>
{
public:
    TimeProperty(const QTime &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT CursorProperty: public AbstractProperty<QCursor>
{
public:
    CursorProperty(const QCursor &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;
    QVariant decoration() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);

private:
    static QString cursorName(int shape);
    static QPixmap cursorPixmap(int shape);
    void addCursor(QComboBox *combo, int shape);
};

class QT_PROPERTYEDITOR_EXPORT KeySequenceProperty: public AbstractProperty<QKeySequence>
{
public:
    KeySequenceProperty(const QKeySequence &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

class QT_PROPERTYEDITOR_EXPORT PaletteProperty: public AbstractProperty<QPalette>
{
public:
    PaletteProperty(const QPalette &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;

    QWidget *createEditor(QWidget *parent, QObject *target, const char *receiver);
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
};

#endif // QPROPERTYEDITOR_ITEMS_P_H
