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

#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include "formeditor_global.h"
#include "formeditor.h"

// sdk
#include <abstractformwindowmanager.h>
#include <abstractformwindow.h>

// Qt
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

class FormWindowCursor;
class DomConnections;
class Connection;

class QLabel;
class QTimer;
class QAction;
class QMenu;
class QtUndoStack;
class QRubberBand;
class WidgetSelection;
class BreakLayoutCommand;
class FormWindowManager;
class OrderIndicator;
class FormEditor;
class FormWindowDnDItem;
class FormWindowWidgetStack;

// ### fake - remove when actions are implemented
class QT_FORMEDITOR_EXPORT WidgetToActionMap
{
public:
    void add(QWidget *w, const QString &action_name)
        { m_map[w].append(action_name); }
    QStringList actions(QWidget *w)
        { return m_map.value(w, QStringList()); }
private:
    typedef QMap<QWidget*, QStringList> Map;
    Map m_map;
};

// ### fake - remove when actions are implemented
struct QT_FORMEDITOR_EXPORT ActionListElt {
    QString name, objectName, menu, icon, iconText, shortcut;
};
typedef QList<ActionListElt> ActionList;

class QT_FORMEDITOR_EXPORT FormWindow: public AbstractFormWindow
{
    Q_OBJECT
public:
    enum HighlightMode
    {
        Restore,
        Highlight
    };

public:
    FormWindow(FormEditor *core, QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~FormWindow();

    virtual AbstractFormEditor *core() const;

    virtual AbstractFormWindowCursor *cursor() const;

    virtual int toolCount() const;
    virtual int currentTool() const;
    virtual void setCurrentTool(int index);
    virtual AbstractFormWindowTool *tool(int index) const;
    virtual void registerTool(AbstractFormWindowTool *tool);

    virtual bool hasFeature(Feature f) const;
    virtual Feature features() const;
    virtual void setFeatures(Feature f);

    virtual EditMode editMode() const;
    virtual void setEditMode(EditMode mode);

    virtual QString author() const;
    virtual QString comment() const;
    virtual void setAuthor(const QString &author);
    virtual void setComment(const QString &comment);

    virtual QString fileName() const;
    virtual void setFileName(const QString &fileName);

    virtual QString contents() const;
    virtual void setContents(const QString &contents);
    virtual void setContents(QIODevice *dev);

    virtual QPoint grid() const { return m_grid; }
    virtual void setGrid(const QPoint &grid) { m_grid = grid; }

    virtual QWidget *mainContainer() const;
    void setMainContainer(QWidget *mainContainer);
    bool isMainContainer(const QWidget *w) const;

    QWidget *currentWidget() const;
    void setCurrentWidget(QWidget *currentWidget);

    virtual QSize sizeHint() const
    { return QSize(400, 300); }  /// ### remove me

    bool hasInsertedChildren(QWidget *w) const;

    QList<QWidget *> selectedWidgets() const;
    void clearSelection(bool changePropertyDisplay=true);
    bool isWidgetSelected(QWidget *w) const;
    void selectWidget(QWidget *w, bool select=true);

    void selectWidgets();
    void repaintSelection();
    void repaintSelection(QWidget *w);
    void updateSelection(QWidget *w);
    void updateChildSelections(QWidget *w);
    void raiseChildSelections(QWidget *w);
    void raiseSelection(QWidget *w);
    void simplifySelection(QList<QWidget*> *sel);
    void hideSelection(QWidget *w);

    inline QList<QWidget *> widgets() const { return m_widgets; }
    inline int widgetCount() const { return m_widgets.count(); }
    inline QWidget *widgetAt(int index) const { return m_widgets.at(index); }

    QList<QWidget *> widgets(QWidget *widget) const;

    QWidget *createWidget(DomUI *ui, const QRect &rect, QWidget *target);
    void insertWidget(QWidget *w, const QRect &rect, QWidget *target);
    void resizeWidget(QWidget *widget, const QRect &geometry);
    void deleteWidgets(const QList<QWidget*> &widget_list);

    bool isManaged(QWidget *w) const;

    void manageWidget(QWidget *w);
    void unmanageWidget(QWidget *w);

    inline QtUndoStack *commandHistory() const
    { return m_commandHistory; }

    void beginCommand(const QString &description);
    void endCommand();

    void emitSelectionChanged();

    bool unify(QObject *w, QString &s, bool changeIt);

    bool isDirty() const;
    void setDirty(bool dirty);

    static FormWindow *findFormWindow(QWidget *w);

    virtual QWidget *containerAt(const QPoint &pos);
    virtual QWidget *widgetAt(const QPoint &pos);
    virtual void highlightWidget(QWidget *w, const QPoint &pos,
                                    HighlightMode mode = Highlight);

    DomConnections *saveConnections();

    void createConnections(DomConnections *connections, QWidget *parent);

    void updateOrderIndicators();

    WidgetToActionMap &widgetToActionMap() { return m_widget_to_action_map; }
    ActionList &actionList() { return m_action_list; }

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

signals:
    void showContextMenu(QWidget *w, const QPoint &pos);

public slots:
    void deleteWidgets();
    void raiseWidgets();
    void lowerWidgets();
    void copy();
    void cut();
    void paste();
    void selectAll();

    void layoutHorizontal();
    void layoutVertical();
    void layoutGrid();
    void layoutHorizontalSplit();
    void layoutVerticalSplit();
    void layoutHorizontalContainer(QWidget *w);
    void layoutVerticalContainer(QWidget *w);
    void layoutGridContainer(QWidget *w);
    void breakLayout(QWidget *w);

    void breakLayout();

protected:
    virtual QMenu *createPopupMenu(QWidget *w);
    virtual void resizeEvent(QResizeEvent *e);

private slots:
    void selectionChangedTimerDone();
    void invalidCheckedSelections();
    void updateDirty();
    void checkSelection();
    void checkSelectionNow();

private:
    void init();
    void initializeCoreTools();

    QPoint gridPoint(const QPoint &p) const;

    enum RectType { Insert, Rubber };

    void startRectDraw(const QPoint &global, QWidget *, RectType t);
    void continueRectDraw(const QPoint &global, QWidget *, RectType t);
    void endRectDraw();

    bool allowMove(QWidget *w);
    QWidget *containerAt(const QPoint &pos, QWidget *notParentOf);

    QList<QWidget*> checkSelectionsForMove(QWidget *w);

    QLabel *sizePreview() const;
    void checkPreviewGeometry(QRect &r);

    void handleContextMenu(QWidget *w, QContextMenuEvent *e);
    void handleMouseButtonDblClickEvent(QWidget *w, QMouseEvent *e);
    void handleMousePressEvent(QWidget *w, QMouseEvent *e);
    void handleMouseMoveEvent(QWidget *w, QMouseEvent *e);
    void handleMouseReleaseEvent(QWidget *w, QMouseEvent *e);
    void handleKeyPressEvent(QWidget *w, QKeyEvent *e);
    void handleKeyReleaseEvent(QWidget *w, QKeyEvent *e);
    void handlePaintEvent(QWidget *w, QPaintEvent *e);

    bool isCentralWidget(QWidget *w) const;
    QWidget *designerWidget(QWidget *w) const;

    BreakLayoutCommand *breakLayoutCommand(QWidget *w);

    void showOrderIndicators();
    void hideOrderIndicators();
    void repositionOrderIndicators();

    void setCursorToAll(const QCursor &c, QWidget *start);
    void restoreCursors(QWidget *start, FormWindow *fw);

    QPoint mapToForm(const QWidget *w, const QPoint &pos) const;
    bool canBeBuddy(QWidget *w) const;

    QWidget *findContainer(QWidget *w, bool excludeLayout) const;
    QWidget *findTargetContainer(QWidget *widget) const;

    static int widgetDepth(QWidget *w);
    static bool isChildOf(QWidget *c, const QWidget *p);

private:
    Feature m_feature;
    FormEditor *m_core;
    FormWindowCursor *m_cursor;
    QWidget *m_mainContainer;
    QWidget *m_currentWidget;
    mutable QLabel *sizePreviewLabel;
    QPoint m_grid;

    bool drawRubber;
    QPoint rectAnchor;
    QRect currRect;
    bool oldRectValid;

    QList<QWidget*> m_widgets;
    QHash<QWidget *, QWidget*> m_insertedWidgets;

    bool checkedSelectionsForMove;
    QList<WidgetSelection *> selections;
    QHash<QWidget *, WidgetSelection *> usedSelections;

    QPoint startPos;
    QPoint currentPos;

    QRect widgetGeom;
    QPoint oldPressPos;
    QPoint origPressPos;
    QWidget *startWidget;
    QWidget *endWidget;

    QWidget *targetContainer;
    bool hadOwnPalette;
    QPalette restorePalette;

    QtUndoStack *m_commandHistory;

    QString m_fileName;
    QString pixLoader;

    QList<OrderIndicator*> orderIndicators;
    QList<QWidget*> orderedWidgets;
    QList<QWidget*> stackedWidgets;

    QMap<QWidget*, QPalette> palettesBeforeHighlight;

    QRubberBand *m_rubberBand;

    QTimer *m_selectionChangedTimer;
    QTimer *m_checkSelectionTimer;
    QTimer *m_geometryChangedTimer;

    int m_dirty;
    int m_lastIndex;

    EditMode m_editMode;

    WidgetToActionMap m_widget_to_action_map;
    ActionList m_action_list;

    int m_currentTool;
    QList<AbstractFormWindowTool*> m_tools;

    QString m_comment;
    QString m_author;

    FormWindowWidgetStack *m_widgetStack;

private:
//    friend class FormWindowManager;
    friend class WidgetHandle;
    friend class WidgetSelection;
    friend class QDesignerWidget;
    friend class WidgetEditorTool;
};

#endif // FORMWINDOW_H
