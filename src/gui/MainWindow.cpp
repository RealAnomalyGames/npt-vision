#include "../gui/MainWindow.h"

#include <QAction>
#include <QColor>
#include <QDockWidget>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
class CanvasWidget final : public QWidget
{
public:
    explicit CanvasWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(400, 300);
        setAutoFillBackground(true);

        QPalette colors = palette();
        colors.setColor(QPalette::Window, QColor("#303238"));
        setPalette(colors);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);

        painter.fillRect(rect(), QColor("#303238"));

        // Draw a centered white animation canvas.
        const int canvasWidth = qMin(640, width() - 48);
        const int canvasHeight = qMin(360, height() - 48);

        if (canvasWidth <= 0 || canvasHeight <= 0)
            return;

        const QRect canvas(
            (width() - canvasWidth) / 2,
            (height() - canvasHeight) / 2,
            canvasWidth,
            canvasHeight
        );

        painter.fillRect(canvas, Qt::white);

        painter.setPen(QPen(QColor("#d8d8d8"), 1));

        constexpr int gridSize = 24;

        for (int x = canvas.left() + gridSize;
             x < canvas.right();
             x += gridSize)
        {
            painter.drawLine(x, canvas.top(), x, canvas.bottom());
        }

        for (int y = canvas.top() + gridSize;
             y < canvas.bottom();
             y += gridSize)
        {
            painter.drawLine(canvas.left(), y, canvas.right(), y);
        }

        painter.setPen(QPen(QColor("#8c8c8c"), 1));
        painter.drawRect(canvas);

        painter.setPen(QColor("#777777"));
        painter.drawText(
            canvas,
            Qt::AlignCenter,
            "NPT Vision - Drawing Canvas\n\n"
            "Drawing tools are coming soon."
        );
    }
};

QDockWidget* makeDock(
    const QString& title,
    QWidget* content,
    QWidget* parent)
{
    auto* dock = new QDockWidget(title, parent);
    dock->setObjectName(title);
    dock->setWidget(content);
    dock->setMinimumWidth(160);

    return dock;
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("NPT Vision 0.1.0");
    resize(1100, 720);
    setMinimumSize(800, 500);

    auto* canvas = new CanvasWidget(this);
    setCentralWidget(canvas);

    createMenus();
    createTimeline();
    createLayersPanel();

    statusBar()->showMessage(
        "Ready | NPT Vision 0.1.0"
    );
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QMenu* viewMenu = menuBar()->addMenu("&View");
    QMenu* helpMenu = menuBar()->addMenu("&Help");

    QAction* newProject = fileMenu->addAction("&New Project");
    newProject->setShortcut(QKeySequence::New);
    connect(newProject, &QAction::triggered, this, [this]()
    {
        statusBar()->showMessage(
            "Project creation will be implemented next."
        );
    });

    QAction* openProject = fileMenu->addAction("&Open Project...");
    openProject->setShortcut(QKeySequence::Open);
    connect(openProject, &QAction::triggered, this, [this]()
    {
        statusBar()->showMessage(
            "Project loading will be implemented next."
        );
    });

    QAction* saveProject = fileMenu->addAction("&Save Project");
    saveProject->setShortcut(QKeySequence::Save);
    connect(saveProject, &QAction::triggered, this, [this]()
    {
        statusBar()->showMessage(
            "Project saving will be implemented next."
        );
    });

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered,
            this, &QWidget::close);

    QAction* undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setEnabled(false);

    QAction* redoAction = editMenu->addAction("&Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setEnabled(false);

    QAction* timelineAction = viewMenu->addAction("Timeline");
    timelineAction->setCheckable(true);
    timelineAction->setChecked(true);

    QAction* layersAction = viewMenu->addAction("Layers");
    layersAction->setCheckable(true);
    layersAction->setChecked(true);

    QAction* aboutAction = helpMenu->addAction("&About NPT Vision");
    connect(aboutAction, &QAction::triggered, this, [this]()
    {
        statusBar()->showMessage(
            "NPT Vision 0.1.0 - Developed by NPT Software"
        );
    });

    // The dock widgets are connected after they are created.
    connect(timelineAction, &QAction::triggered, this, [this](bool visible)
    {
        if (QDockWidget* dock = findChild<QDockWidget*>("Timeline"))
            dock->setVisible(visible);
    });

    connect(layersAction, &QAction::triggered, this, [this](bool visible)
    {
        if (QDockWidget* dock = findChild<QDockWidget*>("Layers"))
            dock->setVisible(visible);
    });
}

void MainWindow::createTimeline()
{
    auto* timeline = new QTableWidget(2, 12, this);
    timeline->setObjectName("TimelineTable");
    timeline->setVerticalHeaderLabels({"Layer 1", "Layer 2"});
    timeline->verticalHeader()->setDefaultSectionSize(28);
    timeline->verticalHeader()->setSectionResizeMode(
        QHeaderView::Fixed
    );
    timeline->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Fixed
    );
    timeline->horizontalHeader()->setDefaultSectionSize(48);
    timeline->setHorizontalHeaderLabels({
        "1", "2", "3", "4", "5", "6",
        "7", "8", "9", "10", "11", "12"
    });
    timeline->setSelectionMode(QAbstractItemView::SingleSelection);
    timeline->setSelectionBehavior(QAbstractItemView::SelectItems);
    timeline->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timeline->setAlternatingRowColors(true);

    auto* dock = makeDock("Timeline", timeline, this);
    dock->setObjectName("Timeline");
    dock->setMinimumHeight(140);

    addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::createLayersPanel()
{
    auto* layers = new QListWidget(this);
    layers->addItem("Layer 1");
    layers->addItem("Layer 2");
    layers->setCurrentRow(0);

    auto* dock = makeDock("Layers", layers, this);
    dock->setObjectName("Layers");

    addDockWidget(Qt::RightDockWidgetArea, dock);
}
