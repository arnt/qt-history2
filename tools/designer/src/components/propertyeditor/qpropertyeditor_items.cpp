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

#include "qpropertyeditor_items_p.h"
#include "flagbox_p.h"
#include "keysequenceeditor.h"
#include "defs.h"

#include <QLineEdit>
#include <QListView>
#include <QComboBox>
#include <QSpinBox>
#include <QValidator>
#include <QFontDatabase>
#include <QPainter>
#include <QDateTimeEdit>
#include <QBitmap>

#include <qdebug.h>
#include <limits.h>

Q_GLOBAL_STATIC(QFontDatabase, fontDatabase)

// -------------------------------------------------------------------------
bool AbstractPropertyGroup::dirty() const
{
    for (int i=0; i<propertyCount(); ++i)
        if (propertyAt(i)->dirty())
            return true;

    return IProperty::dirty();
}

void AbstractPropertyGroup::setDirty(bool b)
{
    IProperty::setDirty(b);

    for (int i=0; i<propertyCount(); ++i)
        propertyAt(i)->setDirty(b);
}

// -------------------------------------------------------------------------
BoolProperty::BoolProperty(bool value, const QString &name)
    : AbstractProperty<bool>(value, name)
{
}

void BoolProperty::setValue(const QVariant &value)
{
    m_value = value.toBool();
}

QString BoolProperty::toString() const
{
    return m_value ? QLatin1String("true") : QLatin1String("false");
}

QWidget *BoolProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QComboBox *combo = new QComboBox(parent);
    combo->insertItems(-1, QStringList() << "false" << "true");
    QObject::connect(combo, SIGNAL(activated(int)), target, receiver);

    return combo;
}

void BoolProperty::updateEditorContents(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        combo->setCurrentItem(m_value ? 1 : 0);
    }
}

void BoolProperty::updateValue(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        bool newValue = combo->currentItem() ? true : false;

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }
    }
}

// -------------------------------------------------------------------------
PointProperty::PointProperty(const QPoint &value, const QString &name)
    : AbstractPropertyGroup(name)
{
    IProperty *px = new IntProperty(value.x(), QLatin1String("x"));
    px->setFake(true);
    px->setParent(this);

    IProperty *py = new IntProperty(value.y(), QLatin1String("y"));
    py->setFake(true);
    py->setParent(this);

    m_properties << px << py;
}

QVariant PointProperty::value() const
{
    return QPoint(propertyAt(0)->value().toInt(),
                  propertyAt(1)->value().toInt());
}

void PointProperty::setValue(const QVariant &value)
{
    QPoint pt = value.toPoint();
    propertyAt(0)->setValue(pt.x());
    propertyAt(1)->setValue(pt.y());
}

// -------------------------------------------------------------------------
PropertyCollection::PropertyCollection(const QString &name)
    : m_name(name)
{
}

PropertyCollection::~PropertyCollection()
{
    qDeleteAll(m_properties);
}

void PropertyCollection::addProperty(IProperty *property)
{
    property->setParent(this);
    m_properties.append(property);
}

void PropertyCollection::removeProperty(IProperty *property)
{
    Q_UNUSED(property);
}

int PropertyCollection::indexOf(IProperty *property) const
{
    return m_properties.indexOf(property);
}

int PropertyCollection::propertyCount() const
{
    return m_properties.size();
}

IProperty *PropertyCollection::propertyAt(int index) const
{
    return m_properties.at(index);
}

QString PropertyCollection::propertyName() const
{
    return m_name;
}

QVariant PropertyCollection::value() const
{
    return QVariant();
}

void PropertyCollection::setValue(const QVariant &value)
{
    Q_UNUSED(value);
}

QString PropertyCollection::toString() const
{
    return QString::null;
}

bool PropertyCollection::hasEditor() const
{
    return false;
}

QWidget *PropertyCollection::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    Q_UNUSED(parent);
    Q_UNUSED(target);
    Q_UNUSED(receiver);
    return 0;
}

bool PropertyCollection::hasExternalEditor() const
{
    return false;
}

QWidget *PropertyCollection::createExternalEditor(QWidget *parent)
{
    Q_UNUSED(parent);
    return 0;
}

// -------------------------------------------------------------------------

StringProperty::StringProperty(const QString &value, const QString &name)
    : AbstractProperty<QString>(value, name)
{
}

void StringProperty::setValue(const QVariant &value)
{
    m_value = value.toString();
}

QString StringProperty::toString() const
{
    return m_value;
}

QWidget *StringProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    if (propertyName() == QLatin1String("objectName")) {
        lineEdit->setValidator(new QRegExpValidator(QRegExp("[_a-zA-Z][_a-zA-Z0-9]*"), lineEdit));
    }

    QObject::connect(lineEdit, SIGNAL(textChanged(const QString&)), target, receiver);
    return lineEdit;
}

void StringProperty::updateEditorContents(QWidget *editor)
{
    if (QLineEdit *lineEdit = qt_cast<QLineEdit*>(editor)) {
        if (lineEdit->text() != m_value)
            lineEdit->setText(m_value);
    }
}

void StringProperty::updateValue(QWidget *editor)
{
    if (QLineEdit *lineEdit = qt_cast<QLineEdit*>(editor)) {
        QString newValue = lineEdit->text();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }

    }
}

// -------------------------------------------------------------------------
ListProperty::ListProperty(const QStringList &items, int value, const QString &name)
    : AbstractProperty<int>(value, name), m_items(items)
{
}

QStringList ListProperty::items() const
{
    return m_items;
}

void ListProperty::setValue(const QVariant &value)
{
    m_value = value.toInt();
}

QString ListProperty::toString() const
{
    return m_items.at(m_value);
}

QWidget *ListProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QComboBox *combo = new QComboBox(parent);
    combo->insertItems(-1, items());
    QObject::connect(combo, SIGNAL(activated(int)), target, receiver);
    return combo;
}

void ListProperty::updateEditorContents(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        combo->setCurrentItem(m_value);
    }
}

void ListProperty::updateValue(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        int newValue = combo->currentItem();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }
    }
}

// -------------------------------------------------------------------------
SizeProperty::SizeProperty(const QSize &value, const QString &name)
    : AbstractPropertyGroup(name)
{
    IntProperty *pw = new IntProperty(value.width(), QLatin1String("width"));
    pw->setFake(true);
    pw->setParent(this);
    pw->setRange(0, INT_MAX);

    IntProperty *ph = new IntProperty(value.height(), QLatin1String("height"));
    ph->setFake(true);
    ph->setParent(this);
    ph->setRange(0, INT_MAX);

    m_properties << pw << ph;
}

QVariant SizeProperty::value() const
{
    return QSize(propertyAt(0)->value().toInt(),
                 propertyAt(1)->value().toInt());
}

void SizeProperty::setValue(const QVariant &value)
{
    QSize pt = value.toSize();
    propertyAt(0)->setValue(pt.width());
    propertyAt(1)->setValue(pt.height());
}

// -------------------------------------------------------------------------
IntProperty::IntProperty(int value, const QString &name)
    : AbstractProperty<int>(value, name), m_low(INT_MIN), m_hi(INT_MAX)
{
}

void IntProperty::setRange(int low, int hi)
{
    m_low = low;
    m_hi = hi;
}

QString IntProperty::specialValue() const
{
    return m_specialValue;
}

void IntProperty::setSpecialValue(const QString &specialValue)
{
    m_specialValue = specialValue;
}

void IntProperty::setValue(const QVariant &value)
{
    m_value = value.toInt();
}

QString IntProperty::toString() const
{
    return QString::number(m_value);
}

QWidget *IntProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QSpinBox *spinBox = new QSpinBox(parent);
    QObject::connect(spinBox, SIGNAL(valueChanged(int)), target, receiver);
    spinBox->setFrame(false);
    spinBox->setSpecialValueText(m_specialValue);
    spinBox->setRange(m_low, m_hi);
    return spinBox;
}

void IntProperty::updateEditorContents(QWidget *editor)
{
    if (QSpinBox *spinBox = qt_cast<QSpinBox*>(editor)) {
        spinBox->setValue(m_value);
        spinBox->selectAll();
    }
}

void IntProperty::updateValue(QWidget *editor)
{
    if (QSpinBox *spinBox = qt_cast<QSpinBox*>(editor)) {
        int newValue = spinBox->value();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);

            if (parent())
                parent()->setChanged(true);
        }
    }
}

// -------------------------------------------------------------------------
RectProperty::RectProperty(const QRect &value, const QString &name)
    : AbstractPropertyGroup(name)
{
    IntProperty *px = new IntProperty(value.x(), QLatin1String("x"));
    px->setFake(true);
    px->setParent(this);

    IntProperty *py = new IntProperty(value.y(), QLatin1String("y"));
    py->setFake(true);
    py->setParent(this);

    IntProperty *pw = new IntProperty(value.width(), QLatin1String("width"));
    pw->setFake(true);
    pw->setParent(this);
    pw->setRange(0, INT_MAX);

    IntProperty *ph = new IntProperty(value.height(), QLatin1String("height"));
    ph->setFake(true);
    ph->setParent(this);
    ph->setRange(0, INT_MAX);

    m_properties << px << py << pw << ph;
}

QVariant RectProperty::value() const
{
    return QRect(propertyAt(0)->value().toInt(),
                 propertyAt(1)->value().toInt(),
                 propertyAt(2)->value().toInt(),
                 propertyAt(3)->value().toInt());
}

void RectProperty::setValue(const QVariant &value)
{
    QRect pt = value.toRect();
    propertyAt(0)->setValue(pt.x());
    propertyAt(1)->setValue(pt.y());
    propertyAt(2)->setValue(pt.width());
    propertyAt(3)->setValue(pt.height());
}


// -------------------------------------------------------------------------
ColorProperty::ColorProperty(const QColor &value, const QString &name)
    : AbstractPropertyGroup(name)
{
    IntProperty *r = new IntProperty(value.red(), QLatin1String("red"));
    r->setFake(true);
    r->setRange(0, 255);
    r->setParent(this);

    IntProperty *g = new IntProperty(value.green(), QLatin1String("green"));
    g->setFake(true);
    g->setRange(0, 255);
    g->setParent(this);

    IntProperty *b = new IntProperty(value.blue(), QLatin1String("blue"));
    b->setFake(true);
    b->setRange(0, 255);
    b->setParent(this);

    m_properties << r << g << b;
}

QVariant ColorProperty::value() const
{
    return qVariant(QColor(propertyAt(0)->value().toInt(),
                  propertyAt(1)->value().toInt(),
                  propertyAt(2)->value().toInt()));
}

void ColorProperty::setValue(const QVariant &value)
{
    QColor c = qVariant_to<QColor>(value);
    propertyAt(0)->setValue(c.red());
    propertyAt(1)->setValue(c.green());
    propertyAt(2)->setValue(c.blue());
}

QVariant ColorProperty::decoration() const
{
    QPixmap pix;
    pix.resize(16, 16);
    pix.fill(qVariant_to<QColor>(value()));
    return qVariant(pix);
}

// -------------------------------------------------------------------------
FontProperty::FontProperty(const QFont &value, const QString &name)
    : AbstractPropertyGroup(name)
{
    QStringList fonts = fontDatabase()->families();
    int index = fonts.indexOf(value.family());
    if (index == -1)
        index = 0;

    IProperty *i = 0;
    i = new ListProperty(fonts, index, QLatin1String("Family"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    IntProperty *ii = new IntProperty(value.pointSize(), QLatin1String("Point Size"));
    ii->setFake(true);
    ii->setRange(1, INT_MAX); // ### check
    ii->setParent(this);
    m_properties << ii;

    i = new BoolProperty(value.bold(), QLatin1String("Bold"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    i = new BoolProperty(value.italic(), QLatin1String("Italic"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    i = new BoolProperty(value.underline(), QLatin1String("Underline"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    i = new BoolProperty(value.strikeOut(), QLatin1String("Strikeout"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;
}

QVariant FontProperty::value() const
{
    QFont fnt;
    fnt.setFamily(propertyAt(0)->toString());
    fnt.setPointSize(propertyAt(1)->value().toInt());
    fnt.setBold(propertyAt(2)->value().toBool());
    fnt.setItalic(propertyAt(3)->value().toBool());
    fnt.setUnderline(propertyAt(4)->value().toBool());
    fnt.setStrikeOut(propertyAt(5)->value().toBool());

    return qVariant(fnt);
}

void FontProperty::setValue(const QVariant &value)
{
    QFont fnt = qVariant_to<QFont>(value);

    int family = fontDatabase()->families().indexOf(fnt.family());

    propertyAt(0)->setValue(family);
    propertyAt(1)->setValue(fnt.pointSize());
    propertyAt(2)->setValue(fnt.bold());
    propertyAt(3)->setValue(fnt.italic());
    propertyAt(4)->setValue(fnt.underline());
    propertyAt(5)->setValue(fnt.strikeOut());
}

QVariant FontProperty::decoration() const
{
    QPixmap pix;
    pix.resize(16, 16);
    pix.fill(Qt::white);
    QPainter p(&pix);
    QFont fnt = qVariant_to<QFont>(value());
    fnt.setPointSize(10); // ### always 10pt!!
    p.drawRect(0, 0, 16, 16);
    p.setFont(fnt);
    p.drawText(0, 16 - 2, QLatin1String("Aa")); // ### 2px for the border!!
    return qVariant(pix);
}

QString FontProperty::toString() const
{
    QString family = propertyAt(0)->toString();
    QString pointSize = propertyAt(1)->value().toString();

    return QLatin1String("  ")  // ### temp hack
        + QLatin1String("[") + family + QLatin1String(", ") + pointSize + QLatin1String("]");
}

// -------------------------------------------------------------------------
MapProperty::MapProperty(const QMap<QString, QVariant> &items,
                         const QVariant &value,
                         const QString &name)
    : AbstractProperty<QVariant>(value, name),
      m_items(items),
      m_keys(m_items.keys())
{
}

QStringList MapProperty::keys() const
{
    return m_keys;
}

QMap<QString, QVariant> MapProperty::items() const
{
    return m_items;
}

QVariant MapProperty::value() const
{
    return m_value;
}

void MapProperty::setValue(const QVariant &value)
{
    m_value = value;
}

QString MapProperty::toString() const
{
    return m_items.key(m_value);
}

int MapProperty::indexOf(const QVariant &value) const
{
    QString key = m_items.key(value);
    return m_keys.indexOf(key);
}

QWidget *MapProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QComboBox *combo = new QComboBox(parent);
    combo->insertItems(-1, m_keys);
    QObject::connect(combo, SIGNAL(activated(int)), target, receiver);

    return combo;
}

void MapProperty::updateEditorContents(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        combo->setCurrentItem(indexOf(m_value));
    }
}

void MapProperty::updateValue(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        QString key = combo->currentText();
        QVariant newValue = m_items.value(key);

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);

            if (parent())
                parent()->setChanged(true);
        }
    }
}

// -------------------------------------------------------------------------
FlagsProperty::FlagsProperty(const QMap<QString, QVariant> &items, int value,
                             const QString &name)
    : MapProperty(items, QVariant(value), name)
{
}

QWidget *FlagsProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QList<FlagBoxModelItem> l;
    QMapIterator<QString, QVariant> it(items());
    while (it.hasNext()) {
        it.next();
        l.append(FlagBoxModelItem(it.key(), it.value().toInt(), /*checked=*/false));
    }

    FlagBox *editor = new FlagBox(parent);
    editor->setItems(l);
    QObject::connect(editor, SIGNAL(activated(int)), target, receiver);
    return editor;
}

void FlagsProperty::updateEditorContents(QWidget *editor)
{
    int v = m_value.toInt();

    if (FlagBox *box = qt_cast<FlagBox*>(editor)) {
        for (int i=0; i<box->count(); ++i) {
            FlagBoxModelItem &item = box->item(i);
            item.setChecked((v & item.value()) != 0);
        }
    }
}

void FlagsProperty::updateValue(QWidget *editor)
{
    int v = 0;

    if (FlagBox *box = qt_cast<FlagBox*>(editor)) {
        for (int i=0; i<box->count(); ++i) {
            FlagBoxModelItem &item = box->item(i);
            if (item.isChecked())
                v |= item.value();
        }
    }
    m_value = v;
}

// -------------------------------------------------------------------------
SizePolicyProperty::SizePolicyProperty(const QSizePolicy &value, const QString &name)
    : AbstractPropertyGroup(name)
{
    QStringList lst;
    lst << "Fixed" << "Minimum" << "Maximum" << "Preferred" << "MinimumExpanding" << "Expanding" << "Ignored";

    IProperty *i = 0;
    i = new ListProperty(lst, size_type_to_int(value.horizontalData()), QLatin1String("hSizeType"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    i = new ListProperty(lst, size_type_to_int(value.verticalData()), QLatin1String("vSizeType"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    i = new IntProperty(value.horizontalStretch(), QLatin1String("horizontalStretch"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;

    i = new IntProperty(value.verticalStretch(), QLatin1String("verticalStretch"));
    i->setFake(true);
    i->setParent(this);
    m_properties << i;
}

QVariant SizePolicyProperty::value() const
{
    QSizePolicy sizePolicy;
    sizePolicy.setHorizontalData(int_to_size_type(propertyAt(0)->value().toInt()));
    sizePolicy.setVerticalData(int_to_size_type(propertyAt(1)->value().toInt()));
    sizePolicy.setHorizontalStretch(propertyAt(2)->value().toInt());
    sizePolicy.setVerticalStretch(propertyAt(3)->value().toInt());
    return qVariant(sizePolicy);
}

void SizePolicyProperty::setValue(const QVariant &value)
{
    QSizePolicy sizePolicy = qVariant_to<QSizePolicy>(value);

    propertyAt(0)->setValue(size_type_to_int(sizePolicy.horizontalData()));
    propertyAt(1)->setValue(size_type_to_int(sizePolicy.verticalData()));
    propertyAt(2)->setValue(sizePolicy.horizontalStretch());
    propertyAt(3)->setValue(sizePolicy.verticalStretch());
}

QVariant SizePolicyProperty::decoration() const
{
    return QVariant();
}

QString SizePolicyProperty::toString() const
{
    return AbstractPropertyGroup::toString();
}

// -------------------------------------------------------------------------
DateTimeProperty::DateTimeProperty(const QDateTime &value, const QString &name)
    : AbstractProperty<QDateTime>(value, name)
{
}

void DateTimeProperty::setValue(const QVariant &value)
{
    m_value = value.toDateTime();
}

QString DateTimeProperty::toString() const
{
    return m_value.toString();
}

QWidget *DateTimeProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QDateTimeEdit *lineEdit = new QDateTimeEdit(parent);
    QObject::connect(lineEdit, SIGNAL(dateTimeChanged(const QDateTime&)), target, receiver);
    return lineEdit;
}

void DateTimeProperty::updateEditorContents(QWidget *editor)
{
    if (QDateTimeEdit *lineEdit = qt_cast<QDateTimeEdit*>(editor)) {
        lineEdit->setDateTime(m_value);
    }
}

void DateTimeProperty::updateValue(QWidget *editor)
{
    if (QDateTimeEdit *lineEdit = qt_cast<QDateTimeEdit*>(editor)) {
        QDateTime newValue = lineEdit->date();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }

    }
}

// -------------------------------------------------------------------------
DateProperty::DateProperty(const QDate &value, const QString &name)
    : AbstractProperty<QDate>(value, name)
{
}

void DateProperty::setValue(const QVariant &value)
{
    m_value = value.toDate();
}

QString DateProperty::toString() const
{
    return m_value.toString();
}

QWidget *DateProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QDateTimeEdit *lineEdit = new QDateTimeEdit(parent);
    QObject::connect(lineEdit, SIGNAL(dateChanged(const QDate&)), target, receiver);
    return lineEdit;
}

void DateProperty::updateEditorContents(QWidget *editor)
{
    if (QDateTimeEdit *lineEdit = qt_cast<QDateTimeEdit*>(editor)) {
        lineEdit->setDate(m_value);
    }
}

void DateProperty::updateValue(QWidget *editor)
{
    if (QDateTimeEdit *lineEdit = qt_cast<QDateTimeEdit*>(editor)) {
        QDate newValue = lineEdit->date();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }

    }
}

// -------------------------------------------------------------------------
TimeProperty::TimeProperty(const QTime &value, const QString &name)
    : AbstractProperty<QTime>(value, name)
{
}

void TimeProperty::setValue(const QVariant &value)
{
    m_value = value.toTime();
}

QString TimeProperty::toString() const
{
    return m_value.toString();
}

QWidget *TimeProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QDateTimeEdit *lineEdit = new QDateTimeEdit(parent);
    QObject::connect(lineEdit, SIGNAL(timeChanged(const QTime&)), target, receiver);
    return lineEdit;
}

void TimeProperty::updateEditorContents(QWidget *editor)
{
    if (QDateTimeEdit *lineEdit = qt_cast<QDateTimeEdit*>(editor)) {
        lineEdit->setTime(m_value);
    }
}

void TimeProperty::updateValue(QWidget *editor)
{
    if (QDateTimeEdit *lineEdit = qt_cast<QDateTimeEdit*>(editor)) {
        QTime newValue = lineEdit->time();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }

    }
}

// -------------------------------------------------------------------------
CursorProperty::CursorProperty(const QCursor &value, const QString &name)
    : AbstractProperty<QCursor>(value, name)
{
}

void CursorProperty::setValue(const QVariant &value)
{
    m_value = qVariant_to<QCursor>(value);
}

QString CursorProperty::toString() const
{
    return cursorName(m_value.shape());
}

QVariant CursorProperty::decoration() const
{
    return qVariant(cursorPixmap(m_value.shape()));
}

QWidget *CursorProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    QComboBox *combo = new QComboBox(parent);

    addCursor(combo, Qt::ArrowCursor);
    addCursor(combo, Qt::UpArrowCursor);
    addCursor(combo, Qt::CrossCursor);
    addCursor(combo, Qt::WaitCursor);
    addCursor(combo, Qt::IBeamCursor);
    addCursor(combo, Qt::SizeVerCursor);
    addCursor(combo, Qt::SizeHorCursor);
    addCursor(combo, Qt::SizeBDiagCursor);
    addCursor(combo, Qt::SizeFDiagCursor);
    addCursor(combo, Qt::SizeAllCursor);
    addCursor(combo, Qt::BlankCursor);
    addCursor(combo, Qt::SplitVCursor);
    addCursor(combo, Qt::SplitHCursor);
    addCursor(combo, Qt::PointingHandCursor);
    addCursor(combo, Qt::ForbiddenCursor);

    QObject::connect(combo, SIGNAL(activated(int)), target, receiver);

    return combo;
}

void CursorProperty::updateEditorContents(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        combo->setCurrentItem(m_value.shape());
    }
}

void CursorProperty::updateValue(QWidget *editor)
{
    if (QComboBox *combo = qt_cast<QComboBox*>(editor)) {
        QCursor newValue(static_cast<Qt::CursorShape>(combo->currentItem()));

        if (newValue.shape() != m_value.shape()) {
            m_value = newValue;
            setChanged(true);
        }
    }
}

QString CursorProperty::cursorName(int shape)
{
    switch (shape) {
    case Qt::ArrowCursor: return QString::fromLatin1("Arrow");
    case Qt::UpArrowCursor: return QString::fromLatin1("Up-Arrow");
    case Qt::CrossCursor: return QString::fromLatin1("Cross");
    case Qt::WaitCursor: return QString::fromLatin1("Waiting");
    case Qt::IBeamCursor: return QString::fromLatin1("IBeam");
    case Qt::SizeVerCursor: return QString::fromLatin1("Size Vertical");
    case Qt::SizeHorCursor: return QString::fromLatin1("Size Horizontal");
    case Qt::SizeBDiagCursor: return QString::fromLatin1("Size Slash");
    case Qt::SizeFDiagCursor: return QString::fromLatin1("Size Backslash");
    case Qt::SizeAllCursor: return QString::fromLatin1("Size All");
    case Qt::BlankCursor: return QString::fromLatin1("Blank");
    case Qt::SplitVCursor: return QString::fromLatin1("Split Vertical");
    case Qt::SplitHCursor: return QString::fromLatin1("Split Horizontal");
    case Qt::PointingHandCursor: return QString::fromLatin1("Pointing Hand");
    case Qt::ForbiddenCursor: return QString::fromLatin1("Forbidden");
    case Qt::WhatsThisCursor: return QString::fromLatin1("Whats This");
    case Qt::BusyCursor: return QString::fromLatin1("Busy");
    default: return QString::null;
    }
}

QPixmap CursorProperty::cursorPixmap(int shape)
{
    switch (shape) {
    case Qt::ArrowCursor: return QPixmap(":/trolltech/formeditor/images/arrow.png");
    case Qt::UpArrowCursor: return QPixmap(":/trolltech/formeditor/images/uparrow.png");
    case Qt::CrossCursor: return QPixmap(":/trolltech/formeditor/images/cross.png");
    case Qt::WaitCursor: return QPixmap(":/trolltech/formeditor/images/wait.png");
    case Qt::IBeamCursor: return QPixmap(":/trolltech/formeditor/images/ibeam.png");
    case Qt::SizeVerCursor: return QPixmap(":/trolltech/formeditor/images/sizev.png");
    case Qt::SizeHorCursor: return QPixmap(":/trolltech/formeditor/images/sizeh.png");
    case Qt::SizeBDiagCursor: return QPixmap(":/trolltech/formeditor/images/sizef.png");
    case Qt::SizeFDiagCursor: return QPixmap(":/trolltech/formeditor/images/sizeb.png");
    case Qt::SizeAllCursor: return QPixmap(":/trolltech/formeditor/images/sizeall.png");
    case Qt::BlankCursor:
    {
        QBitmap cur = QBitmap(25, 25, 1);
        cur.setMask(cur);
        return cur;
    }
    case Qt::SplitVCursor: return QPixmap(":/trolltech/formeditor/images/vsplit.png");
    case Qt::SplitHCursor: return QPixmap(":/trolltech/formeditor/images/hsplit.png");
    case Qt::PointingHandCursor: return QPixmap(":/trolltech/formeditor/images/hand.png");
    case Qt::ForbiddenCursor: return QPixmap(":/trolltech/formeditor/images/no.png");
    case Qt::WhatsThisCursor: return QPixmap(":/trolltech/formeditor/images/whatsthis.png");
    case Qt::BusyCursor: return QPixmap(":/trolltech/formeditor/images/busy.png");
    default: return QPixmap();
    }
}

void CursorProperty::addCursor(QComboBox *combo, int shape) const
{
    combo->insertItem(-1, cursorPixmap(shape), cursorName(shape), shape);
}


// -------------------------------------------------------------------------
KeySequenceProperty::KeySequenceProperty(const QKeySequence &value, const QString &name)
    : AbstractProperty<QKeySequence>(value, name)
{
}

void KeySequenceProperty::setValue(const QVariant &value)
{
    m_value = qVariant_to<QKeySequence>(value);
}

QString KeySequenceProperty::toString() const
{
    return m_value;
}

QWidget *KeySequenceProperty::createEditor(QWidget *parent, const QObject *target, const char *receiver) const
{
    KeySequenceEditor *lineEdit = new KeySequenceEditor(parent);
    QObject::connect(lineEdit, SIGNAL(changed()), target, receiver);
    return lineEdit;
}

void KeySequenceProperty::updateEditorContents(QWidget *editor)
{
    if (KeySequenceEditor *lineEdit = qt_cast<KeySequenceEditor*>(editor)) {
        lineEdit->setKeySequence(m_value);
    }
}

void KeySequenceProperty::updateValue(QWidget *editor)
{
    if (KeySequenceEditor *lineEdit = qt_cast<KeySequenceEditor*>(editor)) {
        QKeySequence newValue = lineEdit->keySequence();

        if (newValue != m_value) {
            m_value = newValue;
            setChanged(true);
        }
    }
}



