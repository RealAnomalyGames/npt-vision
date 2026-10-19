#pragma once

#include <QMainWindow>

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void createMenus();
    void createTimeline();
    void createLayersPanel();
};
