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

public slots:
    void showDialog();

private:
    void init();

    enum Mode { Icon, Pixmap };
    Mode m_mode;

    AbstractFormEditor *m_core;
    QLabel *m_label;
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
    layout->setMargin(2);

    m_label = new QLabel(tr("<no icon>"), this);
    layout->addWidget(m_label);
    layout->addStretch();
    m_button = new QToolButton(this);
    m_button->setText(tr("Change..."));
    layout->addWidget(m_button);
    connect(m_button, SIGNAL(clicked()), this, SLOT(showDialog()));
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

    if (m_icon.isNull()) {
        m_label->setText(tr("<no icon>"));
    } else {
        QString path = m_core->iconCache()->iconToFilePath(pm);
        m_label->setText(QFileInfo(path).fileName());
    }
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

    if (m_pixmap.isNull()) {
        m_label->setText(tr("<no icon>"));
    } else {
        QString path = m_core->iconCache()->pixmapToFilePath(pm);
        m_label->setText(QFileInfo(path).fileName());
    }
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
