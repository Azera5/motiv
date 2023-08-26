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
#include "TimelineLabelList.hpp"
#include "src/models/ViewSettings.hpp"

#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <set>
#include <vector>

#include <QMenu>

TimelineLabelList::TimelineLabelList(TraceDataProxy *data, QWidget *parent) : QListWidget(parent), data(data) {
    this->setFrameShape(QFrame::NoFrame);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->menu = new QMenu(this);
    this->menu->setStyleSheet("QMenu { background-color: rgba(250, 250, 250, 180); border: 2px solid rgba(170, 170, 170, 120); } QMenu::item:selected { background-color: rgba(250, 250, 250, 230); color: black; } ");
    this->labelAction1 = this->menu->addAction("highlight process");
    this->labelAction2 = this->menu->addAction("ignore communication");
    this->labelAction3 = this->menu->addAction("show flamegraph");
    connect(this->labelAction1, &QAction::triggered, this, &TimelineLabelList::highlightPreparation);
    connect(this->labelAction2, &QAction::triggered, this, &TimelineLabelList::ignoreCommPreparation);
    connect(this->labelAction3, &QAction::triggered, this, &TimelineLabelList::flamegraphPreparation);

    // This prevents asynchronous scrolling relative to TimelineView
    this->setVerticalScrollMode(ScrollPerPixel);
    this->setStyleSheet("background: transparent");
    // Why top=20? <= top in TimelineView.cpp
    //setViewportMargins(0, 20, 0, 0);
    // Alternative with a bufferzone we can scroll into
    auto extraSpaceTop = new QListWidgetItem(this);
    extraSpaceTop->setSizeHint(QSize(0, 20));
    this->addItem(extraSpaceTop);
 

    for (const auto &rank: this->data->getSelection()->getSlots()) {
        const QString &rankName = QString::fromStdString(rank.first->name().str());
        auto rankThreadMap = ViewSettings::getInstance()->getRankThreadMap();
        auto plusIcon = ViewSettings::getInstance()->getIcon("plus");
        auto plusIconGrey = ViewSettings::getInstance()->getIcon("plus_grey");
        auto minusIcon = ViewSettings::getInstance()->getIcon("minus");
        auto item = new QListWidgetItem(this);
        

        //QFont font = item->font();
        //font.setPointSize(12);
        //item->setFont(font);
        if(maxLabelLength < fontMetrics().boundingRect(rankName).width()) maxLabelLength=fontMetrics().boundingRect(rankName).width();
        if(maxLabelLength < rankName.size())maxLabelLength=rankName.size();

        // Multithreading check
        // If we haven't seen this rank before...
        if(rankThreadMap->count(rank.first->ref().get())==0){            
            std::map<std::string, std::pair<int, std::vector<bool>>> threadMap{};
            // We use that one to figure out the thread position <=> threadNumber
            std::map<std::string, std::string> threadMap_{};
            //qInfo() << "... working on rank ..." << rank.first->ref().get();
            for (const auto &slot_: rank.second) {
                auto threadRef = std::to_string(slot_->location->ref().get());
                auto threadName = slot_->location->name().str();
                // If we haven't seen this thread before...
                if(!threadMap_.count(threadRef)){
                    threadMap_.insert({threadName, threadRef});
                }
            }
            //std::make_pair(threadRef, bool[3]{false, false, false})
            // Whats their position? (threadNumber for main thread => 1)
            auto threadNumber = threadMap.size();
            for (auto &threadItem: threadMap_) {
                threadNumber++;
                std::vector<bool> boolVector {false, false, false};
                std::pair<int, std::vector<bool>> entry {threadNumber, boolVector};
                //qInfo() << threadItem.first.c_str() << "... is nr. ..." << threadNumber;
                threadMap.insert({threadItem.second, entry});
            }
            rankThreadMap->insert({rank.first->ref().get(), std::make_pair(false, threadMap)});
        }

        // Determine the fitting icon
        auto iconToggleStatus = rankThreadMap->at(rank.first->ref().get()).first;
        auto rankSizeOffset = rankThreadMap->at(rank.first->ref().get()).second.size()-1;
        QBrush backgroundPattern = QBrush(QColorConstants::Svg::lightsteelblue, Qt::Dense4Pattern);

        // If the amount of threads (<=> rankSizeOffset+1) < 2 => non-expandable <=> grey icon
        if(rankSizeOffset<1){
            item->setIcon(*plusIconGrey);
            backgroundPattern.setColor(QColorConstants::Svg::silver);
        }
        else{
            iconToggleStatus ? (item->setIcon(*minusIcon), backgroundPattern.setStyle(Qt::Dense6Pattern)) : item->setIcon(*plusIcon);
        }
        //qInfo() << "rank " << rank.first->ref().get() << " has" << rankThreadMap->at(rank.first->ref().get()).second.size() << "threads";

        item->setText(rankName);
        item->setData(Qt::UserRole, rank.first->ref().get());
        item->setToolTip(QString::fromStdString("node: "+rank.first->parent().name().str()));
        //item->setSizeHint(QSize(0, this->ROW_HEIGHT+(rankOffsetMap->at(rankName)*this->ROW_HEIGHT*toggledRankMap->at(rankName))));
        item->setSizeHint(QSize(0, this->ROW_HEIGHT+(rankSizeOffset*this->ROW_HEIGHT*iconToggleStatus)));
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackground(backgroundPattern); 
        this->addItem(item);
    }
    auto extraSpaceBottom = new QListWidgetItem(this);
    extraSpaceBottom->setSizeHint(QSize(0, this->ROW_HEIGHT));
    this->addItem(extraSpaceBottom);
}

void TimelineLabelList::mousePressEvent(QMouseEvent *event) {
    qInfo() << "mousePressEvent";
    // First: determine where the click happend
    auto *item = this->itemAt(event->pos());  
    // Assuming we have clicked on a rank...
    if (item != nullptr && event->button()==Qt::LeftButton) {
        auto settings = ViewSettings::getInstance();
        QString rankName = item->text();
        int rankRef = item->data(Qt::UserRole).toInt();
        //qInfo() << "expansion ---> " << rankRef;
        // We don't really care if the threads were expanded or not...
        bool formerStatus = settings->getRankThreadMap()->at(rankRef).first;
        // ... we just want to flip the bool value for every mousePressEvent 
        settings->getRankThreadMap()->insert_or_assign(rankRef, std::pair(!formerStatus, settings->getRankThreadMap()->at(rankRef).second));
        Q_EMIT this->data->labelInteractionTrigger();
    }
    else if (item != nullptr && event->button()==Qt::RightButton) {
        item->setSelected(true);
        int rankRef = item->data(Qt::UserRole).toInt();
        this->labelAction3->setData(rankRef);
        this->menu->exec(event->globalPos());
    }
    return;
}

void TimelineLabelList::mouseReleaseEvent(QMouseEvent *) {
    return;
}

void TimelineLabelList::mouseMoveEvent(QMouseEvent *) {
    return;
}

int TimelineLabelList::getMaxLabelLength() {
    return maxLabelLength;
}

void TimelineLabelList::highlightPreparation(){
    qInfo() << "highlight ...";
    //Q_EMIT this->data->labelInteractionTrigger();
}

void TimelineLabelList::ignoreCommPreparation(){
    qInfo() << "ignore ...";
    //Q_EMIT this->data->labelInteractionTrigger();
}

void TimelineLabelList::flamegraphPreparation(){
    
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    int rankRef = action->data().toInt();
    auto settings = ViewSettings::getInstance();
    settings->setFlamegraphRankRef(rankRef);

    Q_EMIT this->data->flamegraphRequest();
    qInfo() << "flamegraph ...";
}