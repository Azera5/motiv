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
#include "TimelineHeader.hpp"

#include <QHBoxLayout>
#include <QLabel>

#include "TimeUnitLabel.hpp"
#include "src/utils.hpp"

TimelineHeader::TimelineHeader(TraceDataProxy *data, QWidget *parent) : QWidget(parent), data(data) {
    this->setLayout(new QHBoxLayout(this));
    this->updateView();

    // @formatter:off
    // Attention: this connect causes redundant rendering of the scenery for TimelineView
    // connect(this->data, SIGNAL(selectionChanged(types::TraceTime,types::TraceTime)), this, SLOT(updateView()));
    // @formatter:on
}

void TimelineHeader::updateView() {
    auto layout = this->layout();
    auto selection = this->data->getSelection();
    auto runtime = static_cast<double>(selection->getRuntime().count());
    auto begin = static_cast<double>(selection->getStartTime().count());

    resetLayout(layout);

    int marks = 4;
    for (int i = 0; i <= marks; i++) {
        double num = begin + static_cast<double>(i)/marks * runtime;
        auto label = new TimeUnitLabel(num, this);
        layout->addWidget(label);
        if (i != marks) {
            dynamic_cast<QHBoxLayout *>(layout)->addStretch();
        }
    }
}
