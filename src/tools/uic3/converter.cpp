/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ui3reader.h"
#include "parser.h"
#include "domtool.h"
#include "ui4.h"
#include "widgetinfo.h"
#include "globaldefs.h"

#include <qdebug.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <globaldefs.h>
#include <qregexp.h>

#include <stdio.h>
#include <stdlib.h>

#define CONVERT_PROPERTY(o, n) \
    do { \
        if (name == QLatin1String(o) \
                && !WidgetInfo::isValidProperty(className, (o)) \
                && WidgetInfo::isValidProperty(className, (n))) { \
            prop->setAttributeName((n)); \
        } \
    } while (0)

DomUI *Ui3Reader::generateUi4(const QDomElement &e)
{
    QDomElement n;
    QDomNodeList nl;

    QString objClass = getClassName(e);
    if (objClass.isEmpty())
        return 0;

    DomUI *ui = new DomUI;
    ui->setAttributeVersion("4.0");

    QStringList ui_tabstops;
    QStringList ui_include_hits;
    QList<DomWidget*> ui_toolbars;
    QList<DomWidget*> ui_menubars;
    QList<DomActionGroup*> ui_action_list;

    QList<DomCustomWidget*> ui_customwidget_list;
    for (n = e; !n.isNull(); n = n.nextSibling().toElement()) {
        QString tagName = n.tagName().toLower();

        if (tagName == QLatin1String("tabstops")) {
            QDomElement n2 = n.firstChild().toElement();
            while (!n2.isNull()) {
                if (n2.tagName().toLower() == "tabstop") {
                    QString name = n2.firstChild().toText().data();
                    ui_tabstops.append(name);
                }
                n2 = n2.nextSibling().toElement();
            }
        } else if (tagName == QLatin1String("includehints")) {
            QDomElement n2 = n.firstChild().toElement();
            while (!n2.isNull()) {
                if (n2.tagName().toLower() == "includehint") {
                    QString name = n2.firstChild().toText().data();
                    ui_include_hits.append(name);
                }
                n2 = n2.nextSibling().toElement();
            }
        } else if (tagName == QLatin1String("layoutdefaults")) {
            QString margin = n.attribute("margin");
            QString spacing = n.attribute("spacing");

            DomLayoutDefault *layoutDefault = new DomLayoutDefault();

            if (!margin.isEmpty())
                layoutDefault->setAttributeMargin(margin.toInt());

            if (!spacing.isEmpty())
                layoutDefault->setAttributeSpacing(spacing.toInt());

            ui->setElementLayoutDefault(layoutDefault);
        } else if (tagName == QLatin1String("images")) {
            QDomNodeList nl = n.elementsByTagName("image");
            QList<DomImage*> ui_image_list;
            for (int i=0; i<(int)nl.length(); i++) {
                QDomElement e = nl.item(i).toElement();

                QDomElement tmp = e.firstChild().toElement();
                if (tmp.tagName().toLower() != QLatin1String("data"))
                    continue;

                // create the image
                DomImage *img = new DomImage();
                img->setAttributeName(e.attribute("name"));

                // create the data
                DomImageData *data = new DomImageData();
                img->setElementData(data);

                if (tmp.hasAttribute("format"))
                    data->setAttributeFormat(tmp.attribute("format", "PNG"));

                if (tmp.hasAttribute("length"))
                    data->setAttributeLength(tmp.attribute("length").toInt());

                data->setText(tmp.firstChild().toText().data());

                ui_image_list.append(img);
            }

            if (ui_image_list.size()) {
                DomImages *images = new DomImages();
                images->setElementImage(ui_image_list);
                ui->setElementImages(images);
            }
        } else if (tagName == QLatin1String("actions")) {
            DomActionGroup *g = new DomActionGroup();
            g->read(n);
            ui_action_list.append(g);
            fixActionGroup(g);
        } else if (tagName == QLatin1String("toolbars")) {
            QDomElement n2 = n.firstChild().toElement();
            while (!n2.isNull()) {
                if (n2.tagName().toLower() == QLatin1String("toolbar")) {
                    DomWidget *tb = createWidget(n2, QLatin1String("QToolBar"));
                    ui_toolbars.append(tb);
                }
                n2 = n2.nextSibling().toElement();
            }
        } else if (tagName == QLatin1String("menubar")) {
            DomWidget *tb = createWidget(n, QLatin1String("QMenuBar"));
            ui_menubars.append(tb);
        } else if (tagName == QLatin1String("customwidgets")) {
            QDomElement n2 = n.firstChild().toElement();
            while (!n2.isNull()) {
                if (n2.tagName().toLower() == QLatin1String("customwidget")) {

                    DomCustomWidget *customWidget = new DomCustomWidget;
                    customWidget->read(n2);

                    QDomElement n3 = n2.firstChild().toElement();
                    QString cl;

                    QList<DomPropertyData*> ui_property_list;

                    while (!n3.isNull()) {
                        QString tagName = n3.tagName().toLower();

                        if (tagName == QLatin1String("property")) {
                            DomPropertyData *p = new DomPropertyData();
                            p->read(n3);

                            ui_property_list.append(p);
                        }

                        n3 = n3.nextSibling().toElement();
                    }

                    if (ui_property_list.size()) {
                        DomProperties *properties = new DomProperties();
                        properties->setElementProperty(ui_property_list);
                        customWidget->setElementProperties(properties);
                    }

                    ui_customwidget_list.append(customWidget);
                }
                n2 = n2.nextSibling().toElement();
            }
        }
    }


    DomWidget *w = createWidget(e);
    Q_ASSERT(w != 0);

    QList<DomWidget*> l = w->elementWidget();
    l += ui_toolbars;
    l += ui_menubars;
    w->setElementWidget(l);
    w->setElementActionGroup(w->elementActionGroup() + ui_action_list);

    ui->setElementWidget(w);
    ui->setElementClass(w->attributeName()); // ### remove me!

    if (ui_customwidget_list.size()) {
        DomCustomWidgets *customWidgets = new DomCustomWidgets();
        customWidgets->setElementCustomWidget(ui_customwidget_list);
        ui->setElementCustomWidgets(customWidgets);
    }

    if (ui_tabstops.size()) {
        DomTabStops *tabStops = new DomTabStops();
        tabStops->setElementTabStop(ui_tabstops);
        ui->setElementTabStops(tabStops);
    }

#if 0 // ### enable me?
    if (ui_include_hits.size()) {
        DomIncludeHints *includeHints = new DomIncludeHints();
        includeHints->setElementIncludeHint(ui_include_hits);
        ui->setElementIncludeHints(includeHints);
    }
#endif

    ui->setAttributeStdSetDef(stdsetdef);

    return ui;
}

void Ui3Reader::fixActionGroup(DomActionGroup *g)
{
    QList<DomActionGroup*> groups = g->elementActionGroup();
    for (int i=0; i<groups.size(); ++i) {
        fixActionGroup(groups.at(i));
    }

    QList<DomAction*> actions = g->elementAction();
    for (int i=0; i<actions.size(); ++i) {
        DomAction *a = actions.at(i);

        QList<DomProperty*> properties = a->elementProperty();
        for (int j=0; j<properties.size(); ++j) {
            DomProperty *p = properties.at(j);
            QString pname = p->attributeName();
            if (pname == QLatin1String("name")) {
                a->setAttributeName(p->elementCstring());
                p->setAttributeName("objectName");
            } else if (pname == QLatin1String("menuText")) {
                p->setAttributeName("text");
            } else if (pname == QLatin1String("iconSet")) {
                p->setAttributeName("icon");
            } else if (pname == QLatin1String("accel")) {
                p->setAttributeName("shortcut");
                p->setElementShortcut(p->elementString());
            }
        }
    }

    QList<DomProperty*> properties = g->elementProperty();
    for (int j=0; j<properties.size(); ++j) {
        DomProperty *p = properties.at(j);
        QString pname = p->attributeName();
        if (pname == QLatin1String("name")) {
            g->setAttributeName(p->elementCstring());
            p->setAttributeName("objectName");
        } else if (pname == QLatin1String("menuText")) {
            p->setAttributeName("text");
        } else if (pname == QLatin1String("iconSet")) {
            p->setAttributeName("icon");
        } else if (pname == QLatin1String("accel")) {
            p->setAttributeName("shortcut");
            p->setElementShortcut(p->elementString());
        }
    }
}

DomWidget *Ui3Reader::createWidget(const QDomElement &w, const QString &widgetClass)
{
    DomWidget *ui_widget = new DomWidget;
    QString className = widgetClass;
    if (className.isEmpty())
        className = w.attribute("class");
    if (className == QLatin1String("QLayoutWidget"))
        className = QLatin1String("QWidget");
    else if (className == QLatin1String("QButtonGroup"))
        className = QLatin1String("Q3ButtonGroup");

    bool isMenuBar = className == QLatin1String("QMenuBar");

    ui_widget->setAttributeClass(className);

    QList<DomWidget*> ui_child_list;
    QList<DomRow*> ui_row_list;
    QList<DomColumn*> ui_column_list;
    QList<DomItem*> ui_item_list;
    QList<DomProperty*> ui_property_list;
    QList<DomProperty*> ui_attribute_list;
    QList<DomLayout*> ui_layout_list;
    QList<DomActionRef*> ui_action_list;

    bool needPolish = FALSE;

    createProperties(w, &ui_property_list);
    createAttributes(w, &ui_attribute_list);

    QDomElement e = w.firstChild().toElement();
    while (!e.isNull()) {
        QString t = e.tagName().toLower();
        if (t == QLatin1String("vbox")
                || t == QLatin1String("hbox")
                || t == QLatin1String("grid")) {
            DomLayout *lay = createLayout(e);
            Q_ASSERT(lay != 0);
            if (ui_layout_list.isEmpty()) {
                ui_layout_list.append(lay);
            } else {
                // ### throw an error.. it's not possible to have more than one layout for widget!
                delete lay;
            }
        } else if (t == QLatin1String("spacer")) {
            // hmm, spacer as child of a widget.. it doesn't make sense, so skip it!
        } else if (t == QLatin1String("widget")) {
            DomWidget *ui_child = createWidget(e);
            Q_ASSERT(ui_child != 0);
            ui_child_list.append(ui_child);
        } else if (t == QLatin1String("action")) {
            DomActionRef *a = new DomActionRef();
            a->read(e);
            ui_action_list.append(a);
        } else if (t == QLatin1String("separator")) {
            DomActionRef *a = new DomActionRef();
            a->setAttributeName("separator");
            ui_action_list.append(a);
        } else if (t == QLatin1String("property")) {
            // skip the property it is already handled by createProperties

            QString name = e.attribute("name");  // change the varname this widget
            if (name == QLatin1String("name"))
                ui_widget->setAttributeName(DomTool::readProperty(w, QLatin1String("name"), QCoreVariant()).toString());
        } else if (t == QLatin1String("row")) {
            DomRow *row = new DomRow();
            row->read(e);
            ui_row_list.append(row);
        } else if (t == QLatin1String("column")) {
            DomColumn *column = new DomColumn();
            column->read(e);
            ui_column_list.append(column);
        } else if (isMenuBar && t == QLatin1String("item")) {
            QString text = e.attribute("text");
            QString name = e.attribute("name");

            QList<DomProperty*> properties;
            QList<DomProperty*> attributes;

            DomProperty *ptext = new DomProperty();
            ptext = new DomProperty();
            ptext->setAttributeName("objectName");
            ptext->setElementString(name);
            properties.append(ptext);

            DomProperty *atitle = new DomProperty();
            atitle->setAttributeName("title");
            atitle->setElementString(text);
            attributes.append(atitle);

            DomWidget *menu = createWidget(e, "QMenu");
            menu->setAttributeName(name);
            menu->setElementProperty(properties);
            menu->setElementAttribute(attributes);
            ui_child_list.append(menu);
        } else if (t == QLatin1String("item")) {
            DomItem *item = new DomItem();
            item->read(e);
            ui_item_list.append(item);
        }

        QString s = getClassName(e);
        if (s == QLatin1String("QDataTable")
                || s == QLatin1String("QDataBrowser")) {
            if (isFrameworkCodeGenerated(e))
                 needPolish = TRUE;
        }

        e = e.nextSibling().toElement();
    }

    ui_widget->setElementWidget(ui_child_list);
    ui_widget->setElementAddAction(ui_action_list);
    ui_widget->setElementRow(ui_row_list);
    ui_widget->setElementColumn(ui_column_list);
    ui_widget->setElementItem(ui_item_list);
    ui_widget->setElementProperty(ui_property_list);
    ui_widget->setElementAttribute(ui_attribute_list);
    ui_widget->setElementLayout(ui_layout_list);

    //ui_widget->setAttributeName(p->elementCstring());

    return ui_widget;
}

DomLayout *Ui3Reader::createLayout(const QDomElement &w)
{
    DomLayout *lay = new DomLayout();

    QList<DomLayoutItem*> ui_item_list;
    QList<DomProperty*> ui_property_list;
    QList<DomProperty*> ui_attribute_list;

    QString tagName = w.tagName().toLower();

    QString className;
    if (tagName == "vbox")
        className = QLatin1String("QVBoxLayout");
    else if (tagName == "hbox")
        className = QLatin1String("QHBoxLayout");
    else
        className = QLatin1String("QGridLayout");

    lay->setAttributeClass(className);

    createProperties(w, &ui_property_list);
    createAttributes(w, &ui_attribute_list);

    bool needPolish = FALSE;
    QDomElement e = w.firstChild().toElement();
    while (!e.isNull()) {
        QString t = e.tagName().toLower();
        if (t == QLatin1String("vbox")
                 || t == QLatin1String("hbox")
                 || t == QLatin1String("grid")
                 || t == QLatin1String("spacer")
                 || t == QLatin1String("widget")) { // ### cleanup
            DomLayoutItem *lay_item = createLayoutItem(e);
            Q_ASSERT(lay_item != 0);
            ui_item_list.append(lay_item);
        }

        QString s = getClassName(e);
        if (s == QLatin1String("QDataTable")
                 || s == QLatin1String("QDataBrowser")) {
            if (isFrameworkCodeGenerated(e))
                 needPolish = TRUE;
        }

        e = e.nextSibling().toElement();
    }

    lay->setElementItem(ui_item_list);
    lay->setElementProperty(ui_property_list);
    lay->setElementAttribute(ui_attribute_list);

    return lay;
}

DomLayoutItem *Ui3Reader::createLayoutItem(const QDomElement &e)
{
    DomLayoutItem *lay_item = new DomLayoutItem;

    QString tagName = e.tagName().toLower();
    if (tagName == QLatin1String("widget")) {
        DomWidget *ui_widget = createWidget(e);
        Q_ASSERT(ui_widget != 0);
        lay_item->setElementWidget(ui_widget);
    } else if (tagName == QLatin1String("spacer")) {
        DomSpacer *ui_spacer = new DomSpacer();
        QList<DomProperty*> properties;

        Size defaultSize;
        defaultSize.init(0, 0);

        QByteArray name = DomTool::readProperty(e, "name", "spacer").toByteArray();
        QCoreVariant def;
        qVariantSet(def, defaultSize, "Variant");
        Size size = asVariant(DomTool::readProperty(e, "sizeHint", def)).size;
        QString sizeType = DomTool::readProperty(e, "sizeType", "Expanding").toString();
        QString orientation = DomTool::readProperty(e, "orientation", "Horizontal").toString();

        ui_spacer->setAttributeName(name);

        DomProperty *prop = 0;

        // sizeHint
        prop = new DomProperty();
        prop->setAttributeName("sizeHint");
        prop->setElementSize(new DomSize());
        prop->elementSize()->setElementWidth(size.width);
        prop->elementSize()->setElementHeight(size.height);
        properties.append(prop);

        // sizeType
        prop = new DomProperty();
        prop->setAttributeName("sizeType");
        prop->setElementEnum(sizeType);
        properties.append(prop);

        // orientation
        prop = new DomProperty();
        prop->setAttributeName("orientation");
        prop->setElementEnum(orientation);
        properties.append(prop);

        ui_spacer->setElementProperty(properties);
        lay_item->setElementSpacer(ui_spacer);
    } else {
        DomLayout *ui_layout = createLayout(e);
        Q_ASSERT(ui_layout != 0);
        lay_item->setElementLayout(ui_layout);
    }

    if (e.hasAttribute("row"))
        lay_item->setAttributeRow(e.attribute("row").toInt());
    if (e.hasAttribute("column"))
        lay_item->setAttributeColumn(e.attribute("column").toInt());
    if (e.hasAttribute("rowspan"))
        lay_item->setAttributeRowSpan(e.attribute("rowspan").toInt());
    if (e.hasAttribute("colspan"))
        lay_item->setAttributeColSpan(e.attribute("colspan").toInt());

    return lay_item;
}

void Ui3Reader::createProperties(const QDomElement &n, QList<DomProperty*> *properties)
{
    QString className = n.attribute("class");

    for (QDomElement e=n.firstChild().toElement(); !e.isNull(); e = e.nextSibling().toElement()) {
        if (e.tagName().toLower() == QLatin1String("property")) {
            QString name = e.attribute("name");

            // changes in QPalette
            if (name == QLatin1String("colorGroup")
                    || name == QLatin1String("paletteForegroundColor")
                    || name == QLatin1String("paletteBackgroundColor")
                    || name == QLatin1String("backgroundMode")
                    || name == QLatin1String("backgroundOrigin")
                    || name == QLatin1String("paletteBackgroundPixmap")
                    || name == QLatin1String("backgroundBrush")) {
                fprintf(stderr, "property '%s' not supported\n", name.latin1());
                continue;
            }

            // ### check if className inheriths from QFrame
            // changes in QFrame
            if (name == QLatin1String("contentsRect")) {
                fprintf(stderr, "property '%s' not supported\n", name.latin1());
                continue;
            }

            // changes in QWidget
            if (name == QLatin1String("underMouse")
                    || name == QLatin1String("ownFont")) {
                fprintf(stderr, "property '%s' not supported\n", name.latin1());
                continue;
            }

            DomProperty *prop = readProperty(e);
            if (!prop)
                continue;

            if (className == QLatin1String("Line")
                    && prop->attributeName() == QLatin1String("orientation")) {
                delete prop;
                continue;
            }

            if (className.mid(1) == QLatin1String("LineEdit")) {
                if (name == QLatin1String("hasMarkedText")) {
                    prop->setAttributeName("hasSelectedText");
                } else if (name == QLatin1String("edited")) {
                    prop->setAttributeName("modified");
                } else if (name == QLatin1String("markedText")) {
                    prop->setAttributeName("selectedText");
                }
            }

            // changes in QWidget
            if (name == QLatin1String("ownCursor")) {
                // skip
            } else if (name == QLatin1String("customWhatsThis")) {
                prop->setAttributeName("whatsThis");
            } else if (name == QLatin1String("icon")) {
                prop->setAttributeName("windowIcon");
            } else if (name == QLatin1String("iconText")) {
                prop->setAttributeName("windowIconText");
            } else if (name == QLatin1String("caption")) {
                prop->setAttributeName("windowTitle");
            }

            if (name == QLatin1String("name")) {
                prop->setAttributeName("objectName");
            }

            if (name == QLatin1String("accel")) {
                prop->setAttributeName("shortcut");
                prop->setElementShortcut(prop->elementString());
            }

            CONVERT_PROPERTY("pixmap", "icon");
            CONVERT_PROPERTY("iconSet", "icon");
            CONVERT_PROPERTY("textLabel", "text");

            CONVERT_PROPERTY("toggleButton", "checkable");
            CONVERT_PROPERTY("isOn", "checked");

            CONVERT_PROPERTY("maxValue", "maximum");
            CONVERT_PROPERTY("minValue", "minimum");
            CONVERT_PROPERTY("lineStep", "singleStep");

            name = prop->attributeName(); // sync the name

            if (className.size()
                    && !(className == QLatin1String("QLabel") && name == QLatin1String("buddy"))
                    && !(name == QLatin1String("buttonGroupId"))
                    && !(name == QLatin1String("frameworkCode"))
                    && !(name == QLatin1String("database"))) {
                if (prop->kind() == DomProperty::Enum
                            && !WidgetInfo::isValidEnumerator(className, prop->elementEnum().latin1())) {
                    fprintf(stderr, "enumerator '%s' for widget '%s' is not supported\n",
                            prop->elementEnum().latin1(), className.latin1());
                    delete prop;
                } else if (!WidgetInfo::isValidProperty(className, name.latin1())) {
                    fprintf(stderr, "property '%s' for widget '%s' is not supported\n",
                            name.latin1(), className.latin1());
                    delete prop;
                } else {
                    properties->append(prop);
                }
            } else {
                properties->append(prop);
            }
        }
    }
}

DomProperty *Ui3Reader::readProperty(const QDomElement &e)
{
    QString name = e.firstChild().toElement().tagName().toLower();

    if (name == QLatin1String("class")) // skip class
        name = e.firstChild().nextSibling().toElement().tagName().toLower();

    DomProperty *p = new DomProperty;
    p->read(e); // ### check if the name is valid!

    if (p->kind() == DomProperty::Unknown) {
        delete p;
        p = 0;
    }

    return p;
}

void Ui3Reader::createAttributes(const QDomElement &n, QList<DomProperty*> *properties)
{
    QString className = n.attribute("class");

    for (QDomElement e=n.firstChild().toElement(); !e.isNull(); e = e.nextSibling().toElement()) {
        if (e.tagName().toLower() == QLatin1String("attribute")) {
            QString name = e.attribute("name");

            DomProperty *prop = readProperty(e);
            if (!prop)
                continue;

            properties->append(prop);
        }
    }
}
