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

#include "qteditorfactory.h"
#include "qtcursordatabase_p.h"
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QScrollBar>
#include <QtGui/QComboBox>
#include <QtGui/QAbstractItemView>
#include <QtGui/QLineEdit>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtCore/QMap>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

// ---------- EditorFactoryPrivate :
// Base class for editor factory private classes. Manages mapping of properties to editors and vice versa.

template <class Editor>
class EditorFactoryPrivate
{
public:

    typedef QList<Editor *> EditorList;
    typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
    typedef QMap<Editor *, QtProperty *> EditorToPropertyMap;

    Editor *createEditor(QtProperty *property, QWidget *parent);
    void initializeEditor(QtProperty *property, Editor *e);
    void slotEditorDestroyed(QObject *object);

    PropertyToEditorListMap  m_createdEditors;
    EditorToPropertyMap m_editorToProperty;
};

template <class Editor>
Editor *EditorFactoryPrivate<Editor>::createEditor(QtProperty *property, QWidget *parent)
{
    Editor *editor = new Editor(parent);
    initializeEditor(property, editor);
    return editor;
}

template <class Editor>
void EditorFactoryPrivate<Editor>::initializeEditor(QtProperty *property, Editor *editor)
{
    Q_TYPENAME PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
    if (it == m_createdEditors.end())
        it = m_createdEditors.insert(property, EditorList());
    it.value().append(editor);
    m_editorToProperty.insert(editor, property);
}

template <class Editor>
void EditorFactoryPrivate<Editor>::slotEditorDestroyed(QObject *object)
{
    const Q_TYPENAME EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
    for (Q_TYPENAME EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor !=  ecend; ++itEditor) {
        if (itEditor.key() == object) {
            Editor *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            const Q_TYPENAME PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
            if (pit != m_createdEditors.end()) {
                pit.value().removeAll(editor);
                if (pit.value().empty())
                    m_createdEditors.erase(pit);
            }
            m_editorToProperty.erase(itEditor);
            return;
        }
    }
}

// ------------ QtSpinBoxFactory

class QtSpinBoxFactoryPrivate : public EditorFactoryPrivate<QSpinBox>
{
    QtSpinBoxFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtSpinBoxFactory)
public:

    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSetValue(int value);
};

void QtSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;
    QListIterator<QSpinBox *> itEditor(m_createdEditors[property]);
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

    QListIterator<QSpinBox *> itEditor(m_createdEditors[property]);
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
    const QMap<QSpinBox *, QtProperty *>::ConstIterator  ecend = m_editorToProperty.constEnd();
    for (QMap<QSpinBox *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor !=  ecend; ++itEditor) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtIntPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
    }
}

/*!
    \class QtSpinBoxFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QSpinBox *editor = d_ptr->createEditor(property, parent);
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    editor->setKeyboardTracking(false);

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

class QtSliderFactoryPrivate : public EditorFactoryPrivate<QSlider>
{
    QtSliderFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtSliderFactory)
public:
    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSetValue(int value);
};

void QtSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;
    QListIterator<QSlider *> itEditor(m_createdEditors[property]);
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

    QListIterator<QSlider *> itEditor(m_createdEditors[property]);
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
    const QMap<QSlider *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QSlider *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor ) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtIntPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
    }
}

/*!
    \class QtSliderFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    d_ptr->initializeEditor(property, editor);
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));

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

class QtScrollBarFactoryPrivate : public  EditorFactoryPrivate<QScrollBar>
{
    QtScrollBarFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtScrollBarFactory)
public:
    void slotPropertyChanged(QtProperty *property, int value);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotSetValue(int value);
};

void QtScrollBarFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QScrollBar *> itEditor( m_createdEditors[property]);
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

    QListIterator<QScrollBar *> itEditor( m_createdEditors[property]);
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
    const QMap<QScrollBar *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QScrollBar *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtIntPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtScrollBarFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    d_ptr->initializeEditor(property, editor);
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
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

class QtCheckBoxFactoryPrivate : public EditorFactoryPrivate<QCheckBox>
{
    QtCheckBoxFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtCheckBoxFactory)
public:
    void slotPropertyChanged(QtProperty *property, bool value);
    void slotSetValue(bool value);
};

void QtCheckBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, bool value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QCheckBox *> itEditor(m_createdEditors[property]);
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

    const QMap<QCheckBox *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QCheckBox *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend;  ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtBoolPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtCheckBoxFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QCheckBox *editor = d_ptr->createEditor(property, parent);
    editor->setChecked(manager->value(property));
    editor->setText(property->valueText());

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

class QtDoubleSpinBoxFactoryPrivate : public EditorFactoryPrivate<QDoubleSpinBox>
{
    QtDoubleSpinBoxFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtDoubleSpinBoxFactory)
public:

    void slotPropertyChanged(QtProperty *property, double value);
    void slotRangeChanged(QtProperty *property, double min, double max);
    void slotDecimalsChanged(QtProperty *property, int prec);
    void slotSetValue(double value);
};

void QtDoubleSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
    QList<QDoubleSpinBox *> editors = m_createdEditors[property];
    QListIterator<QDoubleSpinBox *> itEditor(m_createdEditors[property]);
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
    const QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator itcend = m_editorToProperty.constEnd();
    for (QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != itcend; ++itEditor) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
    }
}

/*! \class QtDoubleSpinBoxFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QDoubleSpinBox *editor = d_ptr->createEditor(property, parent);
    editor->setDecimals(manager->decimals(property));
    editor->setRange(manager->minimum(property), manager->maximum(property));
    editor->setValue(manager->value(property));
    editor->setKeyboardTracking(false);

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

class QtLineEditFactoryPrivate : public EditorFactoryPrivate<QLineEdit>
{
    QtLineEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtLineEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QString &value);
    void slotRegExpChanged(QtProperty *property, const QRegExp &regExp);
    void slotSetValue(const QString &value);
};

void QtLineEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
                const QString &value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QLineEdit *> itEditor( m_createdEditors[property]);
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

    QListIterator<QLineEdit *> itEditor(m_createdEditors[property]);
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
    const QMap<QLineEdit *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QLineEdit *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtStringPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtLineEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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

    QLineEdit *editor = d_ptr->createEditor(property, parent);
    QRegExp regExp = manager->regExp(property);
    if (regExp.isValid()) {
        QValidator *validator = new QRegExpValidator(regExp, editor);
        editor->setValidator(validator);
    }
    editor->setText(manager->value(property));

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

class QtDateEditFactoryPrivate : public EditorFactoryPrivate<QDateEdit>
{
    QtDateEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtDateEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QDate &value);
    void slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max);
    void slotSetValue(const QDate &value);
};

void QtDateEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QDate &value)
{
    if (!m_createdEditors.contains(property))
        return;
    QListIterator<QDateEdit *> itEditor(m_createdEditors[property]);
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

    QListIterator<QDateEdit *> itEditor(m_createdEditors[property]);
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
    const QMap<QDateEdit *, QtProperty *>::ConstIterator  ecend = m_editorToProperty.constEnd();
    for (QMap<QDateEdit *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDatePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtDateEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QDateEdit *editor = d_ptr->createEditor(property, parent);
    editor->setCalendarPopup(true);
    editor->setDateRange(manager->minimum(property), manager->maximum(property));
    editor->setDate(manager->value(property));

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

class QtTimeEditFactoryPrivate : public EditorFactoryPrivate<QTimeEdit>
{
    QtTimeEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtTimeEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QTime &value);
    void slotSetValue(const QTime &value);
};

void QtTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QTime &value)
{
    if (!m_createdEditors.contains(property))
        return;
    QListIterator<QTimeEdit *> itEditor(m_createdEditors[property]);
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
    const  QMap<QTimeEdit *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QTimeEdit *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtTimePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtTimeEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QTimeEdit *editor = d_ptr->createEditor(property, parent);
    editor->setTime(manager->value(property));

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

class QtDateTimeEditFactoryPrivate : public EditorFactoryPrivate<QDateTimeEdit>
{
    QtDateTimeEditFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtDateTimeEditFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QDateTime &value);
    void slotSetValue(const QDateTime &value);

};

void QtDateTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QDateTime &value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QDateTimeEdit *> itEditor(m_createdEditors[property]);
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
    const  QMap<QDateTimeEdit *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QDateTimeEdit *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend;  ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDateTimePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtDateTimeEditFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QDateTimeEdit *editor =  d_ptr->createEditor(property, parent);
    editor->setDateTime(manager->value(property));

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
    : QWidget(parent), m_num(0), m_lineEdit(new QLineEdit(this))
{
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
        const QList<QAction *> actions = menu->actions();
        QListIterator<QAction *> itAction(actions);
        while (itAction.hasNext()) {
            QAction *action = itAction.next();
            action->setShortcut(QKeySequence());
            QString actionString = action->text();
            const int pos = actionString.lastIndexOf(QLatin1Char('\t'));
            if (pos > 0)
                actionString.remove(pos,  actionString.length() - pos);
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
    if (m_keySequence.isEmpty())
        return;
    setKeySequence(QKeySequence());
    emit keySequenceChanged(m_keySequence);
}

void QtKeySequenceEdit::handleKeyEvent(QKeyEvent *e)
{
    int nextKey = e->key();
    if (nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift ||
            nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt ||
            nextKey == Qt::Key_Super_L || nextKey == Qt::Key_AltGr)
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

class QtKeySequenceEditorFactoryPrivate : public EditorFactoryPrivate<QtKeySequenceEdit>
{
    QtKeySequenceEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtKeySequenceEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QKeySequence &value);
    void slotSetValue(const QKeySequence &value);
};

void QtKeySequenceEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QKeySequence &value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QtKeySequenceEdit *> itEditor(m_createdEditors[property]);
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
    const  QMap<QtKeySequenceEdit *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QtKeySequenceEdit *, QtProperty *>::ConstIterator itEditor =  m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtKeySequencePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtKeySequenceEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QtKeySequenceEdit *editor = d_ptr->createEditor(property, parent);
    editor->setKeySequence(manager->value(property));

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

// QtCharEdit

class QtCharEdit : public QWidget
{
    Q_OBJECT
public:
    QtCharEdit(QWidget *parent = 0);

    QChar value() const;
    bool eventFilter(QObject *o, QEvent *e);
public Q_SLOTS:
    void setValue(const QChar &value);
Q_SIGNALS:
    void valueChanged(const QChar &value);
protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    bool event(QEvent *e);
private slots:
    void slotClearChar();
private:
    void handleKeyEvent(QKeyEvent *e);

    QChar m_value;
    QLineEdit *m_lineEdit;
};

QtCharEdit::QtCharEdit(QWidget *parent)
    : QWidget(parent),  m_lineEdit(new QLineEdit(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->setMargin(0);
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setFocusProxy(this);
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
}

bool QtCharEdit::eventFilter(QObject *o, QEvent *e)
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
            const int pos = actionString.lastIndexOf(QLatin1Char('\t'));
            if (pos > 0)
                actionString = actionString.remove(pos, actionString.length() - pos);
            action->setText(actionString);
        }
        QAction *actionBefore = 0;
        if (actions.count() > 0)
            actionBefore = actions[0];
        QAction *clearAction = new QAction(tr("Clear Char"), menu);
        menu->insertAction(actionBefore, clearAction);
        menu->insertSeparator(actionBefore);
        clearAction->setEnabled(!m_value.isNull());
        connect(clearAction, SIGNAL(triggered()), this, SLOT(slotClearChar()));
        menu->exec(c->globalPos());
        delete menu;
        e->accept();
        return true;
    }

    return QWidget::eventFilter(o, e);
}

void QtCharEdit::slotClearChar()
{
    if (m_value.isNull())
        return;
    setValue(QChar());
    emit valueChanged(m_value);
}

void QtCharEdit::handleKeyEvent(QKeyEvent *e)
{
    const int key = e->key();
    switch (key) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_Super_L:
    case Qt::Key_Return:
        return;
    }

    const QString text = e->text();
    if (text.count() != 1)
        return;

    const QChar c = text.at(0);
    if (!c.isPrint())
        return;

    if (m_value == c)
        return;

    m_value = c;
    const QString str = m_value.isNull() ? QString() : QString(m_value);
    m_lineEdit->setText(str);
    e->accept();
    emit valueChanged(m_value);
}

void QtCharEdit::setValue(const QChar &value)
{
    if (value == m_value)
        return;

    m_value = value;
    QString str = value.isNull() ? QString() : QString(value);
    m_lineEdit->setText(str);
}

QChar QtCharEdit::value() const
{
    return m_value;
}

void QtCharEdit::focusInEvent(QFocusEvent *e)
{
    m_lineEdit->event(e);
    m_lineEdit->selectAll();
    QWidget::focusInEvent(e);
}

void QtCharEdit::focusOutEvent(QFocusEvent *e)
{
    m_lineEdit->event(e);
    QWidget::focusOutEvent(e);
}

void QtCharEdit::keyPressEvent(QKeyEvent *e)
{
    handleKeyEvent(e);
    e->accept();
}

void QtCharEdit::keyReleaseEvent(QKeyEvent *e)
{
    m_lineEdit->event(e);
}

bool QtCharEdit::event(QEvent *e)
{
    switch(e->type()) {
    case QEvent::Shortcut:
    case QEvent::ShortcutOverride:
    case QEvent::KeyRelease:
        e->accept();
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

// QtCharEditorFactory

class QtCharEditorFactoryPrivate : public EditorFactoryPrivate<QtCharEdit>
{
    QtCharEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtCharEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, const QChar &value);
    void slotSetValue(const QChar &value);

};

void QtCharEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
            const QChar &value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QtCharEdit *> itEditor(m_createdEditors[property]);
    while (itEditor.hasNext()) {
        QtCharEdit *editor = itEditor.next();
        editor->blockSignals(true);
        editor->setValue(value);
        editor->blockSignals(false);
    }
}

void QtCharEditorFactoryPrivate::slotSetValue(const QChar &value)
{
    QObject *object = q_ptr->sender();
    const QMap<QtCharEdit *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QtCharEdit *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend;  ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtCharPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtCharEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

    \brief The QtCharEditorFactory class provides editor
    widgets for properties created by QtCharPropertyManager objects.

    \sa QtAbstractEditorFactory
*/

/*!
    Creates a factory with the given \a parent.
*/
QtCharEditorFactory::QtCharEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtCharPropertyManager>(parent)
{
    d_ptr = new QtCharEditorFactoryPrivate();
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtCharEditorFactory::~QtCharEditorFactory()
{
    qDeleteAll(d_ptr->m_editorToProperty.keys());
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCharEditorFactory::connectPropertyManager(QtCharPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QChar &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QChar &)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtCharEditorFactory::createEditor(QtCharPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtCharEdit *editor = d_ptr->createEditor(property, parent);
    editor->setValue(manager->value(property));

    connect(editor, SIGNAL(valueChanged(const QChar &)),
                this, SLOT(slotSetValue(const QChar &)));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtCharEditorFactory::disconnectPropertyManager(QtCharPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QChar &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QChar &)));
}

// QtEnumEditorFactory

class QtEnumEditorFactoryPrivate : public EditorFactoryPrivate<QComboBox>
{
    QtEnumEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtEnumEditorFactory)
public:

    void slotPropertyChanged(QtProperty *property, int value);
    void slotEnumNamesChanged(QtProperty *property, const QStringList &);
    void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &);
    void slotSetValue(int value);
};

void QtEnumEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
    if (!m_createdEditors.contains(property))
        return;

    QListIterator<QComboBox *> itEditor(m_createdEditors[property]);
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

    QListIterator<QComboBox *> itEditor(m_createdEditors[property]);
    while (itEditor.hasNext()) {
        QComboBox *editor = itEditor.next();
        editor->blockSignals(true);
        editor->clear();
        editor->addItems(enumNames);
        const int nameCount = enumNames.count();
        for (int i = 0; i < nameCount; i++)
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

    const QStringList enumNames = manager->enumNames(property);
    QListIterator<QComboBox *> itEditor(m_createdEditors[property]);
    while (itEditor.hasNext()) {
        QComboBox *editor = itEditor.next();
        editor->blockSignals(true);
        const int nameCount = enumNames.count();
        for (int i = 0; i < nameCount; i++)
            editor->setItemIcon(i, enumIcons.value(i));
        editor->setCurrentIndex(manager->value(property));
        editor->blockSignals(false);
    }
}

void QtEnumEditorFactoryPrivate::slotSetValue(int value)
{
    QObject *object = q_ptr->sender();
    const  QMap<QComboBox *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QComboBox *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

/*!
    \class QtEnumEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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
    qDeleteAll(d_ptr->m_editorToProperty.keys());
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
    QComboBox *editor = d_ptr->createEditor(property, parent);
    editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    editor->view()->setTextElideMode(Qt::ElideRight);
    QStringList enumNames = manager->enumNames(property);
    editor->addItems(enumNames);
    QMap<int, QIcon> enumIcons = manager->enumIcons(property);
    const int enumNamesCount = enumNames.count();
    for (int i = 0; i < enumNamesCount; i++)
        editor->setItemIcon(i, enumIcons.value(i));
    editor->setCurrentIndex(manager->value(property));

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
    const  QMap<QWidget *, QtProperty *>::ConstIterator ecend = m_editorToEnum.constEnd();
    for (QMap<QWidget *, QtProperty *>::ConstIterator itEditor = m_editorToEnum.constBegin(); itEditor != ecend; ++itEditor)
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
}

/*!
    \class QtCursorEditorFactory
    \internal
    \inmodule QtDesigner
    \since 4.4

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

QT_END_NAMESPACE

#include "moc_qteditorfactory.cpp"
#include "qteditorfactory.moc"
