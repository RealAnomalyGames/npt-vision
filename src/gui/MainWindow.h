#pragma once

#include <QMainWindow>

class QActionGroup;
class QLabel;
class QListWidget;
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

    void addLayer();
    void removeLayer();
    void selectLayer(int layerIndex);
    void linkPreviousCel();
    void makeCelIndependent();
    void refreshTimeline();

    CanvasWidget* canvasWidget_ = nullptr;
    QActionGroup* toolGroup_ = nullptr;
    QLabel* toolStatusLabel_ = nullptr;
    QTableWidget* timelineTable_ = nullptr;
    QListWidget* layersList_ = nullptr;
};
