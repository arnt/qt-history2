#include "qteditorfactory.h"
#include "qtcursordatabase_p.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QScrollBar>
#include <QComboBox>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QMenu>
#include <QMap>
#include <QKeyEvent>

// QtSpinBoxFactory

class QtSpinBoxFactoryPrivate
{
    QtSpinBoxFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtSpinBoxFactory)
public:
    QMap<QtProperty *, QList<QSpinBox *> > m_createdEditors;
    QMap<QSpinBox *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSetValue(int value);
    void slotEditorDestroyed(QObject *object);
};

void QtSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QSpinBox *> editors = m_createdEditors[property];
    QListIterator<QSpinBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QSpinBox *editor = itEditor.next();
        if (editor->value() != value) {
            editor->blockSignals(true);
            editor->setValue(value);
            editor->blockSignals(false);
        }
    }
}

void QtSpinBoxFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    if (!m_createdEditors.contains(property))
        return;

    QtIntPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QSpinBox *> editors = m_createdEditors[property];
    QListIterator<QSpinBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QSpinBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtSpinBoxFactoryPrivate::slotSetValue(int value)
{
    QObject *object = q_ptr->sender();
    QMap<QSpinBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtIntPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtSpinBoxFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QSpinBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QSpinBox *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtSpinBoxFactory

    \brief The QtSpinBoxFactory class provides QSpinBox widgets for
    properties created by QtIntPropertyManager objects.

    \sa QtAbstractEditorFactory, QtIntPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtSpinBoxFactory::QtSpinBoxFactory(QObject *parent)
    : QtAbstractEditorFactory<QtIntPropertyManager>(parent)
{
    d_ptr = new QtSpinBoxFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtSpinBoxFactory::~QtSpinBoxFactory()
{
    QMap<QSpinBox *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QSpinBox *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSpinBoxFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    connect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtSpinBoxFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(valueChanged(int)), this, SLOT(slotSetValue(int)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSpinBoxFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    disconnect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
}

// QtSliderFactory

class QtSliderFactoryPrivate
{
    QtSliderFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtSliderFactory)
public:
    QMap<QtProperty *, QList<QSlider *> > m_createdEditors;
    QMap<QSlider *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSetValue(int value);
    void slotEditorDestroyed(QObject *object);
};

void QtSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QSlider *> editors = m_createdEditors[property];
    QListIterator<QSlider *> itEditor(editors);
    while (itEditor.hasNext()) {
        QSlider *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setValue(value);
        editor->blockSignals(false);
    }
}

void QtSliderFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    if (!m_createdEditors.contains(property))
        return;

    QtIntPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QSlider *> editors = m_createdEditors[property];
    QListIterator<QSlider *> itEditor(editors);
    while (itEditor.hasNext()) {
        QSlider *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtSliderFactoryPrivate::slotSetValue(int value)
{
    QObject *object = q_ptr->sender();
    QMap<QSlider *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtIntPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtSliderFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QSlider *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QSlider *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtSliderFactory

    \brief The QtSliderFactory class provides QSlider widgets for
    properties created by QtIntPropertyManager objects.

    \sa QtAbstractEditorFactory, QtIntPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtSliderFactory::QtSliderFactory(QObject *parent)
    : QtAbstractEditorFactory<QtIntPropertyManager>(parent)
{
    d_ptr = new QtSliderFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtSliderFactory::~QtSliderFactory()
{
    QMap<QSlider *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QSlider *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSliderFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    connect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtSliderFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QSlider *editor = new QSlider(Qt::Horizontal, parent);
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(valueChanged(int)), this, SLOT(slotSetValue(int)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtSliderFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    disconnect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
}

// QtSliderFactory

class QtScrollBarFactoryPrivate
{
    QtScrollBarFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtScrollBarFactory)
public:
    QMap<QtProperty *, QList<QScrollBar *> > m_createdEditors;
    QMap<QScrollBar *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSetValue(int value);
    void slotEditorDestroyed(QObject *object);
};

void QtScrollBarFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QScrollBar *> editors = m_createdEditors[property];
    QListIterator<QScrollBar *> itEditor(editors);
    while (itEditor.hasNext()) {
        QScrollBar *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setValue(value);
        editor->blockSignals(false);
    }
}

void QtScrollBarFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    if (!m_createdEditors.contains(property))
        return;

    QtIntPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QScrollBar *> editors = m_createdEditors[property];
    QListIterator<QScrollBar *> itEditor(editors);
    while (itEditor.hasNext()) {
        QScrollBar *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtScrollBarFactoryPrivate::slotSetValue(int value)
{
    QObject *object = q_ptr->sender();
    QMap<QScrollBar *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtIntPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtScrollBarFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QScrollBar *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QScrollBar *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtScrollBarFactory

    \brief The QtScrollBarFactory class provides QScrollBar widgets for
    properties created by QtIntPropertyManager objects.

    \sa QtAbstractEditorFactory, QtIntPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtScrollBarFactory::QtScrollBarFactory(QObject *parent)
    : QtAbstractEditorFactory<QtIntPropertyManager>(parent)
{
    d_ptr = new QtScrollBarFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtScrollBarFactory::~QtScrollBarFactory()
{
    QMap<QScrollBar *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QScrollBar *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtScrollBarFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    connect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtScrollBarFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QScrollBar *editor = new QScrollBar(Qt::Horizontal, parent);
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(valueChanged(int)), this, SLOT(slotSetValue(int)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtScrollBarFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    disconnect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
}

// QtCheckBoxFactory

class QtCheckBoxFactoryPrivate
{
    QtCheckBoxFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtCheckBoxFactory)
public:
    QMap<QtProperty *, QList<QCheckBox *> > m_createdEditors;
    QMap<QCheckBox *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, bool value);
    void slotSetValue(bool value);
    void slotEditorDestroyed(QObject *object);
};

void QtCheckBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, bool value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QCheckBox *> editors = m_createdEditors[property];
    QListIterator<QCheckBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QCheckBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setChecked(value);
        editor->setText(property->valueText());
        editor->blockSignals(false);
    }
}

void QtCheckBoxFactoryPrivate::slotSetValue(bool value)
{
    QObject *object = q_ptr->sender();
    QMap<QCheckBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtBoolPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtCheckBoxFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QCheckBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QCheckBox *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtCheckBoxFactory

    \brief The QtCheckBoxFactory class provides QCheckBox widgets for
    properties created by QtBoolPropertyManager objects.

    \sa QtAbstractEditorFactory, QtBoolPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtCheckBoxFactory::QtCheckBoxFactory(QObject *parent)
    : QtAbstractEditorFactory<QtBoolPropertyManager>(parent)
{
    d_ptr = new QtCheckBoxFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtCheckBoxFactory::~QtCheckBoxFactory()
{
    QMap<QCheckBox *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QCheckBox *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCheckBoxFactory::connectPropertyManager(QtBoolPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotPropertyChanged(QtProperty *, bool)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtCheckBoxFactory::createEditor(QtBoolPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QCheckBox *editor = new QCheckBox(parent);
    editor->setChecked(manager->value(property));
    editor->setText(property->valueText());
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(toggled(bool)), this, SLOT(slotSetValue(bool)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCheckBoxFactory::disconnectPropertyManager(QtBoolPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotPropertyChanged(QtProperty *, bool)));
}

// QtDoubleSpinBoxFactory

class QtDoubleSpinBoxFactoryPrivate
{
    QtDoubleSpinBoxFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtDoubleSpinBoxFactory)
public:
    QMap<QtProperty *, QList<QDoubleSpinBox *> > m_createdEditors;
    QMap<QDoubleSpinBox *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, double value);
    void slotRangeChanged(QtProperty *property, double min, double max);
    void slotDecimalsChanged(QtProperty *property, int prec);
    void slotSetValue(double value);
    void slotEditorDestroyed(QObject *object);
};

void QtDoubleSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QDoubleSpinBox *> editors = m_createdEditors[property];
    QListIterator<QDoubleSpinBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QDoubleSpinBox *editor = itEditor.next();
        if (editor->value() != value) {
            editor->blockSignals(true);
            editor->setValue(value);
            editor->blockSignals(false);
        }
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotRangeChanged(QtProperty *property,
            double min, double max)
{
    if (!m_createdEditors.contains(property))
        return;

    QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QDoubleSpinBox *> editors = m_createdEditors[property];
    QListIterator<QDoubleSpinBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QDoubleSpinBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setRange(min, max);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotDecimalsChanged(QtProperty *property, int prec)
{
    if (!m_createdEditors.contains(property))
        return;

    QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QDoubleSpinBox *> editors = m_createdEditors[property];
    QListIterator<QDoubleSpinBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QDoubleSpinBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setDecimals(prec);
        editor->setValue(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotSetValue(double value)
{
    QObject *object = q_ptr->sender();
    QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtDoubleSpinBoxFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QDoubleSpinBox *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*! \class QtDoubleSpinBoxFactory

    \brief The QtDoubleSpinBoxFactory class provides QDoubleSpinBox
    widgets for properties created by QtDoublePropertyManager objects.

    \sa QtAbstractEditorFactory, QtDoublePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtDoubleSpinBoxFactory::QtDoubleSpinBoxFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDoublePropertyManager>(parent)
{
    d_ptr = new QtDoubleSpinBoxFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtDoubleSpinBoxFactory::~QtDoubleSpinBoxFactory()
{
    QMap<QDoubleSpinBox *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDoubleSpinBoxFactory::connectPropertyManager(QtDoublePropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotPropertyChanged(QtProperty *, double)));
    connect(manager, SIGNAL(rangeChanged(QtProperty *, double, double)),
                this, SLOT(slotRangeChanged(QtProperty *, double, double)));
    connect(manager, SIGNAL(decimalsChanged(QtProperty *, int)),
                this, SLOT(slotDecimalsChanged(QtProperty *, int)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtDoubleSpinBoxFactory::createEditor(QtDoublePropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setDecimals(manager->decimals(property));
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(valueChanged(double)), this, SLOT(slotSetValue(double)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDoubleSpinBoxFactory::disconnectPropertyManager(QtDoublePropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotPropertyChanged(QtProperty *, double)));
    disconnect(manager, SIGNAL(rangeChanged(QtProperty *, double, double)),
                this, SLOT(slotRangeChanged(QtProperty *, double, double)));
    disconnect(manager, SIGNAL(decimalsChanged(QtProperty *, int)),
                this, SLOT(slotDecimalsChanged(QtProperty *, int)));
}

// QtLineEditFactory

class QtLineEditFactoryPrivate
{
    QtLineEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtLineEditFactory)
public:
    QMap<QtProperty *, QList<QLineEdit *> > m_createdEditors;
    QMap<QLineEdit *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, const QString &value);
    void slotRegExpChanged(QtProperty *property, const QRegExp &regExp);
    void slotSetValue(const QString &value);
    void slotEditorDestroyed(QObject *object);
};

void QtLineEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
                const QString &value)
{
    if (!m_createdEditors.contains(property))
        return;

    QList<QLineEdit *> editors = m_createdEditors[property];
    QListIterator<QLineEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QLineEdit *editor = itEditor.next();
        //editor->blockSignals(true);
        if (editor->text() != value)
            editor->setText(value);
        //editor->blockSignals(false);
    }
}

void QtLineEditFactoryPrivate::slotRegExpChanged(QtProperty *property,
            const QRegExp &regExp)
{
    if (!m_createdEditors.contains(property))
        return;

    QtStringPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QLineEdit *> editors = m_createdEditors[property];
    QListIterator<QLineEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QLineEdit *editor = itEditor.next();
        editor->blockSignals(true);
        const QValidator *oldValidator = editor->validator();
        QValidator *newValidator = 0;
        if (regExp.isValid()) {
            newValidator = new QRegExpValidator(regExp, editor);
        }
        editor->setValidator(newValidator);
        if (oldValidator)
            delete oldValidator;
        editor->blockSignals(false);
    }
}

void QtLineEditFactoryPrivate::slotSetValue(const QString &value)
{
    QObject *object = q_ptr->sender();
    QMap<QLineEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtStringPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtLineEditFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QLineEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QLineEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtLineEditFactory

    \brief The QtLineEditFactory class provides QLineEdit widgets for
    properties created by QtStringPropertyManager objects.

    \sa QtAbstractEditorFactory, QtStringPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtLineEditFactory::QtLineEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtStringPropertyManager>(parent)
{
    d_ptr = new QtLineEditFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtLineEditFactory::~QtLineEditFactory()
{
    QMap<QLineEdit *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QLineEdit *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtLineEditFactory::connectPropertyManager(QtStringPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QString &)));
    connect(manager, SIGNAL(regExpChanged(QtProperty *, const QRegExp &)),
                this, SLOT(slotRegExpChanged(QtProperty *, const QRegExp &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtLineEditFactory::createEditor(QtStringPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QLineEdit *editor = new QLineEdit(parent);
    QRegExp regExp = manager->regExp(property);
    if (regExp.isValid()) {
        QValidator *validator = new QRegExpValidator(regExp, editor);
        editor->setValidator(validator);
    }
    editor->setText(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(textEdited(const QString &)),
                this, SLOT(slotSetValue(const QString &)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtLineEditFactory::disconnectPropertyManager(QtStringPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QString &)));
    disconnect(manager, SIGNAL(regExpChanged(QtProperty *, const QRegExp &)),
                this, SLOT(slotRegExpChanged(QtProperty *, const QRegExp &)));
}

// QtDateEditFactory

class QtDateEditFactoryPrivate
{
    QtDateEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtDateEditFactory)
public:
    QMap<QtProperty *, QList<QDateEdit *> > m_createdEditors;
    QMap<QDateEdit *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, const QDate &value);
    void slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max);
    void slotSetValue(const QDate &value);
    void slotEditorDestroyed(QObject *object);
};

void QtDateEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QDate &value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QDateEdit *> editors = m_createdEditors[property];
    QListIterator<QDateEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QDateEdit *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setDate(value);
        editor->blockSignals(false);
    }
}

void QtDateEditFactoryPrivate::slotRangeChanged(QtProperty *property,
                const QDate &min, const QDate &max)
{
    if (!m_createdEditors.contains(property))
        return;

    QtDatePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QList<QDateEdit *> editors = m_createdEditors[property];
    QListIterator<QDateEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QDateEdit *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setDateRange(min, max);
        editor->setDate(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtDateEditFactoryPrivate::slotSetValue(const QDate &value)
{
    QObject *object = q_ptr->sender();
    QMap<QDateEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDatePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtDateEditFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QDateEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QDateEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtDateEditFactory

    \brief The QtDateEditFactory class provides QDateEdit widgets for
    properties created by QtDatePropertyManager objects.

    \sa QtAbstractEditorFactory, QtDatePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtDateEditFactory::QtDateEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDatePropertyManager>(parent)
{
    d_ptr = new QtDateEditFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtDateEditFactory::~QtDateEditFactory()
{
    QMap<QDateEdit *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QDateEdit *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateEditFactory::connectPropertyManager(QtDatePropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QDate &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QDate &)));
    connect(manager, SIGNAL(rangeChanged(QtProperty *, const QDate &, const QDate &)),
                this, SLOT(slotRangeChanged(QtProperty *, const QDate &, const QDate &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtDateEditFactory::createEditor(QtDatePropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QDateEdit *editor = new QDateEdit(parent);
    editor->setCalendarPopup(true);
    editor->setDateRange(manager->minimum(property), manager->maximum(property));
    editor->setDate(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(dateChanged(const QDate &)),
                this, SLOT(slotSetValue(const QDate &)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateEditFactory::disconnectPropertyManager(QtDatePropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QDate &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QDate &)));
    disconnect(manager, SIGNAL(rangeChanged(QtProperty *, const QDate &, const QDate &)),
                this, SLOT(slotRangeChanged(QtProperty *, const QDate &, const QDate &)));
}

// QtTimeEditFactory

class QtTimeEditFactoryPrivate
{
    QtTimeEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtTimeEditFactory)
public:
    QMap<QtProperty *, QList<QTimeEdit *> > m_createdEditors;
    QMap<QTimeEdit *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, const QTime &value);
    void slotSetValue(const QTime &value);
    void slotEditorDestroyed(QObject *object);
};

void QtTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QTime &value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QTimeEdit *> editors = m_createdEditors[property];
    QListIterator<QTimeEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QTimeEdit *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setTime(value);
        editor->blockSignals(false);
    }
}

void QtTimeEditFactoryPrivate::slotSetValue(const QTime &value)
{
    QObject *object = q_ptr->sender();
    QMap<QTimeEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtTimePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtTimeEditFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QTimeEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QTimeEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtTimeEditFactory

    \brief The QtTimeEditFactory class provides QTimeEdit widgets for
    properties created by QtTimePropertyManager objects.

    \sa QtAbstractEditorFactory, QtTimePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtTimeEditFactory::QtTimeEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtTimePropertyManager>(parent)
{
    d_ptr = new QtTimeEditFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtTimeEditFactory::~QtTimeEditFactory()
{
    QMap<QTimeEdit *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QTimeEdit *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtTimeEditFactory::connectPropertyManager(QtTimePropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QTime &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QTime &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtTimeEditFactory::createEditor(QtTimePropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QTimeEdit *editor = new QTimeEdit(parent);
    editor->setTime(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(timeChanged(const QTime &)),
                this, SLOT(slotSetValue(const QTime &)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtTimeEditFactory::disconnectPropertyManager(QtTimePropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QTime &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QTime &)));
}

// QtDateTimeEditFactory

class QtDateTimeEditFactoryPrivate
{
    QtDateTimeEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtDateTimeEditFactory)
public:
    QMap<QtProperty *, QList<QDateTimeEdit *> > m_createdEditors;
    QMap<QDateTimeEdit *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, const QDateTime &value);
    void slotSetValue(const QDateTime &value);
    void slotEditorDestroyed(QObject *object);
};

void QtDateTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QDateTime &value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QDateTimeEdit *> editors = m_createdEditors[property];
    QListIterator<QDateTimeEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QDateTimeEdit *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setDateTime(value);
        editor->blockSignals(false);
    }
}

void QtDateTimeEditFactoryPrivate::slotSetValue(const QDateTime &value)
{
    QObject *object = q_ptr->sender();
    QMap<QDateTimeEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDateTimePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtDateTimeEditFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QDateTimeEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QDateTimeEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtDateTimeEditFactory

    \brief The QtDateTimeEditFactory class provides QDateTimeEdit
    widgets for properties created by QtDateTimePropertyManager objects.

    \sa QtAbstractEditorFactory, QtDateTimePropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtDateTimeEditFactory::QtDateTimeEditFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDateTimePropertyManager>(parent)
{
    d_ptr = new QtDateTimeEditFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtDateTimeEditFactory::~QtDateTimeEditFactory()
{
    QMap<QDateTimeEdit *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QDateTimeEdit *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateTimeEditFactory::connectPropertyManager(QtDateTimePropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QDateTime &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QDateTime &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtDateTimeEditFactory::createEditor(QtDateTimePropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QDateTimeEdit *editor = new QDateTimeEdit(parent);
    editor->setDateTime(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(dateTimeChanged(const QDateTime &)),
                this, SLOT(slotSetValue(const QDateTime &)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtDateTimeEditFactory::disconnectPropertyManager(QtDateTimePropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QDateTime &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QDateTime &)));
}

// QtKeqSequenceEdit

class QtKeySequenceEdit : public QWidget
{
    Q_OBJECT
public:
    QtKeySequenceEdit(QWidget *parent = 0);

    QKeySequence keySequence() const;
    bool eventFilter(QObject *o, QEvent *e);
public Q_SLOTS:
    void setKeySequence(const QKeySequence &sequence);
Q_SIGNALS:
    void keySequenceChanged(const QKeySequence &sequence);
protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    bool event(QEvent *e);
private slots:
    void slotClearShortcut();
private:
    void handleKeyEvent(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers state) const;

    int m_num;
    QKeySequence m_keySequence;
    QLineEdit *m_lineEdit;
};

QtKeySequenceEdit::QtKeySequenceEdit(QWidget *parent)
    : QWidget(parent), m_num(0)
{
    m_lineEdit = new QLineEdit(this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->setMargin(0);
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setFocusProxy(this);
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
}

bool QtKeySequenceEdit::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_lineEdit && e->type() == QEvent::ContextMenu) {
        QContextMenuEvent *c = static_cast<QContextMenuEvent *>(e);
        QMenu *menu = m_lineEdit->createStandardContextMenu();
        QList<QAction *> actions = menu->actions();
        QListIterator<QAction *> itAction(actions);
        while (itAction.hasNext()) {
            QAction *action = itAction.next();
            action->setShortcut(QKeySequence());
            QString actionString = action->text();
            int pos = actionString.lastIndexOf("\t");
            if (pos > 0) {
                actionString = actionString.mid(0, pos);
            }
            action->setText(actionString);
        }
        QAction *actionBefore = 0;
        if (actions.count() > 0)
            actionBefore = actions[0];
        QAction *clearAction = new QAction(tr("Clear Shortcut"), menu);
        menu->insertAction(actionBefore, clearAction);
        menu->insertSeparator(actionBefore);
        clearAction->setEnabled(!m_keySequence.isEmpty());
        connect(clearAction, SIGNAL(triggered()), this, SLOT(slotClearShortcut()));
        menu->exec(c->globalPos());
        delete menu;
        e->accept();
        return true;
    }

    return QWidget::eventFilter(o, e);
}

void QtKeySequenceEdit::slotClearShortcut()
{
    setKeySequence(QKeySequence());
}

void QtKeySequenceEdit::handleKeyEvent(QKeyEvent *e)
{
    int nextKey = e->key();
    if (nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift ||
            nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt || nextKey == Qt::Key_Super_L)
        return;

    nextKey |= translateModifiers(e->modifiers());
    int k0 = m_keySequence[0];
    int k1 = m_keySequence[1];
    int k2 = m_keySequence[2];
    int k3 = m_keySequence[3];
    switch (m_num) {
        case 0: k0 = nextKey; k1 = 0; k2 = 0; k3 = 0; break;
        case 1: k1 = nextKey; k2 = 0; k3 = 0; break;
        case 2: k2 = nextKey; k3 = 0; break;
        case 3: k3 = nextKey; break;
        default: break;
    }
    ++m_num;
    if (m_num > 3)
        m_num = 0;
    m_keySequence = QKeySequence(k0, k1, k2, k3);
    m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
    e->accept();
    emit keySequenceChanged(m_keySequence);
}

void QtKeySequenceEdit::setKeySequence(const QKeySequence &sequence)
{
    if (sequence == m_keySequence)
        return;
    m_num = 0;
    m_keySequence = sequence;
    m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
}

QKeySequence QtKeySequenceEdit::keySequence() const
{
    return m_keySequence;
}

int QtKeySequenceEdit::translateModifiers(Qt::KeyboardModifiers state) const
{
    int result = 0;
    if (state & Qt::ShiftModifier)
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

void QtKeySequenceEdit::focusInEvent(QFocusEvent *e)
{
    m_lineEdit->event(e);
    m_lineEdit->selectAll();
    QWidget::focusInEvent(e);
}

void QtKeySequenceEdit::focusOutEvent(QFocusEvent *e)
{
    m_num = 0;
    m_lineEdit->event(e);
    QWidget::focusOutEvent(e);
}

void QtKeySequenceEdit::keyPressEvent(QKeyEvent *e)
{
    handleKeyEvent(e);
    e->accept();
}

void QtKeySequenceEdit::keyReleaseEvent(QKeyEvent *e)
{
    m_lineEdit->event(e);
}

bool QtKeySequenceEdit::event(QEvent *e)
{
    if (e->type() == QEvent::Shortcut ||
            e->type() == QEvent::ShortcutOverride  ||
            e->type() == QEvent::KeyRelease) {
        e->accept();
        return true;
    }
    return QWidget::event(e);
}

// QtKeySequenceEditorFactory

class QtKeySequenceEditorFactoryPrivate
{
    QtKeySequenceEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtKeySequenceEditorFactory)
public:
    QMap<QtProperty *, QList<QtKeySequenceEdit *> > m_createdEditors;
    QMap<QtKeySequenceEdit *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, const QKeySequence &value);
    void slotSetValue(const QKeySequence &value);
    void slotEditorDestroyed(QObject *object);
};

void QtKeySequenceEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QKeySequence &value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QtKeySequenceEdit *> editors = m_createdEditors[property];
    QListIterator<QtKeySequenceEdit *> itEditor(editors);
    while (itEditor.hasNext()) {
        QtKeySequenceEdit *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setKeySequence(value);
        editor->blockSignals(false);
    }
}

void QtKeySequenceEditorFactoryPrivate::slotSetValue(const QKeySequence &value)
{
    QObject *object = q_ptr->sender();
    QMap<QtKeySequenceEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtKeySequencePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtKeySequenceEditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QtKeySequenceEdit *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtKeySequenceEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtKeySequenceEditorFactory

    \brief The QtKeySequenceEditorFactory class provides editor
    widgets for properties created by QtKeySequencePropertyManager objects.

    \sa QtAbstractEditorFactory
*/

/*!
    Creates a factory with the given \a parent.
*/
QtKeySequenceEditorFactory::QtKeySequenceEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtKeySequencePropertyManager>(parent)
{
    d_ptr = new QtKeySequenceEditorFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtKeySequenceEditorFactory::~QtKeySequenceEditorFactory()
{
    QMap<QtKeySequenceEdit *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QtKeySequenceEdit *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtKeySequenceEditorFactory::connectPropertyManager(QtKeySequencePropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QKeySequence &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QKeySequence &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtKeySequenceEditorFactory::createEditor(QtKeySequencePropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtKeySequenceEdit *editor = new QtKeySequenceEdit(parent);
    editor->setKeySequence(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(keySequenceChanged(const QKeySequence &)),
                this, SLOT(slotSetValue(const QKeySequence &)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtKeySequenceEditorFactory::disconnectPropertyManager(QtKeySequencePropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QKeySequence &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QKeySequence &)));
}

// QtEnumEditorFactory

class QtEnumEditorFactoryPrivate
{
    QtEnumEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtEnumEditorFactory)
public:
    QMap<QtProperty *, QList<QComboBox *> > m_createdEditors;
    QMap<QComboBox *, QtProperty *> m_editorToProperty;

    void slotPropertyChanged(QtProperty *property, int value);
    void slotEnumNamesChanged(QtProperty *property, const QStringList &);
    void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &);
    void slotSetValue(int value);
    void slotEditorDestroyed(QObject *object);
};

void QtEnumEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;
    QList<QComboBox *> editors = m_createdEditors[property];
    QListIterator<QComboBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QComboBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setCurrentIndex(value);
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotEnumNamesChanged(QtProperty *property,
                const QStringList &enumNames)
{
    if (!m_createdEditors.contains(property))
        return;

    QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QMap<int, QIcon> enumIcons = manager->enumIcons(property);

    QList<QComboBox *> editors = m_createdEditors[property];
    QListIterator<QComboBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QComboBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->clear();
        editor->addItems(enumNames);
        for (int i = 0; i < enumNames.count(); i++)
            editor->setItemIcon(i, enumIcons.value(i));
        editor->setCurrentIndex(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotEnumIconsChanged(QtProperty *property,
                const QMap<int, QIcon> &enumIcons)
{
    if (!m_createdEditors.contains(property))
        return;

    QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QStringList enumNames = manager->enumNames(property);

    QList<QComboBox *> editors = m_createdEditors[property];
    QListIterator<QComboBox *> itEditor(editors);
    while (itEditor.hasNext()) {
        QComboBox *editor = itEditor.next();
        editor->blockSignals(true);
        for (int i = 0; i < enumNames.count(); i++)
            editor->setItemIcon(i, enumIcons.value(i));
        editor->setCurrentIndex(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotSetValue(int value)
{
    QObject *object = q_ptr->sender();
    QMap<QComboBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void QtEnumEditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    QMap<QComboBox *, QtProperty *>::ConstIterator itEditor =
                m_editorToProperty.constBegin();
    while (itEditor != m_editorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QComboBox *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_editorToProperty.remove(editor);
            m_createdEditors[property].removeAll(editor);
            if (m_createdEditors[property].isEmpty())
                m_createdEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtEnumEditorFactory

    \brief The QtEnumEditorFactory class provides QComboBox widgets for
    properties created by QtEnumPropertyManager objects.

    \sa QtAbstractEditorFactory, QtEnumPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtEnumEditorFactory::QtEnumEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtEnumPropertyManager>(parent)
{
    d_ptr = new QtEnumEditorFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtEnumEditorFactory::~QtEnumEditorFactory()
{
    QMap<QComboBox *, QtProperty *> editorToProperty = d_ptr->m_editorToProperty;
    QMap<QComboBox *, QtProperty *>::ConstIterator it = editorToProperty.constBegin();
    while (it != editorToProperty.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtEnumEditorFactory::connectPropertyManager(QtEnumPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    connect(manager, SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
                this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtEnumEditorFactory::createEditor(QtEnumPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QComboBox *editor = new QComboBox(parent);
    editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    editor->view()->setTextElideMode(Qt::ElideRight);
    QStringList enumNames = manager->enumNames(property);
    editor->addItems(enumNames);
    QMap<int, QIcon> enumIcons = manager->enumIcons(property);
    for (int i = 0; i < enumNames.count(); i++)
        editor->setItemIcon(i, enumIcons.value(i));
    editor->setCurrentIndex(manager->value(property));
    d_ptr->m_createdEditors[property].append(editor);
    d_ptr->m_editorToProperty[editor] = property;

    connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtEnumEditorFactory::disconnectPropertyManager(QtEnumPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotPropertyChanged(QtProperty *, int)));
    disconnect(manager, SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
                this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
}

// QtCursorEditorFactory

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

class QtCursorEditorFactoryPrivate
{
    QtCursorEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtCursorEditorFactory)
public:
    QtCursorEditorFactoryPrivate();

    void slotPropertyChanged(QtProperty *property, const QCursor &cursor);
    void slotEnumChanged(QtProperty *property, int value);
    void slotEditorDestroyed(QObject *object);

    QtEnumEditorFactory *m_enumEditorFactory;
    QtEnumPropertyManager *m_enumPropertyManager;

    QMap<QtProperty *, QtProperty *> m_propertyToEnum;
    QMap<QtProperty *, QtProperty *> m_enumToProperty;
    QMap<QtProperty *, QList<QWidget *> > m_enumToEditors;
    QMap<QWidget *, QtProperty *> m_editorToEnum;
    bool m_updatingEnum;
};

QtCursorEditorFactoryPrivate::QtCursorEditorFactoryPrivate()
    : m_updatingEnum(false)
{

}

void QtCursorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, const QCursor &cursor)
{
    // update enum property
    QtProperty *enumProp = m_propertyToEnum.value(property);
    if (!enumProp)
        return;

    m_updatingEnum = true;
    m_enumPropertyManager->setValue(enumProp, cursorDatabase()->cursorToValue(cursor));
    m_updatingEnum = false;
}

void QtCursorEditorFactoryPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (m_updatingEnum)
        return;
    // update cursor property
    QtProperty *prop = m_enumToProperty.value(property);
    if (!prop)
        return;
    QtCursorPropertyManager *cursorManager = q_ptr->propertyManager(prop);
    if (!cursorManager)
        return;
    cursorManager->setValue(prop, QCursor(cursorDatabase()->valueToCursor(value)));
}

void QtCursorEditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
    // remove from m_editorToEnum map;
    // remove from m_enumToEditors map;
    // if m_enumToEditors doesn't contains more editors delete enum property;
    QMap<QWidget *, QtProperty *>::ConstIterator itEditor =
                m_editorToEnum.constBegin();
    while (itEditor != m_editorToEnum.constEnd()) {
        if (itEditor.key() == object) {
            QWidget *editor = itEditor.key();
            QtProperty *enumProp = itEditor.value();
            m_editorToEnum.remove(editor);
            m_enumToEditors[enumProp].removeAll(editor);
            if (m_enumToEditors[enumProp].isEmpty()) {
                m_enumToEditors.remove(enumProp);
                QtProperty *property = m_enumToProperty.value(enumProp);
                m_enumToProperty.remove(enumProp);
                m_propertyToEnum.remove(property);
                delete enumProp;
            }
            return;
        }
        itEditor++;
    }
}

/*!
    \class QtCursorEditorFactory

    \brief The QtCursorEditorFactory class provides QComboBox widgets for
    properties created by QtCursorPropertyManager objects.

    \sa QtAbstractEditorFactory, QtCursorPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtCursorEditorFactory::QtCursorEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtCursorPropertyManager>(parent)
{
    d_ptr = new QtCursorEditorFactoryPrivate();
    d_ptr->q_ptr = this;

    d_ptr->m_enumEditorFactory = new QtEnumEditorFactory(this);
    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));
    d_ptr->m_enumEditorFactory->addPropertyManager(d_ptr->m_enumPropertyManager);
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtCursorEditorFactory::~QtCursorEditorFactory()
{
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCursorEditorFactory::connectPropertyManager(QtCursorPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QCursor &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QCursor &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtCursorEditorFactory::createEditor(QtCursorPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QtProperty *enumProp = 0;
    if (d_ptr->m_propertyToEnum.contains(property)) {
        enumProp = d_ptr->m_propertyToEnum[property];
    } else {
        enumProp = d_ptr->m_enumPropertyManager->addProperty(property->propertyName());
        d_ptr->m_enumPropertyManager->setEnumNames(enumProp, cursorDatabase()->cursorShapeNames());
        d_ptr->m_enumPropertyManager->setEnumIcons(enumProp, cursorDatabase()->cursorShapeIcons());
        d_ptr->m_enumPropertyManager->setValue(enumProp, cursorDatabase()->cursorToValue(manager->value(property)));
        d_ptr->m_propertyToEnum[property] = enumProp;
        d_ptr->m_enumToProperty[enumProp] = property;
    }
    QtAbstractEditorFactoryBase *af = d_ptr->m_enumEditorFactory;
    QWidget *editor = af->createEditor(enumProp, parent);
    d_ptr->m_enumToEditors[enumProp].append(editor);
    d_ptr->m_editorToEnum[editor] = enumProp;
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCursorEditorFactory::disconnectPropertyManager(QtCursorPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QCursor &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QCursor &)));
}

#include "moc_qteditorfactory.cpp"
#include "qteditorfactory.moc"
