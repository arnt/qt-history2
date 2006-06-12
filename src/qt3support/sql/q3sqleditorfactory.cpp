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

#include "q3sqleditorfactory.h"

#ifndef QT_NO_SQL_EDIT_WIDGETS

#include "qsqlfield.h"
#include "q3cleanuphandler.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qcombobox.h"
#include "qdatetimeedit.h"

/*!
    \class Q3SqlEditorFactory qsqleditorfactory.h
    \brief The Q3SqlEditorFactory class is used to create the editors
    used by Q3DataTable and Q3SqlForm.

    \compat

    Q3SqlEditorFactory is used by Q3DataTable and Q3SqlForm to
    automatically create appropriate editors for a given QSqlField.
    For example if the field is a QVariant::String a QLineEdit would
    be the default editor, whereas a QVariant::Int's default editor
    would be a QSpinBox.

    If you want to create different editors for fields with the same
    data type, subclass Q3SqlEditorFactory and reimplement the
    createEditor() function.

    \sa Q3DataTable, Q3SqlForm
*/


/*!
    Constructs a SQL editor factory with parent \a parent.
*/

Q3SqlEditorFactory::Q3SqlEditorFactory (QObject * parent)
    : Q3EditorFactory(parent)
{

}

/*!
    Destroys the object and frees any allocated resources.
*/

Q3SqlEditorFactory::~Q3SqlEditorFactory()
{

}

static Q3SqlEditorFactory * defaultfactory = 0;
static Q3CleanupHandler< Q3SqlEditorFactory > qsql_cleanup_editor_factory;

/*!
    Returns an instance of a default editor factory.
*/

Q3SqlEditorFactory * Q3SqlEditorFactory::defaultFactory()
{
    if(defaultfactory == 0){
        defaultfactory = new Q3SqlEditorFactory();
        qsql_cleanup_editor_factory.add(&defaultfactory);
    }

    return defaultfactory;
}

/*!
    Replaces the default editor factory with \a factory. All
    Q3DataTable and Q3SqlForm instantiations will use this new factory
    for creating field editors. \e{Q3SqlEditorFactory takes ownership
    of \a factory, and destroys it when it is no longer needed.}
*/

void Q3SqlEditorFactory::installDefaultFactory(Q3SqlEditorFactory * factory)
{
    if(factory == 0) return;

    if(defaultfactory != 0){
        qsql_cleanup_editor_factory.remove(&defaultfactory);
        delete defaultfactory;
    }
    defaultfactory = factory;
    qsql_cleanup_editor_factory.add(&defaultfactory);
}

/*!
    Creates and returns the appropriate editor widget for the QVariant
    \a variant.

    The widget that is returned has the parent \a parent (which may be
    zero). If \a variant is invalid, 0 is returned.
*/

QWidget * Q3SqlEditorFactory::createEditor(QWidget * parent,
                                           const QVariant & variant)
{
    return Q3EditorFactory::createEditor(parent, variant);
}

/*!
    \overload

    Creates and returns the appropriate editor for the QSqlField \a
    field.
*/

QWidget * Q3SqlEditorFactory::createEditor(QWidget * parent,
                                           const QSqlField * field)
{
    if (!field) {
        return 0;
    }

    QWidget * w = 0;
    switch(field->type()){
        case QVariant::Invalid:
            w = 0;
            break;
        case QVariant::Bool:
            w = new QComboBox(parent, "qt_editor_bool");
            ((QComboBox *) w)->insertItem("False");
            ((QComboBox *) w)->insertItem("True");
            break;
        case QVariant::UInt:
            w = new QSpinBox(0, 2147483647, 1, parent, "qt_editor_spinbox");
            break;
        case QVariant::Int:
            w = new QSpinBox(-2147483647, 2147483647, 1, parent, "qt_editor_int");
            break;
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::String:
        case QVariant::Double:
            w = new QLineEdit(parent, "qt_editor_double");
            ((QLineEdit*)w)->setFrame(false);
            break;
        case QVariant::Date: {
            QDateTimeEdit *edit = new QDateTimeEdit(parent);
            edit->setDisplayFormat("yyyy/MM/dd");
            edit->setObjectName("qt_editor_date");
            w = edit; }
            break;
        case QVariant::Time: {
            QDateTimeEdit *edit = new QDateTimeEdit(parent);
            edit->setDisplayFormat("hh:mm");
            edit->setObjectName("qt_editor_time");
            w = edit; }
            break;
        case QVariant::DateTime:
            w = new QDateTimeEdit(parent);
            w->setObjectName("qt_editor_datetime");
            break;
#ifndef QT_NO_LABEL
        case QVariant::Pixmap:
            w = new QLabel(parent, "qt_editor_pixmap");
            break;
#endif
        case QVariant::Palette:
        case QVariant::Color:
        case QVariant::Font:
        case QVariant::Brush:
        case QVariant::Bitmap:
        case QVariant::Cursor:
        case QVariant::Map:
        case QVariant::StringList:
        case QVariant::Rect:
        case QVariant::Size:
        case QVariant::IconSet:
        case QVariant::Point:
        case QVariant::PointArray:
        case QVariant::Region:
        case QVariant::SizePolicy:
        case QVariant::ByteArray:
        default:
            w = new QWidget(parent, "qt_editor_default");
            break;
    }
    return w;
}

#endif // QT_NO_SQL
