#pragma once

#include <QMainWindow>

class QActionGroup;
class QLabel;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void createMenus();
    void createToolBar();
    void createTimeline();
    void createLayersPanel();

    QActionGroup* toolGroup_ = nullptr;
    QLabel* toolStatusLabel_ = nullptr;
};
