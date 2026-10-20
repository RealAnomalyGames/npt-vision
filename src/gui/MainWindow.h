#pragma once

#include <QMainWindow>
#include <QAction>
#include <QTimer>

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

    void undo();
    void redo();
    void updateUndoRedoActions();

    void togglePlayback();
    void stopPlayback();
    void advancePlayback();
    void setPlaybackFps(int fps);

    CanvasWidget* canvasWidget_ = nullptr;
    QActionGroup* toolGroup_ = nullptr;
    QLabel* toolStatusLabel_ = nullptr;
    QTableWidget* timelineTable_ = nullptr;
    QListWidget* layersList_ = nullptr;
    QAction* undoAction_ = nullptr;
    QAction* redoAction_ = nullptr;
    QAction* playAction_ = nullptr;
    QTimer* playbackTimer_ = nullptr;
    int playbackFps_ = 12;
    bool isPlaying_ = false;
};
