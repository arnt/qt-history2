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
#ifndef UI4_H
#define UI4_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QtXml/QDomDocument>

/*******************************************************************************
** Forward declarations
*/

class DomUI;
class DomIncludes;
class DomInclude;
class DomResources;
class DomResource;
class DomActionGroup;
class DomAction;
class DomActionRef;
class DomImages;
class DomImage;
class DomImageData;
class DomCustomWidgets;
class DomHeader;
class DomCustomWidget;
class DomProperties;
class DomPropertyData;
class DomSizePolicyData;
class DomLayoutDefault;
class DomLayoutFunction;
class DomTabStops;
class DomLayout;
class DomLayoutItem;
class DomRow;
class DomColumn;
class DomItem;
class DomWidget;
class DomSpacer;
class DomColor;
class DomColorGroup;
class DomPalette;
class DomFont;
class DomPoint;
class DomRect;
class DomSizePolicy;
class DomSize;
class DomDate;
class DomTime;
class DomDateTime;
class DomStringList;
class DomResourcePixmap;
class DomString;
class DomProperty;

/*******************************************************************************
** Declarations
*/

class DomUI {
public:
    DomUI();
    ~DomUI();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeVersion() { return m_has_attr_version; }
    inline QString attributeVersion() { return m_attr_version; }
    inline void setAttributeVersion(const QString& a) { m_attr_version = a; m_has_attr_version = true; }
    inline void clearAttributeVersion() { m_has_attr_version = false; }

    inline bool hasAttributeStdSetDef() { return m_has_attr_stdSetDef; }
    inline int attributeStdSetDef() { return m_attr_stdSetDef; }
    inline void setAttributeStdSetDef(int a) { m_attr_stdSetDef = a; m_has_attr_stdSetDef = true; }
    inline void clearAttributeStdSetDef() { m_has_attr_stdSetDef = false; }

    // child element accessors
    inline DomString* elementAuthor() { return m_author; }
    void setElementAuthor(DomString* a);

    inline QString elementComment() { return m_comment; }
    void setElementComment(const QString& a);

    inline DomString* elementExportMacro() { return m_exportMacro; }
    void setElementExportMacro(DomString* a);

    inline QString elementClass() { return m_class; }
    void setElementClass(const QString& a);

    inline DomWidget* elementWidget() { return m_widget; }
    void setElementWidget(DomWidget* a);

    inline DomLayoutDefault* elementLayoutDefault() { return m_layoutDefault; }
    void setElementLayoutDefault(DomLayoutDefault* a);

    inline DomLayoutFunction* elementLayoutFunction() { return m_layoutFunction; }
    void setElementLayoutFunction(DomLayoutFunction* a);

    inline QString elementPixmapFunction() { return m_pixmapFunction; }
    void setElementPixmapFunction(const QString& a);

    inline DomCustomWidgets* elementCustomWidgets() { return m_customWidgets; }
    void setElementCustomWidgets(DomCustomWidgets* a);

    inline DomTabStops* elementTabStops() { return m_tabStops; }
    void setElementTabStops(DomTabStops* a);

    inline DomImages* elementImages() { return m_images; }
    void setElementImages(DomImages* a);

    inline DomIncludes* elementIncludes() { return m_includes; }
    void setElementIncludes(DomIncludes* a);

    inline DomResources* elementResources() { return m_resources; }
    void setElementResources(DomResources* a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_version;
    bool m_has_attr_version;

    int m_attr_stdSetDef;
    bool m_has_attr_stdSetDef;

    // child element data
    DomString* m_author;
    QString m_comment;
    DomString* m_exportMacro;
    QString m_class;
    DomWidget* m_widget;
    DomLayoutDefault* m_layoutDefault;
    DomLayoutFunction* m_layoutFunction;
    QString m_pixmapFunction;
    DomCustomWidgets* m_customWidgets;
    DomTabStops* m_tabStops;
    DomImages* m_images;
    DomIncludes* m_includes;
    DomResources* m_resources;

    DomUI(const DomUI &other);
    void operator = (const DomUI&other);
};

class DomIncludes {
public:
    DomIncludes();
    ~DomIncludes();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomInclude*> elementInclude() { return m_include; }
    void setElementInclude(const QList<DomInclude*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomInclude*> m_include;

    DomIncludes(const DomIncludes &other);
    void operator = (const DomIncludes&other);
};

class DomInclude {
public:
    DomInclude();
    ~DomInclude();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeLocation() { return m_has_attr_location; }
    inline QString attributeLocation() { return m_attr_location; }
    inline void setAttributeLocation(const QString& a) { m_attr_location = a; m_has_attr_location = true; }
    inline void clearAttributeLocation() { m_has_attr_location = false; }

    inline bool hasAttributeImpldecl() { return m_has_attr_impldecl; }
    inline QString attributeImpldecl() { return m_attr_impldecl; }
    inline void setAttributeImpldecl(const QString& a) { m_attr_impldecl = a; m_has_attr_impldecl = true; }
    inline void clearAttributeImpldecl() { m_has_attr_impldecl = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_location;
    bool m_has_attr_location;

    QString m_attr_impldecl;
    bool m_has_attr_impldecl;

    // child element data

    DomInclude(const DomInclude &other);
    void operator = (const DomInclude&other);
};

class DomResources {
public:
    DomResources();
    ~DomResources();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    // child element accessors
    inline QList<DomResource*> elementInclude() { return m_include; }
    void setElementInclude(const QList<DomResource*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    // child element data
    QList<DomResource*> m_include;

    DomResources(const DomResources &other);
    void operator = (const DomResources&other);
};

class DomResource {
public:
    DomResource();
    ~DomResource();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeLocation() { return m_has_attr_location; }
    inline QString attributeLocation() { return m_attr_location; }
    inline void setAttributeLocation(const QString& a) { m_attr_location = a; m_has_attr_location = true; }
    inline void clearAttributeLocation() { m_has_attr_location = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_location;
    bool m_has_attr_location;

    // child element data

    DomResource(const DomResource &other);
    void operator = (const DomResource&other);
};

class DomActionGroup {
public:
    DomActionGroup();
    ~DomActionGroup();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    // child element accessors
    inline QList<DomAction*> elementAction() { return m_action; }
    void setElementAction(const QList<DomAction*>& a);

    inline QList<DomActionGroup*> elementActionGroup() { return m_actionGroup; }
    void setElementActionGroup(const QList<DomActionGroup*>& a);

    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

    inline QList<DomProperty*> elementAttribute() { return m_attribute; }
    void setElementAttribute(const QList<DomProperty*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    // child element data
    QList<DomAction*> m_action;
    QList<DomActionGroup*> m_actionGroup;
    QList<DomProperty*> m_property;
    QList<DomProperty*> m_attribute;

    DomActionGroup(const DomActionGroup &other);
    void operator = (const DomActionGroup&other);
};

class DomAction {
public:
    DomAction();
    ~DomAction();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    inline bool hasAttributeMenu() { return m_has_attr_menu; }
    inline QString attributeMenu() { return m_attr_menu; }
    inline void setAttributeMenu(const QString& a) { m_attr_menu = a; m_has_attr_menu = true; }
    inline void clearAttributeMenu() { m_has_attr_menu = false; }

    // child element accessors
    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

    inline QList<DomProperty*> elementAttribute() { return m_attribute; }
    void setElementAttribute(const QList<DomProperty*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    QString m_attr_menu;
    bool m_has_attr_menu;

    // child element data
    QList<DomProperty*> m_property;
    QList<DomProperty*> m_attribute;

    DomAction(const DomAction &other);
    void operator = (const DomAction&other);
};

class DomActionRef {
public:
    DomActionRef();
    ~DomActionRef();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    // child element data

    DomActionRef(const DomActionRef &other);
    void operator = (const DomActionRef&other);
};

class DomImages {
public:
    DomImages();
    ~DomImages();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomImage*> elementImage() { return m_image; }
    void setElementImage(const QList<DomImage*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomImage*> m_image;

    DomImages(const DomImages &other);
    void operator = (const DomImages&other);
};

class DomImage {
public:
    DomImage();
    ~DomImage();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    // child element accessors
    inline DomImageData* elementData() { return m_data; }
    void setElementData(DomImageData* a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    // child element data
    DomImageData* m_data;

    DomImage(const DomImage &other);
    void operator = (const DomImage&other);
};

class DomImageData {
public:
    DomImageData();
    ~DomImageData();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeFormat() { return m_has_attr_format; }
    inline QString attributeFormat() { return m_attr_format; }
    inline void setAttributeFormat(const QString& a) { m_attr_format = a; m_has_attr_format = true; }
    inline void clearAttributeFormat() { m_has_attr_format = false; }

    inline bool hasAttributeLength() { return m_has_attr_length; }
    inline int attributeLength() { return m_attr_length; }
    inline void setAttributeLength(int a) { m_attr_length = a; m_has_attr_length = true; }
    inline void clearAttributeLength() { m_has_attr_length = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_format;
    bool m_has_attr_format;

    int m_attr_length;
    bool m_has_attr_length;

    // child element data

    DomImageData(const DomImageData &other);
    void operator = (const DomImageData&other);
};

class DomCustomWidgets {
public:
    DomCustomWidgets();
    ~DomCustomWidgets();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomCustomWidget*> elementCustomWidget() { return m_customWidget; }
    void setElementCustomWidget(const QList<DomCustomWidget*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomCustomWidget*> m_customWidget;

    DomCustomWidgets(const DomCustomWidgets &other);
    void operator = (const DomCustomWidgets&other);
};

class DomHeader {
public:
    DomHeader();
    ~DomHeader();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeLocation() { return m_has_attr_location; }
    inline QString attributeLocation() { return m_attr_location; }
    inline void setAttributeLocation(const QString& a) { m_attr_location = a; m_has_attr_location = true; }
    inline void clearAttributeLocation() { m_has_attr_location = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_location;
    bool m_has_attr_location;

    // child element data

    DomHeader(const DomHeader &other);
    void operator = (const DomHeader&other);
};

class DomCustomWidget {
public:
    DomCustomWidget();
    ~DomCustomWidget();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QString elementClass() { return m_class; }
    void setElementClass(const QString& a);

    inline QString elementExtends() { return m_extends; }
    void setElementExtends(const QString& a);

    inline DomHeader* elementHeader() { return m_header; }
    void setElementHeader(DomHeader* a);

    inline DomSize* elementSizeHint() { return m_sizeHint; }
    void setElementSizeHint(DomSize* a);

    inline int elementContainer() { return m_container; }
    void setElementContainer(int a);

    inline DomSizePolicyData* elementSizePolicy() { return m_sizePolicy; }
    void setElementSizePolicy(DomSizePolicyData* a);

    inline DomString* elementPixmap() { return m_pixmap; }
    void setElementPixmap(DomString* a);

    inline DomProperties* elementProperties() { return m_properties; }
    void setElementProperties(DomProperties* a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QString m_class;
    QString m_extends;
    DomHeader* m_header;
    DomSize* m_sizeHint;
    int m_container;
    DomSizePolicyData* m_sizePolicy;
    DomString* m_pixmap;
    DomProperties* m_properties;

    DomCustomWidget(const DomCustomWidget &other);
    void operator = (const DomCustomWidget&other);
};

class DomProperties {
public:
    DomProperties();
    ~DomProperties();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomPropertyData*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomPropertyData*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomPropertyData*> m_property;

    DomProperties(const DomProperties &other);
    void operator = (const DomProperties&other);
};

class DomPropertyData {
public:
    DomPropertyData();
    ~DomPropertyData();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeType() { return m_has_attr_type; }
    inline QString attributeType() { return m_attr_type; }
    inline void setAttributeType(const QString& a) { m_attr_type = a; m_has_attr_type = true; }
    inline void clearAttributeType() { m_has_attr_type = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_type;
    bool m_has_attr_type;

    // child element data

    DomPropertyData(const DomPropertyData &other);
    void operator = (const DomPropertyData&other);
};

class DomSizePolicyData {
public:
    DomSizePolicyData();
    ~DomSizePolicyData();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementHorData() { return m_horData; }
    void setElementHorData(int a);

    inline int elementVerData() { return m_verData; }
    void setElementVerData(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_horData;
    int m_verData;

    DomSizePolicyData(const DomSizePolicyData &other);
    void operator = (const DomSizePolicyData&other);
};

class DomLayoutDefault {
public:
    DomLayoutDefault();
    ~DomLayoutDefault();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeSpacing() { return m_has_attr_spacing; }
    inline int attributeSpacing() { return m_attr_spacing; }
    inline void setAttributeSpacing(int a) { m_attr_spacing = a; m_has_attr_spacing = true; }
    inline void clearAttributeSpacing() { m_has_attr_spacing = false; }

    inline bool hasAttributeMargin() { return m_has_attr_margin; }
    inline int attributeMargin() { return m_attr_margin; }
    inline void setAttributeMargin(int a) { m_attr_margin = a; m_has_attr_margin = true; }
    inline void clearAttributeMargin() { m_has_attr_margin = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    int m_attr_spacing;
    bool m_has_attr_spacing;

    int m_attr_margin;
    bool m_has_attr_margin;

    // child element data

    DomLayoutDefault(const DomLayoutDefault &other);
    void operator = (const DomLayoutDefault&other);
};

class DomLayoutFunction {
public:
    DomLayoutFunction();
    ~DomLayoutFunction();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeSpacing() { return m_has_attr_spacing; }
    inline QString attributeSpacing() { return m_attr_spacing; }
    inline void setAttributeSpacing(const QString& a) { m_attr_spacing = a; m_has_attr_spacing = true; }
    inline void clearAttributeSpacing() { m_has_attr_spacing = false; }

    inline bool hasAttributeMargin() { return m_has_attr_margin; }
    inline QString attributeMargin() { return m_attr_margin; }
    inline void setAttributeMargin(const QString& a) { m_attr_margin = a; m_has_attr_margin = true; }
    inline void clearAttributeMargin() { m_has_attr_margin = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_spacing;
    bool m_has_attr_spacing;

    QString m_attr_margin;
    bool m_has_attr_margin;

    // child element data

    DomLayoutFunction(const DomLayoutFunction &other);
    void operator = (const DomLayoutFunction&other);
};

class DomTabStops {
public:
    DomTabStops();
    ~DomTabStops();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QStringList elementTabStop() { return m_tabStop; }
    void setElementTabStop(const QStringList& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QStringList m_tabStop;

    DomTabStops(const DomTabStops &other);
    void operator = (const DomTabStops&other);
};

class DomLayout {
public:
    DomLayout();
    ~DomLayout();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeClass() { return m_has_attr_class; }
    inline QString attributeClass() { return m_attr_class; }
    inline void setAttributeClass(const QString& a) { m_attr_class = a; m_has_attr_class = true; }
    inline void clearAttributeClass() { m_has_attr_class = false; }

    // child element accessors
    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

    inline QList<DomProperty*> elementAttribute() { return m_attribute; }
    void setElementAttribute(const QList<DomProperty*>& a);

    inline QList<DomLayoutItem*> elementItem() { return m_item; }
    void setElementItem(const QList<DomLayoutItem*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_class;
    bool m_has_attr_class;

    // child element data
    QList<DomProperty*> m_property;
    QList<DomProperty*> m_attribute;
    QList<DomLayoutItem*> m_item;

    DomLayout(const DomLayout &other);
    void operator = (const DomLayout&other);
};

class DomLayoutItem {
public:
    DomLayoutItem();
    ~DomLayoutItem();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeRow() { return m_has_attr_row; }
    inline int attributeRow() { return m_attr_row; }
    inline void setAttributeRow(int a) { m_attr_row = a; m_has_attr_row = true; }
    inline void clearAttributeRow() { m_has_attr_row = false; }

    inline bool hasAttributeColumn() { return m_has_attr_column; }
    inline int attributeColumn() { return m_attr_column; }
    inline void setAttributeColumn(int a) { m_attr_column = a; m_has_attr_column = true; }
    inline void clearAttributeColumn() { m_has_attr_column = false; }

    inline bool hasAttributeRowSpan() { return m_has_attr_rowSpan; }
    inline int attributeRowSpan() { return m_attr_rowSpan; }
    inline void setAttributeRowSpan(int a) { m_attr_rowSpan = a; m_has_attr_rowSpan = true; }
    inline void clearAttributeRowSpan() { m_has_attr_rowSpan = false; }

    inline bool hasAttributeColSpan() { return m_has_attr_colSpan; }
    inline int attributeColSpan() { return m_attr_colSpan; }
    inline void setAttributeColSpan(int a) { m_attr_colSpan = a; m_has_attr_colSpan = true; }
    inline void clearAttributeColSpan() { m_has_attr_colSpan = false; }

    // child element accessors
    enum Kind { Unknown = 0, Widget, Layout, Spacer };
    inline Kind kind() { return m_kind; }

    inline DomWidget* elementWidget() { return m_widget; }
    void setElementWidget(DomWidget* a);

    inline DomLayout* elementLayout() { return m_layout; }
    void setElementLayout(DomLayout* a);

    inline DomSpacer* elementSpacer() { return m_spacer; }
    void setElementSpacer(DomSpacer* a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    int m_attr_row;
    bool m_has_attr_row;

    int m_attr_column;
    bool m_has_attr_column;

    int m_attr_rowSpan;
    bool m_has_attr_rowSpan;

    int m_attr_colSpan;
    bool m_has_attr_colSpan;

    // child element data
    Kind m_kind;
    DomWidget* m_widget;
    DomLayout* m_layout;
    DomSpacer* m_spacer;

    DomLayoutItem(const DomLayoutItem &other);
    void operator = (const DomLayoutItem&other);
};

class DomRow {
public:
    DomRow();
    ~DomRow();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomProperty*> m_property;

    DomRow(const DomRow &other);
    void operator = (const DomRow&other);
};

class DomColumn {
public:
    DomColumn();
    ~DomColumn();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomProperty*> m_property;

    DomColumn(const DomColumn &other);
    void operator = (const DomColumn&other);
};

class DomItem {
public:
    DomItem();
    ~DomItem();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

    inline QList<DomItem*> elementItem() { return m_item; }
    void setElementItem(const QList<DomItem*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomProperty*> m_property;
    QList<DomItem*> m_item;

    DomItem(const DomItem &other);
    void operator = (const DomItem&other);
};

class DomWidget {
public:
    DomWidget();
    ~DomWidget();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeClass() { return m_has_attr_class; }
    inline QString attributeClass() { return m_attr_class; }
    inline void setAttributeClass(const QString& a) { m_attr_class = a; m_has_attr_class = true; }
    inline void clearAttributeClass() { m_has_attr_class = false; }

    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    // child element accessors
    inline QStringList elementClass() { return m_class; }
    void setElementClass(const QStringList& a);

    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

    inline QList<DomProperty*> elementAttribute() { return m_attribute; }
    void setElementAttribute(const QList<DomProperty*>& a);

    inline QList<DomRow*> elementRow() { return m_row; }
    void setElementRow(const QList<DomRow*>& a);

    inline QList<DomColumn*> elementColumn() { return m_column; }
    void setElementColumn(const QList<DomColumn*>& a);

    inline QList<DomItem*> elementItem() { return m_item; }
    void setElementItem(const QList<DomItem*>& a);

    inline QList<DomLayout*> elementLayout() { return m_layout; }
    void setElementLayout(const QList<DomLayout*>& a);

    inline QList<DomWidget*> elementWidget() { return m_widget; }
    void setElementWidget(const QList<DomWidget*>& a);

    inline QList<DomAction*> elementAction() { return m_action; }
    void setElementAction(const QList<DomAction*>& a);

    inline QList<DomActionGroup*> elementActionGroup() { return m_actionGroup; }
    void setElementActionGroup(const QList<DomActionGroup*>& a);

    inline QList<DomActionRef*> elementAddAction() { return m_addAction; }
    void setElementAddAction(const QList<DomActionRef*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_class;
    bool m_has_attr_class;

    QString m_attr_name;
    bool m_has_attr_name;

    // child element data
    QStringList m_class;
    QList<DomProperty*> m_property;
    QList<DomProperty*> m_attribute;
    QList<DomRow*> m_row;
    QList<DomColumn*> m_column;
    QList<DomItem*> m_item;
    QList<DomLayout*> m_layout;
    QList<DomWidget*> m_widget;
    QList<DomAction*> m_action;
    QList<DomActionGroup*> m_actionGroup;
    QList<DomActionRef*> m_addAction;

    DomWidget(const DomWidget &other);
    void operator = (const DomWidget&other);
};

class DomSpacer {
public:
    DomSpacer();
    ~DomSpacer();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    // child element accessors
    inline QList<DomProperty*> elementProperty() { return m_property; }
    void setElementProperty(const QList<DomProperty*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    // child element data
    QList<DomProperty*> m_property;

    DomSpacer(const DomSpacer &other);
    void operator = (const DomSpacer&other);
};

class DomColor {
public:
    DomColor();
    ~DomColor();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementRed() { return m_red; }
    void setElementRed(int a);

    inline int elementGreen() { return m_green; }
    void setElementGreen(int a);

    inline int elementBlue() { return m_blue; }
    void setElementBlue(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_red;
    int m_green;
    int m_blue;

    DomColor(const DomColor &other);
    void operator = (const DomColor&other);
};

class DomColorGroup {
public:
    DomColorGroup();
    ~DomColorGroup();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QList<DomColor*> elementColor() { return m_color; }
    void setElementColor(const QList<DomColor*>& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QList<DomColor*> m_color;

    DomColorGroup(const DomColorGroup &other);
    void operator = (const DomColorGroup&other);
};

class DomPalette {
public:
    DomPalette();
    ~DomPalette();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline DomColorGroup* elementActive() { return m_active; }
    void setElementActive(DomColorGroup* a);

    inline DomColorGroup* elementInactive() { return m_inactive; }
    void setElementInactive(DomColorGroup* a);

    inline DomColorGroup* elementDisabled() { return m_disabled; }
    void setElementDisabled(DomColorGroup* a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    DomColorGroup* m_active;
    DomColorGroup* m_inactive;
    DomColorGroup* m_disabled;

    DomPalette(const DomPalette &other);
    void operator = (const DomPalette&other);
};

class DomFont {
public:
    DomFont();
    ~DomFont();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QString elementFamily() { return m_family; }
    void setElementFamily(const QString& a);

    inline int elementPointSize() { return m_pointSize; }
    void setElementPointSize(int a);

    inline int elementWeight() { return m_weight; }
    void setElementWeight(int a);

    inline bool elementItalic() { return m_italic; }
    void setElementItalic(bool a);

    inline bool elementBold() { return m_bold; }
    void setElementBold(bool a);

    inline bool elementUnderline() { return m_underline; }
    void setElementUnderline(bool a);

    inline bool elementStrikeOut() { return m_strikeOut; }
    void setElementStrikeOut(bool a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QString m_family;
    int m_pointSize;
    int m_weight;
    bool m_italic;
    bool m_bold;
    bool m_underline;
    bool m_strikeOut;

    DomFont(const DomFont &other);
    void operator = (const DomFont&other);
};

class DomPoint {
public:
    DomPoint();
    ~DomPoint();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementX() { return m_x; }
    void setElementX(int a);

    inline int elementY() { return m_y; }
    void setElementY(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_x;
    int m_y;

    DomPoint(const DomPoint &other);
    void operator = (const DomPoint&other);
};

class DomRect {
public:
    DomRect();
    ~DomRect();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementX() { return m_x; }
    void setElementX(int a);

    inline int elementY() { return m_y; }
    void setElementY(int a);

    inline int elementWidth() { return m_width; }
    void setElementWidth(int a);

    inline int elementHeight() { return m_height; }
    void setElementHeight(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_x;
    int m_y;
    int m_width;
    int m_height;

    DomRect(const DomRect &other);
    void operator = (const DomRect&other);
};

class DomSizePolicy {
public:
    DomSizePolicy();
    ~DomSizePolicy();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementHSizeType() { return m_hSizeType; }
    void setElementHSizeType(int a);

    inline int elementVSizeType() { return m_vSizeType; }
    void setElementVSizeType(int a);

    inline int elementHorStretch() { return m_horStretch; }
    void setElementHorStretch(int a);

    inline int elementVerStretch() { return m_verStretch; }
    void setElementVerStretch(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_hSizeType;
    int m_vSizeType;
    int m_horStretch;
    int m_verStretch;

    DomSizePolicy(const DomSizePolicy &other);
    void operator = (const DomSizePolicy&other);
};

class DomSize {
public:
    DomSize();
    ~DomSize();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementWidth() { return m_width; }
    void setElementWidth(int a);

    inline int elementHeight() { return m_height; }
    void setElementHeight(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_width;
    int m_height;

    DomSize(const DomSize &other);
    void operator = (const DomSize&other);
};

class DomDate {
public:
    DomDate();
    ~DomDate();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementYear() { return m_year; }
    void setElementYear(int a);

    inline int elementMonth() { return m_month; }
    void setElementMonth(int a);

    inline int elementDay() { return m_day; }
    void setElementDay(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_year;
    int m_month;
    int m_day;

    DomDate(const DomDate &other);
    void operator = (const DomDate&other);
};

class DomTime {
public:
    DomTime();
    ~DomTime();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementHour() { return m_hour; }
    void setElementHour(int a);

    inline int elementMinute() { return m_minute; }
    void setElementMinute(int a);

    inline int elementSecond() { return m_second; }
    void setElementSecond(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_hour;
    int m_minute;
    int m_second;

    DomTime(const DomTime &other);
    void operator = (const DomTime&other);
};

class DomDateTime {
public:
    DomDateTime();
    ~DomDateTime();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline int elementHour() { return m_hour; }
    void setElementHour(int a);

    inline int elementMinute() { return m_minute; }
    void setElementMinute(int a);

    inline int elementSecond() { return m_second; }
    void setElementSecond(int a);

    inline int elementYear() { return m_year; }
    void setElementYear(int a);

    inline int elementMonth() { return m_month; }
    void setElementMonth(int a);

    inline int elementDay() { return m_day; }
    void setElementDay(int a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    int m_hour;
    int m_minute;
    int m_second;
    int m_year;
    int m_month;
    int m_day;

    DomDateTime(const DomDateTime &other);
    void operator = (const DomDateTime&other);
};

class DomStringList {
public:
    DomStringList();
    ~DomStringList();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    // child element accessors
    inline QStringList elementString() { return m_string; }
    void setElementString(const QStringList& a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    // child element data
    QStringList m_string;

    DomStringList(const DomStringList &other);
    void operator = (const DomStringList&other);
};

class DomResourcePixmap {
public:
    DomResourcePixmap();
    ~DomResourcePixmap();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeResource() { return m_has_attr_resource; }
    inline QString attributeResource() { return m_attr_resource; }
    inline void setAttributeResource(const QString& a) { m_attr_resource = a; m_has_attr_resource = true; }
    inline void clearAttributeResource() { m_has_attr_resource = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_resource;
    bool m_has_attr_resource;

    // child element data

    DomResourcePixmap(const DomResourcePixmap &other);
    void operator = (const DomResourcePixmap&other);
};

class DomString {
public:
    DomString();
    ~DomString();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeNotr() { return m_has_attr_notr; }
    inline QString attributeNotr() { return m_attr_notr; }
    inline void setAttributeNotr(const QString& a) { m_attr_notr = a; m_has_attr_notr = true; }
    inline void clearAttributeNotr() { m_has_attr_notr = false; }

    // child element accessors
private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_notr;
    bool m_has_attr_notr;

    // child element data

    DomString(const DomString &other);
    void operator = (const DomString&other);
};

class DomProperty {
public:
    DomProperty();
    ~DomProperty();

    void read(const QDomElement &node);
    QDomElement write(QDomDocument &doc, const QString &tagName = QString());
    inline QString text() const { return m_text; }
    inline void setText(const QString &s) { m_text = s; }

    // attribute accessors
    inline bool hasAttributeName() { return m_has_attr_name; }
    inline QString attributeName() { return m_attr_name; }
    inline void setAttributeName(const QString& a) { m_attr_name = a; m_has_attr_name = true; }
    inline void clearAttributeName() { m_has_attr_name = false; }

    inline bool hasAttributeStdset() { return m_has_attr_stdset; }
    inline int attributeStdset() { return m_attr_stdset; }
    inline void setAttributeStdset(int a) { m_attr_stdset = a; m_has_attr_stdset = true; }
    inline void clearAttributeStdset() { m_has_attr_stdset = false; }

    // child element accessors
    enum Kind { Unknown = 0, Bool, Color, Cstring, Cursor, Enum, Font, IconSet, Pixmap, Palette, Point, Rect, Set, SizePolicy, Size, String, StringList, Number, Date, Time, DateTime };
    inline Kind kind() { return m_kind; }

    inline QString elementBool() { return m_bool; }
    void setElementBool(const QString& a);

    inline DomColor* elementColor() { return m_color; }
    void setElementColor(DomColor* a);

    inline QString elementCstring() { return m_cstring; }
    void setElementCstring(const QString& a);

    inline int elementCursor() { return m_cursor; }
    void setElementCursor(int a);

    inline QString elementEnum() { return m_enum; }
    void setElementEnum(const QString& a);

    inline DomFont* elementFont() { return m_font; }
    void setElementFont(DomFont* a);

    inline DomResourcePixmap* elementIconSet() { return m_iconSet; }
    void setElementIconSet(DomResourcePixmap* a);

    inline DomResourcePixmap* elementPixmap() { return m_pixmap; }
    void setElementPixmap(DomResourcePixmap* a);

    inline DomPalette* elementPalette() { return m_palette; }
    void setElementPalette(DomPalette* a);

    inline DomPoint* elementPoint() { return m_point; }
    void setElementPoint(DomPoint* a);

    inline DomRect* elementRect() { return m_rect; }
    void setElementRect(DomRect* a);

    inline QString elementSet() { return m_set; }
    void setElementSet(const QString& a);

    inline DomSizePolicy* elementSizePolicy() { return m_sizePolicy; }
    void setElementSizePolicy(DomSizePolicy* a);

    inline DomSize* elementSize() { return m_size; }
    void setElementSize(DomSize* a);

    inline DomString* elementString() { return m_string; }
    void setElementString(DomString* a);

    inline DomStringList* elementStringList() { return m_stringList; }
    void setElementStringList(DomStringList* a);

    inline int elementNumber() { return m_number; }
    void setElementNumber(int a);

    inline DomDate* elementDate() { return m_date; }
    void setElementDate(DomDate* a);

    inline DomTime* elementTime() { return m_time; }
    void setElementTime(DomTime* a);

    inline DomDateTime* elementDateTime() { return m_dateTime; }
    void setElementDateTime(DomDateTime* a);

private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    int m_attr_stdset;
    bool m_has_attr_stdset;

    // child element data
    Kind m_kind;
    QString m_bool;
    DomColor* m_color;
    QString m_cstring;
    int m_cursor;
    QString m_enum;
    DomFont* m_font;
    DomResourcePixmap* m_iconSet;
    DomResourcePixmap* m_pixmap;
    DomPalette* m_palette;
    DomPoint* m_point;
    DomRect* m_rect;
    QString m_set;
    DomSizePolicy* m_sizePolicy;
    DomSize* m_size;
    DomString* m_string;
    DomStringList* m_stringList;
    int m_number;
    DomDate* m_date;
    DomTime* m_time;
    DomDateTime* m_dateTime;

    DomProperty(const DomProperty &other);
    void operator = (const DomProperty&other);
};


/*******************************************************************************
** Implementations
*/

inline void DomUI::clear(bool clear_all)
{
    delete m_author;
    delete m_exportMacro;
    delete m_widget;
    delete m_layoutDefault;
    delete m_layoutFunction;
    delete m_customWidgets;
    delete m_tabStops;
    delete m_images;
    delete m_includes;
    delete m_resources;

    if (clear_all) {
    m_text = QString();
    m_has_attr_version = false;
    m_has_attr_stdSetDef = false;
    m_attr_stdSetDef = 0;
    }

    m_author = 0;
    m_exportMacro = 0;
    m_widget = 0;
    m_layoutDefault = 0;
    m_layoutFunction = 0;
    m_customWidgets = 0;
    m_tabStops = 0;
    m_images = 0;
    m_includes = 0;
    m_resources = 0;
}

inline DomUI::DomUI()
{
    m_has_attr_version = false;
    m_has_attr_stdSetDef = false;
    m_attr_stdSetDef = 0;
    m_author = 0;
    m_exportMacro = 0;
    m_widget = 0;
    m_layoutDefault = 0;
    m_layoutFunction = 0;
    m_customWidgets = 0;
    m_tabStops = 0;
    m_images = 0;
    m_includes = 0;
    m_resources = 0;
}

inline DomUI::~DomUI()
{
    delete m_author;
    delete m_exportMacro;
    delete m_widget;
    delete m_layoutDefault;
    delete m_layoutFunction;
    delete m_customWidgets;
    delete m_tabStops;
    delete m_images;
    delete m_includes;
    delete m_resources;
}

inline void DomUI::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("version")))
        setAttributeVersion(node.attribute(QLatin1String("version")));
    if (node.hasAttribute(QLatin1String("stdsetdef")))
        setAttributeStdSetDef(node.attribute(QLatin1String("stdsetdef")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("author")) {
            DomString *v = new DomString();
            v->read(e);
            setElementAuthor(v);
            continue;
        }
        if (tag == QLatin1String("comment")) {
            setElementComment(e.text());
            continue;
        }
        if (tag == QLatin1String("exportmacro")) {
            DomString *v = new DomString();
            v->read(e);
            setElementExportMacro(v);
            continue;
        }
        if (tag == QLatin1String("class")) {
            setElementClass(e.text());
            continue;
        }
        if (tag == QLatin1String("widget")) {
            DomWidget *v = new DomWidget();
            v->read(e);
            setElementWidget(v);
            continue;
        }
        if (tag == QLatin1String("layoutdefault")) {
            DomLayoutDefault *v = new DomLayoutDefault();
            v->read(e);
            setElementLayoutDefault(v);
            continue;
        }
        if (tag == QLatin1String("layoutfunction")) {
            DomLayoutFunction *v = new DomLayoutFunction();
            v->read(e);
            setElementLayoutFunction(v);
            continue;
        }
        if (tag == QLatin1String("pixmapfunction")) {
            setElementPixmapFunction(e.text());
            continue;
        }
        if (tag == QLatin1String("customwidgets")) {
            DomCustomWidgets *v = new DomCustomWidgets();
            v->read(e);
            setElementCustomWidgets(v);
            continue;
        }
        if (tag == QLatin1String("tabstops")) {
            DomTabStops *v = new DomTabStops();
            v->read(e);
            setElementTabStops(v);
            continue;
        }
        if (tag == QLatin1String("images")) {
            DomImages *v = new DomImages();
            v->read(e);
            setElementImages(v);
            continue;
        }
        if (tag == QLatin1String("includes")) {
            DomIncludes *v = new DomIncludes();
            v->read(e);
            setElementIncludes(v);
            continue;
        }
        if (tag == QLatin1String("resources")) {
            DomResources *v = new DomResources();
            v->read(e);
            setElementResources(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomUI::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("ui") : tagName.toLower());

    QDomElement child;

    if (hasAttributeVersion())
        e.setAttribute(QLatin1String("version"), attributeVersion());

    if (hasAttributeStdSetDef())
        e.setAttribute(QLatin1String("stdsetdef"), attributeStdSetDef());

    if (m_author != 0)
        e.appendChild(m_author->write(doc, QLatin1String("author")));

    child = doc.createElement(QLatin1String("comment"));
    child.appendChild(doc.createTextNode(m_comment));
    e.appendChild(child);

    if (m_exportMacro != 0)
        e.appendChild(m_exportMacro->write(doc, QLatin1String("exportmacro")));

    child = doc.createElement(QLatin1String("class"));
    child.appendChild(doc.createTextNode(m_class));
    e.appendChild(child);

    if (m_widget != 0)
        e.appendChild(m_widget->write(doc, QLatin1String("widget")));

    if (m_layoutDefault != 0)
        e.appendChild(m_layoutDefault->write(doc, QLatin1String("layoutdefault")));

    if (m_layoutFunction != 0)
        e.appendChild(m_layoutFunction->write(doc, QLatin1String("layoutfunction")));

    child = doc.createElement(QLatin1String("pixmapfunction"));
    child.appendChild(doc.createTextNode(m_pixmapFunction));
    e.appendChild(child);

    if (m_customWidgets != 0)
        e.appendChild(m_customWidgets->write(doc, QLatin1String("customwidgets")));

    if (m_tabStops != 0)
        e.appendChild(m_tabStops->write(doc, QLatin1String("tabstops")));

    if (m_images != 0)
        e.appendChild(m_images->write(doc, QLatin1String("images")));

    if (m_includes != 0)
        e.appendChild(m_includes->write(doc, QLatin1String("includes")));

    if (m_resources != 0)
        e.appendChild(m_resources->write(doc, QLatin1String("resources")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomUI::setElementAuthor(DomString* a)
{
    delete m_author;
    m_author = a;
}

inline void DomUI::setElementComment(const QString& a)
{
    m_comment = a;
}

inline void DomUI::setElementExportMacro(DomString* a)
{
    delete m_exportMacro;
    m_exportMacro = a;
}

inline void DomUI::setElementClass(const QString& a)
{
    m_class = a;
}

inline void DomUI::setElementWidget(DomWidget* a)
{
    delete m_widget;
    m_widget = a;
}

inline void DomUI::setElementLayoutDefault(DomLayoutDefault* a)
{
    delete m_layoutDefault;
    m_layoutDefault = a;
}

inline void DomUI::setElementLayoutFunction(DomLayoutFunction* a)
{
    delete m_layoutFunction;
    m_layoutFunction = a;
}

inline void DomUI::setElementPixmapFunction(const QString& a)
{
    m_pixmapFunction = a;
}

inline void DomUI::setElementCustomWidgets(DomCustomWidgets* a)
{
    delete m_customWidgets;
    m_customWidgets = a;
}

inline void DomUI::setElementTabStops(DomTabStops* a)
{
    delete m_tabStops;
    m_tabStops = a;
}

inline void DomUI::setElementImages(DomImages* a)
{
    delete m_images;
    m_images = a;
}

inline void DomUI::setElementIncludes(DomIncludes* a)
{
    delete m_includes;
    m_includes = a;
}

inline void DomUI::setElementResources(DomResources* a)
{
    delete m_resources;
    m_resources = a;
}

inline void DomIncludes::clear(bool clear_all)
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomIncludes::DomIncludes()
{
}

inline DomIncludes::~DomIncludes()
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();
}

inline void DomIncludes::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("include")) {
            DomInclude *v = new DomInclude();
            v->read(e);
            m_include.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomIncludes::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("includes") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_include.size(); ++i) {
        DomInclude* v = m_include[i];
        QDomNode child = v->write(doc, QLatin1String("include"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomIncludes::setElementInclude(const QList<DomInclude*>& a)
{
    m_include = a;
}

inline void DomInclude::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_location = false;
    m_has_attr_impldecl = false;
    }

}

inline DomInclude::DomInclude()
{
    m_has_attr_location = false;
    m_has_attr_impldecl = false;
}

inline DomInclude::~DomInclude()
{
}

inline void DomInclude::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("location")))
        setAttributeLocation(node.attribute(QLatin1String("location")));
    if (node.hasAttribute(QLatin1String("impldecl")))
        setAttributeImpldecl(node.attribute(QLatin1String("impldecl")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomInclude::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("include") : tagName.toLower());

    QDomElement child;

    if (hasAttributeLocation())
        e.setAttribute(QLatin1String("location"), attributeLocation());

    if (hasAttributeImpldecl())
        e.setAttribute(QLatin1String("impldecl"), attributeImpldecl());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomResources::clear(bool clear_all)
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    }

}

inline DomResources::DomResources()
{
    m_has_attr_name = false;
}

inline DomResources::~DomResources()
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();
}

inline void DomResources::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("include")) {
            DomResource *v = new DomResource();
            v->read(e);
            m_include.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomResources::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("resources") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_include.size(); ++i) {
        DomResource* v = m_include[i];
        QDomNode child = v->write(doc, QLatin1String("include"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomResources::setElementInclude(const QList<DomResource*>& a)
{
    m_include = a;
}

inline void DomResource::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_location = false;
    }

}

inline DomResource::DomResource()
{
    m_has_attr_location = false;
}

inline DomResource::~DomResource()
{
}

inline void DomResource::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("location")))
        setAttributeLocation(node.attribute(QLatin1String("location")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomResource::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("resource") : tagName.toLower());

    QDomElement child;

    if (hasAttributeLocation())
        e.setAttribute(QLatin1String("location"), attributeLocation());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomActionGroup::clear(bool clear_all)
{
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    }

}

inline DomActionGroup::DomActionGroup()
{
    m_has_attr_name = false;
}

inline DomActionGroup::~DomActionGroup()
{
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
}

inline void DomActionGroup::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("action")) {
            DomAction *v = new DomAction();
            v->read(e);
            m_action.append(v);
            continue;
        }
        if (tag == QLatin1String("actiongroup")) {
            DomActionGroup *v = new DomActionGroup();
            v->read(e);
            m_actionGroup.append(v);
            continue;
        }
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomActionGroup::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("actiongroup") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_action.size(); ++i) {
        DomAction* v = m_action[i];
        QDomNode child = v->write(doc, QLatin1String("action"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_actionGroup.size(); ++i) {
        DomActionGroup* v = m_actionGroup[i];
        QDomNode child = v->write(doc, QLatin1String("actiongroup"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomActionGroup::setElementAction(const QList<DomAction*>& a)
{
    m_action = a;
}

inline void DomActionGroup::setElementActionGroup(const QList<DomActionGroup*>& a)
{
    m_actionGroup = a;
}

inline void DomActionGroup::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomActionGroup::setElementAttribute(const QList<DomProperty*>& a)
{
    m_attribute = a;
}

inline void DomAction::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    m_has_attr_menu = false;
    }

}

inline DomAction::DomAction()
{
    m_has_attr_name = false;
    m_has_attr_menu = false;
}

inline DomAction::~DomAction()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
}

inline void DomAction::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));
    if (node.hasAttribute(QLatin1String("menu")))
        setAttributeMenu(node.attribute(QLatin1String("menu")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomAction::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("action") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (hasAttributeMenu())
        e.setAttribute(QLatin1String("menu"), attributeMenu());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomAction::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomAction::setElementAttribute(const QList<DomProperty*>& a)
{
    m_attribute = a;
}

inline void DomActionRef::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    }

}

inline DomActionRef::DomActionRef()
{
    m_has_attr_name = false;
}

inline DomActionRef::~DomActionRef()
{
}

inline void DomActionRef::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomActionRef::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("actionref") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomImages::clear(bool clear_all)
{
    for (int i = 0; i < m_image.size(); ++i)
        delete m_image[i];
    m_image.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomImages::DomImages()
{
}

inline DomImages::~DomImages()
{
    for (int i = 0; i < m_image.size(); ++i)
        delete m_image[i];
    m_image.clear();
}

inline void DomImages::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("image")) {
            DomImage *v = new DomImage();
            v->read(e);
            m_image.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomImages::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("images") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_image.size(); ++i) {
        DomImage* v = m_image[i];
        QDomNode child = v->write(doc, QLatin1String("image"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomImages::setElementImage(const QList<DomImage*>& a)
{
    m_image = a;
}

inline void DomImage::clear(bool clear_all)
{
    delete m_data;

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    }

    m_data = 0;
}

inline DomImage::DomImage()
{
    m_has_attr_name = false;
    m_data = 0;
}

inline DomImage::~DomImage()
{
    delete m_data;
}

inline void DomImage::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("data")) {
            DomImageData *v = new DomImageData();
            v->read(e);
            setElementData(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomImage::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("image") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (m_data != 0)
        e.appendChild(m_data->write(doc, QLatin1String("data")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomImage::setElementData(DomImageData* a)
{
    delete m_data;
    m_data = a;
}

inline void DomImageData::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_format = false;
    m_has_attr_length = false;
    m_attr_length = 0;
    }

}

inline DomImageData::DomImageData()
{
    m_has_attr_format = false;
    m_has_attr_length = false;
    m_attr_length = 0;
}

inline DomImageData::~DomImageData()
{
}

inline void DomImageData::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("format")))
        setAttributeFormat(node.attribute(QLatin1String("format")));
    if (node.hasAttribute(QLatin1String("length")))
        setAttributeLength(node.attribute(QLatin1String("length")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomImageData::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("imagedata") : tagName.toLower());

    QDomElement child;

    if (hasAttributeFormat())
        e.setAttribute(QLatin1String("format"), attributeFormat());

    if (hasAttributeLength())
        e.setAttribute(QLatin1String("length"), attributeLength());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomCustomWidgets::clear(bool clear_all)
{
    for (int i = 0; i < m_customWidget.size(); ++i)
        delete m_customWidget[i];
    m_customWidget.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomCustomWidgets::DomCustomWidgets()
{
}

inline DomCustomWidgets::~DomCustomWidgets()
{
    for (int i = 0; i < m_customWidget.size(); ++i)
        delete m_customWidget[i];
    m_customWidget.clear();
}

inline void DomCustomWidgets::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("customwidget")) {
            DomCustomWidget *v = new DomCustomWidget();
            v->read(e);
            m_customWidget.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomCustomWidgets::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("customwidgets") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_customWidget.size(); ++i) {
        DomCustomWidget* v = m_customWidget[i];
        QDomNode child = v->write(doc, QLatin1String("customwidget"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomCustomWidgets::setElementCustomWidget(const QList<DomCustomWidget*>& a)
{
    m_customWidget = a;
}

inline void DomHeader::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_location = false;
    }

}

inline DomHeader::DomHeader()
{
    m_has_attr_location = false;
}

inline DomHeader::~DomHeader()
{
}

inline void DomHeader::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("location")))
        setAttributeLocation(node.attribute(QLatin1String("location")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomHeader::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("header") : tagName.toLower());

    QDomElement child;

    if (hasAttributeLocation())
        e.setAttribute(QLatin1String("location"), attributeLocation());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomCustomWidget::clear(bool clear_all)
{
    delete m_header;
    delete m_sizeHint;
    delete m_sizePolicy;
    delete m_pixmap;
    delete m_properties;

    if (clear_all) {
    m_text = QString();
    }

    m_header = 0;
    m_sizeHint = 0;
    m_container = 0;
    m_sizePolicy = 0;
    m_pixmap = 0;
    m_properties = 0;
}

inline DomCustomWidget::DomCustomWidget()
{
    m_header = 0;
    m_sizeHint = 0;
    m_container = 0;
    m_sizePolicy = 0;
    m_pixmap = 0;
    m_properties = 0;
}

inline DomCustomWidget::~DomCustomWidget()
{
    delete m_header;
    delete m_sizeHint;
    delete m_sizePolicy;
    delete m_pixmap;
    delete m_properties;
}

inline void DomCustomWidget::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("class")) {
            setElementClass(e.text());
            continue;
        }
        if (tag == QLatin1String("extends")) {
            setElementExtends(e.text());
            continue;
        }
        if (tag == QLatin1String("header")) {
            DomHeader *v = new DomHeader();
            v->read(e);
            setElementHeader(v);
            continue;
        }
        if (tag == QLatin1String("sizehint")) {
            DomSize *v = new DomSize();
            v->read(e);
            setElementSizeHint(v);
            continue;
        }
        if (tag == QLatin1String("container")) {
            setElementContainer(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("sizepolicy")) {
            DomSizePolicyData *v = new DomSizePolicyData();
            v->read(e);
            setElementSizePolicy(v);
            continue;
        }
        if (tag == QLatin1String("pixmap")) {
            DomString *v = new DomString();
            v->read(e);
            setElementPixmap(v);
            continue;
        }
        if (tag == QLatin1String("properties")) {
            DomProperties *v = new DomProperties();
            v->read(e);
            setElementProperties(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomCustomWidget::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("customwidget") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("class"));
    child.appendChild(doc.createTextNode(m_class));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("extends"));
    child.appendChild(doc.createTextNode(m_extends));
    e.appendChild(child);

    if (m_header != 0)
        e.appendChild(m_header->write(doc, QLatin1String("header")));

    if (m_sizeHint != 0)
        e.appendChild(m_sizeHint->write(doc, QLatin1String("sizehint")));

    child = doc.createElement(QLatin1String("container"));
    child.appendChild(doc.createTextNode(QString::number(m_container)));
    e.appendChild(child);

    if (m_sizePolicy != 0)
        e.appendChild(m_sizePolicy->write(doc, QLatin1String("sizepolicy")));

    if (m_pixmap != 0)
        e.appendChild(m_pixmap->write(doc, QLatin1String("pixmap")));

    if (m_properties != 0)
        e.appendChild(m_properties->write(doc, QLatin1String("properties")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomCustomWidget::setElementClass(const QString& a)
{
    m_class = a;
}

inline void DomCustomWidget::setElementExtends(const QString& a)
{
    m_extends = a;
}

inline void DomCustomWidget::setElementHeader(DomHeader* a)
{
    delete m_header;
    m_header = a;
}

inline void DomCustomWidget::setElementSizeHint(DomSize* a)
{
    delete m_sizeHint;
    m_sizeHint = a;
}

inline void DomCustomWidget::setElementContainer(int a)
{
    m_container = a;
}

inline void DomCustomWidget::setElementSizePolicy(DomSizePolicyData* a)
{
    delete m_sizePolicy;
    m_sizePolicy = a;
}

inline void DomCustomWidget::setElementPixmap(DomString* a)
{
    delete m_pixmap;
    m_pixmap = a;
}

inline void DomCustomWidget::setElementProperties(DomProperties* a)
{
    delete m_properties;
    m_properties = a;
}

inline void DomProperties::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomProperties::DomProperties()
{
}

inline DomProperties::~DomProperties()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

inline void DomProperties::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomPropertyData *v = new DomPropertyData();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomProperties::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("properties") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomPropertyData* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomProperties::setElementProperty(const QList<DomPropertyData*>& a)
{
    m_property = a;
}

inline void DomPropertyData::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_type = false;
    }

}

inline DomPropertyData::DomPropertyData()
{
    m_has_attr_type = false;
}

inline DomPropertyData::~DomPropertyData()
{
}

inline void DomPropertyData::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("type")))
        setAttributeType(node.attribute(QLatin1String("type")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomPropertyData::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("propertydata") : tagName.toLower());

    QDomElement child;

    if (hasAttributeType())
        e.setAttribute(QLatin1String("type"), attributeType());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomSizePolicyData::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_horData = 0;
    m_verData = 0;
}

inline DomSizePolicyData::DomSizePolicyData()
{
    m_horData = 0;
    m_verData = 0;
}

inline DomSizePolicyData::~DomSizePolicyData()
{
}

inline void DomSizePolicyData::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hordata")) {
            setElementHorData(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("verdata")) {
            setElementVerData(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomSizePolicyData::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("sizepolicydata") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("hordata"));
    child.appendChild(doc.createTextNode(QString::number(m_horData)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("verdata"));
    child.appendChild(doc.createTextNode(QString::number(m_verData)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomSizePolicyData::setElementHorData(int a)
{
    m_horData = a;
}

inline void DomSizePolicyData::setElementVerData(int a)
{
    m_verData = a;
}

inline void DomLayoutDefault::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_spacing = false;
    m_attr_spacing = 0;
    m_has_attr_margin = false;
    m_attr_margin = 0;
    }

}

inline DomLayoutDefault::DomLayoutDefault()
{
    m_has_attr_spacing = false;
    m_attr_spacing = 0;
    m_has_attr_margin = false;
    m_attr_margin = 0;
}

inline DomLayoutDefault::~DomLayoutDefault()
{
}

inline void DomLayoutDefault::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("spacing")))
        setAttributeSpacing(node.attribute(QLatin1String("spacing")).toInt());
    if (node.hasAttribute(QLatin1String("margin")))
        setAttributeMargin(node.attribute(QLatin1String("margin")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomLayoutDefault::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("layoutdefault") : tagName.toLower());

    QDomElement child;

    if (hasAttributeSpacing())
        e.setAttribute(QLatin1String("spacing"), attributeSpacing());

    if (hasAttributeMargin())
        e.setAttribute(QLatin1String("margin"), attributeMargin());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomLayoutFunction::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_spacing = false;
    m_has_attr_margin = false;
    }

}

inline DomLayoutFunction::DomLayoutFunction()
{
    m_has_attr_spacing = false;
    m_has_attr_margin = false;
}

inline DomLayoutFunction::~DomLayoutFunction()
{
}

inline void DomLayoutFunction::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("spacing")))
        setAttributeSpacing(node.attribute(QLatin1String("spacing")));
    if (node.hasAttribute(QLatin1String("margin")))
        setAttributeMargin(node.attribute(QLatin1String("margin")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomLayoutFunction::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("layoutfunction") : tagName.toLower());

    QDomElement child;

    if (hasAttributeSpacing())
        e.setAttribute(QLatin1String("spacing"), attributeSpacing());

    if (hasAttributeMargin())
        e.setAttribute(QLatin1String("margin"), attributeMargin());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomTabStops::clear(bool clear_all)
{
    m_tabStop.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomTabStops::DomTabStops()
{
}

inline DomTabStops::~DomTabStops()
{
    m_tabStop.clear();
}

inline void DomTabStops::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("tabstop")) {
            m_tabStop.append(e.text());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomTabStops::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("tabstops") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_tabStop.size(); ++i) {
        QString v = m_tabStop[i];
        QDomNode child = doc.createTextNode(v);
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomTabStops::setElementTabStop(const QStringList& a)
{
    m_tabStop = a;
}

inline void DomLayout::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();

    if (clear_all) {
    m_text = QString();
    m_has_attr_class = false;
    }

}

inline DomLayout::DomLayout()
{
    m_has_attr_class = false;
}

inline DomLayout::~DomLayout()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
}

inline void DomLayout::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("class")))
        setAttributeClass(node.attribute(QLatin1String("class")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
        if (tag == QLatin1String("item")) {
            DomLayoutItem *v = new DomLayoutItem();
            v->read(e);
            m_item.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomLayout::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("layout") : tagName.toLower());

    QDomElement child;

    if (hasAttributeClass())
        e.setAttribute(QLatin1String("class"), attributeClass());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_item.size(); ++i) {
        DomLayoutItem* v = m_item[i];
        QDomNode child = v->write(doc, QLatin1String("item"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomLayout::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomLayout::setElementAttribute(const QList<DomProperty*>& a)
{
    m_attribute = a;
}

inline void DomLayout::setElementItem(const QList<DomLayoutItem*>& a)
{
    m_item = a;
}

inline void DomLayoutItem::clear(bool clear_all)
{
    delete m_widget;
    delete m_layout;
    delete m_spacer;

    if (clear_all) {
    m_text = QString();
    m_has_attr_row = false;
    m_attr_row = 0;
    m_has_attr_column = false;
    m_attr_column = 0;
    m_has_attr_rowSpan = false;
    m_attr_rowSpan = 0;
    m_has_attr_colSpan = false;
    m_attr_colSpan = 0;
    }

    m_kind = Unknown;

    m_widget = 0;
    m_layout = 0;
    m_spacer = 0;
}

inline DomLayoutItem::DomLayoutItem()
{
    m_kind = Unknown;

    m_has_attr_row = false;
    m_attr_row = 0;
    m_has_attr_column = false;
    m_attr_column = 0;
    m_has_attr_rowSpan = false;
    m_attr_rowSpan = 0;
    m_has_attr_colSpan = false;
    m_attr_colSpan = 0;
    m_widget = 0;
    m_layout = 0;
    m_spacer = 0;
}

inline DomLayoutItem::~DomLayoutItem()
{
    delete m_widget;
    delete m_layout;
    delete m_spacer;
}

inline void DomLayoutItem::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("row")))
        setAttributeRow(node.attribute(QLatin1String("row")).toInt());
    if (node.hasAttribute(QLatin1String("column")))
        setAttributeColumn(node.attribute(QLatin1String("column")).toInt());
    if (node.hasAttribute(QLatin1String("rowspan")))
        setAttributeRowSpan(node.attribute(QLatin1String("rowspan")).toInt());
    if (node.hasAttribute(QLatin1String("colspan")))
        setAttributeColSpan(node.attribute(QLatin1String("colspan")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("widget")) {
            DomWidget *v = new DomWidget();
            v->read(e);
            setElementWidget(v);
            continue;
        }
        if (tag == QLatin1String("layout")) {
            DomLayout *v = new DomLayout();
            v->read(e);
            setElementLayout(v);
            continue;
        }
        if (tag == QLatin1String("spacer")) {
            DomSpacer *v = new DomSpacer();
            v->read(e);
            setElementSpacer(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomLayoutItem::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("layoutitem") : tagName.toLower());

    QDomElement child;

    if (hasAttributeRow())
        e.setAttribute(QLatin1String("row"), attributeRow());

    if (hasAttributeColumn())
        e.setAttribute(QLatin1String("column"), attributeColumn());

    if (hasAttributeRowSpan())
        e.setAttribute(QLatin1String("rowspan"), attributeRowSpan());

    if (hasAttributeColSpan())
        e.setAttribute(QLatin1String("colspan"), attributeColSpan());

    switch(kind()) {
        case Widget: {
            DomWidget* v = elementWidget();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("widget"));
                e.appendChild(child);
            }
            break;
        }
        case Layout: {
            DomLayout* v = elementLayout();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("layout"));
                e.appendChild(child);
            }
            break;
        }
        case Spacer: {
            DomSpacer* v = elementSpacer();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("spacer"));
                e.appendChild(child);
            }
            break;
        }
        default:
            break;
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomLayoutItem::setElementWidget(DomWidget* a)
{
    clear(false);
    m_kind = Widget;
    m_widget = a;
}

inline void DomLayoutItem::setElementLayout(DomLayout* a)
{
    clear(false);
    m_kind = Layout;
    m_layout = a;
}

inline void DomLayoutItem::setElementSpacer(DomSpacer* a)
{
    clear(false);
    m_kind = Spacer;
    m_spacer = a;
}

inline void DomRow::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomRow::DomRow()
{
}

inline DomRow::~DomRow()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

inline void DomRow::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomRow::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("row") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomRow::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomColumn::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomColumn::DomColumn()
{
}

inline DomColumn::~DomColumn()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

inline void DomColumn::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomColumn::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("column") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomColumn::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomItem::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomItem::DomItem()
{
}

inline DomItem::~DomItem()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
}

inline void DomItem::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("item")) {
            DomItem *v = new DomItem();
            v->read(e);
            m_item.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomItem::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("item") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_item.size(); ++i) {
        DomItem* v = m_item[i];
        QDomNode child = v->write(doc, QLatin1String("item"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomItem::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomItem::setElementItem(const QList<DomItem*>& a)
{
    m_item = a;
}

inline void DomWidget::clear(bool clear_all)
{
    m_class.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_row.size(); ++i)
        delete m_row[i];
    m_row.clear();
    for (int i = 0; i < m_column.size(); ++i)
        delete m_column[i];
    m_column.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
    for (int i = 0; i < m_layout.size(); ++i)
        delete m_layout[i];
    m_layout.clear();
    for (int i = 0; i < m_widget.size(); ++i)
        delete m_widget[i];
    m_widget.clear();
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_addAction.size(); ++i)
        delete m_addAction[i];
    m_addAction.clear();

    if (clear_all) {
    m_text = QString();
    m_has_attr_class = false;
    m_has_attr_name = false;
    }

}

inline DomWidget::DomWidget()
{
    m_has_attr_class = false;
    m_has_attr_name = false;
}

inline DomWidget::~DomWidget()
{
    m_class.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_row.size(); ++i)
        delete m_row[i];
    m_row.clear();
    for (int i = 0; i < m_column.size(); ++i)
        delete m_column[i];
    m_column.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
    for (int i = 0; i < m_layout.size(); ++i)
        delete m_layout[i];
    m_layout.clear();
    for (int i = 0; i < m_widget.size(); ++i)
        delete m_widget[i];
    m_widget.clear();
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_addAction.size(); ++i)
        delete m_addAction[i];
    m_addAction.clear();
}

inline void DomWidget::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("class")))
        setAttributeClass(node.attribute(QLatin1String("class")));
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("class")) {
            m_class.append(e.text());
            continue;
        }
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
        if (tag == QLatin1String("row")) {
            DomRow *v = new DomRow();
            v->read(e);
            m_row.append(v);
            continue;
        }
        if (tag == QLatin1String("column")) {
            DomColumn *v = new DomColumn();
            v->read(e);
            m_column.append(v);
            continue;
        }
        if (tag == QLatin1String("item")) {
            DomItem *v = new DomItem();
            v->read(e);
            m_item.append(v);
            continue;
        }
        if (tag == QLatin1String("layout")) {
            DomLayout *v = new DomLayout();
            v->read(e);
            m_layout.append(v);
            continue;
        }
        if (tag == QLatin1String("widget")) {
            DomWidget *v = new DomWidget();
            v->read(e);
            m_widget.append(v);
            continue;
        }
        if (tag == QLatin1String("action")) {
            DomAction *v = new DomAction();
            v->read(e);
            m_action.append(v);
            continue;
        }
        if (tag == QLatin1String("actiongroup")) {
            DomActionGroup *v = new DomActionGroup();
            v->read(e);
            m_actionGroup.append(v);
            continue;
        }
        if (tag == QLatin1String("addaction")) {
            DomActionRef *v = new DomActionRef();
            v->read(e);
            m_addAction.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomWidget::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("widget") : tagName.toLower());

    QDomElement child;

    if (hasAttributeClass())
        e.setAttribute(QLatin1String("class"), attributeClass());

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_class.size(); ++i) {
        QString v = m_class[i];
        QDomNode child = doc.createTextNode(v);
        e.appendChild(child);
    }
    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_row.size(); ++i) {
        DomRow* v = m_row[i];
        QDomNode child = v->write(doc, QLatin1String("row"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_column.size(); ++i) {
        DomColumn* v = m_column[i];
        QDomNode child = v->write(doc, QLatin1String("column"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_item.size(); ++i) {
        DomItem* v = m_item[i];
        QDomNode child = v->write(doc, QLatin1String("item"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_layout.size(); ++i) {
        DomLayout* v = m_layout[i];
        QDomNode child = v->write(doc, QLatin1String("layout"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_widget.size(); ++i) {
        DomWidget* v = m_widget[i];
        QDomNode child = v->write(doc, QLatin1String("widget"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_action.size(); ++i) {
        DomAction* v = m_action[i];
        QDomNode child = v->write(doc, QLatin1String("action"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_actionGroup.size(); ++i) {
        DomActionGroup* v = m_actionGroup[i];
        QDomNode child = v->write(doc, QLatin1String("actiongroup"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_addAction.size(); ++i) {
        DomActionRef* v = m_addAction[i];
        QDomNode child = v->write(doc, QLatin1String("addaction"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomWidget::setElementClass(const QStringList& a)
{
    m_class = a;
}

inline void DomWidget::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomWidget::setElementAttribute(const QList<DomProperty*>& a)
{
    m_attribute = a;
}

inline void DomWidget::setElementRow(const QList<DomRow*>& a)
{
    m_row = a;
}

inline void DomWidget::setElementColumn(const QList<DomColumn*>& a)
{
    m_column = a;
}

inline void DomWidget::setElementItem(const QList<DomItem*>& a)
{
    m_item = a;
}

inline void DomWidget::setElementLayout(const QList<DomLayout*>& a)
{
    m_layout = a;
}

inline void DomWidget::setElementWidget(const QList<DomWidget*>& a)
{
    m_widget = a;
}

inline void DomWidget::setElementAction(const QList<DomAction*>& a)
{
    m_action = a;
}

inline void DomWidget::setElementActionGroup(const QList<DomActionGroup*>& a)
{
    m_actionGroup = a;
}

inline void DomWidget::setElementAddAction(const QList<DomActionRef*>& a)
{
    m_addAction = a;
}

inline void DomSpacer::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    }

}

inline DomSpacer::DomSpacer()
{
    m_has_attr_name = false;
}

inline DomSpacer::~DomSpacer()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

inline void DomSpacer::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomSpacer::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("spacer") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomSpacer::setElementProperty(const QList<DomProperty*>& a)
{
    m_property = a;
}

inline void DomColor::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_red = 0;
    m_green = 0;
    m_blue = 0;
}

inline DomColor::DomColor()
{
    m_red = 0;
    m_green = 0;
    m_blue = 0;
}

inline DomColor::~DomColor()
{
}

inline void DomColor::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("red")) {
            setElementRed(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("green")) {
            setElementGreen(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("blue")) {
            setElementBlue(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomColor::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("color") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("red"));
    child.appendChild(doc.createTextNode(QString::number(m_red)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("green"));
    child.appendChild(doc.createTextNode(QString::number(m_green)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("blue"));
    child.appendChild(doc.createTextNode(QString::number(m_blue)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomColor::setElementRed(int a)
{
    m_red = a;
}

inline void DomColor::setElementGreen(int a)
{
    m_green = a;
}

inline void DomColor::setElementBlue(int a)
{
    m_blue = a;
}

inline void DomColorGroup::clear(bool clear_all)
{
    for (int i = 0; i < m_color.size(); ++i)
        delete m_color[i];
    m_color.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomColorGroup::DomColorGroup()
{
}

inline DomColorGroup::~DomColorGroup()
{
    for (int i = 0; i < m_color.size(); ++i)
        delete m_color[i];
    m_color.clear();
}

inline void DomColorGroup::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("color")) {
            DomColor *v = new DomColor();
            v->read(e);
            m_color.append(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomColorGroup::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("colorgroup") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_color.size(); ++i) {
        DomColor* v = m_color[i];
        QDomNode child = v->write(doc, QLatin1String("color"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomColorGroup::setElementColor(const QList<DomColor*>& a)
{
    m_color = a;
}

inline void DomPalette::clear(bool clear_all)
{
    delete m_active;
    delete m_inactive;
    delete m_disabled;

    if (clear_all) {
    m_text = QString();
    }

    m_active = 0;
    m_inactive = 0;
    m_disabled = 0;
}

inline DomPalette::DomPalette()
{
    m_active = 0;
    m_inactive = 0;
    m_disabled = 0;
}

inline DomPalette::~DomPalette()
{
    delete m_active;
    delete m_inactive;
    delete m_disabled;
}

inline void DomPalette::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("active")) {
            DomColorGroup *v = new DomColorGroup();
            v->read(e);
            setElementActive(v);
            continue;
        }
        if (tag == QLatin1String("inactive")) {
            DomColorGroup *v = new DomColorGroup();
            v->read(e);
            setElementInactive(v);
            continue;
        }
        if (tag == QLatin1String("disabled")) {
            DomColorGroup *v = new DomColorGroup();
            v->read(e);
            setElementDisabled(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomPalette::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("palette") : tagName.toLower());

    QDomElement child;

    if (m_active != 0)
        e.appendChild(m_active->write(doc, QLatin1String("active")));

    if (m_inactive != 0)
        e.appendChild(m_inactive->write(doc, QLatin1String("inactive")));

    if (m_disabled != 0)
        e.appendChild(m_disabled->write(doc, QLatin1String("disabled")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomPalette::setElementActive(DomColorGroup* a)
{
    delete m_active;
    m_active = a;
}

inline void DomPalette::setElementInactive(DomColorGroup* a)
{
    delete m_inactive;
    m_inactive = a;
}

inline void DomPalette::setElementDisabled(DomColorGroup* a)
{
    delete m_disabled;
    m_disabled = a;
}

inline void DomFont::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_pointSize = 0;
    m_weight = 0;
    m_italic = false;
    m_bold = false;
    m_underline = false;
    m_strikeOut = false;
}

inline DomFont::DomFont()
{
    m_pointSize = 0;
    m_weight = 0;
    m_italic = false;
    m_bold = false;
    m_underline = false;
    m_strikeOut = false;
}

inline DomFont::~DomFont()
{
}

inline void DomFont::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("family")) {
            setElementFamily(e.text());
            continue;
        }
        if (tag == QLatin1String("pointsize")) {
            setElementPointSize(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("weight")) {
            setElementWeight(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("italic")) {
            setElementItalic((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
        if (tag == QLatin1String("bold")) {
            setElementBold((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
        if (tag == QLatin1String("underline")) {
            setElementUnderline((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
        if (tag == QLatin1String("strikeout")) {
            setElementStrikeOut((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomFont::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("font") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("family"));
    child.appendChild(doc.createTextNode(m_family));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("pointsize"));
    child.appendChild(doc.createTextNode(QString::number(m_pointSize)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("weight"));
    child.appendChild(doc.createTextNode(QString::number(m_weight)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("italic"));
    child.appendChild(doc.createTextNode((m_italic ? QLatin1String("true") : QLatin1String("false"))));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("bold"));
    child.appendChild(doc.createTextNode((m_bold ? QLatin1String("true") : QLatin1String("false"))));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("underline"));
    child.appendChild(doc.createTextNode((m_underline ? QLatin1String("true") : QLatin1String("false"))));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("strikeout"));
    child.appendChild(doc.createTextNode((m_strikeOut ? QLatin1String("true") : QLatin1String("false"))));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomFont::setElementFamily(const QString& a)
{
    m_family = a;
}

inline void DomFont::setElementPointSize(int a)
{
    m_pointSize = a;
}

inline void DomFont::setElementWeight(int a)
{
    m_weight = a;
}

inline void DomFont::setElementItalic(bool a)
{
    m_italic = a;
}

inline void DomFont::setElementBold(bool a)
{
    m_bold = a;
}

inline void DomFont::setElementUnderline(bool a)
{
    m_underline = a;
}

inline void DomFont::setElementStrikeOut(bool a)
{
    m_strikeOut = a;
}

inline void DomPoint::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_x = 0;
    m_y = 0;
}

inline DomPoint::DomPoint()
{
    m_x = 0;
    m_y = 0;
}

inline DomPoint::~DomPoint()
{
}

inline void DomPoint::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomPoint::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("point") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("x"));
    child.appendChild(doc.createTextNode(QString::number(m_x)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("y"));
    child.appendChild(doc.createTextNode(QString::number(m_y)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomPoint::setElementX(int a)
{
    m_x = a;
}

inline void DomPoint::setElementY(int a)
{
    m_y = a;
}

inline void DomRect::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
}

inline DomRect::DomRect()
{
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
}

inline DomRect::~DomRect()
{
}

inline void DomRect::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("width")) {
            setElementWidth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("height")) {
            setElementHeight(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomRect::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("rect") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("x"));
    child.appendChild(doc.createTextNode(QString::number(m_x)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("y"));
    child.appendChild(doc.createTextNode(QString::number(m_y)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("width"));
    child.appendChild(doc.createTextNode(QString::number(m_width)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("height"));
    child.appendChild(doc.createTextNode(QString::number(m_height)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomRect::setElementX(int a)
{
    m_x = a;
}

inline void DomRect::setElementY(int a)
{
    m_y = a;
}

inline void DomRect::setElementWidth(int a)
{
    m_width = a;
}

inline void DomRect::setElementHeight(int a)
{
    m_height = a;
}

inline void DomSizePolicy::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_hSizeType = 0;
    m_vSizeType = 0;
    m_horStretch = 0;
    m_verStretch = 0;
}

inline DomSizePolicy::DomSizePolicy()
{
    m_hSizeType = 0;
    m_vSizeType = 0;
    m_horStretch = 0;
    m_verStretch = 0;
}

inline DomSizePolicy::~DomSizePolicy()
{
}

inline void DomSizePolicy::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hsizetype")) {
            setElementHSizeType(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("vsizetype")) {
            setElementVSizeType(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("horstretch")) {
            setElementHorStretch(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("verstretch")) {
            setElementVerStretch(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomSizePolicy::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("sizepolicy") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("hsizetype"));
    child.appendChild(doc.createTextNode(QString::number(m_hSizeType)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("vsizetype"));
    child.appendChild(doc.createTextNode(QString::number(m_vSizeType)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("horstretch"));
    child.appendChild(doc.createTextNode(QString::number(m_horStretch)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("verstretch"));
    child.appendChild(doc.createTextNode(QString::number(m_verStretch)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomSizePolicy::setElementHSizeType(int a)
{
    m_hSizeType = a;
}

inline void DomSizePolicy::setElementVSizeType(int a)
{
    m_vSizeType = a;
}

inline void DomSizePolicy::setElementHorStretch(int a)
{
    m_horStretch = a;
}

inline void DomSizePolicy::setElementVerStretch(int a)
{
    m_verStretch = a;
}

inline void DomSize::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_width = 0;
    m_height = 0;
}

inline DomSize::DomSize()
{
    m_width = 0;
    m_height = 0;
}

inline DomSize::~DomSize()
{
}

inline void DomSize::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("width")) {
            setElementWidth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("height")) {
            setElementHeight(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomSize::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("size") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("width"));
    child.appendChild(doc.createTextNode(QString::number(m_width)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("height"));
    child.appendChild(doc.createTextNode(QString::number(m_height)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomSize::setElementWidth(int a)
{
    m_width = a;
}

inline void DomSize::setElementHeight(int a)
{
    m_height = a;
}

inline void DomDate::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_year = 0;
    m_month = 0;
    m_day = 0;
}

inline DomDate::DomDate()
{
    m_year = 0;
    m_month = 0;
    m_day = 0;
}

inline DomDate::~DomDate()
{
}

inline void DomDate::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("year")) {
            setElementYear(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("month")) {
            setElementMonth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("day")) {
            setElementDay(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomDate::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("date") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("year"));
    child.appendChild(doc.createTextNode(QString::number(m_year)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("month"));
    child.appendChild(doc.createTextNode(QString::number(m_month)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("day"));
    child.appendChild(doc.createTextNode(QString::number(m_day)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomDate::setElementYear(int a)
{
    m_year = a;
}

inline void DomDate::setElementMonth(int a)
{
    m_month = a;
}

inline void DomDate::setElementDay(int a)
{
    m_day = a;
}

inline void DomTime::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_hour = 0;
    m_minute = 0;
    m_second = 0;
}

inline DomTime::DomTime()
{
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
}

inline DomTime::~DomTime()
{
}

inline void DomTime::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hour")) {
            setElementHour(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("minute")) {
            setElementMinute(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("second")) {
            setElementSecond(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomTime::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("time") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("hour"));
    child.appendChild(doc.createTextNode(QString::number(m_hour)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("minute"));
    child.appendChild(doc.createTextNode(QString::number(m_minute)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("second"));
    child.appendChild(doc.createTextNode(QString::number(m_second)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomTime::setElementHour(int a)
{
    m_hour = a;
}

inline void DomTime::setElementMinute(int a)
{
    m_minute = a;
}

inline void DomTime::setElementSecond(int a)
{
    m_second = a;
}

inline void DomDateTime::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    }

    m_hour = 0;
    m_minute = 0;
    m_second = 0;
    m_year = 0;
    m_month = 0;
    m_day = 0;
}

inline DomDateTime::DomDateTime()
{
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
    m_year = 0;
    m_month = 0;
    m_day = 0;
}

inline DomDateTime::~DomDateTime()
{
}

inline void DomDateTime::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hour")) {
            setElementHour(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("minute")) {
            setElementMinute(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("second")) {
            setElementSecond(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("year")) {
            setElementYear(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("month")) {
            setElementMonth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("day")) {
            setElementDay(e.text().toInt());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomDateTime::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("datetime") : tagName.toLower());

    QDomElement child;

    child = doc.createElement(QLatin1String("hour"));
    child.appendChild(doc.createTextNode(QString::number(m_hour)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("minute"));
    child.appendChild(doc.createTextNode(QString::number(m_minute)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("second"));
    child.appendChild(doc.createTextNode(QString::number(m_second)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("year"));
    child.appendChild(doc.createTextNode(QString::number(m_year)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("month"));
    child.appendChild(doc.createTextNode(QString::number(m_month)));
    e.appendChild(child);

    child = doc.createElement(QLatin1String("day"));
    child.appendChild(doc.createTextNode(QString::number(m_day)));
    e.appendChild(child);

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomDateTime::setElementHour(int a)
{
    m_hour = a;
}

inline void DomDateTime::setElementMinute(int a)
{
    m_minute = a;
}

inline void DomDateTime::setElementSecond(int a)
{
    m_second = a;
}

inline void DomDateTime::setElementYear(int a)
{
    m_year = a;
}

inline void DomDateTime::setElementMonth(int a)
{
    m_month = a;
}

inline void DomDateTime::setElementDay(int a)
{
    m_day = a;
}

inline void DomStringList::clear(bool clear_all)
{
    m_string.clear();

    if (clear_all) {
    m_text = QString();
    }

}

inline DomStringList::DomStringList()
{
}

inline DomStringList::~DomStringList()
{
    m_string.clear();
}

inline void DomStringList::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("string")) {
            m_string.append(e.text());
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomStringList::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("stringlist") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_string.size(); ++i) {
        QString v = m_string[i];
        QDomNode child = doc.createTextNode(v);
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomStringList::setElementString(const QStringList& a)
{
    m_string = a;
}

inline void DomResourcePixmap::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_resource = false;
    }

}

inline DomResourcePixmap::DomResourcePixmap()
{
    m_has_attr_resource = false;
}

inline DomResourcePixmap::~DomResourcePixmap()
{
}

inline void DomResourcePixmap::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("resource")))
        setAttributeResource(node.attribute(QLatin1String("resource")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomResourcePixmap::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("resourcepixmap") : tagName.toLower());

    QDomElement child;

    if (hasAttributeResource())
        e.setAttribute(QLatin1String("resource"), attributeResource());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomString::clear(bool clear_all)
{

    if (clear_all) {
    m_text = QString();
    m_has_attr_notr = false;
    }

}

inline DomString::DomString()
{
    m_has_attr_notr = false;
}

inline DomString::~DomString()
{
}

inline void DomString::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("notr")))
        setAttributeNotr(node.attribute(QLatin1String("notr")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text = node.text();
}

inline QDomElement DomString::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("string") : tagName.toLower());

    QDomElement child;

    if (hasAttributeNotr())
        e.setAttribute(QLatin1String("notr"), attributeNotr());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomProperty::clear(bool clear_all)
{
    delete m_color;
    delete m_font;
    delete m_iconSet;
    delete m_pixmap;
    delete m_palette;
    delete m_point;
    delete m_rect;
    delete m_sizePolicy;
    delete m_size;
    delete m_string;
    delete m_stringList;
    delete m_date;
    delete m_time;
    delete m_dateTime;

    if (clear_all) {
    m_text = QString();
    m_has_attr_name = false;
    m_has_attr_stdset = false;
    m_attr_stdset = 0;
    }

    m_kind = Unknown;

    m_color = 0;
    m_cursor = 0;
    m_font = 0;
    m_iconSet = 0;
    m_pixmap = 0;
    m_palette = 0;
    m_point = 0;
    m_rect = 0;
    m_sizePolicy = 0;
    m_size = 0;
    m_string = 0;
    m_stringList = 0;
    m_number = 0;
    m_date = 0;
    m_time = 0;
    m_dateTime = 0;
}

inline DomProperty::DomProperty()
{
    m_kind = Unknown;

    m_has_attr_name = false;
    m_has_attr_stdset = false;
    m_attr_stdset = 0;
    m_color = 0;
    m_cursor = 0;
    m_font = 0;
    m_iconSet = 0;
    m_pixmap = 0;
    m_palette = 0;
    m_point = 0;
    m_rect = 0;
    m_sizePolicy = 0;
    m_size = 0;
    m_string = 0;
    m_stringList = 0;
    m_number = 0;
    m_date = 0;
    m_time = 0;
    m_dateTime = 0;
}

inline DomProperty::~DomProperty()
{
    delete m_color;
    delete m_font;
    delete m_iconSet;
    delete m_pixmap;
    delete m_palette;
    delete m_point;
    delete m_rect;
    delete m_sizePolicy;
    delete m_size;
    delete m_string;
    delete m_stringList;
    delete m_date;
    delete m_time;
    delete m_dateTime;
}

inline void DomProperty::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));
    if (node.hasAttribute(QLatin1String("stdset")))
        setAttributeStdset(node.attribute(QLatin1String("stdset")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("bool")) {
            setElementBool(e.text());
            continue;
        }
        if (tag == QLatin1String("color")) {
            DomColor *v = new DomColor();
            v->read(e);
            setElementColor(v);
            continue;
        }
        if (tag == QLatin1String("cstring")) {
            setElementCstring(e.text());
            continue;
        }
        if (tag == QLatin1String("cursor")) {
            setElementCursor(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("enum")) {
            setElementEnum(e.text());
            continue;
        }
        if (tag == QLatin1String("font")) {
            DomFont *v = new DomFont();
            v->read(e);
            setElementFont(v);
            continue;
        }
        if (tag == QLatin1String("iconset")) {
            DomResourcePixmap *v = new DomResourcePixmap();
            v->read(e);
            setElementIconSet(v);
            continue;
        }
        if (tag == QLatin1String("pixmap")) {
            DomResourcePixmap *v = new DomResourcePixmap();
            v->read(e);
            setElementPixmap(v);
            continue;
        }
        if (tag == QLatin1String("palette")) {
            DomPalette *v = new DomPalette();
            v->read(e);
            setElementPalette(v);
            continue;
        }
        if (tag == QLatin1String("point")) {
            DomPoint *v = new DomPoint();
            v->read(e);
            setElementPoint(v);
            continue;
        }
        if (tag == QLatin1String("rect")) {
            DomRect *v = new DomRect();
            v->read(e);
            setElementRect(v);
            continue;
        }
        if (tag == QLatin1String("set")) {
            setElementSet(e.text());
            continue;
        }
        if (tag == QLatin1String("sizepolicy")) {
            DomSizePolicy *v = new DomSizePolicy();
            v->read(e);
            setElementSizePolicy(v);
            continue;
        }
        if (tag == QLatin1String("size")) {
            DomSize *v = new DomSize();
            v->read(e);
            setElementSize(v);
            continue;
        }
        if (tag == QLatin1String("string")) {
            DomString *v = new DomString();
            v->read(e);
            setElementString(v);
            continue;
        }
        if (tag == QLatin1String("stringlist")) {
            DomStringList *v = new DomStringList();
            v->read(e);
            setElementStringList(v);
            continue;
        }
        if (tag == QLatin1String("number")) {
            setElementNumber(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("date")) {
            DomDate *v = new DomDate();
            v->read(e);
            setElementDate(v);
            continue;
        }
        if (tag == QLatin1String("time")) {
            DomTime *v = new DomTime();
            v->read(e);
            setElementTime(v);
            continue;
        }
        if (tag == QLatin1String("datetime")) {
            DomDateTime *v = new DomDateTime();
            v->read(e);
            setElementDateTime(v);
            continue;
        }
    }

    m_text = node.text();
}

inline QDomElement DomProperty::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QLatin1String("property") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (hasAttributeStdset())
        e.setAttribute(QLatin1String("stdset"), attributeStdset());

    switch(kind()) {
        case Bool: {
            QDomElement child = doc.createElement("bool");
            QDomText text = doc.createTextNode(elementBool());
            child.appendChild(text);
            e.appendChild(child);
            break;
        }
        case Color: {
            DomColor* v = elementColor();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("color"));
                e.appendChild(child);
            }
            break;
        }
        case Cstring: {
            QDomElement child = doc.createElement("cstring");
            QDomText text = doc.createTextNode(elementCstring());
            child.appendChild(text);
            e.appendChild(child);
            break;
        }
        case Cursor: {
            QDomElement child = doc.createElement("cursor");
            QDomText text = doc.createTextNode(QString::number(elementCursor()));
            child.appendChild(text);
            e.appendChild(child);
            break;
        }
        case Enum: {
            QDomElement child = doc.createElement("enum");
            QDomText text = doc.createTextNode(elementEnum());
            child.appendChild(text);
            e.appendChild(child);
            break;
        }
        case Font: {
            DomFont* v = elementFont();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("font"));
                e.appendChild(child);
            }
            break;
        }
        case IconSet: {
            DomResourcePixmap* v = elementIconSet();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("iconset"));
                e.appendChild(child);
            }
            break;
        }
        case Pixmap: {
            DomResourcePixmap* v = elementPixmap();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("pixmap"));
                e.appendChild(child);
            }
            break;
        }
        case Palette: {
            DomPalette* v = elementPalette();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("palette"));
                e.appendChild(child);
            }
            break;
        }
        case Point: {
            DomPoint* v = elementPoint();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("point"));
                e.appendChild(child);
            }
            break;
        }
        case Rect: {
            DomRect* v = elementRect();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("rect"));
                e.appendChild(child);
            }
            break;
        }
        case Set: {
            QDomElement child = doc.createElement("set");
            QDomText text = doc.createTextNode(elementSet());
            child.appendChild(text);
            e.appendChild(child);
            break;
        }
        case SizePolicy: {
            DomSizePolicy* v = elementSizePolicy();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("sizepolicy"));
                e.appendChild(child);
            }
            break;
        }
        case Size: {
            DomSize* v = elementSize();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("size"));
                e.appendChild(child);
            }
            break;
        }
        case String: {
            DomString* v = elementString();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("string"));
                e.appendChild(child);
            }
            break;
        }
        case StringList: {
            DomStringList* v = elementStringList();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("stringlist"));
                e.appendChild(child);
            }
            break;
        }
        case Number: {
            QDomElement child = doc.createElement("number");
            QDomText text = doc.createTextNode(QString::number(elementNumber()));
            child.appendChild(text);
            e.appendChild(child);
            break;
        }
        case Date: {
            DomDate* v = elementDate();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("date"));
                e.appendChild(child);
            }
            break;
        }
        case Time: {
            DomTime* v = elementTime();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("time"));
                e.appendChild(child);
            }
            break;
        }
        case DateTime: {
            DomDateTime* v = elementDateTime();
            if (v != 0) {
                QDomElement child = v->write(doc, QLatin1String("datetime"));
                e.appendChild(child);
            }
            break;
        }
        default:
            break;
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

inline void DomProperty::setElementBool(const QString& a)
{
    clear(false);
    m_kind = Bool;
    m_bool = a;
}

inline void DomProperty::setElementColor(DomColor* a)
{
    clear(false);
    m_kind = Color;
    m_color = a;
}

inline void DomProperty::setElementCstring(const QString& a)
{
    clear(false);
    m_kind = Cstring;
    m_cstring = a;
}

inline void DomProperty::setElementCursor(int a)
{
    clear(false);
    m_kind = Cursor;
    m_cursor = a;
}

inline void DomProperty::setElementEnum(const QString& a)
{
    clear(false);
    m_kind = Enum;
    m_enum = a;
}

inline void DomProperty::setElementFont(DomFont* a)
{
    clear(false);
    m_kind = Font;
    m_font = a;
}

inline void DomProperty::setElementIconSet(DomResourcePixmap* a)
{
    clear(false);
    m_kind = IconSet;
    m_iconSet = a;
}

inline void DomProperty::setElementPixmap(DomResourcePixmap* a)
{
    clear(false);
    m_kind = Pixmap;
    m_pixmap = a;
}

inline void DomProperty::setElementPalette(DomPalette* a)
{
    clear(false);
    m_kind = Palette;
    m_palette = a;
}

inline void DomProperty::setElementPoint(DomPoint* a)
{
    clear(false);
    m_kind = Point;
    m_point = a;
}

inline void DomProperty::setElementRect(DomRect* a)
{
    clear(false);
    m_kind = Rect;
    m_rect = a;
}

inline void DomProperty::setElementSet(const QString& a)
{
    clear(false);
    m_kind = Set;
    m_set = a;
}

inline void DomProperty::setElementSizePolicy(DomSizePolicy* a)
{
    clear(false);
    m_kind = SizePolicy;
    m_sizePolicy = a;
}

inline void DomProperty::setElementSize(DomSize* a)
{
    clear(false);
    m_kind = Size;
    m_size = a;
}

inline void DomProperty::setElementString(DomString* a)
{
    clear(false);
    m_kind = String;
    m_string = a;
}

inline void DomProperty::setElementStringList(DomStringList* a)
{
    clear(false);
    m_kind = StringList;
    m_stringList = a;
}

inline void DomProperty::setElementNumber(int a)
{
    clear(false);
    m_kind = Number;
    m_number = a;
}

inline void DomProperty::setElementDate(DomDate* a)
{
    clear(false);
    m_kind = Date;
    m_date = a;
}

inline void DomProperty::setElementTime(DomTime* a)
{
    clear(false);
    m_kind = Time;
    m_time = a;
}

inline void DomProperty::setElementDateTime(DomDateTime* a)
{
    clear(false);
    m_kind = DateTime;
    m_dateTime = a;
}


#endif // UI4_H
