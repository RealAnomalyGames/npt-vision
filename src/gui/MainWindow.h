#pragma once

#include <QMainWindow>

class QActionGroup;
class QLabel;
class QTableWidget;
class CanvasWidget;

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

    void addFrame();
    void removeFrame();
    void selectFrame(int frameIndex);

    CanvasWidget* canvasWidget_ = nullptr;
    QActionGroup* toolGroup_ = nullptr;
    QLabel* toolStatusLabel_ = nullptr;
    QTableWidget* timelineTable_ = nullptr;
};
