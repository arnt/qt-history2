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
#include "findicondialog.h"
#include "qpropertyeditor_model_p.h"

#include <qextensionmanager.h>
#include <propertysheet.h>
#include <container.h>
#include <abstracticoncache.h>
#include <abstractformwindowmanager.h>

#include <QtGui/QVBoxLayout>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>
#include <QtGui/QHBoxWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QFileDialog>
#include <QtGui/qevent.h>
#include <QtCore/QTimer>
#include <QtCore/qsignal.h>

using namespace QPropertyEditor;

// ### remove me
IProperty *PropertyEditor::createSpecialProperty(const QVariant &value,
        const QString &name)
{
    if (name == QLatin1String("alignment"))
        return new IntProperty(value.toInt(), name);

    return 0;
}

// ---------------------------------------------------------------------------------

class IconProperty : public AbstractProperty<QIcon>
{
public:
    IconProperty(AbstractFormEditor *core, const QIcon &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;
    QVariant decoration() const;

    QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver) const;
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
private:
    AbstractFormEditor *m_core;
};

class IconPropertyEditor : public QWidget
{
    Q_OBJECT
public:
    IconPropertyEditor(AbstractFormEditor *core, const QIcon &pm, QWidget *parent);
    ~IconPropertyEditor();

    void setIcon(const QIcon &pm);
    QIcon icon() const { return m_icon; }

signals:
    void iconChanged(const QIcon &pm);

public slots:
    void showDialog();

private:
    AbstractFormEditor *m_core;
    QLabel *m_label;
    QPushButton *m_button;
    QIcon m_icon;
};

IconPropertyEditor::~IconPropertyEditor()
{
}

IconPropertyEditor::IconPropertyEditor(AbstractFormEditor *core, const QIcon &pm,
                                                QWidget *parent)
    : QWidget(parent)
{
    m_core = core;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(2);

    m_label = new QLabel(tr("<no icon>"), this);
    layout->addWidget(m_label);
    layout->addStretch();
    m_button = new QPushButton(tr("Change..."), this);
    layout->addWidget(m_button);
    connect(m_button, SIGNAL(clicked()), this, SLOT(showDialog()));

    setIcon(pm);
}

void IconPropertyEditor::showDialog()
{
    FindIconDialog dialog(0);
    QString file_path;
    if (m_icon.isNull()) {
        QString ui_name = m_core->formWindowManager()->activeFormWindow()->fileName();
        if (ui_name.isEmpty())
            file_path = QDir::currentPath() + QDir::separator();
        else
            file_path = QFileInfo(ui_name).path();
    } else {
        file_path = m_core->iconCache()->iconToFilePath(m_icon);
    }
    dialog.setPaths(QString(), file_path);
    if (dialog.exec()) {
        QString name = dialog.filePath();
        if (!name.isEmpty())
            setIcon(m_core->iconCache()->nameToIcon(name));
    }
}

void IconPropertyEditor::setIcon(const QIcon &pm)
{
    if (pm.isNull() && m_icon.isNull())
        return;
    if (pm.serialNumber() == m_icon.serialNumber())
        return;

    m_icon = pm;

    if (m_icon.isNull()) {
        m_label->setText(tr("<no icon>"));
    } else {
        QString path = m_core->iconCache()->iconToFilePath(pm);
        m_label->setText(QFileInfo(path).fileName());
    }
    emit iconChanged(m_icon);
}

IconProperty::IconProperty(AbstractFormEditor *core, const QIcon &value, const QString &name)
    : AbstractProperty<QIcon>(value, name)
{
    m_core = core;
}

void IconProperty::setValue(const QVariant &value)
{
    m_value = qvariant_cast<QIcon>(value);
}

QString IconProperty::toString() const
{
    QString path = m_core->iconCache()->iconToFilePath(m_value);
    return QFileInfo(path).fileName();
}

QVariant IconProperty::decoration() const
{
    static QIcon empty_icon;
    if (empty_icon.isNull())
        empty_icon = QIcon(QLatin1String(":/trolltech/formeditor/images/emptyicon.png"));

    if (m_value.isNull())
        return qVariantFromValue(empty_icon);
    return qVariantFromValue(m_value);
}

QWidget *IconProperty::createEditor(QWidget *parent, const QObject *target,
                                        const char *receiver) const
{
    IconPropertyEditor *editor = new IconPropertyEditor(m_core, m_value, parent);

    QObject::connect(editor, SIGNAL(iconChanged(const QIcon&)), target, receiver);

    return editor;
}

void IconProperty::updateEditorContents(QWidget *editor)
{
    if (IconPropertyEditor *ed = qobject_cast<IconPropertyEditor*>(editor)) {
        ed->setIcon(m_value);
    }
}

void IconProperty::updateValue(QWidget *editor)
{
    if (IconPropertyEditor *ed = qobject_cast<IconPropertyEditor*>(editor)) {
        QIcon newValue = ed->icon();

        if (newValue.serialNumber() != m_value.serialNumber()) {
            m_value = newValue;
            setChanged(true);
        }
    }
}

// -------------------------------------------------------------------------------------

void PropertyEditor::createPropertySheet(PropertyCollection *root, QObject *object)
{
    QExtensionManager *m = m_core->extensionManager();
    IPropertySheet *sheet = qobject_cast<IPropertySheet*>(m->extension(object, Q_TYPEID(IPropertySheet)));
    QHash<QString, PropertyCollection*> g;
    for (int i=0; i<sheet->count(); ++i) {
        if (!sheet->isVisible(i))
            continue;

        QString pname = sheet->propertyName(i);
        QVariant value = sheet->property(i);

        IProperty *p = 0;
        if (qVariantCanConvert<FlagType>(value)) {
            FlagType f = qvariant_cast<FlagType>(value);
            p = new FlagsProperty(f.items, f.value.toInt(), pname);
        } else if (qVariantCanConvert<EnumType>(value)) {
            EnumType e = qvariant_cast<EnumType>(value);
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
            case QVariant::Icon:
                p = new IconProperty(m_core, qvariant_cast<QIcon>(value), pname);
                break;
            case QVariant::Font:
                p = new FontProperty(qvariant_cast<QFont>(value), pname);
                break;
            case QVariant::Color:
                p = new ColorProperty(qvariant_cast<QColor>(value), pname);
                break;
            case QVariant::SizePolicy:
                p = new SizePolicyProperty(qvariant_cast<QSizePolicy>(value), pname);
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
                p = new CursorProperty(qvariant_cast<QCursor>(value), pname);
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
                // ### qWarning() << "property" << pname << "with type" << value.type() << "not supported yet!";
                break;
            } // end switch
        }

        if (p) {
            p->setHasReset(sheet->hasReset(i));
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
        createPropertySheet(collection, object);
        m_properties = collection;
    }

    m_editor->setInitialInput(m_properties);
}

#include "propertyeditor.moc"
