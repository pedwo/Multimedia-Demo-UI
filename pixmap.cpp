/*
 * Create objects derived from QGraphicsWidget from a QPixmap.
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

#include <QtCore>
#include <QtGui>
#include "pixmap.h"

/* Scaled pixmap.
 * QGraphicsPixmapItem doesn't inherit from QObject, so it's not possible
 * to assign the geometry properties that are needed for animation.
 */
Pixmap::Pixmap(const QPixmap &pix, QGraphicsItem *parent)
	: QGraphicsWidget(parent), orig(pix), p(pix)
{
}

void Pixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->drawPixmap(QPointF(), p);
}

void Pixmap::setGeometry(const QRectF &rect)
{
	QGraphicsWidget::setGeometry(rect);
	if (psize.size().toSize() != rect.size().toSize()) {
		p = orig.scaled(rect.size().toSize());
		psize = rect;
	}
}

void Pixmap::mousePressEvent(QGraphicsSceneMouseEvent *)
{
	emit clicked();
}

