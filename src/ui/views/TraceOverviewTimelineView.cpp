/*
 * Marvelous OTF2 Traces Interactive Visualizer (MOTIV)
 * Copyright (C) 2023 Florian Gallrein, Björn Gehrke
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "TraceOverviewTimelineView.hpp"
#include "src/ui/views/CommunicationIndicator.hpp"
#include "src/ui/ColorGenerator.hpp"
#include "src/ui/Constants.hpp"
#include "src/models/UITrace.hpp"


#include <QGraphicsRectItem>
#include <QApplication>
#include <QWheelEvent>
#include <QRubberBand>

TraceOverviewTimelineView::TraceOverviewTimelineView(Trace *fullTrace, QWidget *parent) : QGraphicsView(parent), fullTrace(fullTrace) {
    auto scene = new QGraphicsScene(this);
    this->setAutoFillBackground(false);
    this->setStyleSheet("background: transparent");
    this->setScene(scene);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    selectionFrom = types::TraceTime(0);
    selectionTo = fullTrace->getRuntime();
}


void TraceOverviewTimelineView::populateScene(QGraphicsScene *scene) {
    auto width = scene->width();
    auto runtime = uiTrace->getRuntime().count();
    auto begin = 0;
    auto end = begin + runtime;

    qreal top = 0;
    auto ROW_HEIGHT = scene->height() / static_cast<qreal>(uiTrace->getSlots().size());
    for (const auto &item: uiTrace->getSlots()) {
        // Display slots
        for (const auto &slot: item.second) {
            auto startTime = slot->startTime.count();
            auto endTime = slot->endTime.count();


            // Ensures slots starting before `begin` (like main) are considered to start at begin
            auto effectiveStartTime = qMax(begin, startTime);
            // Ensures slots ending after `end` (like main) are considered to end at end
            auto effectiveEndTime = qMin(end, endTime);

            auto slotBeginPos = qMax(0.0,
                                     (static_cast<qreal>(effectiveStartTime - begin) / static_cast<qreal>(runtime)) *
                                     width);
            auto slotRuntime = static_cast<qreal>(effectiveEndTime - effectiveStartTime);
            auto rectWidth = (slotRuntime / static_cast<qreal>(runtime)) * width;

            QRectF rect(slotBeginPos, top, qMax(rectWidth, 5.0), ROW_HEIGHT);
            auto rectItem = scene->addRect(rect);

            // Determine color based on name
            QColor rectColor = slot->color;
            rectItem->setZValue(slot->priority);
            
            /*
            switch (slot->getKind()) {
                case ::MPI:
                    rectColor = colors::COLOR_SLOT_MPI;
                    rectItem->setZValue(layers::Z_LAYER_SLOTS_MIN_PRIORITY + 2);
                    break;
                case ::OpenMP:
                    rectColor = colors::COLOR_SLOT_OPEN_MP;
                    rectItem->setZValue(layers::Z_LAYER_SLOTS_MIN_PRIORITY + 1);
                    break;
                case ::None:
                case ::Plain:
                default:
                    rectColor = colors::COLOR_SLOT_PLAIN;
                    rectItem->setZValue(layers::Z_LAYER_SLOTS_MIN_PRIORITY + 0);
                    break;
            }
            */

            rectItem->setBrush(rectColor);
        }

        top += ROW_HEIGHT;
    }

    QPen selectionPen(Qt::black);
    QBrush selectionBrush(QColor(0xFF, 0xFF, 0xFF, 0x7F));
    selectionRectRight = scene->addRect(width - 1,0, 0, top, selectionPen, selectionBrush);
    selectionRectRight->setZValue(layers::Z_LAYER_SELECTION);
    selectionRectLeft = scene->addRect(0,0, 0, top, selectionPen, selectionBrush);
    selectionRectLeft->setZValue(layers::Z_LAYER_SELECTION);

    setSelectionWindow(selectionFrom, selectionTo);
}


void TraceOverviewTimelineView::resizeEvent(QResizeEvent *event) {
    uiTrace = UITrace::forResolution(fullTrace, event->size().width());

    this->updateView();
    QGraphicsView::resizeEvent(event);
}

void TraceOverviewTimelineView::updateView() {
    this->scene()->clear();

    auto sceneRect = this->rect();
    sceneRect.setHeight(size().height() - 2);

    this->scene()->setSceneRect(sceneRect);
    this->populateScene(this->scene());
}

void TraceOverviewTimelineView::setSelectionWindow(types::TraceTime from, types::TraceTime to) {
    selectionFrom = from;
    selectionTo = to;
    auto durationR = static_cast<qreal>(uiTrace->getRuntime().count());
    auto fromR = static_cast<qreal>(from.count());
    auto toR = static_cast<qreal>(to.count());
    auto width = this->width();

    auto l = selectionRectLeft->rect();
    l.setWidth(fromR / durationR * width);
    selectionRectLeft->setRect(l);

    auto r = selectionRectRight->rect();
    r.setX(toR / durationR * width);
    selectionRectRight->setRect(r);
}



void TraceOverviewTimelineView::mousePressEvent(QMouseEvent *event)
{
    rubberBandOrigin = event->pos();
    rubberBandOrigin.setY(0);
    if (!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(QRect(rubberBandOrigin, QSize(0, this->height())));
    rubberBand->show();
}

void TraceOverviewTimelineView::mouseMoveEvent(QMouseEvent *event)
{
    auto nextPoint = event->pos();
    nextPoint.setY(this->height());
    rubberBand->setGeometry(QRect(rubberBandOrigin, nextPoint).normalized());
}

void TraceOverviewTimelineView::mouseReleaseEvent(QMouseEvent *)
{
    rubberBand->hide();

    auto from = (rubberBand->geometry().x() * fullTrace->getRuntime()) / this->width();
    auto to = from + (rubberBand->geometry().width()  * fullTrace->getRuntime()) / this->width();

    Q_EMIT windowSelectionChanged(from, to);
}
