/****************************************************************************
**
** Definition of QGroupBox widget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGROUPBOX_H
#define QGROUPBOX_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_GROUPBOX


class QAccel;
class QGroupBoxPrivate;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;

class Q_GUI_EXPORT QGroupBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString title READ title WRITE setTitle )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation DESIGNABLE false )
    Q_PROPERTY( int columns READ columns WRITE setColumns DESIGNABLE false )
    Q_PROPERTY( bool flat READ isFlat WRITE setFlat )
    Q_PROPERTY( bool checkable READ isCheckable WRITE setCheckable )
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked )
public:
    QGroupBox( QWidget* parent=0, const char* name=0 );
    QGroupBox( const QString &title,
	       QWidget* parent=0, const char* name=0 );
    QGroupBox( int strips, Orientation o,
	       QWidget* parent=0, const char* name=0 );
    QGroupBox( int strips, Orientation o, const QString &title,
	       QWidget* parent=0, const char* name=0 );
    ~QGroupBox();

    virtual void setColumnLayout(int strips, Orientation o);

    QString title() const;
    virtual void setTitle( const QString &);

    int alignment() const;
    virtual void setAlignment( int );

    int columns() const;
    void setColumns( int );

    Orientation orientation() const;
    void setOrientation( Orientation );

    int insideMargin() const;
    int insideSpacing() const;
    void setInsideMargin( int m );
    void setInsideSpacing( int s );

    void addSpace( int );
    QSize sizeHint() const;

    bool isFlat() const;
    void setFlat( bool b );
    bool isCheckable() const;
    void setCheckable( bool b );
    bool isChecked() const;

#ifdef QT_COMPAT
    inline QT_COMPAT int margin() const { return insideMargin(); }
    inline QT_COMPAT void setMargin(int m) { setInsideMargin(m); }
#endif

public slots:
    void setChecked( bool b );

signals:
    void toggled( bool );

protected:
    bool event( QEvent * );
    void childEvent( QChildEvent * );
    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void focusInEvent( QFocusEvent * );
    void changeEvent( QEvent * );

private slots:
    void fixFocus();
    void setChildrenEnabled( bool b );

private:
    Q_DECL_PRIVATE(QGroupBox);

#if defined(Q_DISABLE_COPY)
    QGroupBox( const QGroupBox & );
    QGroupBox &operator=( const QGroupBox & );
#endif
};


#endif // QT_NO_GROUPBOX

#endif // QGROUPBOX_H
