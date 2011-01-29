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

/*
 * Notes:
 *
 * The items on the carousel are resized to VGA aspect ratio, so if
 * your items are different, they will be streched. The only reason
 * for this is that it makes the calculation of where to place the
 * items easier.
 */

#include <QtCore>
#include <QtGui>
#include "pixmap.h"
#include "carousel.h"

static const float PI = 3.14159265;

/* The carousel graphics widget */
CarouselGraphicsWidget::CarouselGraphicsWidget(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameStyle(0);
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	machine = new QStateMachine;
	animationGroup = new QParallelAnimationGroup;
}

QGraphicsItem *CarouselGraphicsWidget::addItem(QGraphicsWidget *p)
{
	scene()->addItem(p);
	icons.append(p);

	// Set the speed of the animation (it has to be set on the objects in the scene)
	QPropertyAnimation *anim = new QPropertyAnimation(p, "geometry");
	anim->setDuration(500);
	animationGroup->addAnimation(anim);

	QState *newState = new QState(machine);
	QState *lastState = states.at(states.size()-1);

	if (states.size() == 0) {
		machine->setInitialState(newState);
	} else {
		// Link this new state to the next state
		QSignalTransition *transition;
		transition = lastState->addTransition(this, SIGNAL(m_next()), newState);
		transition->addAnimation(animationGroup);

		// Link the next state to this new state
		transition = newState->addTransition(this, SIGNAL(m_back()), lastState);
		transition->addAnimation(animationGroup);
	}
	states.append(newState);

	// NB: Don't update the scene yet. See resizeEvent comment

	return p;
}

QGraphicsItem *CarouselGraphicsWidget::addItem(const QPixmap &pix)
{
	Pixmap *p = new Pixmap(pix);
	return addItem(p);
}

void CarouselGraphicsWidget::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);

	QGraphicsItem *item = itemAt(event->pos());
	if (item && item->isEnabled()) {
		emit itemClicked(item);
	} else {
		// TODO really want mouse/touchscreen drag
		// Clicking anywhere other than the item in the foreground
		// will cause the carousel to rotate one way or the other
		if (event->x() < scene()->sceneRect().width()/2)
			m_back();
		else
			m_next();
	}
}

// NB: resizeEvent should just change the positions & sizes. However,
// moving the first to last state transitions every time an item is
// added is a pain. This means that setGeometry on this widget must
// only be called after all items have been added.
void CarouselGraphicsWidget::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();

	// TODO view changes size, should we really be changeing the scene size?
	scene()->setSceneRect(0, 0, size.width(), size.height());

	// Use icons with same aspect ratio as VGA.
	int tw = size.width();
	int th = size.height();
	if (tw > th*640/480) tw = th*640/480;
	if (th > tw*480/640) th = tw*480/640;

	int iw = tw / 2;
	int ih = th / 2;
	int isw = tw * 3 / 16;
	int ish = th * 3 / 16;
	int w = (size.width()  - iw/2 - isw/2) / 2;
	int h = (size.height() - ih/2 - ish/2) / 2;
	int cx = size.width()/2;
	int cy = (size.height() - ish)/2;

	int num_objects = icons.size();


	for (int i=0; i<num_objects; ++i) {
		float angle = 2.0*PI*i / num_objects;
		QRect r;

		// When the icon is at the bottom of the screen (i=0), make it larger.
		// Though it would look nicer if the size was based on the 'depth', this
		// would involve a lot of scaling and then likely hit performance.
		if (i == 0)
			r.setRect(-iw/2, -ih/2, iw, ih);
		else
			r.setRect(-isw/2, -ish/2, isw, ish);
		r.translate(cx, cy);
		r.translate(w*sin(angle), h*cos(angle));

		for (int j=0; j<num_objects; ++j) {
			QState *state = states.at((i+j) % num_objects);
			QObject *o = icons.at((num_objects-j) % num_objects);

			// Set the position & make only the item at the bottom
			// of the widget clickable
			state->assignProperty(o, "geometry", r);
			state->assignProperty(o, "enabled", i==0); 
			state->assignProperty(o, "focus", i==0); 
		}
	}

	if (states.size() > 1) {
		QSignalTransition *transition;
		QState *firstState = states.at(0);
		QState *lastState = states.at(states.size()-1);

		// Link to the next state - special case
		transition = lastState->addTransition(this, SIGNAL(m_next()), firstState);
		transition->addAnimation(animationGroup);

		// Link to the previous state - special case
		transition = firstState->addTransition(this, SIGNAL(m_back()), lastState);
		transition->addAnimation(animationGroup);

		machine->start();
	}
}

void CarouselGraphicsWidget::next() { m_next(); }
void CarouselGraphicsWidget::back() { m_back(); }

