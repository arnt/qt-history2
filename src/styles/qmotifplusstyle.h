/****************************************************************************
**
** Definition of QMotifPlusStyle class.
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

#ifndef QMOTIFPLUSSTYLE_H
#define QMOTIFPLUSSTYLE_H


#ifndef QT_H
#include "qmotifstyle.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_MOTIFPLUS) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_MOTIFPLUS
#else
#define Q_EXPORT_STYLE_MOTIFPLUS Q_EXPORT
#endif

class Q_EXPORT_STYLE_MOTIFPLUS QMotifPlusStyle : public QMotifStyle
{
    Q_OBJECT

public:
    QMotifPlusStyle(bool hoveringHighlight = TRUE);
    virtual ~QMotifPlusStyle();

    void polish(QPalette &pal);
    void polish(QWidget *widget);
    void unPolish(QWidget*widget);

    void polish(QApplication *app);
    void unPolish(QApplication *app);

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QPalette &pal,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QPalette &pal,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    QRect subRect(SubRect r, const QWidget *widget) const;

    void drawComplexControl(ComplexControl control,
			    QPainter *p,
			    const QWidget *widget,
			    const QRect &r,
			    const QPalette &pal,
			    SFlags how = Style_Default,
			    SCFlags controls = SC_All,
			    SCFlags active = SC_None,
			    const QStyleOption& = QStyleOption::Default ) const;

    QRect querySubControlMetrics(ComplexControl control,
				 const QWidget *widget,
				 SubControl subcontrol,
				 const QStyleOption& = QStyleOption::Default) const;

    int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const;

    int styleHint(StyleHint sh, const QWidget *, const QStyleOption & = QStyleOption::Default,
		  QStyleHintReturn* = 0) const;

protected:
    bool eventFilter(QObject *, QEvent *);


private:
    bool useHoveringHighlight;
};


#endif // QT_NO_STYLE_MOTIFPLUS

#endif // QMOTIFPLUSSTYLE_H
