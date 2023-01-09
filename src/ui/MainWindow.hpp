#ifndef MOTIV_MAINWINDOW_HPP
#define MOTIV_MAINWINDOW_HPP


#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

Q_SIGNALS:
    // TODO

public Q_SLOTS:
    // TODO

private: // methods
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void createCentralWidget();

private: // widgets
    QToolBar *topToolbar = nullptr;
    QToolBar *bottomToolbar = nullptr;

    QList<QDockWidget *> *docks = nullptr;

private: // properties
    QString filepath;
};


#endif //MOTIV_MAINWINDOW_HPP
