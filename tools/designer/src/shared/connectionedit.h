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

#ifndef CONNECTIONEDIT_H
#define CONNECTIONEDIT_H

#include "shared_global.h"

#include <QWidget>
#include <QVariant>
#include <QPixmap>
#include <QPoint>
#include <QList>
#include <QMap>
#include <QPolygon>

#include <qdebug.h>

class ConnectionEdit;
class Connection;

class QT_SHARED_EXPORT CEItem : public QObject
{
    Q_OBJECT
public:
    enum Status { Normal, New, Selected, UnderMouse, Dragged };
    // order in Type is significant, see ConnectionEdit::insertItem()
    enum Type { UnknownItem, WidgetItem, EdgeItem, LabelItem, EndPointItem };

    CEItem(ConnectionEdit *edit);
    ConnectionEdit *edit() const;

    Status status() const;
    bool underMouse() const;
    virtual bool selectable() const;

    bool visible() const;
    void setVisible(bool b);

    virtual QRect rect() const = 0;
    virtual bool contains(const QPoint &p) const;

    virtual void paint(QPainter *p);
    virtual void update() const;

    virtual void move(const QPoint &delta) = 0;

    virtual Type type() const;

signals:
    void moved();

protected:
    virtual QColor colorForStatus();

    friend class ConnectionEdit;

private:
    bool m_visible;
};

class QT_SHARED_EXPORT CELabelItem : public CEItem
{
    Q_OBJECT
public:
    CELabelItem(ConnectionEdit *edit);

    QString text() const;
    void setText(const QString &text);

    virtual QRect rect() const;
    virtual QPoint pos() const;
    virtual bool selectable() const;
    virtual void move(const QPoint &delta);
    virtual void paint(QPainter *p);

    void setAnchorPos(const QPoint &pos);
    QPoint anchorPos() const;

    virtual Type type() const;

private:
    QString m_text;
    QRect m_rect;
    QPoint m_anchor_pos;
};

class CEEdgeItem;

class CEEndPointItem : public CEItem
{
    Q_OBJECT
public:
    CEEndPointItem(const QPoint &pos, ConnectionEdit *edit);
    virtual QRect rect() const;
    virtual void paint(QPainter *p);
    virtual void move(const QPoint &delta);
    virtual QPoint pos() const;
    virtual bool selectable() const;

    CEEdgeItem *sourceEdge() const;
    CEEdgeItem *destinationEdge() const;
    CEEdgeItem *otherEdge(const CEEdgeItem *e) const;
    void addEdge(CEEdgeItem *edge);
    int edgeCount() const;
    CEEdgeItem *edge(int i) const;
    CEEdgeItem *edgeTo(CEEndPointItem *other) const;
    void adjustPos();
    void adjustRatio();

    QList<CEEdgeItem*> edgeList() const { return m_edge_list; } // ###

    virtual Type type() const;

    double xRatio() const;
    double yRatio() const;

public slots:
    virtual void edgeDestroyed(QObject *o);

protected:
    QPoint m_pos;

    typedef QList<CEEdgeItem*> EdgeList;
    EdgeList m_edge_list;

    double m_x_ratio, m_y_ratio;
    bool m_have_ratio;
};

class CEWidgetItem : public CEEndPointItem
{
    Q_OBJECT
public:
    CEWidgetItem(QWidget *w, ConnectionEdit *edit);
    virtual QRect rect() const;
    virtual QPoint pos() const;
    virtual void move(const QPoint &);
    virtual void paint(QPainter *p);
    virtual bool selectable() const;
    QWidget *widget() const;

    virtual Type type() const;

    bool updateGeometry();

private:
    QRect widgetRect() const;

    QWidget *m_widget;
    QRect m_rect;
};

class CEEdgeItem : public CEItem
{
    Q_OBJECT
public:
    CEEdgeItem(CEEndPointItem *ep1, CEEndPointItem *ep2, ConnectionEdit *edit);

    virtual QRect rect() const;
    virtual bool contains(const QPoint &p) const;
    CEEndPointItem *endPoint1() const;
    CEEndPointItem *endPoint2() const;
    CEEndPointItem *otherEndPoint(const CEEndPointItem *ep) const;
    virtual void paint(QPainter *p);
    virtual void move(const QPoint &delta);
    virtual bool selectable() const;

    void recalculate();

    QPoint exitPos() const;
    void setExitPos(const QPoint &pos);
    QPoint enterPos() const;
    void setEnterPos(const QPoint &pos);

    virtual Type type() const;

public slots:
    void endPointDestroyed(QObject *o);
    void endPointMoved();

private:
    CEEndPointItem *m_ep1, *m_ep2;
    QPoint m_top, m_side1, m_side2, m_bottom;
    QPoint m_pos1, m_pos2;
    QPolygon m_arrow_head;
    QPoint m_exit_pos, m_enter_pos;
};

struct QT_SHARED_EXPORT ConnectionHint
{
    enum Type { EndPoint, SourceLabel, DestinationLabel };
    inline ConnectionHint(Type t = EndPoint, const QPoint &p = QPoint())
                : type(t), pos(p) {}
    Type type;
    QPoint pos;
};

class QT_SHARED_EXPORT Connection : public QObject
{
    Q_OBJECT
public:
    enum EndPointStyle { Line, Arrow, Square };
    enum LabelRole { DisplayRole, DecorationRole, CustomRole = 1000 };
    typedef QList<ConnectionHint> HintList;

    Connection(ConnectionEdit *edit);

    void setEndPointStyle(EndPointStyle source, EndPointStyle dest);

    void setVisible(bool b);
    bool visible() const;

    QWidget *source() const;
    QWidget *destination() const;
    virtual void setSource(QWidget *src);
    virtual void setDestination(QWidget *dest);

    void setSourceLabel(LabelRole role, const QVariant &v);
    QVariant sourceLabel(LabelRole role) const;
    void setDestinationLabel(LabelRole role, const QVariant &v);
    QVariant destinationLabel(LabelRole role) const;

    CELabelItem *sourceLabelItem() const;
    CELabelItem *destinationLabelItem() const;
    void setLabelItems(CELabelItem *source_label, CELabelItem *destination_label);

    HintList hints() const;

    ConnectionEdit *edit() const { return m_edit; }

signals:
    void aboutToDelete(Connection*);
    void selected(Connection*);

private:
    QWidget *m_source, *m_destination;

    typedef QMap<LabelRole, QVariant> LabelData;
    LabelData m_source_label_data, m_destination_label_data;

    CELabelItem *m_source_label_item, *m_destination_label_item;

    ConnectionEdit *m_edit;
};

class QT_SHARED_EXPORT ConnectionEdit : public QWidget
{
    Q_OBJECT
public:
    ConnectionEdit(QWidget *parent);
    ~ConnectionEdit();
    inline QWidget *background() const { return m_bg_widget; }
    void setBackground(QWidget *background);

    void setSelected(CEItem *item, bool selected);
    bool isSelected(const CEItem *item) const;
    bool isNew(const CEItem *item) const;
    bool isUnderMouse(const CEItem *item) const;
    int selectedCount() const;
    void selectNone();

    int connectionCount() const;
    Connection *connection(int idx) const;
    QList<CEItem*> connectionItems(const Connection *con) const;

    void clear();
    void dumpItems() const;
    void updateAllItems();

public slots:
    void updateBackground();
    void deleteWidgetItem(QWidget *w);

    void deleteItems();

signals:
    void added(Connection*);
    void aboutToRemove(Connection*);
    void selected(Connection*);
    void activated(Connection*);

protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    virtual Connection *createConnection(QWidget *source, QWidget *destination);
    void initConnection(Connection *con, const Connection::HintList &hint_list);

    virtual QWidget *widgetAt(const QPoint &pos) const;

private:
    QWidget *m_bg_widget;
    QPixmap m_bg_pixmap;

    typedef QList<CEItem*> ItemList;
    typedef QMap<CEItem*, CEItem*> SelectedSet;
    typedef QList<Connection*> ConnectionList;
    typedef QMultiMap<CEItem*, Connection*> ConnectionMap;
    typedef QMap<Connection*, Connection*> ConnectionSet;

    ItemList m_new_item_list;
    CEEndPointItem *lastEndPoint() const;
    CEWidgetItem *firstEndPoint() const;

    ItemList m_item_list;
    SelectedSet m_selected_item_set;
    ConnectionList m_connection_list;
    ConnectionMap m_connection_map;

    QList<CEItem*> m_items_under_mouse;

    void updateUnderMouse(const QPoint &pos);

    CEItem *m_dragged_item, *m_drag_on_move_item;
    bool m_start_draw_on_move;
    bool m_start_drag_on_move;
    QPoint m_last_mouse_pos;

    CEItem *draggedItem() const;

    enum BorderType { ActiveBorder, SelectedBorder };
    QRect widgetRect(QWidget *w) const;

    void abortNewItems();
    void initConnection(Connection *con, const ItemList &item_list);
    void insertEndPoint(const QPoint &pos);

    enum Mode { DrawMode, EditMode, DragMode };
    inline Mode mode() const;

    void insertItem(CEItem *item);
    void deleteItem(CEItem *item);
    void deleteItems(ItemList item_list);
    void setSelectedItems(const ItemList &item_list, bool selected);
    void deleteWidgetItem(CEWidgetItem *widget_item);
    void checkConnection(Connection *con);

    void deleteEndPoint(CEEndPointItem *ep);

    enum LineType { TopLoopLine, RightLoopLine, LeftLoopLine, BottomLoopLine,
                    TopLine, RightLine, LeftLine, BottomLine, NoLine };
    LineType m_current_line;
    CEWidgetItem *m_old_target;
    void createLine(LineType type, CEWidgetItem *target, const QPoint &pos);
    LineType classifyLine(CEWidgetItem *source, CEWidgetItem *target, const QPoint &pos) const;
    void addEdgeTo(const QPoint &pos);

    void updateLine(Connection *con);
    void updateLine(CEEdgeItem *edge);

    void dumpItems();

    CEWidgetItem *widgetItem(QWidget *widget) const;
    CEItem *itemUnderMouse(CEItem::Type type) const;
    CEItem *itemUnderMouse() const;

    friend class CEItem;
    friend class CEEndPointItem;
    friend class CEWidgetItem;
    friend class CEEdgeItem;
};

/*******************************************************************************
** CEItem
*/

inline ConnectionEdit *CEItem::edit() const
    { return static_cast<ConnectionEdit*>(parent()); }

inline bool CEItem::contains(const QPoint &p) const
    { return rect().contains(p); }

inline bool CEItem::underMouse() const
    { return edit()->isUnderMouse(this); }

inline bool CEItem::selectable() const
    { return true; }

inline bool CEItem::visible() const
    { return m_visible; }

inline CEItem::Type CEItem::type() const
    { return UnknownItem; }

/*******************************************************************************
** CELabelItem
*/

inline QRect CELabelItem::rect() const
    { return m_rect; }

inline QPoint CELabelItem::pos() const
    { return m_rect.topLeft(); }

inline bool CELabelItem::selectable() const
    { return true; }

inline QString CELabelItem::text() const
    { return m_text; }

inline QPoint CELabelItem::anchorPos() const
    { return m_anchor_pos; }

inline CEItem::Type CELabelItem::type() const
    { return LabelItem; }

/*******************************************************************************
** CEEndPointItem
*/

inline CEEndPointItem::CEEndPointItem(const QPoint &pos, ConnectionEdit *edit)
    : CEItem(edit), m_pos(pos), m_x_ratio(0.0), m_y_ratio(0.0), m_have_ratio(false) {}

inline QPoint CEEndPointItem::pos() const
    { return m_pos; }

inline bool CEEndPointItem::selectable() const
    { return edit()->mode() == ConnectionEdit::EditMode; }

inline int CEEndPointItem::edgeCount() const
    { return m_edge_list.size(); }

inline CEEdgeItem *CEEndPointItem::edge(int i) const
    { return m_edge_list.at(i); }

inline CEItem::Type CEEndPointItem::type() const
    { return EndPointItem; }

inline double CEEndPointItem::xRatio() const
    { return m_x_ratio; }

inline double CEEndPointItem::yRatio() const
    { return m_y_ratio; }

/*******************************************************************************
** CEWidgetItem
*/

inline void CEWidgetItem::move(const QPoint &)
    {}

inline QRect CEWidgetItem::rect() const
    { return m_rect; }

inline bool CEWidgetItem::selectable() const
    { return true; }

inline QWidget *CEWidgetItem::widget() const
    { return m_widget; }

inline QPoint CEWidgetItem::pos() const
    { return rect().center(); }

inline CEItem::Type CEWidgetItem::type() const
    { return WidgetItem; }

/*******************************************************************************
** CEEdgeItem
*/

inline CEEndPointItem *CEEdgeItem::endPoint1() const
    { return m_ep1; }

inline CEEndPointItem *CEEdgeItem::endPoint2() const
    { return m_ep2; }

inline bool CEEdgeItem::selectable() const
    { return edit()->mode() == ConnectionEdit::EditMode; }

inline QPoint CEEdgeItem::exitPos() const
    { return m_exit_pos; }

inline QPoint CEEdgeItem::enterPos() const
    { return m_enter_pos; }

inline CEItem::Type CEEdgeItem::type() const
    { return EdgeItem; }

/*******************************************************************************
** Connection
*/

inline CELabelItem *Connection::sourceLabelItem() const
    { return m_source_label_item; }

inline CELabelItem *Connection::destinationLabelItem() const
    { return m_destination_label_item; }

inline QVariant Connection::sourceLabel(LabelRole role) const
    { return m_source_label_data.value(role); }

inline QVariant Connection::destinationLabel(LabelRole role) const
    { return m_destination_label_data.value(role); }

inline QWidget *Connection::source() const
    { return m_source; }

inline QWidget *Connection::destination() const
    { return m_destination; }

inline void Connection::setSource(QWidget *src)
    { m_source = src; }

inline void Connection::setDestination(QWidget *dest)
    { m_destination = dest; }


/*******************************************************************************
** ConnectionEdit
*/

inline int ConnectionEdit::connectionCount() const
    { return m_connection_list.count(); }

inline Connection *ConnectionEdit::connection(int idx) const
    { return m_connection_list.at(idx); }

inline ConnectionEdit::Mode ConnectionEdit::mode() const
{
    if (m_dragged_item != 0)
        return DragMode;
    else if (lastEndPoint() != 0)
        return DrawMode;
    else
        return EditMode;
}

inline bool ConnectionEdit::isSelected(const CEItem *item) const
    { return m_selected_item_set.contains(const_cast<CEItem*>(item)); }

inline bool ConnectionEdit::isNew(const CEItem *item) const
    { return m_new_item_list.contains(const_cast<CEItem*>(item)); }

inline bool ConnectionEdit::isUnderMouse(const CEItem *item) const
    { return itemUnderMouse() == item; }

inline CEItem *ConnectionEdit::draggedItem() const
    { return m_dragged_item; }

inline int ConnectionEdit::selectedCount() const
    { return m_selected_item_set.size(); }

inline QList<CEItem*> ConnectionEdit::connectionItems(const Connection *con) const
    { return m_connection_map.keys(const_cast<Connection*>(con)); }

inline void ConnectionEdit::setBackground(QWidget *background)
{ m_bg_widget = background; updateBackground(); }

#endif // CONNECTIONEDIT_H
