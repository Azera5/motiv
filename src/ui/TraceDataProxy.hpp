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
#ifndef MOTIV_TRACEDATAPROXY_HPP
#define MOTIV_TRACEDATAPROXY_HPP


#include <QObject>

#include "src/models/Filetrace.hpp"
#include "src/models/ViewSettings.hpp"


/**
 * @brief Model class providing access to data and pub/sub architecture of change events
 *
 * TraceDataProxy acts as an intermediate class between the views and the data.
 * This class tracks all state changes related to the representation of the trace, e.g. selections
 * and emits a signal on changes.
 */
class TraceDataProxy : public QObject {
    Q_OBJECT

public: //constructors
    /**
     * @brief Constructs a new TraceDataProxy.
     *
     * The object takes ownership of the supplied FileTrace.
     * @param trace The entire trace
     * @param settings The view settings
     * @param parent The parent QObject
     */
    TraceDataProxy(FileTrace *trace, ViewSettings *settings, QObject *parent = nullptr);
    virtual ~TraceDataProxy() override;


public: // methods
    /**
     * @brief Returns the current selection
     *
     * The selection refers to the parts of the trace that is in the selected time window
     *
     * @return The current selection
     */
    [[nodiscard]] Trace *getSelection() const;
    /**
     * @brief Returns the selected start time
     * @return The selected start time
     */
    [[nodiscard]] types::TraceTime getBegin() const;

    /**
     * @brief Returns the selected end time
     * @return The selected end time
     */
    [[nodiscard]] types::TraceTime getEnd() const;

    /**
     * @brief Returns the current view settings
     * @return The current view settings
     */
    [[nodiscard]] ViewSettings *getSettings() const;

    /**
     * @brief Returns the entire trace
     * @return The full trace
     */
    [[nodiscard]] Trace *getFullTrace() const;

    /**
     * Returns the runtime of the entire loaded trace
     * @return
     */
    [[nodiscard]] types::TraceTime getTotalRuntime() const;

public: Q_SIGNALS:
    /**
     * Signals the selection has been changed
     */
    void selectionChanged(types::TraceTime newBegin, types::TraceTime newEnd);
    /**
     * Signals the begin was changed
     */
    void beginChanged(types::TraceTime newBegin);
    /**
     * Signals the end was changed
     */
    void endChanged(types::TraceTime newEnd);
    /**
     * Signals a change to the selected slot, nullptr if none
     */
    void infoElementSelected(TimedElement *);

     /**
     * Signals that the color of a slot has changed
     */
    void colorChanged();

    /**
     * Signals the filter was changes
     */
    void filterChanged(Filter);

    void verticalZoomChanged();

    void refreshOverviewRequest();

    void refreshButtonPressed();

    void flamegraphRequest();

    void triggerUITimerStartIfPossible();
    void triggerUITimerEndIfPossible();

public Q_SLOTS:
    /**
     * Change the start time of the selection
     * @param newBegin
     * @invariant may not be larger than end or total runtime
     */
    void setSelectionBegin(types::TraceTime newBegin);
    /**
     * Change the end time of the selection
     * @param newEnd
     * @invariant may not be smaller than begin and not larger than runtime
     */
    void setSelectionEnd(types::TraceTime newEnd);
    /**
     * Change the start and end time of the selection
     * @param newBegin
     * @param newEnd
     * @invariant @c newBegin may not be larger than end or total runtime
     * @invariant @c newEnd may not be smaller than begin and not larger than runtime
     */
    void setSelection(types::TraceTime newBegin, types::TraceTime newEnd);

    /**
     * Change the filter
     * @param filter
     */
    void setFilter(Filter filter);

    /**
     * Change the selected slot
     * @param newSlot pass nullptr if none selected
     */
    void setTimeElementSelection(TimedElement *newSlot);

private: // methods
    void updateSelection();
    void updateSlotSelection();

private: // data
    FileTrace *trace = nullptr;
    Trace *selection = nullptr;
    ViewSettings *settings = nullptr;

    types::TraceTime begin{0};
    types::TraceTime end{0};
};


#endif //MOTIV_TRACEDATAPROXY_HPP
