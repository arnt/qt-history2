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
#include <abstractwidgetdatabase.h>
#include <iconloader.h>

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

class PixmapProperty : public AbstractProperty<QPixmap>
{
public:
    PixmapProperty(AbstractFormEditor *core, const QPixmap &pixmap, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;
    QVariant decoration() const;

    QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver) const;
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);
private:
    AbstractFormEditor *m_core;
};

// This handles editing of pixmap and icon properties
class GraphicsPropertyEditor : public QWidget
{
    Q_OBJECT
public:
    GraphicsPropertyEditor(AbstractFormEditor *core, const QIcon &pm, QWidget *parent);
    GraphicsPropertyEditor(AbstractFormEditor *core, const QPixmap &pixmap, QWidget *parent);
    ~GraphicsPropertyEditor();

    void setIcon(const QIcon &pm);
    void setPixmap(const QPixmap &pm);
    QIcon icon() const { return m_mode == Icon ? m_icon : QIcon(); }
    QPixmap pixmap() const { return m_mode == Pixmap ? m_pixmap : QPixmap(); }

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
    Mode m_mode;

    AbstractFormEditor *m_core;
    QComboBox *m_combo;
    QToolButton *m_button;
    QIcon m_icon;
    QPixmap m_pixmap;
};

GraphicsPropertyEditor::~GraphicsPropertyEditor()
{
}

void GraphicsPropertyEditor::init()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_combo = new QComboBox(this);
    m_combo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_combo->setEditable(false);
    layout->addWidget(m_combo);
    m_button = new QToolButton(this);
    m_button->setIcon(createIconSet(QLatin1String("fileopen.png")));
    layout->addWidget(m_button);
    connect(m_button, SIGNAL(clicked()), this, SLOT(showDialog()));
    connect(m_combo, SIGNAL(activated(int)), this, SLOT(comboActivated(int)));

    populateCombo();
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
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    QStringList qrc_list = form->resourceFiles();

    m_combo->clear();

    AbstractIconCache *cache = m_core->iconCache();
    if (m_mode == Icon) {
        m_combo->addItem(tr("<no icon>"));
        QList<QIcon> icon_list = cache->iconList();
        foreach (QIcon icon, icon_list) {
            QString qrc_path = cache->iconToQrcPath(icon);
            if (!qrc_path.isEmpty() && !qrc_list.contains(qrc_path))
                continue;
            m_combo->addItem(icon, QFileInfo(cache->iconToFilePath(icon)).fileName(),
                                QVariant(icon));
        }
    } else {
        m_combo->addItem(tr("<no pixmap>"));
        QList<QPixmap> pixmap_list = cache->pixmapList();
        foreach (QPixmap pixmap, pixmap_list) {
            QString qrc_path = cache->iconToQrcPath(pixmap);
            if (!qrc_path.isEmpty() && !qrc_list.contains(qrc_path))
                continue;
            m_combo->addItem(QIcon(pixmap),
                                QFileInfo(cache->pixmapToFilePath(pixmap)).fileName(),
                                QVariant(pixmap));
        }
    }
    bool blocked = m_combo->blockSignals(true);
    m_combo->setCurrentIndex(0);
    m_combo->blockSignals(blocked);
}

GraphicsPropertyEditor::GraphicsPropertyEditor(AbstractFormEditor *core, const QIcon &pm,
                                                QWidget *parent)
    : QWidget(parent)
{
    m_mode = Icon;
    m_core = core;
    init();
    setIcon(pm);
}

GraphicsPropertyEditor::GraphicsPropertyEditor(AbstractFormEditor *core, const QPixmap &pm,
                                                QWidget *parent)
    : QWidget(parent)
{
    m_mode = Pixmap;
    m_core = core;
    init();
    setPixmap(pm);
}

void GraphicsPropertyEditor::showDialog()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;

    FindIconDialog dialog(form, 0);
    QString file_path;
    QString qrc_path;
    if (m_mode == Icon) {
        if (m_icon.isNull()) {
            file_path = form->absolutePath(QString()) + QDir::separator();
        } else {
            file_path = m_core->iconCache()->iconToFilePath(m_icon);
            qrc_path = m_core->iconCache()->iconToQrcPath(m_icon);
        }
    } else {
        if (m_pixmap.isNull()) {
            file_path = form->absolutePath(QString()) + QDir::separator();
        } else {
            file_path = m_core->iconCache()->pixmapToFilePath(m_pixmap);
            qrc_path = m_core->iconCache()->pixmapToQrcPath(m_pixmap);
        }
    }

    dialog.setPaths(qrc_path, file_path);
    if (dialog.exec()) {
        file_path = dialog.filePath();
        qrc_path = dialog.qrcPath();
        if (!file_path.isEmpty()) {
            if (m_mode == Icon)
                setIcon(m_core->iconCache()->nameToIcon(file_path, qrc_path));
            else
                setPixmap(m_core->iconCache()->nameToPixmap(file_path, qrc_path));
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

    bool blocked = m_combo->blockSignals(true);
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

    bool blocked = m_combo->blockSignals(true);
    m_combo->setCurrentIndex(indexOfPixmap(m_pixmap));
    m_combo->blockSignals(blocked);

    emit pixmapChanged(m_pixmap);
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
    GraphicsPropertyEditor *editor = new GraphicsPropertyEditor(m_core, m_value, parent);

    QObject::connect(editor, SIGNAL(iconChanged(const QIcon&)), target, receiver);

    return editor;
}

void IconProperty::updateEditorContents(QWidget *editor)
{
    if (GraphicsPropertyEditor *ed = qobject_cast<GraphicsPropertyEditor*>(editor)) {
        ed->setIcon(m_value);
    }
}

void IconProperty::updateValue(QWidget *editor)
{
    if (GraphicsPropertyEditor *ed = qobject_cast<GraphicsPropertyEditor*>(editor)) {
        QIcon newValue = ed->icon();

        if (newValue.serialNumber() != m_value.serialNumber()) {
            m_value = newValue;
            setChanged(true);
        }
    }
}

PixmapProperty::PixmapProperty(AbstractFormEditor *core, const QPixmap &pixmap, const QString &name)
    : AbstractProperty<QPixmap>(pixmap, name)
{
    m_core = core;
}

void PixmapProperty::setValue(const QVariant &value)
{
    m_value = qvariant_cast<QPixmap>(value);
}

QString PixmapProperty::toString() const
{
    QString path = m_core->iconCache()->pixmapToFilePath(m_value);
    return QFileInfo(path).fileName();
}

QVariant PixmapProperty::decoration() const
{
    static QIcon empty_icon;
    if (empty_icon.isNull())
        empty_icon = QIcon(QLatin1String(":/trolltech/formeditor/images/emptyicon.png"));

    if (m_value.isNull())
        return qVariantFromValue(empty_icon);
    return qVariantFromValue(QIcon(m_value));
}

QWidget *PixmapProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    GraphicsPropertyEditor *editor = new GraphicsPropertyEditor(m_core, m_value, parent);

    QObject::connect(editor, SIGNAL(pixmapChanged(const QPixmap&)), target, receiver);

    return editor;
}

void PixmapProperty::updateEditorContents(QWidget *editor)
{
    if (GraphicsPropertyEditor *ed = qobject_cast<GraphicsPropertyEditor*>(editor)) {
        ed->setPixmap(m_value);
    }
}

void PixmapProperty::updateValue(QWidget *editor)
{
    if (GraphicsPropertyEditor *ed = qobject_cast<GraphicsPropertyEditor*>(editor)) {
        QPixmap newValue = ed->pixmap();

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
    m_prop_sheet = qobject_cast<IPropertySheet*>(m->extension(object, Q_TYPEID(IPropertySheet)));
    QHash<QString, PropertyCollection*> g;
    for (int i=0; i<m_prop_sheet->count(); ++i) {
        if (!m_prop_sheet->isVisible(i))
            continue;

        QString pname = m_prop_sheet->propertyName(i);
        QVariant value = m_prop_sheet->property(i);

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
            case QVariant::Pixmap:
                p = new PixmapProperty(m_core, qvariant_cast<QPixmap>(value), pname);
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
            p->setHasReset(m_prop_sheet->hasReset(i));
            p->setBold(m_prop_sheet->isChanged(i));
            QString group = m_prop_sheet->propertyGroup(i);
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
    m_prop_sheet = 0;

    connect(m_editor, SIGNAL(propertyChanged(IProperty*)),
        this, SLOT(firePropertyChanged(IProperty*)));
    connect(m_editor->editorModel(), SIGNAL(resetProperty(const QString&)),
                this, SLOT(resetProperty(const QString&)));
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
    if (p->propertyName() == name)
        return p;

    if (p->kind() == IProperty::Property_Group) {
        IPropertyGroup *g = static_cast<IPropertyGroup*>(p);
        for (int i=0; i<g->propertyCount(); ++i)
            if (IProperty *c = propertyByName(g->propertyAt(i), name))
                return c;
    }
                
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
    m_prop_sheet = 0;

    if (m_object) {
        PropertyCollection *collection = new PropertyCollection("<root>");
        createPropertySheet(collection, object);
        m_properties = collection;
    }

    m_editor->setInitialInput(m_properties);
}

void PropertyEditor::resetProperty(const QString &prop_name)
{
    int idx = m_prop_sheet->indexOf(prop_name);
    
    if (idx == -1) {
        qWarning("PropertyEditor::resetProperty(): no property \"%s\"",
                    prop_name.toLatin1().constData());
        return;
    }

    if (!m_prop_sheet->reset(idx)) {
        int item_idx =  m_core->widgetDataBase()->indexOfObject(m_object);
        if (item_idx == -1) {
            qWarning("PropertyEditor::resetProperty(): object \"%s\" not in widget data base",
                        m_object->metaObject()->className());
            return;
        }
        AbstractWidgetDataBaseItem *item = m_core->widgetDataBase()->item(item_idx);
        QVariant value = item->defaultPropertyValues().at(idx);
        m_prop_sheet->setProperty(idx, value);
    }
    
    m_prop_sheet->setChanged(idx, false);
    if (IProperty *p = propertyByName(m_editor->initialInput(), prop_name)) {
        p->setValue(m_prop_sheet->property(idx));
        p->setBold(false);
        m_editor->editorModel()->refresh(p);

        if (p->kind() == IProperty::Property_Group) {
            IPropertyGroup *group = static_cast<IPropertyGroup*>(p);
            for (int i = 0; i < group->propertyCount(); ++i) {
                IProperty *child_prop = group->propertyAt(i);
                child_prop->setBold(false);
                m_editor->editorModel()->refresh(child_prop);
            }
        }
    }
}


#include "propertyeditor.moc"
