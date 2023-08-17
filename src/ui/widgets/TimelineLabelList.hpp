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
#ifndef MOTIV_TIMELINELABELLIST_HPP
#define MOTIV_TIMELINELABELLIST_HPP


#include <QListWidget>

#include "src/models/ViewSettings.hpp"
#include "src/ui/TraceDataProxy.hpp"

/**
 * @brief The TimelineLabelList displays a vertical bar with a list of rank names.
 *
 * TODO: for configurable region heights, the height of the labels should be adjusted here too
 */
class TimelineLabelList : public QListWidget {
    Q_OBJECT

public:
    /**
     * @brief Creates a new instance of the TimelineLabelList class
     *
     * @param data A pointer to a TraceDataProxy
     * @param parent The parent QWidget
     */
    TimelineLabelList(TraceDataProxy *data, QWidget *parent = nullptr);

    int getMaxLabelLength();

protected:
    /*
     * NOTE: we override this function to prevent the items from being clicked/activated.
     * this is quite hacky and there might be a better solution.
     */

    /**
     * @copydoc QGraphicsView::mousePressEvent(QMouseEvent*)
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * @copydoc QGraphicsView::mouseReleaseEvent(QMouseEvent*)
     */
    void mouseReleaseEvent(QMouseEvent *event) override;
    /**
     * @copydoc QGraphicsView::mouseMoveEvent(QMouseEvent*)
     */
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    TraceDataProxy *data = nullptr;
    QListWidget* widgetList;
    QMenu *menu;
    int ROW_HEIGHT = ViewSettings::getInstance()->getRowHeight();
    int maxLabelLength = 0;
    QAction *labelAction1;
    void highlightPreparation();
    QAction *labelAction2;
    void ignoreCommPreparation();
    QAction *labelAction3;
    void flamegraphPreparation();
};


#endif //MOTIV_TIMELINELABELLIST_HPP
