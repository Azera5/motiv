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
#ifndef MOTIV_SLOT_HPP
#define MOTIV_SLOT_HPP

#include <otf2xx/otf2.hpp>
#include "Builder.hpp"
#include "src/types.hpp"
#include "TimedElement.hpp"

enum SlotKind {
    None    = 0b0000,
    MPI     = 0b0001,
    OpenMP  = 0b0010,
    Plain   = 0b0100
};

class Slot : public TimedElement {
public:
    Slot(const otf2::chrono::duration &start, const otf2::chrono::duration &end,
         otf2::definition::location *location, otf2::definition::region *region);

    /**
     * Start time of the slot relative to the trace start time
     */
    otf2::chrono::duration startTime;

    /**
     * End time of the slot relative to the trace start time
     */
    types::TraceTime endTime;

    /**
     * Location of the slot (thread) containing the location group (MPI rank)
     */
    otf2::definition::location *location;

    /**
     * Region the slot occurred in. For example, the source file and line.
     */
    otf2::definition::region *region;

    /**
     *
     *  @brief Returns the kind of the current Slot object.
     *
     *  This function returns the kind of the current Slot object based on the name of the region that the Slot belongs to.
     *  @return The kind of the current Slot object.
     */
    [[nodiscard]] SlotKind getKind() const;

    /**
     * @copydoc TimedElement::getStartTime()
     */
    [[nodiscard]] types::TraceTime getStartTime() const override;

    /**
     * @copydoc TimedElement::getStartTime()
     */
    [[nodiscard]] types::TraceTime getEndTime() const override;


    BUILDER(Slot,
            BUILDER_FIELD(otf2::chrono::duration, start)
                BUILDER_FIELD(otf2::chrono::duration, end)
                BUILDER_FIELD(otf2::definition::location * , location)
                BUILDER_FIELD(otf2::definition::region * , region),
            start, end, location, region)
};

#endif //MOTIV_SLOT_HPP
