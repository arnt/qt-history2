/****************************************************************************
**
** Definition of QMenuBar class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMENUBAR_H
#define QMENUBAR_H

#ifndef QT_H
#include "qpopupmenu.h" // ### remove or keep for users' convenience?
#include "qframe.h"
#include "qmenudata.h"
#endif // QT_H

#ifndef QT_NO_MENUBAR

class QPopupMenu;

class Q_GUI_EXPORT QMenuBar : public Q3Frame, public QMenuData
{
    Q_OBJECT
    Q_ENUMS( Separator )
    Q_PROPERTY( Separator separator READ separator WRITE setSeparator DESIGNABLE false )
    Q_PROPERTY( bool defaultUp READ isDefaultUp WRITE setDefaultUp )

public:
    QMenuBar( QWidget* parent=0, const char* name=0 );
    ~QMenuBar();

    void	updateItem( int id );

    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

    bool	eventFilter( QObject *, QEvent * );

    int		heightForWidth(int) const;

    enum	Separator { Never=0, InWindowsStyle=1 };
    Separator 	separator() const;
    virtual void	setSeparator( Separator when );

    void	setDefaultUp( bool );
    bool	isDefaultUp() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    void activateItemAt( int index );

#if defined(Q_WS_MAC) && !defined(QMAC_QMENUBAR_NO_NATIVE)
    static void initialize();
    static void cleanup();
#endif

signals:
    void	activated( int itemId );
    void	highlighted( int itemId );

protected:
    void	drawContents( QPainter * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	resizeEvent( QResizeEvent * );
    void	leaveEvent( QEvent * );
    void	menuContentsChanged();
    void	menuStateChanged();
    void        changeEvent( QEvent * );
    int	itemAtPos( const QPoint & );
    void	hidePopups();
    QRect	itemRect( int item );

private slots:
    void	subActivated( int itemId );
    void	subHighlighted( int itemId );
#ifndef QT_NO_ACCEL
    void	accelActivated( int itemId );
    void	accelDestroyed();
#endif
    void	popupDestroyed( QObject* );
    void 	performDelayedChanges();

private:
    void 	performDelayedContentsChanged();
    void 	performDelayedStateChanged();
    void	menuInsPopup( QPopupMenu * );
    void	menuDelPopup( QPopupMenu * );
    void	frameChanged();

    bool	tryMouseEvent( QPopupMenu *, QMouseEvent * );
    void	tryKeyEvent( QPopupMenu *, QKeyEvent * );
    void	goodbye( bool cancelled = FALSE );
    void	openActPopup();

    void setActiveItem( int index, bool show = TRUE, bool activate_first_item = TRUE );
    void setAltMode( bool );

    int		calculateRects( int max_width = -1 );

#ifndef QT_NO_ACCEL
    void	setupAccelerators();
    QAccel     *autoaccel;
#endif
    QRect      *irects;
    int		rightSide;

    uint	mseparator : 1;
    uint	waitforalt : 1;
    uint	popupvisible  : 1;
    uint	hasmouse : 1;
    uint 	defaultup : 1;
    uint 	toggleclose : 1;
    uint        pendingDelayedContentsChanges : 1;
    uint        pendingDelayedStateChanges : 1;

    friend class QPopupMenu;

#if defined(Q_WS_MAC) && !defined(QMAC_QMENUBAR_NO_NATIVE)
    friend class QWidget;
    friend class QApplication;
    friend class Q4MenuBar;
    friend void qt_mac_set_modal_state(bool, QMenuBar *);

    void macCreateNativeMenubar();
    void macRemoveNativeMenubar();
    void macDirtyNativeMenubar();

#if !defined(QMAC_QMENUBAR_NO_EVENT)
    static void qt_mac_install_menubar_event(MenuRef);
    static OSStatus qt_mac_menubar_event(EventHandlerCallRef, EventRef, void *);
#endif
    virtual void macWidgetChangedWindow();
    bool syncPopups(MenuRef ret, QPopupMenu *d);
    MenuRef createMacPopup(QPopupMenu *d, int id, bool =FALSE);
    bool updateMenuBar();
#if !defined(QMAC_QMENUBAR_NO_MERGE)
    uint isCommand(QMenuItem *, bool just_check=FALSE);
#endif

    uint mac_eaten_menubar : 1;
    class MacPrivate;
    MacPrivate *mac_d;
    static bool activate(MenuRef, short, bool highlight=FALSE, bool by_accel=FALSE);
    static bool activateCommand(uint cmd);
    static bool macUpdateMenuBar();
    static bool macUpdatePopupVisible(MenuRef, bool);
    static bool macUpdatePopup(MenuRef);
#endif

private:	// Disabled copy constructor and operator=

#if defined(Q_DISABLE_COPY)
    QMenuBar( const QMenuBar & );
    QMenuBar &operator=( const QMenuBar & );
#endif
};


#endif // QT_NO_MENUBAR

#endif // QMENUBAR_H
