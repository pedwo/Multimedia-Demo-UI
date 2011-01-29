/*
 * Carousel Menu Selector.
 * Simple Qt class to present selections as images in a carousel.
 * 
 * Copyright (c) 2010, Renesas Electronics Europe Ltd
 * Written by Phil Edworthy
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CAROUSEL_GRAPHICS_WIDGET_H
#define CAROUSEL_GRAPHICS_WIDGET_H

#include <QtCore>
#include <QtGui>

class CarouselGraphicsWidget : public QGraphicsView
{
	Q_OBJECT

public:
	CarouselGraphicsWidget(QGraphicsScene *scene = 0, QWidget *parent = 0);

	QGraphicsItem *addItem(QGraphicsWidget *p);
	QGraphicsItem *addItem(const QPixmap &pix);
	virtual void mousePressEvent(QMouseEvent *event);

	// NB: resizeEvent should just change the positions & sizes. However, moving the first to last
	// state transitions every time an item is added is a pain. This means that setGeometry on this
	// widget must only be called after all items have been added.
	virtual void resizeEvent(QResizeEvent *event);

public slots:
	void next();
	void back();

Q_SIGNALS:
	// These 2 signal are private! They are used to connect the transitions
	// to the states. There are probably better ways to do this...
	void m_next();
	void m_back();

	void itemClicked(QGraphicsItem *);

private:
	QStateMachine *machine;
	QVector<QState *> states;
	QParallelAnimationGroup *animationGroup;

	QVector<QGraphicsWidget *> icons;
};

#endif // CAROUSEL_GRAPHICS_WIDGET_H

