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

#include "propertyeditor.h"
#include "qpropertyeditor_model_p.h"

#include <qextensionmanager.h>
#include <propertysheet.h>
#include <container.h>

#include <QVBoxLayout>
#include <QMetaObject>
#include <QMetaProperty>
#include <qdebug.h>

using namespace QPropertyEditor;

// ### remove me
IProperty *PropertyEditor::createSpecialProperty(const QVariant &value,
        const QString &name)
{
    if (name == QLatin1String("alignment"))
        return new IntProperty(value.toInt(), name);

    return 0;
}

void PropertyEditor::createPropertySheet(PropertyCollection *root,
        QExtensionManager *m,
        QObject *object)
{
    IPropertySheet *sheet = qt_cast<IPropertySheet*>(m->extension(object, Q_TYPEID(IPropertySheet)));
    QHash<QString, PropertyCollection*> g;
    for (int i=0; i<sheet->count(); ++i) {
        if (!sheet->isVisible(i))
            continue;

        QString pname = sheet->propertyName(i);
        QVariant value = sheet->property(i);

        IProperty *p = 0;
        EnumType e;
        FlagType f;
        if (qVariantGet(value, f, "FlagType")) {
            p = new FlagsProperty(f.items, f.value.toInt(), pname);
        } else if (qVariantGet(value, e, "EnumType")) {
            p = new MapProperty(e.items, e.value, pname);
        }

        if (!p) {
            switch (value.type()) {
            case 0:
                p = createSpecialProperty(value, pname);
                break;
            case QVariant::Int:
                p = new IntProperty(value.toInt(), pname);
                break;
            case QVariant::UInt:
                p = new IntProperty(value.toUInt(), pname);
                break;
            case QVariant::Bool:
                p = new BoolProperty(value.toBool(), pname);
                break;
            case QVariant::String:
                p = new StringProperty(value.toString(), pname);
                break;
            case QVariant::Size:
                p = new SizeProperty(value.toSize(), pname);
                break;
            case QVariant::Point:
                p = new PointProperty(value.toPoint(), pname);
                break;
            case QVariant::Rect:
                p = new RectProperty(value.toRect(), pname);
                break;
            case QVariant::Font:
                p = new FontProperty(value.toFont(), pname);
                break;
            case QVariant::Color:
                p = new ColorProperty(value.toColor(), pname);
                break;
            case QVariant::SizePolicy:
                p = new SizePolicyProperty(value.toSizePolicy(), pname);
                break;
            case QVariant::DateTime:
                p = new DateTimeProperty(value.toDateTime(), pname);
                break;
            case QVariant::Date:
                p = new DateProperty(value.toDate(), pname);
                break;
            case QVariant::Time:
                p = new TimeProperty(value.toTime(), pname);
                break;
            case QVariant::Cursor:
                p = new CursorProperty(value.toCursor(), pname);
                break;
#if 0 // ### disabled for now
            case QVariant::KeySequence:
                p = new KeySequenceProperty(value.toKeySequence(), pname);
                break;
            case QVariant::Palette:
                p = new PaletteProperty(value.toPalette(), pname);
                break;
#endif
            default:
                qWarning("property '%s' with type '%d' not supported yet!", pname.latin1(), value.type());
                break;
            } // end switch
        }

        if (p) {
            QString group = sheet->propertyGroup(i);
            PropertyCollection *c = group.isEmpty() ? root : g.value(group);
            if (!c) {
                c = new PropertyCollection(group);
                g.insert(group, c);
            }

            c->addProperty(p);
        }
    }

    QHashIterator<QString, PropertyCollection*> it(g);
    while (it.hasNext()) {
        root->addProperty(it.next().value());
    }
}

PropertyEditor::PropertyEditor(AbstractFormEditor *core,
            QWidget *parent, Qt::WFlags flags)
    : AbstractPropertyEditor(parent, flags),
      m_core(core),
      m_properties(0)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin(0);
    m_editor = new QPropertyEditor::View(this);
    lay->addWidget(m_editor);

    connect(m_editor, SIGNAL(propertyChanged(IProperty*)),
        this, SLOT(firePropertyChanged(IProperty*)));
}

PropertyEditor::~PropertyEditor()
{
}

bool PropertyEditor::isReadOnly() const
{
    return m_editor->isReadOnly();
}

void PropertyEditor::setReadOnly(bool readOnly)
{
    m_editor->setReadOnly(readOnly);
}

AbstractFormEditor *PropertyEditor::core() const
{
    return m_core;
}

IProperty *PropertyEditor::propertyByName(IProperty *p, const QString &name)
{
    if (p->kind() == IProperty::Property_Normal)
        return p->propertyName() == name ? p : 0;

    IPropertyGroup *g = static_cast<IPropertyGroup*>(p);
    for (int i=0; i<g->propertyCount(); ++i)
        if (IProperty *c = propertyByName(g->propertyAt(i), name))
            return c;

    return 0;
}

void PropertyEditor::setPropertyValue(const QString &name, const QVariant &value)
{
    if (isReadOnly())
        return;

    if (IProperty *p = propertyByName(m_editor->initialInput(), name)) {
        p->setValue(value);
        m_editor->editorModel()->refresh(p);
    }
}

void PropertyEditor::firePropertyChanged(IProperty *p)
{
    if (isReadOnly())
        return;

    emit propertyChanged(p->propertyName(), p->value());
}

void PropertyEditor::clearDirty(IProperty *p)
{
    p->setDirty(false);

    if (p->kind() == IProperty::Property_Normal)
        return;

    IPropertyGroup *g = static_cast<IPropertyGroup*>(p);
    for (int i=0; i<g->propertyCount(); ++i)
        clearDirty(g->propertyAt(i));
}

void PropertyEditor::setObject(QObject *object)
{
    if (m_editor->initialInput())
        clearDirty(m_editor->initialInput());

    m_object = object;

    delete m_properties;
    m_properties = 0;

    if (m_object) {
        PropertyCollection *collection = new PropertyCollection("<root>");
        createPropertySheet(collection, core()->extensionManager(), object);
        m_properties = collection;
    }

    m_editor->setInitialInput(m_properties);
}
