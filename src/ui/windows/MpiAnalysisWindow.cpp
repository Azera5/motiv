/*
 * Marvelous OTF2 Traces Interactive Visualizer (MOTIV)
 * Copyright (C) 2023 Jessica Lafontaine
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


#include <algorithm>
#include <unordered_set>
#include <QApplication>
#include <QCoreApplication>
#include <QErrorMessage>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QToolBar>
#include <QStatusBar>
#include <string>
#include <utility>

#include <QGraphicsScene>
#include <QGraphicsView>

#include "src/models/AppSettings.hpp"
#include "src/ui/widgets/infostrategies/InformationDockSlotStrategy.hpp"
#include "src/ui/widgets/infostrategies/InformationDockTraceStrategy.hpp"
#include "src/ui/widgets/infostrategies/InformationDockCommunicationStrategy.hpp"
#include "src/ui/widgets/infostrategies/InformationDockCollectiveCommunicationStrategy.hpp"
#include "src/ui/windows/MpiAnalysisWindow.hpp"
#include "src/ui/widgets/mpianalysis/LogicalClock.hpp"

#include "src/models/Slot.hpp"
#include <tuple>
#include <bitset>


MpiAnalysisWindow::MpiAnalysisWindow(QString filepath) : QMainWindow(nullptr), filepath(std::move(filepath)){
    if (this->filepath.isEmpty()) {
            this->promptFile();
        }
    
    AppSettings::getInstance().setColorConfigName(this->filepath);
    AppSettings::getInstance().setMode(MPI_Analysis);

    this->createMenus();
    this->loadTrace();
    this->createCentralWidget();
}

MpiAnalysisWindow::~MpiAnalysisWindow() {}


void MpiAnalysisWindow::createMenus(){
    auto menuBar = this->menuBar();

    /// File menu
    auto openTraceAction = new QAction(tr("&Open..."), this);
    openTraceAction->setShortcut(tr("Ctrl+O"));
    connect(openTraceAction, &QAction::triggered, this, &MpiAnalysisWindow::openNewTrace);
    auto openRecentMenu = new QMenu(tr("&Open recent"));
    if (AppSettings::getInstance().recentlyOpenedFiles().isEmpty()) {
        auto emptyAction = openRecentMenu->addAction(tr("&(Empty)"));
        emptyAction->setEnabled(false);
    } else {
        // TODO this is not updated on call to clear
        for (const auto &recent: AppSettings::getInstance().recentlyOpenedFiles()) {
            auto recentAction = new QAction(recent, openRecentMenu);
            openRecentMenu->addAction(recentAction);
            connect(recentAction, &QAction::triggered, [&, this] {
                this->openNewWindow(recent);
            });
        }
        openRecentMenu->addSeparator();

        auto clearRecentMenuAction = new QAction(tr("&Clear history"));
        openRecentMenu->addAction(clearRecentMenuAction);
        connect(clearRecentMenuAction, &QAction::triggered, [&, openRecentMenu] {
        AppSettings::getInstance().recentlyOpenedFilesClear(this->filepath);
            
        // Make a copy of the list of actions
        QList<QAction*> actions = openRecentMenu->actions();

        for (auto action : actions) {                
            if (action != clearRecentMenuAction && action->text()!= this->filepath) {
                openRecentMenu->removeAction(action);                    
                action->deleteLater();
            }
        }
        });
    }

    auto quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    auto fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(openTraceAction);
    fileMenu->addMenu(openRecentMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    /// Help menu
    auto showLicenseAction = new QAction(tr("&View license"));
    connect(showLicenseAction, &QAction::triggered, this, [this] {
        if(!this->licenseWindow) this->licenseWindow = new License;
        this->licenseWindow->show();
    });
    auto showHelpAction = new QAction(tr("&Show help"));
    showHelpAction->setShortcut(tr("Ctrl+H"));
    connect(showHelpAction, &QAction::triggered, this, [this] {
        if(!this->helpWindow) this->helpWindow = new Help;
        this->helpWindow->show();
    });
    // auto showAboutQtAction = new QAction(tr("&About Qt"));
    // connect(showAboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    // auto showAboutAction = new QAction(tr("&About"));
    // connect(showAboutAction, &QAction::triggered, this, [this] {
    //     if(!this->aboutWindow) this->aboutWindow= new About;
    //     this->aboutWindow->show();
    // });

    auto helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(showLicenseAction);
    helpMenu->addAction(showHelpAction);
    // helpMenu->addAction(showAboutQtAction);
    // helpMenu->addAction(showAboutAction);
}

void MpiAnalysisWindow::openNewTrace() {
    auto path = this->promptFile();
    this->openNewWindow(path);
}

//Open new Window in MPI Communication mode
void MpiAnalysisWindow::openNewWindow(QString path) {
    QStringList arguments;
    arguments << "-m" << "1" << path;
    QProcess::startDetached(QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath(),
                        arguments);
}

void MpiAnalysisWindow::createCentralWidget(){
    QWidget *centralWidget = new QWidget(this);   
    
    auto layout = new QGridLayout(centralWidget);
    

    auto logicalClock = new LogicalClock(this->data, this->nodes, this);
    
    QBrush backgroundPattern = QBrush(QColorConstants::Svg::silver, Qt::Dense7Pattern);
    logicalClock->setBackgroundBrush(backgroundPattern);

    layout->addWidget(logicalClock,0,1);
    
    createInformationWidget();
    layout->addWidget(this->information, 0,2);
    layout->setAlignment(this->information, Qt::AlignRight);
    
    centralWidget->setLayout(layout);
    this->setCentralWidget(centralWidget);

}

void MpiAnalysisWindow::createInformationWidget(){
    this->information = new InformationDock();
    information->addElementStrategy(new InformationDockSlotStrategy());
    information->addElementStrategy(new InformationDockTraceStrategy());
    information->addElementStrategy(new InformationDockCommunicationStrategy());
    information->addElementStrategy(new InformationDockCollectiveCommunicationStrategy());

    this->information->setElement(this->data->getFullTrace());
    // @formatter:off
    connect(information, SIGNAL(zoomToWindow(types::TraceTime, types::TraceTime)), data,
            SLOT(setSelection(types::TraceTime, types::TraceTime)));

    connect(data, SIGNAL(infoElementSelected(TimedElement * )), information, SLOT(setElement(TimedElement * )));
 
}

QString MpiAnalysisWindow::promptFile() {
    auto newFilePath = QFileDialog::getOpenFileName(this, QFileDialog::tr("Open trace"), QString(),
                                                    QFileDialog::tr("OTF Traces (*.otf *.otf2)"));

    // TODO this is still not really a great way to deal with that, especially for the initial open
    if (newFilePath.isEmpty()) {
        auto errorMsg = new QErrorMessage(nullptr);
        errorMsg->showMessage("The chosen file is invalid!");
    }

    return newFilePath;
}

void MpiAnalysisWindow::loadTrace() {

    QElapsedTimer loadTraceTimer;
    // if(testRun==true){
    //     loadTraceTimer.start();
    // }

    this->reader = new otf2::reader::reader(this->filepath.toStdString());

    this->callbacks = new ReaderCallbacks(*reader);

    this->reader->set_callback(*callbacks);
    this->reader->read_definitions();
    this->reader->read_events();


    auto slots = this->callbacks->getSlots();
    auto communications = this->callbacks->getCommunications();
    auto collectives = this->callbacks->getCollectiveCommunications();

    auto trace = new FileTrace(slots, communications, collectives, this->callbacks->duration());

    this->data = new TraceDataProxy(trace, this->settings, this);
    this->buildNodes();

    // if(testRun==true){
    //     std::cout << "%MainWindow::loadTrace()%" << loadTraceTimer.elapsed() << "%ms%";
    // }
}

constexpr size_t IDX_P2P_EVENTS = 0;
constexpr size_t IDX_COLLECTIVES_EVENTS = 1;

constexpr size_t IDX_NODES = 0;
constexpr size_t IDX_COMMUNICATIONS = 1;

constexpr size_t IDX_BARRIER = 0;
constexpr size_t IDX_COLL_ONE2ALL = 1;
constexpr size_t IDX_ALL2ONE = 2;
constexpr size_t IDX_COLL_ALL2ALL = 3;
constexpr size_t IDX_OTHER = 4;


void MpiAnalysisWindow::buildNodes(){

    // Pending Events: Connects Slots with corresponding CommunicationEvents (P2P or collective communication).
    using pendingP2PEvents =  std::tuple<std::vector<Node*>, std::vector<Communication*>>;
    using pendingCollectivesEvents =  std::tuple<std::vector<Node*>, std::map<uint64_t, std::vector<std::pair<CollectiveCommunicationEvent*, CollectiveCommunicationEvent::Member*>>>>;
   
    using pendingEvents = std::tuple<pendingP2PEvents, pendingCollectivesEvents>;

    // Pending Node to connect with matching Node (e.g sender, receiver), key is their CommunicationEvent    
    std::map<Communication*, Node*> pendingNodesP2P;
    
    std::map<CollectiveCommunicationEvent*, Node*> pendingNodesCollectives;
    
    std::map<int, pendingEvents> pendingMap;

    std::unordered_set<otf2::common::role_type> collectiveRoles ={
        otf2::common::role_type::barrier,
        otf2::common::role_type::coll_one2all,
        otf2::common::role_type::coll_all2one,
        otf2::common::role_type::coll_all2all,
        otf2::common::role_type::coll_other,
    };

    for(auto &item: data->getFullTrace()->getSlots()){
        for(auto &slot: item.second){
            Node* node_ = new Node(slot);           
            QString regionName = QString::fromStdString(slot->region->name().str());
            auto DEBUG_REGION_NAME = slot->region->name().str();
            auto roleType = slot->region->role();
            auto location = slot->location->ref(); 

            if(roleType == otf2::common::role_type::point2point){
                    if (pendingMap.find(location) == pendingMap.end()) pendingMap[location] = pendingEvents();
                    std::get<IDX_NODES>(std::get<IDX_P2P_EVENTS>(pendingMap[location])).push_back(node_);
            }else 

            if(collectiveRoles.find(roleType) != collectiveRoles.end()){
                if (pendingMap.find(location) == pendingMap.end()) pendingMap[location] = pendingEvents();
                std::get<IDX_NODES>(std::get<IDX_COLLECTIVES_EVENTS>(pendingMap[location])).push_back(node_);
            }            
            this->nodes[location].push_back(node_);
        }
            
    }
    
    //Pending communications
    auto allCommunications = data->getFullTrace()->getCommunications();
    for(auto &item: data->getFullTrace()->getCommunications()){
        const auto startEvent = item->getStartEvent();
        const auto endEvent = item->getEndEvent();
        auto startLoc = startEvent->getLocation()->ref();
        auto endLoc = endEvent->getLocation()->ref();

        if (pendingMap.find(startEvent->getLocation()->ref()) == pendingMap.end()) pendingMap[startEvent->getLocation()->ref()] = pendingEvents();
        if (pendingMap.find(endEvent->getLocation()->ref()) == pendingMap.end()) pendingMap[endEvent->getLocation()->ref()] = pendingEvents();

        // if(startLoc != endLoc){           
        std::get<IDX_COMMUNICATIONS>(std::get<IDX_P2P_EVENTS>(pendingMap[startLoc])).push_back(item);
        std::get<IDX_COMMUNICATIONS>(std::get<IDX_P2P_EVENTS>(pendingMap[endLoc])).push_back(item);
        // } else std::get<1>(pendingMap[startLoc]).push_back(item);
    }

    // Resort P2P communication by rank
    for (auto &item : pendingMap) {
        auto locationKey = item.first;
        auto &communications = std::get<IDX_COMMUNICATIONS>(std::get<IDX_P2P_EVENTS>(item.second));
        std::sort(communications.begin(), communications.end(), 
                  [this, locationKey](Communication* a, Communication* b) {
                      return this->customP2PCommunicationCompare(a, b, locationKey);
                  });
    }

    //Pending collective communications
    for(auto &item: data->getFullTrace()->getCollectiveCommunications()){
        auto typeIndex = getTypeIndex(item->getOperation());        
        for(auto &member: item->getMembers()){
            auto memberLocation = member->getLocation()->ref();
            if(pendingMap.find(memberLocation) == pendingMap.end()) pendingMap[memberLocation] = pendingEvents();               
            auto pair = std::make_pair(item,member);
            std::get<IDX_COMMUNICATIONS>(std::get<IDX_COLLECTIVES_EVENTS>(pendingMap[memberLocation]))[typeIndex].push_back(pair);
        }
    }
    
    //Adds communication to matching Node
    for (auto &locationEntry : pendingMap) {     
        auto &communications = std::get<IDX_COMMUNICATIONS>(std::get<IDX_P2P_EVENTS>(locationEntry.second));
        auto &collectives = std::get<IDX_COMMUNICATIONS>(std::get<IDX_COLLECTIVES_EVENTS>(locationEntry.second));
        
        for (auto node : std::get<IDX_NODES>(std::get<IDX_P2P_EVENTS>(locationEntry.second))){
            auto slot = node->getSlot();
            QString regionName = QString::fromStdString(slot->region->name().str());            

            if(!communications.empty()){
                node->setCommunication(communications.front());

                // Add connected Nodes
                if(pendingNodesP2P.find(communications.front()) == pendingNodesP2P.end()) pendingNodesP2P[communications.front()] = node;
                else {
                    node->addConnectedNodes(pendingNodesP2P[communications.front()]);
                    pendingNodesP2P[communications.front()]->addConnectedNodes(node);
                }
                communications.erase(communications.begin());
                //if(QString::fromStdString(node->getSlot().region->name().str()).contains("sendrecv",Qt::CaseInsensitive)) communications.erase(communications.begin());
            }
        }

        for (auto node : std::get<IDX_NODES>(std::get<IDX_COLLECTIVES_EVENTS>(locationEntry.second))) {           
            if(!collectives.empty()){
                auto typeIndex = getTypeIndex(node->getSlot()->region->role());
                node->setCollectiveCommunication(collectives[typeIndex].front().first);
                node->setCollectiveCommunicationMemberRef(collectives[typeIndex].front().second);

                if(pendingNodesCollectives.find(collectives[typeIndex].front().first) == pendingNodesCollectives.end()) pendingNodesCollectives[collectives[typeIndex].front().first] = node;
                else {
                    node->addConnectedNodes(pendingNodesCollectives[collectives[typeIndex].front().first]);
                    pendingNodesCollectives[collectives[typeIndex].front().first]->addConnectedNodes(node);
                }

                collectives[typeIndex].erase(collectives[typeIndex].begin());
            }
        }
               
    }
}

bool MpiAnalysisWindow::customP2PCommunicationCompare(Communication* a, Communication* b, uint64_t locationKey) {
    auto startEventA = a->getStartEvent();
    auto startEventB = b->getStartEvent();
    
    auto EndEventA = a->getEndEvent();
    auto EndEventB = b->getEndEvent();
    
    bool isStartEventAMatching = startEventA->getLocation()->ref() == locationKey;
    bool isStartEventBMatching = startEventB->getLocation()->ref() == locationKey;

    auto timeA = isStartEventAMatching ? startEventA->getStartTime() : EndEventA->getStartTime();
    auto timeB = isStartEventBMatching ? startEventB->getStartTime() : EndEventB->getStartTime();

    return timeA < timeB;
}

size_t MpiAnalysisWindow::getTypeIndex(otf2::common::collective_type collectiveType){
    switch(collectiveType){
        case otf2::common::collective_type::barrier:
            return IDX_BARRIER;
        case otf2::common::collective_type::broadcast:
        case otf2::common::collective_type::scatter:
        case otf2::common::collective_type::scatterv:
            return IDX_COLL_ONE2ALL;
        case otf2::common::collective_type::gather:
        case otf2::common::collective_type::gatherv:
        case otf2::common::collective_type::reduce:
            return IDX_ALL2ONE;
        case otf2::common::collective_type::all_gather:
        case otf2::common::collective_type::all_gatherv:
        case otf2::common::collective_type::all_to_all:
        case otf2::common::collective_type::all_to_allv:
        case otf2::common::collective_type::all_to_allw:
        case otf2::common::collective_type::all_reduce:
        case otf2::common::collective_type::reduce_scatter:
        case otf2::common::collective_type::reduce_scatter_block:
        case otf2::common::collective_type::scan:
        case otf2::common::collective_type::exscan:
            return IDX_COLL_ALL2ALL;
        case otf2::common::collective_type::create_handle:
        case otf2::common::collective_type::destroy_handle:
        case otf2::common::collective_type::allocate:
        case otf2::common::collective_type::deallocate:
        case otf2::common::collective_type::create_handle_and_allocate:
        case otf2::common::collective_type::destroy_handle_and_deallocate:
            return IDX_OTHER;
        default:
            return IDX_OTHER;
    } 
}

size_t MpiAnalysisWindow::getTypeIndex(otf2::common::role_type roleType){
      switch(roleType){
        case otf2::common::role_type::barrier: 
            return IDX_BARRIER;
        case otf2::common::role_type::coll_one2all:
            return IDX_COLL_ONE2ALL;
        case otf2::common::role_type::coll_all2one:
            return IDX_ALL2ONE;
        case otf2::common::role_type::coll_all2all:
            return IDX_COLL_ALL2ALL;
        default:
            return IDX_OTHER;
    }  
}