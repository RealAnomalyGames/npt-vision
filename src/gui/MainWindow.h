#pragma once

#include <QMainWindow>

class QActionGroup;
class QLabel;
class QWidget;
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

    CanvasWidget* canvasWidget_ = nullptr;
    QActionGroup* toolGroup_ = nullptr;
    QLabel* toolStatusLabel_ = nullptr;
};
