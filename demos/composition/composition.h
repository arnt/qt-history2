#ifndef COMPOSITION_H
#define COMPOSITION_H

#include <qpainter.h>
#include <qevent.h>

#include "arthurwidgets.h"

class QPushButton;
class QRadioButton;

class CompositionWidget : public QWidget
{
	Q_OBJECT

public:
	CompositionWidget(QWidget *parent);

public slots:
	void nextMode();
	
private:
	bool m_cycle_enabled;

	QRadioButton *rbClear;
	QRadioButton *rbSource;
	QRadioButton *rbDest;
	QRadioButton *rbSourceOver;
	QRadioButton *rbDestOver;
	QRadioButton *rbSourceIn;
	QRadioButton *rbDestIn;
	QRadioButton *rbSourceOut;
	QRadioButton *rbDestOut;
	QRadioButton *rbSourceAtop;
	QRadioButton *rbDestAtop;
	QRadioButton *rbXor;
};

class CompositionRenderer : public ArthurFrame
{	
	Q_OBJECT

	enum ObjectType { NoObject, Circle, Rectangle, Image };

public:
	CompositionRenderer(QWidget *parent);

	void paint(QPainter *);

	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

	void setCirclePos(const QPointF &pos) { m_circle_pos = pos; update(); }

	QSize sizeHint() const { return QSize(500, 400); }

public slots:
	void setClearMode() { m_composition_mode = QPainter::CompositionMode_Clear; update(); }
	void setSourceMode() { m_composition_mode = QPainter::CompositionMode_Source; update(); }
	void setDestMode() { m_composition_mode = QPainter::CompositionMode_Destination; update(); }
	void setSourceOverMode() { m_composition_mode = QPainter::CompositionMode_SourceOver; update(); }
	void setDestOverMode() { m_composition_mode = QPainter::CompositionMode_DestinationOver; update(); }
	void setSourceInMode() { m_composition_mode = QPainter::CompositionMode_SourceIn; update(); }
	void setDestInMode() { m_composition_mode = QPainter::CompositionMode_DestinationIn; update(); }
	void setSourceOutMode() { m_composition_mode = QPainter::CompositionMode_SourceOut; update(); }
	void setDestOutMode() { m_composition_mode = QPainter::CompositionMode_DestinationOut; update(); }
	void setSourceAtopMode() { m_composition_mode = QPainter::CompositionMode_SourceAtop; update(); }
	void setDestAtopMode() { m_composition_mode = QPainter::CompositionMode_DestinationAtop; update(); }
	void setXorMode() { m_composition_mode = QPainter::CompositionMode_Xor; update(); }

	void setCircleAlpha(int alpha) { m_circle_alpha = alpha; update(); }
	void setCircleColor(int hue) { m_circle_hue = hue; update(); }
	void setAnimationEnabled(bool enabled) { m_animation_enabled = enabled; update(); }

private:
	void updateCirclePos();
	QPainter::CompositionMode m_composition_mode;

	QImage m_image;
	QImage m_buffer;
	QImage m_base_buffer;

	int m_circle_alpha;
	int m_circle_hue;

	QPointF m_circle_pos;
	QPointF m_offset;

	ObjectType m_current_object;
	bool m_animation_enabled;
};

#endif // COMPOSITION_H