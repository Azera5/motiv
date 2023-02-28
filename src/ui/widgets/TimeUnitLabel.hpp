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
#ifndef MOTIV_TIMEUNITLABEL_HPP
#define MOTIV_TIMEUNITLABEL_HPP


#include <QLabel>
#include "src/types.hpp"

/**
 * @brief TimeUnitLabel constructs a QLabel from a double value that represents a time point.
 *
 * It will always show the greatest possible unit for witch the value is greater than zero.
 */
class TimeUnitLabel : public QLabel {
    Q_OBJECT

public:
    /**
     * @param time value to display in ns
     * @param parent
     */
    explicit TimeUnitLabel(double time, QWidget *parent = nullptr);

    /**
     * Update value.
     * @param time value to display in ns
     */
    void setTime(double time);
    /**
     * @return value in ns
     */
    [[nodiscard]] double getTime() const;

    /**
     * @brief Updates the view to reflect the current state
     */
    void updateView();

private:
    double time;
};


#endif //MOTIV_TIMEUNITLABEL_HPP
