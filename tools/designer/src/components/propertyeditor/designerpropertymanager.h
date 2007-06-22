#ifndef DESIGNERPROPERTYMANAGER_H
#define DESIGNERPROPERTYMANAGER_H

#include "qtvariantproperty.h"

typedef QPair<QString, uint> DesignerIntPair;
Q_DECLARE_METATYPE(DesignerIntPair)
typedef QList<DesignerIntPair> DesignerFlagList;
Q_DECLARE_METATYPE(DesignerFlagList)

class QDesignerFormEditorInterface;

namespace qdesigner_internal
{

class ResetWidget;

class TextPropertyEditor;
class PaletteEditorButton;
class GraphicsPropertyEditor;

class ResetDecorator : public QObject
{
    Q_OBJECT
public:
    ResetDecorator(QObject *parent = 0) : QObject(parent), m_spacing(-1) {}
    ~ResetDecorator();

    void connectPropertyManager(QtAbstractPropertyManager *manager);
    QWidget *editor(QWidget *subEditor, bool resetable, QtAbstractPropertyManager *manager, QtProperty *property,
                QWidget *parent);
    void disconnectPropertyManager(QtAbstractPropertyManager *manager);
    void setSpacing(int spacing);
signals:
    void resetProperty(QtProperty *property);
private slots:
    void slotPropertyChanged(QtProperty *property);
    void slotEditorDestroyed(QObject *object);
private:
    QMap<QtProperty *, QList<ResetWidget *> > m_createdResetWidgets;
    QMap<ResetWidget *, QtProperty *> m_resetWidgetToProperty;
    int m_spacing;
};

class DesignerPropertyManager : public QtVariantPropertyManager
{
    Q_OBJECT
public:
    DesignerPropertyManager(QDesignerFormEditorInterface *core, QObject *parent = 0);

    virtual QStringList attributes(int propertyType) const;
    virtual int attributeType(int propertyType, const QString &attribute) const;

    virtual QVariant attributeValue(const QtProperty *property, const QString &attribute);
    virtual bool isPropertyTypeSupported(int propertyType) const;
    virtual QVariant value(const QtProperty *property) const;
    virtual int valueType(int propertyType) const;
    virtual QString valueText(const QtProperty *property) const;
    virtual QIcon valueIcon(const QtProperty *property) const;

    static int designerFlagTypeId();
    static int designerFlagListTypeId();
    static int designerAlignmentTypeId();

public Q_SLOTS:
    virtual void setAttribute(QtProperty *property,
                const QString &attribute, const QVariant &value);
    virtual void setValue(QtProperty *property, const QVariant &value);
protected:
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private Q_SLOTS:
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotPropertyDestroyed(QtProperty *property);
private:
    QMap<QtProperty *, bool> m_resetMap;

    int bitCount(int mask) const;
    struct FlagData
    {
        FlagData() : val(0) {}
        uint val;
        DesignerFlagList flags;
        QList<uint> values;
    };
    QMap<QtProperty *, FlagData> m_flagValues;
    QMap<QtProperty *, QList<QtProperty *> > m_propertyToFlags;
    QMap<QtProperty *, QtProperty *> m_flagToProperty;

    int alignToIndexH(uint align) const;
    int alignToIndexV(uint align) const;
    uint indexHToAlign(int idx) const;
    uint indexVToAlign(int idx) const;
    QString indexHToString(int idx) const;
    QString indexVToString(int idx) const;
    QMap<QtProperty *, uint> m_alignValues;
    QMap<QtProperty *, QtProperty *> m_propertyToAlignH;
    QMap<QtProperty *, QtProperty *> m_propertyToAlignV;
    QMap<QtProperty *, QtProperty *> m_alignHToProperty;
    QMap<QtProperty *, QtProperty *> m_alignVToProperty;

    int antialiasingToIndex(QFont::StyleStrategy antialias) const;
    QFont::StyleStrategy indexToAntialiasing(int idx) const;
    QString indexAntialiasingToString(int idx) const;
    QMap<QtProperty *, QtProperty *> m_propertyToAntialiasing;
    QMap<QtProperty *, QtProperty *> m_antialiasingToProperty;

    struct PaletteData
    {
        QPalette val;
        QPalette superPalette;
    };
    QMap<QtProperty *, PaletteData> m_paletteValues;

    QMap<QtProperty *, QIcon> m_iconValues;

    QMap<QtProperty *, QPixmap> m_pixmapValues;

    QMap<QtProperty *, int> m_stringAttributes;

    bool m_changingSubValue;
    QDesignerFormEditorInterface *m_core;
};

class DesignerEditorFactory : public QtVariantEditorFactory
{
    Q_OBJECT
public:
    DesignerEditorFactory(QDesignerFormEditorInterface *core, QObject *parent = 0);
    ~DesignerEditorFactory();
    void setSpacing(int spacing);
signals:
    void resetProperty(QtProperty *property);
protected:
    void connectPropertyManager(QtVariantPropertyManager *manager);
    QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property,
                QWidget *parent);
    void disconnectPropertyManager(QtVariantPropertyManager *manager);
private slots:
    void slotEditorDestroyed(QObject *object);
    void slotAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &value);
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotStringTextChanged(const QString &value);
    void slotPaletteChanged(const QPalette &value);
    void slotIconChanged(const QIcon &value);
    void slotPixmapChanged(const QPixmap &value);
private:
    ResetDecorator *m_resetDecorator;
    bool m_changingPropertyValue;
    QDesignerFormEditorInterface *m_core;

    int m_spacing;

    QMap<QtProperty *, QList<TextPropertyEditor *> > m_stringPropertyToEditors;
    QMap<TextPropertyEditor *, QtProperty *> m_editorToStringProperty;
    QMap<QtProperty *, QList<PaletteEditorButton *> > m_palettePropertyToEditors;
    QMap<PaletteEditorButton *, QtProperty *> m_editorToPaletteProperty;
    QMap<QtProperty *, QList<GraphicsPropertyEditor *> > m_iconPropertyToEditors;
    QMap<GraphicsPropertyEditor *, QtProperty *> m_editorToIconProperty;
    QMap<QtProperty *, QList<GraphicsPropertyEditor *> > m_pixmapPropertyToEditors;
    QMap<GraphicsPropertyEditor *, QtProperty *> m_editorToPixmapProperty;
};

}

#endif

