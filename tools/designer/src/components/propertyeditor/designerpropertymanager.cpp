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

#include "designerpropertymanager.h"
#include "textpropertyeditor_p.h"
#include "graphicspropertyeditor.h"
#include "stylesheeteditor_p.h"
#include "richtexteditor_p.h"
#include "shared_enums_p.h"
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>
#include <QtCore/QFileInfo>
#include <QtGui/QLineEdit>
#include <QtCore/QUrl>
#include "paletteeditorbutton.h"
#include "stringlisteditorbutton.h"
#include <QtDesigner/QDesignerIconCacheInterface>
#include <private/qfont_p.h>
#include <qlonglongvalidator.h>

#include <iconloader_p.h>

#include <QtCore/QDebug>

class DesignerFlagPropertyType
{
};

Q_DECLARE_METATYPE(DesignerFlagPropertyType)

class DesignerAlignmentPropertyType
{
};

Q_DECLARE_METATYPE(DesignerAlignmentPropertyType)

namespace qdesigner_internal {


class TextEditor : public QWidget
{
    Q_OBJECT
public:
    TextEditor(QWidget *parent);

    TextPropertyValidationMode textPropertyValidationMode() const;
    void setTextPropertyValidationMode(TextPropertyValidationMode vm);

    void setSpacing(int spacing);
public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);

private slots:
    void buttonClicked();
private:
    TextPropertyEditor *m_editor;
    QToolButton *m_button;
    QHBoxLayout *m_layout;
};

TextEditor::TextEditor(QWidget *parent)
    : QWidget(parent)
{
    m_editor = new TextPropertyEditor(this);
    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(m_editor);
    m_button = new QToolButton(this);
    m_button->setText(tr("..."));
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    m_layout->addWidget(m_button);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    connect(m_editor, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
    connect(m_button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    m_button->setVisible(false);
}

void TextEditor::setSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
}

TextPropertyValidationMode TextEditor::textPropertyValidationMode() const
{
    return m_editor->textPropertyValidationMode();
}

void TextEditor::setTextPropertyValidationMode(TextPropertyValidationMode vm)
{
    m_editor->setTextPropertyValidationMode(vm);
    m_button->setVisible(vm == ValidationStyleSheet/* || vm == ValidationMultiLine*/);
}

void TextEditor::setText(const QString &text)
{
    m_editor->setText(text);
}

void TextEditor::buttonClicked()
{
    if (textPropertyValidationMode() == ValidationStyleSheet) {
        StyleSheetEditorDialog dlg(this);
        dlg.setText(m_editor->text());
        if (dlg.exec() == QDialog::Accepted) {
            const QString text = dlg.text();
            m_editor->setText(text);
            emit textChanged(text);
        }
        /*
    } else if (textPropertyValidationMode() == ValidationMultiLine) {
        RichTextEditorDialog dlg(this);
        dlg.editor()->setText(m_editor->text());
        if (dlg.exec() == QDialog::Accepted) {
            const QString text = dlg.editor()->text(Qt::RichText);
            m_editor->setText(text);
            emit textChanged(text);
        }
        */
    }
}

class ResetWidget : public QWidget
{
    Q_OBJECT
public:
    ResetWidget(QtProperty *property, QWidget *parent = 0);

    void setWidget(QWidget *widget);
    void setResetEnabled(bool enabled);
    void setValueText(const QString &text);
    void setValueIcon(const QIcon &icon);
    void setSpacing(int spacing);
signals:
    void resetProperty(QtProperty *property);
private slots:
    void slotClicked();
private:
    QtProperty *m_property;
    QLabel *m_textLabel;
    QLabel *m_iconLabel;
    QToolButton *m_button;
    int m_spacing;
};

ResetWidget::ResetWidget(QtProperty *property, QWidget *parent)
    : QWidget(parent), m_property(property)
{
    m_spacing = -1;
    m_textLabel = new QLabel(this);
    m_textLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    m_iconLabel = new QLabel(this);
    m_iconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_button = new QToolButton(this);
    m_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_button->setIcon(createIconSet(QLatin1String("resetproperty.png")));
    m_button->setIconSize(QSize(8,8));
    m_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    connect(m_button, SIGNAL(clicked()), this, SLOT(slotClicked()));
    QLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(m_spacing);
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);
    layout->addWidget(m_button);
    setFocusProxy(m_textLabel);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

void ResetWidget::setSpacing(int spacing)
{
    m_spacing = spacing;
    layout()->setSpacing(m_spacing);
}

void ResetWidget::setWidget(QWidget *widget)
{
    if (m_textLabel) {
        delete m_textLabel;
        m_textLabel = 0;
    }
    if (m_iconLabel) {
        delete m_iconLabel;
        m_iconLabel = 0;
    }
    delete layout();
    QLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(m_spacing);
    layout->addWidget(widget);
    layout->addWidget(m_button);
    setFocusProxy(widget);
}

void ResetWidget::setResetEnabled(bool enabled)
{
    m_button->setEnabled(enabled);
}

void ResetWidget::setValueText(const QString &text)
{
    if (m_textLabel)
        m_textLabel->setText(text);
}

void ResetWidget::setValueIcon(const QIcon &icon)
{
    QPixmap pix = icon.pixmap(QSize(16, 16));
    if (m_iconLabel) {
        m_iconLabel->setVisible(!pix.isNull());
        m_iconLabel->setPixmap(pix);
    }
}

void ResetWidget::slotClicked()
{
    emit resetProperty(m_property);
}


////////////////////////////////

DesignerPropertyManager::DesignerPropertyManager(QDesignerFormEditorInterface *core, QObject *parent)
    : QtVariantPropertyManager(parent), m_changingSubValue(false)
{
    m_core = core;
    m_createdFontProperty = 0;
    connect(this, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
            this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
    connect(this, SIGNAL(propertyDestroyed(QtProperty *)),
            this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

int DesignerPropertyManager::bitCount(int mask) const
{
    int count = 0;
    for (int i = 31; i >= 0; --i)
        count += ((mask >> i) & 1) ? 1 : 0;
    return count;
}

int DesignerPropertyManager::alignToIndexH(uint align) const
{
    if (align & Qt::AlignLeft)
        return 0;
    if (align & Qt::AlignHCenter)
        return 1;
    if (align & Qt::AlignRight)
        return 2;
    if (align & Qt::AlignJustify)
        return 3;
    return 0;
}

int DesignerPropertyManager::alignToIndexV(uint align) const
{
    if (align & Qt::AlignTop)
        return 0;
    if (align & Qt::AlignVCenter)
        return 1;
    if (align & Qt::AlignBottom)
        return 2;
    return 1;
}

uint DesignerPropertyManager::indexHToAlign(int idx) const
{
    switch (idx) {
        case 0: return Qt::AlignLeft;
        case 1: return Qt::AlignHCenter;
        case 2: return Qt::AlignRight;
        case 3: return Qt::AlignJustify;
        default: break;
    }
    return Qt::AlignLeft;
}

uint DesignerPropertyManager::indexVToAlign(int idx) const
{
    switch (idx) {
        case 0: return Qt::AlignTop;
        case 1: return Qt::AlignVCenter;
        case 2: return Qt::AlignBottom;
        default: break;
    }
    return Qt::AlignVCenter;
}

QString DesignerPropertyManager::indexHToString(int idx) const
{
    switch (idx) {
        case 0: return tr("AlignLeft");
        case 1: return tr("AlignHCenter");
        case 2: return tr("AlignRight");
        case 3: return tr("AlignJustify");
        default: break;
    }
    return tr("AlignLeft");
}

QString DesignerPropertyManager::indexVToString(int idx) const
{
    switch (idx) {
        case 0: return tr("AlignTop");
        case 1: return tr("AlignVCenter");
        case 2: return tr("AlignBottom");
        default: break;
    }
    return tr("AlignVCenter");
}

int DesignerPropertyManager::antialiasingToIndex(QFont::StyleStrategy antialias) const
{
    switch (antialias) {
        case QFont::PreferDefault:   return 0;
        case QFont::NoAntialias:     return 1;
        case QFont::PreferAntialias: return 2;
        default: break;
    }
    return 0;
}

QFont::StyleStrategy DesignerPropertyManager::indexToAntialiasing(int idx) const
{
    switch (idx) {
        case 0: return QFont::PreferDefault;
        case 1: return QFont::NoAntialias;
        case 2: return QFont::PreferAntialias;
    }
    return QFont::PreferDefault;
}

QString DesignerPropertyManager::indexAntialiasingToString(int idx) const
{
    switch (idx) {
        case 0: return tr("PreferDefault");
        case 1: return tr("NoAntialias");
        case 2: return tr("PreferAntialias");
    }
    return tr("PreferDefault");
}

unsigned DesignerPropertyManager::fontFlag(int idx) const
{
    switch (idx) {
        case 0: return QFontPrivate::Family;
        case 1: return QFontPrivate::Size;
        case 2: return QFontPrivate::Weight;
        case 3: return QFontPrivate::Style;
        case 4: return QFontPrivate::Underline;
        case 5: return QFontPrivate::StrikeOut;
        case 6: return QFontPrivate::Kerning;
        case 7: return QFontPrivate::StyleStrategy;
    }
    return 0;
}

void DesignerPropertyManager::slotValueChanged(QtProperty *property, const QVariant &value)
{
    if (m_changingSubValue)
        return;

    if (m_flagToProperty.contains(property)) {
        QtProperty *flagProperty = m_flagToProperty.value(property);
        QList<QtProperty *> subFlags = m_propertyToFlags.value(flagProperty);
        // flag changed
        bool subValue = variantProperty(property)->value().toBool();
        int subIndex = subFlags.indexOf(property);
        if (subIndex < 0)
            return;

        uint newValue = 0;

        m_changingSubValue = true;

        FlagData data = m_flagValues.value(flagProperty);
        QList<uint> values = data.values;
        // Compute new value, without including (additional) supermasks
        if (values.at(subIndex) == 0) {
            for (int i = 0; i < subFlags.count(); ++i) {
                QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
                subFlag->setValue(i == subIndex);
            }
        } else {
            if (subValue)
                newValue = values.at(subIndex); // value mask of subValue
            for (int i = 0; i < subFlags.count(); ++i) {
                QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
                if (subFlag->value().toBool() && bitCount(values.at(i)) == 1)
                    newValue |= values.at(i);
            }
            if (newValue == 0) {
                // Uncheck all items except 0-mask
                for (int i = 0; i < subFlags.count(); ++i) {
                    QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
                    subFlag->setValue(values.at(i) == 0);
                }
            } else if (newValue == data.val) {
                if (!subValue && bitCount(values.at(subIndex)) > 1) {
                    // We unchecked something, but the original value still holds
                    variantProperty(property)->setValue(true);
                }
            } else {
                // Make sure 0-mask is not selected
                for (int i = 0; i < subFlags.count(); ++i) {
                    QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
                    if (values.at(i) == 0)
                        subFlag->setValue(false);
                }
                // Check/uncheck proper masks
                if (subValue) {
                    // Make sure submasks and supermasks are selected
                    for (int i = 0; i < subFlags.count(); ++i) {
                        QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
                        if ((values.at(i) != 0) && ((values.at(i) & newValue) == values.at(i)) && !subFlag->value().toBool())
                            subFlag->setValue(true);
                    }
                } else {
                    // Make sure supermasks are not selected if they're no longer valid
                    for (int i = 0; i < subFlags.count(); ++i) {
                        QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
                        if (subFlag->value().toBool() && ((values.at(i) == subValue) || ((values.at(i) & newValue) != values.at(i))))
                            subFlag->setValue(false);
                    }
                }
            }
        }
        m_changingSubValue = false;
        data.val = newValue;
        QVariant v;
        qVariantSetValue(v, data.val);
        variantProperty(flagProperty)->setValue(v);
    } else if (m_alignHToProperty.contains(property)) {
        QtProperty *alignProperty = m_alignHToProperty.value(property);

        uint v = m_alignValues.value(alignProperty);
        uint newValue = indexHToAlign(value.toInt()) | indexVToAlign(alignToIndexV(v));
        if (v == newValue)
            return;

        variantProperty(alignProperty)->setValue(newValue);
    } else if (m_alignVToProperty.contains(property)) {
        QtProperty *alignProperty = m_alignVToProperty.value(property);

        uint v = m_alignValues.value(alignProperty);
        uint newValue = indexVToAlign(value.toInt()) | indexHToAlign(alignToIndexH(v));
        if (v == newValue)
            return;

        variantProperty(alignProperty)->setValue(newValue);
    } else if (m_antialiasingToProperty.contains(property)) {
        QtVariantProperty *fontProperty = variantProperty(m_antialiasingToProperty.value(property));

        QFont font = qVariantValue<QFont>(fontProperty->value());
        font.setStyleStrategy(indexToAntialiasing(value.toInt()));
        QVariant v;
        qVariantSetValue(v, font);

        fontProperty->setValue(v);
    }
}

void DesignerPropertyManager::slotPropertyDestroyed(QtProperty *property)
{
    if (m_flagToProperty.contains(property)) {
        QtProperty *flagProperty = m_flagToProperty[property];
        m_propertyToFlags[flagProperty].replace(m_propertyToFlags[flagProperty].indexOf(property), 0);
        m_flagToProperty.remove(property);
    } else if (m_alignHToProperty.contains(property)) {
        QtProperty *alignProperty = m_alignHToProperty[property];
        m_propertyToAlignH.remove(alignProperty);
        m_alignHToProperty.remove(property);
    } else if (m_alignVToProperty.contains(property)) {
        QtProperty *alignProperty = m_alignVToProperty[property];
        m_propertyToAlignV.remove(alignProperty);
        m_alignVToProperty.remove(property);
    }
}

QStringList DesignerPropertyManager::attributes(int propertyType) const
{
    if (!isPropertyTypeSupported(propertyType))
        return QStringList();

    QStringList list = QtVariantPropertyManager::attributes(propertyType);
    if (propertyType == designerFlagTypeId())
        list.append(QLatin1String("flags"));
    if (propertyType == QVariant::String)
        list.append(QLatin1String("validationMode"));
    if (propertyType == QVariant::Palette)
        list.append(QLatin1String("superPalette"));
    list.append(QLatin1String("resetable"));
    return list;
}

int DesignerPropertyManager::attributeType(int propertyType, const QString &attribute) const
{
    if (!isPropertyTypeSupported(propertyType))
        return 0;

    if (attribute == QLatin1String("resetable"))
        return QVariant::Bool;
    if (attribute == QLatin1String("flags") && propertyType == designerFlagTypeId())
        return designerFlagListTypeId();
    if (attribute == QLatin1String("validationMode") && propertyType == QVariant::String)
        return QVariant::Int;
    if (attribute == QLatin1String("superPalette") && propertyType == QVariant::Palette)
        return QVariant::Palette;
    return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant DesignerPropertyManager::attributeValue(const QtProperty *property, const QString &attribute)
{
    QtProperty *prop = const_cast<QtProperty *>(property);
    if (attribute == QLatin1String("resetable") && m_resetMap.contains(prop))
        return m_resetMap.value(prop);
    if (attribute == QLatin1String("flags") && m_flagValues.contains(prop)) {
        QVariant v;
        qVariantSetValue(v, m_flagValues.value(prop).flags);
        return v;
    }
    if (attribute == QLatin1String("validationMode") && m_stringAttributes.contains(prop)) {
        return m_stringAttributes.value(prop);
    }
    if (attribute == QLatin1String("superPalette") && m_paletteValues.contains(prop)) {
        return m_paletteValues.value(prop).superPalette;
    }
    return QtVariantPropertyManager::attributeValue(property, attribute);
}

void DesignerPropertyManager::setAttribute(QtProperty *property,
            const QString &attribute, const QVariant &value)
{
    if (attribute == QLatin1String("resetable") && m_resetMap.contains(property)) {
        if (value.userType() != QVariant::Bool)
            return;
        bool val = value.toBool();
        if (m_resetMap[property] == val)
            return;
        m_resetMap[property] = val;
        emit attributeChanged(variantProperty(property), attribute, value);
        return;
    } else if (attribute == QLatin1String("flags") && m_flagValues.contains(property)) {
        if (value.userType() != designerFlagListTypeId())
            return;

        DesignerFlagList flags = qVariantValue<DesignerFlagList>(value);
        FlagData data = m_flagValues.value(property);
        if (data.flags == flags)
            return;

        QListIterator<QtProperty *> itProp(m_propertyToFlags[property]);
        while (itProp.hasNext()) {
            QtProperty *prop = itProp.next();
            if (prop) {
                delete prop;
                m_flagToProperty.remove(prop);
            }
        }
        m_propertyToFlags[property].clear();

        QList<uint> values;

        QListIterator<QPair<QString, uint> > itFlag(flags);
        while (itFlag.hasNext()) {
            const QPair<QString, uint> pair = itFlag.next();
            QString flagName = pair.first;
            QtProperty *prop = addProperty(QVariant::Bool);
            prop->setPropertyName(flagName);
            property->addSubProperty(prop);
            m_propertyToFlags[property].append(prop);
            m_flagToProperty[prop] = property;
            values.append(pair.second);
        }

        data.val = 0;
        data.flags = flags;
        data.values = values;

        m_flagValues[property] = data;

        QVariant v;
        qVariantSetValue(v, flags);
        emit attributeChanged(property, attribute, v);

        emit propertyChanged(property);
        emit valueChanged(property, data.val);
    } else if (attribute == QLatin1String("validationMode") && m_stringAttributes.contains(property)) {
        if (value.userType() != QVariant::Int)
            return;

        const int oldValue = m_stringAttributes.value(property);

        int newValue = value.toInt();

        if (oldValue == newValue)
            return;

        m_stringAttributes[property] = newValue;

        emit attributeChanged(property, attribute, newValue);
    } else if (attribute == QLatin1String("superPalette") && m_paletteValues.contains(property)) {
        if (value.userType() != QVariant::Palette)
            return;

        QPalette superPalette = qVariantValue<QPalette>(value);

        PaletteData data = m_paletteValues.value(property);
        if (data.superPalette == superPalette)
            return;

        data.superPalette = superPalette;
        // resolve here
        const uint mask = data.val.resolve();
        data.val = data.val.resolve(superPalette);
        data.val.resolve(mask);

        m_paletteValues[property] = data;

        QVariant v;
        qVariantSetValue(v, superPalette);
        emit attributeChanged(property, attribute, v);

        emit propertyChanged(property);
        emit valueChanged(property, data.val); // if resolve was done, this is also for consistency
    }
    QtVariantPropertyManager::setAttribute(property, attribute, value);
}

int DesignerPropertyManager::designerFlagTypeId()
{
    return qMetaTypeId<DesignerFlagPropertyType>();
}

int DesignerPropertyManager::designerFlagListTypeId()
{
    return qMetaTypeId<DesignerFlagList>();
}

int DesignerPropertyManager::designerAlignmentTypeId()
{
    return qMetaTypeId<DesignerAlignmentPropertyType>();
}

bool DesignerPropertyManager::isPropertyTypeSupported(int propertyType) const
{
    switch (propertyType) {
    case QVariant::Palette:
    case QVariant::Icon:
    case QVariant::Pixmap:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::Url:
    case QVariant::StringList:
        return true;
    default:
        break;
    }

    if (propertyType == designerFlagTypeId())
        return true;
    if (propertyType == designerAlignmentTypeId())
        return true;
    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

QString DesignerPropertyManager::valueText(const QtProperty *property) const
{
    if (m_flagValues.contains(const_cast<QtProperty *>(property))) {
        const FlagData data = m_flagValues.value(const_cast<QtProperty *>(property));
        const uint v = data.val;
        const QChar bar = QLatin1Char('|');
        QString valueStr;
        const QList<QPair<QString, uint> > flags = data.flags;
        const  QList<QPair<QString, uint> >::const_iterator fcend = flags.constEnd();
        for (QList<QPair<QString, uint> >::const_iterator it = flags.constBegin(); it != fcend; ++it) {
            const uint val = it->second;
            const bool checked = (val == 0) ? (v == 0) : ((val & v) == val);
            if (checked) {
                if (!valueStr.isEmpty())
                    valueStr += bar;
                valueStr += it->first;
            }
        }
        return valueStr;
    }
    if (m_alignValues.contains(const_cast<QtProperty *>(property))) {
        const uint v = m_alignValues.value(const_cast<QtProperty *>(property));
        return tr("%1, %2").arg(indexHToString(alignToIndexH(v))).arg(indexVToString(alignToIndexV(v)));
    }
    if (m_paletteValues.contains(const_cast<QtProperty *>(property))) {
        const PaletteData data = m_paletteValues.value(const_cast<QtProperty *>(property));
        const uint mask = data.val.resolve();
        if (mask)
            return tr("Customized (%n roles)", 0, bitCount(mask));
        static const QString inherited = tr("Inherited");
        return inherited;
    }
    if (m_iconValues.contains(const_cast<QtProperty *>(property))) {
        const QIcon v = m_iconValues.value(const_cast<QtProperty *>(property));
        const QString path = m_core->iconCache()->iconToFilePath(v);
        return QFileInfo(path).fileName();
    }
    if (m_pixmapValues.contains(const_cast<QtProperty *>(property))) {
        const QPixmap v = m_pixmapValues.value(const_cast<QtProperty *>(property));
        const QString path = m_core->iconCache()->pixmapToFilePath(v);
        return QFileInfo(path).fileName();
    }
    if (m_uintValues.contains(const_cast<QtProperty *>(property))) {
        return QString::number(m_uintValues.value(const_cast<QtProperty *>(property)));
    }
    if (m_longLongValues.contains(const_cast<QtProperty *>(property))) {
        return QString::number(m_longLongValues.value(const_cast<QtProperty *>(property)));
    }
    if (m_uLongLongValues.contains(const_cast<QtProperty *>(property))) {
        return QString::number(m_uLongLongValues.value(const_cast<QtProperty *>(property)));
    }
    if (m_urlValues.contains(const_cast<QtProperty *>(property))) {
        return m_urlValues.value(const_cast<QtProperty *>(property)).toString();
    }
    if (m_stringListValues.contains(const_cast<QtProperty *>(property))) {
        return m_stringListValues.value(const_cast<QtProperty *>(property)).join(QLatin1String("; "));
    }
    return QtVariantPropertyManager::valueText(property);
}

QIcon DesignerPropertyManager::valueIcon(const QtProperty *property) const
{
    if (m_iconValues.contains(const_cast<QtProperty *>(property))) {
        const QIcon v = m_iconValues.value(const_cast<QtProperty *>(property));
        if (v.isNull())
            return emptyIcon();
        return v;
    }
    if (m_pixmapValues.contains(const_cast<QtProperty *>(property))) {
        const QPixmap v = m_pixmapValues.value(const_cast<QtProperty *>(property));
        if (v.isNull())
            return emptyIcon();
        return QIcon(v);
    }
    return QtVariantPropertyManager::valueIcon(property);
}

QVariant DesignerPropertyManager::value(const QtProperty *property) const
{
    if (m_flagValues.contains(const_cast<QtProperty *>(property)))
        return m_flagValues.value(const_cast<QtProperty *>(property)).val;
    if (m_alignValues.contains(const_cast<QtProperty *>(property)))
        return m_alignValues.value(const_cast<QtProperty *>(property));
    if (m_paletteValues.contains(const_cast<QtProperty *>(property)))
        return m_paletteValues.value(const_cast<QtProperty *>(property)).val;
    if (m_iconValues.contains(const_cast<QtProperty *>(property)))
        return m_iconValues.value(const_cast<QtProperty *>(property));
    if (m_pixmapValues.contains(const_cast<QtProperty *>(property)))
        return m_pixmapValues.value(const_cast<QtProperty *>(property));
    if (m_uintValues.contains(const_cast<QtProperty *>(property)))
        return m_uintValues.value(const_cast<QtProperty *>(property));
    if (m_longLongValues.contains(const_cast<QtProperty *>(property)))
        return m_longLongValues.value(const_cast<QtProperty *>(property));
    if (m_uLongLongValues.contains(const_cast<QtProperty *>(property)))
        return m_uLongLongValues.value(const_cast<QtProperty *>(property));
    if (m_urlValues.contains(const_cast<QtProperty *>(property)))
        return m_urlValues.value(const_cast<QtProperty *>(property));
    if (m_stringListValues.contains(const_cast<QtProperty *>(property)))
        return m_stringListValues.value(const_cast<QtProperty *>(property));
    return QtVariantPropertyManager::value(property);
}

int DesignerPropertyManager::valueType(int propertyType) const
{
    switch (propertyType) {
    case QVariant::Palette:
    case QVariant::Icon:
    case QVariant::Pixmap:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Url:
    case QVariant::StringList:
        return propertyType;
    default:
        break;
    }
    if (propertyType == designerFlagTypeId())
        return QVariant::UInt;
    if (propertyType == designerAlignmentTypeId())
        return QVariant::UInt;
    return QtVariantPropertyManager::valueType(propertyType);
}

void DesignerPropertyManager::setValue(QtProperty *property, const QVariant &value)
{
    if (m_flagValues.contains(property)) {
        if (value.type() != QVariant::UInt && !value.canConvert(QVariant::UInt))
            return;

        const uint v = value.toUInt();

        FlagData data = m_flagValues.value(property);
        if (data.val == v)
            return;

        // set Value

        const QList<uint> values = data.values;
        QList<QtProperty *> subFlags = m_propertyToFlags.value(property);
        for (int i = 0; i < subFlags.count(); ++i) {
            QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
            const uint val = values.at(i);
            const bool checked = (val == 0) ? (v == 0) : ((val & v) == val);
            subFlag->setValue(checked);
        }

        for (int i = 0; i < subFlags.count(); ++i) {
            QtVariantProperty *subFlag = variantProperty(subFlags.at(i));
            const uint val = values.at(i);
            const bool checked = (val == 0) ? (v == 0) : ((val & v) == val);
            bool enabled = true;
            if (val == 0) {
                if (checked)
                    enabled = false;
            } else if (bitCount(val) > 1) {
                // Disabled if all flags contained in the mask are checked
                uint currentMask = 0;
                for (int j = 0; j < subFlags.count(); ++j) {
                    QtVariantProperty *subFlag = variantProperty(subFlags.at(j));
                    if (bitCount(values.at(j)) == 1)
                        currentMask |= subFlag->value().toBool() ? values.at(j) : 0;
                }
                if ((currentMask & values.at(i)) == values.at(i))
                    enabled = false;
            }
            subFlag->setEnabled(enabled);
        }

        data.val = v;
        m_flagValues[property] = data;

        emit valueChanged(property, data.val);
        emit propertyChanged(property);

        return;
    } else if (m_alignValues.contains(property)) {
        if (value.type() != QVariant::UInt && !value.canConvert(QVariant::UInt))
            return;

        const uint v = value.toUInt();

        uint val = m_alignValues.value(property);

        if (val == v)
            return;

        QtVariantProperty *alignH = variantProperty(m_propertyToAlignH.value(property));
        QtVariantProperty *alignV = variantProperty(m_propertyToAlignV.value(property));

        if (alignH)
            alignH->setValue(alignToIndexH(v));
        if (alignV)
            alignV->setValue(alignToIndexV(v));

        m_alignValues[property] = v;

        emit valueChanged(property, v);
        emit propertyChanged(property);

        return;
    } else if (m_propertyToFontSubProperties.contains(property)) {
        QMap<int, QtProperty *> subProperties = m_propertyToFontSubProperties.value(property);
        QFont font = qVariantValue<QFont>(value);
        unsigned mask = font.resolve();
        QMapIterator<int, QtProperty *> itSub(subProperties);
        int index = 0;
        while (itSub.hasNext()) {
            unsigned flag = fontFlag(index);
            QtProperty *fontSubProperty = itSub.next().value();
            fontSubProperty->setModified(mask & flag);
            ++index;
        }
        if (m_propertyToAntialiasing.contains(property)) {
            QFont font = qVariantValue<QFont>(value);
            QtVariantProperty *antialiasing = variantProperty(m_propertyToAntialiasing.value(property));
            if (antialiasing)
                antialiasing->setValue(antialiasingToIndex(font.styleStrategy()));
        }
    } else if (m_paletteValues.contains(property)) {
        if (value.type() != QVariant::Palette && !value.canConvert(QVariant::Palette))
            return;

        QPalette p = qVariantValue<QPalette>(value);

        PaletteData data = m_paletteValues.value(property);

        const uint mask = p.resolve();
        p = p.resolve(data.superPalette);
        p.resolve(mask);

        if (data.val == p && data.val.resolve() == p.resolve())
            return;

        data.val = p;
        m_paletteValues[property] = data;

        emit valueChanged(property, data.val);
        emit propertyChanged(property);

        return;
    } else if (m_iconValues.contains(property)) {
        if (value.type() != QVariant::Icon && !value.canConvert(QVariant::Icon))
            return;

        QIcon icon = qVariantValue<QIcon>(value);

        QIcon oldIcon = m_iconValues.value(property);
        if (icon.serialNumber() == oldIcon.serialNumber())
            return;

        m_iconValues[property] = icon;

        emit valueChanged(property, icon);
        emit propertyChanged(property);

        return;
    } else if (m_pixmapValues.contains(property)) {
        if (value.type() != QVariant::Pixmap && !value.canConvert(QVariant::Pixmap))
            return;

        QPixmap pixmap = qVariantValue<QPixmap>(value);

        QPixmap oldPixmap = m_pixmapValues.value(property);
        if (pixmap.serialNumber() == oldPixmap.serialNumber())
            return;

        m_pixmapValues[property] = pixmap;

        emit valueChanged(property, pixmap);
        emit propertyChanged(property);

        return;
    } else if (m_uintValues.contains(property)) {
        if (value.type() != QVariant::UInt && !value.canConvert(QVariant::UInt))
            return;

        uint v = value.toUInt(0);

        uint oldValue = m_uintValues.value(property);
        if (v == oldValue)
            return;

        m_uintValues[property] = v;

        emit valueChanged(property, v);
        emit propertyChanged(property);

        return;
    } else if (m_longLongValues.contains(property)) {
        if (value.type() != QVariant::LongLong && !value.canConvert(QVariant::LongLong))
            return;

        qlonglong v = value.toLongLong(0);

        qlonglong oldValue = m_longLongValues.value(property);
        if (v == oldValue)
            return;

        m_longLongValues[property] = v;

        emit valueChanged(property, v);
        emit propertyChanged(property);

        return;
    } else if (m_uLongLongValues.contains(property)) {
        if (value.type() != QVariant::ULongLong && !value.canConvert(QVariant::ULongLong))
            return;

        qulonglong v = value.toULongLong(0);

        qulonglong oldValue = m_uLongLongValues.value(property);
        if (v == oldValue)
            return;

        m_uLongLongValues[property] = v;

        emit valueChanged(property, v);
        emit propertyChanged(property);

        return;
    } else if (m_urlValues.contains(property)) {
        if (value.type() != QVariant::Url && !value.canConvert(QVariant::Url))
            return;

        QUrl v = value.toUrl();

        QUrl oldValue = m_urlValues.value(property);
        if (v == oldValue)
            return;

        m_urlValues[property] = v;

        emit valueChanged(property, v);
        emit propertyChanged(property);

        return;
    } else if (m_stringListValues.contains(property)) {
        if (value.type() != QVariant::StringList && !value.canConvert(QVariant::StringList))
            return;

        QStringList v = value.toStringList();

        QStringList oldValue = m_stringListValues.value(property);
        if (v == oldValue)
            return;

        m_stringListValues[property] = v;

        emit valueChanged(property, v);
        emit propertyChanged(property);

        return;
    }
    QtVariantPropertyManager::setValue(property, value);
}

void DesignerPropertyManager::initializeProperty(QtProperty *property)
{
    m_resetMap[property] = false;

    if (m_createdFontProperty) {
        m_propertyToFontSubProperties[m_createdFontProperty][m_lastSubFontIndex] = property;
        m_fontSubPropertyToFlag[property] = m_lastSubFontIndex;
        m_fontSubPropertyToProperty[property] = m_createdFontProperty;
        m_resetMap[property] = true;
        ++m_lastSubFontIndex;
    }

    if (propertyType(property) == designerFlagTypeId()) {
        m_flagValues[property] = FlagData();
        m_propertyToFlags[property] = QList<QtProperty *>();
    } else if (propertyType(property) == designerAlignmentTypeId()) {
        const uint align = Qt::AlignLeft | Qt::AlignVCenter;
        m_alignValues[property] = align;

        QtVariantProperty *alignH = addProperty(enumTypeId(), tr("Horizontal"));
        QStringList namesH;
        namesH << indexHToString(0) << indexHToString(1) << indexHToString(2) << indexHToString(3);
        alignH->setAttribute(QLatin1String("enumNames"), namesH);
        alignH->setValue(alignToIndexH(align));
        m_propertyToAlignH[property] = alignH;
        m_alignHToProperty[alignH] = property;
        property->addSubProperty(alignH);

        QtVariantProperty *alignV = addProperty(enumTypeId(), tr("Vertical"));
        QStringList namesV;
        namesV << indexVToString(0) << indexVToString(1) << indexVToString(2);
        alignV->setAttribute(QLatin1String("enumNames"), namesV);
        alignV->setValue(alignToIndexV(align));
        m_propertyToAlignV[property] = alignV;
        m_alignVToProperty[alignV] = property;
        property->addSubProperty(alignV);
    } else if (propertyType(property) == QVariant::String) {
        m_stringAttributes[property] = ValidationMultiLine;
    } else if (propertyType(property) == QVariant::Palette) {
        m_paletteValues[property] = PaletteData();
    } else if (propertyType(property) == QVariant::Icon) {
        m_iconValues[property] = QIcon();
    } else if (propertyType(property) == QVariant::Pixmap) {
        m_pixmapValues[property] = QPixmap();
    } else if (propertyType(property) == QVariant::Font) {
        m_createdFontProperty = property;
        m_lastSubFontIndex = 0;
    } else if (propertyType(property) == QVariant::UInt) {
        m_uintValues[property] = 0;
    } else if (propertyType(property) == QVariant::LongLong) {
        m_longLongValues[property] = 0;
    } else if (propertyType(property) == QVariant::ULongLong) {
        m_uLongLongValues[property] = 0;
    } else if (propertyType(property) == QVariant::Url) {
        m_urlValues[property] = 0;
    } else if (propertyType(property) == QVariant::StringList) {
        m_stringListValues[property] = QStringList();
    }

    QtVariantPropertyManager::initializeProperty(property);
    if (propertyType(property) == QVariant::Font) {
        QtVariantProperty *antialiasing = addProperty(DesignerPropertyManager::enumTypeId(), tr("Antialiasing"));
        QFont font = qVariantValue<QFont>(variantProperty(property)->value());
        QStringList names;
        names << indexAntialiasingToString(0) << indexAntialiasingToString(1) << indexAntialiasingToString(2);
        antialiasing->setAttribute(QLatin1String("enumNames"), names);
        antialiasing->setValue(antialiasingToIndex(font.styleStrategy()));
        property->addSubProperty(antialiasing);

        m_propertyToAntialiasing[property] = antialiasing;
        m_antialiasingToProperty[antialiasing] = property;
        m_createdFontProperty = 0;
    }
}

void DesignerPropertyManager::uninitializeProperty(QtProperty *property)
{
    m_resetMap.remove(property);

    QListIterator<QtProperty *> itProp(m_propertyToFlags[property]);
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop) {
            delete prop;
            m_flagToProperty.remove(prop);
        }
    }
    m_propertyToFlags.remove(property);
    m_flagValues.remove(property);

    QtProperty *alignH = m_propertyToAlignH.value(property);
    if (alignH) {
        delete alignH;
        m_alignHToProperty.remove(alignH);
    }
    QtProperty *alignV = m_propertyToAlignV.value(property);
    if (alignV) {
        delete alignV;
        m_alignVToProperty.remove(alignV);
    }
    m_propertyToAlignH.remove(property);
    m_propertyToAlignV.remove(property);

    m_stringAttributes.remove(property);

    QtProperty *antialiasing = m_propertyToAntialiasing.value(property);
    if (antialiasing) {
        delete antialiasing;
        m_antialiasingToProperty.remove(antialiasing);
    }
    m_propertyToAntialiasing.remove(property);

    m_paletteValues.remove(property);

    m_iconValues.remove(property);

    m_pixmapValues.remove(property);

    m_propertyToFontSubProperties.remove(property);
    m_fontSubPropertyToFlag.remove(property);
    m_fontSubPropertyToProperty.remove(property);

    m_uintValues.remove(property);
    m_longLongValues.remove(property);
    m_uLongLongValues.remove(property);
    m_urlValues.remove(property);
    m_stringListValues.remove(property);

    QtVariantPropertyManager::uninitializeProperty(property);
}

bool DesignerPropertyManager::resetFontSubProperty(QtProperty *property)
{
    if (!m_fontSubPropertyToProperty.contains(property))
        return false;

    QtVariantProperty *fontProperty = variantProperty(m_fontSubPropertyToProperty.value(property));

    QVariant v = fontProperty->value();
    QFont font = qvariant_cast<QFont>(v);
    unsigned mask = font.resolve();
    const unsigned flag = fontFlag(m_fontSubPropertyToFlag.value(property));

    mask &= ~flag;
    font.resolve(mask);
    qVariantSetValue(v, font);
    fontProperty->setValue(v);
    return true;
}

DesignerEditorFactory::DesignerEditorFactory(QDesignerFormEditorInterface *core, QObject *parent)
    : QtVariantEditorFactory(parent)
{
    m_core = core;
    m_spacing = -1;
    m_resetDecorator = new ResetDecorator(this);
    m_changingPropertyValue = false;
    connect(m_resetDecorator, SIGNAL(resetProperty(QtProperty *)), this, SIGNAL(resetProperty(QtProperty *)));
}

DesignerEditorFactory::~DesignerEditorFactory()
{
}

void DesignerEditorFactory::setSpacing(int spacing)
{
    m_spacing = spacing;
    m_resetDecorator->setSpacing(spacing);
}

void DesignerEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    m_resetDecorator->connectPropertyManager(manager);
    connect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
                this, SLOT(slotAttributeChanged(QtProperty *, const QString &, const QVariant &)));
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
    QtVariantEditorFactory::connectPropertyManager(manager);
}

void DesignerEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    m_resetDecorator->disconnectPropertyManager(manager);
    disconnect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
                this, SLOT(slotAttributeChanged(QtProperty *, const QString &, const QVariant &)));
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void DesignerEditorFactory::slotAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &value)
{
    QtVariantPropertyManager *manager = propertyManager(property);
    if (manager->propertyType(property) == QVariant::String && attribute == QLatin1String("validationMode")) {
        QList<TextEditor *> editors = m_stringPropertyToEditors.value(property);
        QListIterator<TextEditor *> it(editors);
        while (it.hasNext()) {
            TextEditor *editor = it.next();
            editor->setTextPropertyValidationMode(static_cast<TextPropertyValidationMode>(value.toInt()));
        }
    } else if (manager->propertyType(property) == QVariant::Palette && attribute == QLatin1String("superPalette")) {
        QList<PaletteEditorButton *> editors = m_palettePropertyToEditors.value(property);
        QListIterator<PaletteEditorButton *> it(editors);
        while (it.hasNext()) {
            PaletteEditorButton *editor = it.next();
            editor->setSuperPalette(qvariant_cast<QPalette>(value));
        }
    }
}

void DesignerEditorFactory::slotValueChanged(QtProperty *property, const QVariant &value)
{
    if (m_changingPropertyValue)
        return;

    QtVariantPropertyManager *manager = propertyManager(property);
    if (manager->propertyType(property) == QVariant::String) {
        QList<TextEditor *> editors = m_stringPropertyToEditors.value(property);
        QListIterator<TextEditor *> it(editors);
        while (it.hasNext()) {
            TextEditor *editor = it.next();
            editor->setText(value.toString());
        }
    } else if (manager->propertyType(property) == QVariant::Palette) {
        QList<PaletteEditorButton *> editors = m_palettePropertyToEditors.value(property);
        QListIterator<PaletteEditorButton *> it(editors);
        while (it.hasNext()) {
            PaletteEditorButton *editor = it.next();
            editor->setPalette(qvariant_cast<QPalette>(value));
        }
    } else if (manager->propertyType(property) == QVariant::Icon) {
        QList<GraphicsPropertyEditor *> editors = m_iconPropertyToEditors.value(property);
        QListIterator<GraphicsPropertyEditor *> it(editors);
        while (it.hasNext()) {
            GraphicsPropertyEditor *editor = it.next();
            editor->setIcon(qvariant_cast<QIcon>(value));
        }
    } else if (manager->propertyType(property) == QVariant::Pixmap) {
        QList<GraphicsPropertyEditor *> editors = m_pixmapPropertyToEditors.value(property);
        QListIterator<GraphicsPropertyEditor *> it(editors);
        while (it.hasNext()) {
            GraphicsPropertyEditor *editor = it.next();
            editor->setPixmap(qvariant_cast<QPixmap>(value));
        }
    } else if (manager->propertyType(property) == QVariant::UInt) {
        QList<QLineEdit *> editors = m_uintPropertyToEditors.value(property);
        QListIterator<QLineEdit *> it(editors);
        while (it.hasNext()) {
            QLineEdit *editor = it.next();
            editor->setText(QString::number(value.toUInt()));
        }
    } else if (manager->propertyType(property) == QVariant::LongLong) {
        QList<QLineEdit *> editors = m_longLongPropertyToEditors.value(property);
        QListIterator<QLineEdit *> it(editors);
        while (it.hasNext()) {
            QLineEdit *editor = it.next();
            editor->setText(QString::number(value.toLongLong()));
        }
    } else if (manager->propertyType(property) == QVariant::ULongLong) {
        QList<QLineEdit *> editors = m_uLongLongPropertyToEditors.value(property);
        QListIterator<QLineEdit *> it(editors);
        while (it.hasNext()) {
            QLineEdit *editor = it.next();
            editor->setText(QString::number(value.toULongLong()));
        }
    } else if (manager->propertyType(property) == QVariant::Url) {
        QList<QLineEdit *> editors = m_urlPropertyToEditors.value(property);
        QListIterator<QLineEdit *> it(editors);
        while (it.hasNext()) {
            QLineEdit *editor = it.next();
            editor->setText(value.toUrl().toString());
        }
    } else if (manager->propertyType(property) == QVariant::StringList) {
        QList<StringListEditorButton *> editors = m_stringListPropertyToEditors.value(property);
        QListIterator<StringListEditorButton *> it(editors);
        while (it.hasNext()) {
            StringListEditorButton *editor = it.next();
            editor->setStringList(value.toStringList());
        }
    }
}

QWidget *DesignerEditorFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property,
            QWidget *parent)
{
    QWidget *editor = 0;
    if (manager->propertyType(property) == QVariant::String) {
        TextEditor *ed = new TextEditor(parent);
        ed->setText(manager->value(property).toString());
        ed->setSpacing(m_spacing);
        ed->setTextPropertyValidationMode(static_cast<TextPropertyValidationMode>(manager->attributeValue(property, QLatin1String("validationMode")).toInt()));
        m_stringPropertyToEditors[property].append(ed);
        m_editorToStringProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(textChanged(const QString &)), this, SLOT(slotStringTextChanged(const QString &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::Palette) {
        PaletteEditorButton *ed = new PaletteEditorButton(m_core, qvariant_cast<QPalette>(manager->value(property)), parent);
        ed->setSuperPalette(qvariant_cast<QPalette>(manager->attributeValue(property, QLatin1String("superPalette"))));
        m_palettePropertyToEditors[property].append(ed);
        m_editorToPaletteProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(paletteChanged(const QPalette &)), this, SLOT(slotPaletteChanged(const QPalette &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::Icon) {
        GraphicsPropertyEditor *ed = new GraphicsPropertyEditor(m_core, qvariant_cast<QIcon>(manager->value(property)), parent);
        ed->setSpacing(m_spacing);
        m_iconPropertyToEditors[property].append(ed);
        m_editorToIconProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(iconChanged(const QIcon &)), this, SLOT(slotIconChanged(const QIcon &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::Pixmap) {
        GraphicsPropertyEditor *ed = new GraphicsPropertyEditor(m_core, qvariant_cast<QPixmap>(manager->value(property)), parent);
        ed->setSpacing(m_spacing);
        m_pixmapPropertyToEditors[property].append(ed);
        m_editorToPixmapProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(pixmapChanged(const QPixmap &)), this, SLOT(slotPixmapChanged(const QPixmap &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::UInt) {
        QLineEdit *ed = new QLineEdit(parent);
        ed->setValidator(new QULongLongValidator(0, UINT_MAX, ed));
        ed->setText(QString::number(manager->value(property).toUInt()));
        m_uintPropertyToEditors[property].append(ed);
        m_editorToUintProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(textChanged(const QString &)), this, SLOT(slotUintChanged(const QString &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::LongLong) {
        QLineEdit *ed = new QLineEdit(parent);
        ed->setValidator(new QLongLongValidator(ed));
        ed->setText(QString::number(manager->value(property).toLongLong()));
        m_longLongPropertyToEditors[property].append(ed);
        m_editorToLongLongProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(textChanged(const QString &)), this, SLOT(slotLongLongChanged(const QString &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::ULongLong) {
        QLineEdit *ed = new QLineEdit(parent);
        ed->setValidator(new QULongLongValidator(ed));
        ed->setText(QString::number(manager->value(property).toULongLong()));
        m_uLongLongPropertyToEditors[property].append(ed);
        m_editorToULongLongProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(textChanged(const QString &)), this, SLOT(slotULongLongChanged(const QString &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::Url) {
        QLineEdit *ed = new QLineEdit(parent);
        ed->setText(manager->value(property).toUrl().toString());
        m_urlPropertyToEditors[property].append(ed);
        m_editorToUrlProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(textChanged(const QString &)), this, SLOT(slotUrlChanged(const QString &)));
        editor = ed;
    } else if (manager->propertyType(property) == QVariant::StringList) {
        StringListEditorButton *ed = new StringListEditorButton(manager->value(property).toStringList(), parent);
        m_stringListPropertyToEditors[property].append(ed);
        m_editorToStringListProperty[ed] = property;
        connect(ed, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(ed, SIGNAL(stringListChanged(const QStringList &)), this, SLOT(slotStringListChanged(const QStringList &)));
        editor = ed;
    } else {
        editor = QtVariantEditorFactory::createEditor(manager, property, parent);
    }
    return m_resetDecorator->editor(editor,
            manager->variantProperty(property)->attributeValue(QLatin1String("resetable")).toBool(),
            manager, property, parent);
}

template <class Editor>
bool removeEditor(QObject *object,
                QMap<QtProperty *, QList<Editor> > *propertyToEditors,
                QMap<Editor, QtProperty *> *editorToProperty)
{
    if (!propertyToEditors)
        return false;
    if (!editorToProperty)
        return false;
    QMapIterator<Editor, QtProperty *> it(*editorToProperty);
    while (it.hasNext()) {
        Editor editor = it.next().key();
        if (editor == object) {
            QtProperty *prop = it.value();
            (*propertyToEditors)[prop].removeAll(editor);
            if ((*propertyToEditors)[prop].count() == 0)
                propertyToEditors->remove(prop);
            editorToProperty->remove(editor);
            return true;
        }
    }
    return false;
}

void DesignerEditorFactory::slotEditorDestroyed(QObject *object)
{
    if (removeEditor(object, &m_stringPropertyToEditors, &m_editorToStringProperty))
        return;
    if (removeEditor(object, &m_palettePropertyToEditors, &m_editorToPaletteProperty))
        return;
    if (removeEditor(object, &m_iconPropertyToEditors, &m_editorToIconProperty))
        return;
    if (removeEditor(object, &m_pixmapPropertyToEditors, &m_editorToPixmapProperty))
        return;
    if (removeEditor(object, &m_uintPropertyToEditors, &m_editorToUintProperty))
        return;
    if (removeEditor(object, &m_longLongPropertyToEditors, &m_editorToLongLongProperty))
        return;
    if (removeEditor(object, &m_uLongLongPropertyToEditors, &m_editorToULongLongProperty))
        return;
    if (removeEditor(object, &m_urlPropertyToEditors, &m_editorToUrlProperty))
        return;
    if (removeEditor(object, &m_stringListPropertyToEditors, &m_editorToStringListProperty))
        return;
}

template<class Editor>
bool updateManager(QtVariantEditorFactory *factory, bool *changingPropertyValue,
        const QMap<Editor, QtProperty *> &editorToProperty, QWidget *editor, const QVariant &value)
{
    if (!editor)
        return false;
    QMapIterator<Editor, QtProperty *> it(editorToProperty);
    while (it.hasNext()) {
        if (it.next().key() == editor) {
            QtProperty *prop = it.value();
            QtVariantPropertyManager *manager = factory->propertyManager(prop);
            *changingPropertyValue = true;
            manager->variantProperty(prop)->setValue(value);
            *changingPropertyValue = false;
            return true;
        }
    }
    return false;
}

void DesignerEditorFactory::slotUintChanged(const QString &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToUintProperty, qobject_cast<QWidget *>(sender()), value.toUInt());
}

void DesignerEditorFactory::slotLongLongChanged(const QString &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToLongLongProperty, qobject_cast<QWidget *>(sender()), value.toLongLong());
}

void DesignerEditorFactory::slotULongLongChanged(const QString &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToULongLongProperty, qobject_cast<QWidget *>(sender()), value.toULongLong());
}

void DesignerEditorFactory::slotUrlChanged(const QString &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToUrlProperty, qobject_cast<QWidget *>(sender()), QUrl(value));
}

void DesignerEditorFactory::slotStringTextChanged(const QString &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToStringProperty, qobject_cast<QWidget *>(sender()), value);
}

void DesignerEditorFactory::slotPaletteChanged(const QPalette &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToPaletteProperty, qobject_cast<QWidget *>(sender()), qVariantFromValue(value));
}

void DesignerEditorFactory::slotIconChanged(const QIcon &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToIconProperty, qobject_cast<QWidget *>(sender()), qVariantFromValue(value));
}

void DesignerEditorFactory::slotPixmapChanged(const QPixmap &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToPixmapProperty, qobject_cast<QWidget *>(sender()), qVariantFromValue(value));
}

void DesignerEditorFactory::slotStringListChanged(const QStringList &value)
{
    updateManager(this, &m_changingPropertyValue, m_editorToStringListProperty, qobject_cast<QWidget *>(sender()), qVariantFromValue(value));
}

ResetDecorator::~ResetDecorator()
{
    QList<ResetWidget *> editors = m_resetWidgetToProperty.keys();
    QListIterator<ResetWidget *> it(editors);
    while (it.hasNext())
        delete it.next();
}

void ResetDecorator::connectPropertyManager(QtAbstractPropertyManager *manager)
{
    connect(manager, SIGNAL(propertyChanged(QtProperty *)),
            this, SLOT(slotPropertyChanged(QtProperty *)));
}

void ResetDecorator::disconnectPropertyManager(QtAbstractPropertyManager *manager)
{
    disconnect(manager, SIGNAL(propertyChanged(QtProperty *)),
            this, SLOT(slotPropertyChanged(QtProperty *)));
}

void ResetDecorator::setSpacing(int spacing)
{
    m_spacing = spacing;
}

QWidget *ResetDecorator::editor(QWidget *subEditor, bool resetable, QtAbstractPropertyManager *manager, QtProperty *property,
            QWidget *parent)
{
    Q_UNUSED(manager)

    ResetWidget *resetWidget = 0;
    if (resetable) {
        resetWidget = new ResetWidget(property, parent);
        resetWidget->setSpacing(m_spacing);
        resetWidget->setResetEnabled(property->isModified());
        resetWidget->setValueText(property->valueText());
        resetWidget->setValueIcon(property->valueIcon());
        connect(resetWidget, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
        connect(resetWidget, SIGNAL(resetProperty(QtProperty *)), this, SIGNAL(resetProperty(QtProperty *)));
        m_createdResetWidgets[property].append(resetWidget);
        m_resetWidgetToProperty[resetWidget] = property;
    }
    if (subEditor) {
        if (resetWidget) {
            subEditor->setParent(resetWidget);
            resetWidget->setWidget(subEditor);
        }
    }
    if (resetWidget)
        return resetWidget;
    return subEditor;
}

void ResetDecorator::slotPropertyChanged(QtProperty *property)
{
    QMap<QtProperty *, QList<ResetWidget *> >::ConstIterator prIt = m_createdResetWidgets.constFind(property);
    if (prIt == m_createdResetWidgets.constEnd())
        return;

    const QList<ResetWidget *> editors = prIt.value();
    const QList<ResetWidget *>::ConstIterator cend = editors.constEnd();
    for (QList<ResetWidget *>::ConstIterator itEditor = editors.constBegin(); itEditor != cend; ++itEditor) {
        ResetWidget *widget = *itEditor;
        widget->setResetEnabled(property->isModified());
        widget->setValueText(property->valueText());
        widget->setValueIcon(property->valueIcon());
    }
}

void ResetDecorator::slotEditorDestroyed(QObject *object)
{
    const  QMap<ResetWidget *, QtProperty *>::ConstIterator rcend = m_resetWidgetToProperty.constEnd();
    for (QMap<ResetWidget *, QtProperty *>::ConstIterator itEditor =  m_resetWidgetToProperty.constBegin(); itEditor != rcend; ++itEditor) {
        if (itEditor.key() == object) {
            ResetWidget *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_resetWidgetToProperty.remove(editor);
            m_createdResetWidgets[property].removeAll(editor);
            if (m_createdResetWidgets[property].isEmpty())
                m_createdResetWidgets.remove(property);
            return;
        }
    }
}

}

#include "designerpropertymanager.moc"
