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

#include "qitemeditorfactory.h"
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qdatetimeedit.h>
#include <limits.h>

QWidget *QItemEditorFactory::createEditor(QVariant::Type type, QWidget *parent) const
{
    QItemEditorCreatorBase *creator = creatorMap.value(type, 0);
    if (!creator)
        return defaultFactory()->createEditor(type, parent);
    return creator->createWidget(parent);
}

QByteArray QItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
    QItemEditorCreatorBase *creator = creatorMap.value(type, 0);
    if (!creator)
        return defaultFactory()->valuePropertyName(type);
    return creator->valuePropertyName();
}


QItemEditorFactory::~QItemEditorFactory()
{

}

void QItemEditorFactory::registerEditor(QVariant::Type type, QItemEditorCreatorBase *creator)
{
   delete creatorMap.value(type, 0);
   creatorMap[type] = creator;
}

class QDefaultItemEditorFactory: public QItemEditorFactory
{
public:
    inline QDefaultItemEditorFactory() {}
    QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
    QByteArray valuePropertyName(QVariant::Type) const;
};

QWidget *QDefaultItemEditorFactory::createEditor(QVariant::Type type, QWidget *parent) const
{
    switch (type) {
    case QVariant::Bool: {
        QComboBox *cb = new QComboBox(parent);
        cb->insertItem("False");
        cb->insertItem("True");
        return cb; }
    case QVariant::UInt: {
        QSpinBox *sb = new QSpinBox(parent);
        sb->setTracking(true);
        sb->setMaximum(INT_MAX);
        return sb; }
    case QVariant::Int: {
        QSpinBox *sb = new QSpinBox(parent);
        sb->setTracking(true);
        sb->setMinimum(INT_MIN);
        sb->setMaximum(INT_MAX);
        return sb; }
    case QVariant::Date: {
        QDateTimeEdit *ed = new QDateTimeEdit(QDate(), parent);
        ed->setTracking(true);
        return ed; }
    case QVariant::Time: {
        QDateTimeEdit *ed = new QDateTimeEdit(QTime(), parent);
        ed->setTracking(true);
        return ed; }
    case QVariant::DateTime: {
        QDateTimeEdit *ed = new QDateTimeEdit(parent);
        ed->setTracking(true);
        return ed; }
    case QVariant::Pixmap:
        return new QLabel(parent);
    case QVariant::String:
    case QVariant::Double:
    default: {
        // the default editor is a lineedit
        QLineEdit *le = new QLineEdit(parent);
        le->setFrame(false);
        return le; }
    }
}

QByteArray QDefaultItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
    switch (type) {
    case QVariant::Bool:
        return "currentItem";
    case QVariant::UInt:
    case QVariant::Int:
        return "value";
    case QVariant::Date:
        return "date";
    case QVariant::Time:
        return "time";
    case QVariant::DateTime:
        return "dateTime";
    case QVariant::String:
    case QVariant::Double:
    default:
        // the default editor is a lineedit
        return "text";
    }
}

static QItemEditorFactory *q_default_factory = 0;
struct QDefaultFactoryCleaner
{
    inline QDefaultFactoryCleaner() {}
    ~QDefaultFactoryCleaner() { delete q_default_factory; q_default_factory = 0; }
};

const QItemEditorFactory *QItemEditorFactory::defaultFactory()
{
    static const QDefaultItemEditorFactory factory;
    if (q_default_factory)
        return q_default_factory;
    return &factory;
}

void QItemEditorFactory::setDefaultFactory(QItemEditorFactory *factory)
{
    static const QDefaultFactoryCleaner cleaner;
    delete q_default_factory;
    q_default_factory = factory;
}
