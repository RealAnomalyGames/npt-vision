#include "MainWindow.h"

#include <QActionGroup>
#include <QApplication>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <QStyle>
#include <QAction>
#include <QColor>
#include <QColorDialog>
#include <QImage>
#include <QDockWidget>
#include <QFile>
#include <QMouseEvent>
#include <QHeaderView>
#include <QListWidget>
#include <QMenu>
#include <QPen>
#include <QPixmap>
#include <QMenuBar>
#include <QPainter>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QSize>

class CanvasWidget final : public QWidget
{
public:
    enum class Tool
    {
        Pencil,
        Eraser
    };

    explicit CanvasWidget(QWidget* parent = nullptr)
        : QWidget(parent),
          drawing_(640, 360, QImage::Format_ARGB32_Premultiplied)
    {
        drawing_.fill(Qt::transparent);

        setMinimumSize(400, 300);
        setMouseTracking(true);
        setAutoFillBackground(false);
    }

    void setTool(Tool tool)
    {
        tool_ = tool;
        update();
    }

    void setDrawingColor(const QColor& color)
    {
        color_ = color;
    }

    QColor drawingColor() const
    {
        return color_;
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), QColor("#303238"));

        const QRect canvas = canvasRect();

        if (canvas.width() <= 0 || canvas.height() <= 0)
            return;

        painter.fillRect(canvas, Qt::white);

        // Subtle grid, matching the existing canvas appearance.
        painter.save();
        painter.setClipRect(canvas);
        painter.setPen(QPen(QColor("#e4e4e4"), 1));

        constexpr int gridSize = 24;

        for (int x = canvas.left() + gridSize;
             x < canvas.right(); x += gridSize)
        {
            painter.drawLine(x, canvas.top(), x, canvas.bottom());
        }

        for (int y = canvas.top() + gridSize;
             y < canvas.bottom(); y += gridSize)
        {
            painter.drawLine(canvas.left(), y, canvas.right(), y);
        }

        painter.restore();

        // Draw the artwork over the background grid.
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(canvas, drawing_);

        painter.setPen(QPen(QColor("#808080"), 1));
        painter.drawRect(canvas);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton)
            return;

        const QPoint point = toDrawingPoint(event->position().toPoint());

        if (!QRect(QPoint(0, 0), drawing_.size()).contains(point))
            return;

        drawing_ = drawing_.copy();
        lastPoint_ = point;
        drawingInProgress_ = true;

        drawTo(point);
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (!drawingInProgress_ ||
            !(event->buttons() & Qt::LeftButton))
        {
            return;
        }

        drawTo(toDrawingPoint(event->position().toPoint()));
        update();
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
            drawingInProgress_ = false;
    }

private:
    QRect canvasRect() const
    {
        const int canvasWidth = qMin(640, width() - 48);
        const int canvasHeight = qMin(360, height() - 48);

        if (canvasWidth <= 0 || canvasHeight <= 0)
            return {};

        return QRect(
            (width() - canvasWidth) / 2,
            (height() - canvasHeight) / 2,
            canvasWidth,
            canvasHeight
        );
    }

    QPoint toDrawingPoint(const QPoint& point) const
    {
        const QRect canvas = canvasRect();

        if (canvas.width() <= 0 || canvas.height() <= 0)
            return QPoint(-1, -1);

        return QPoint(
            (point.x() - canvas.left()) * drawing_.width()
                / canvas.width(),
            (point.y() - canvas.top()) * drawing_.height()
                / canvas.height()
        );
    }

    void drawTo(const QPoint& point)
    {
        const QRect bounds(QPoint(0, 0), drawing_.size());

        if (!bounds.contains(point))
        {
            lastPoint_ = point;
            return;
        }

        QPainter painter(&drawing_);
        painter.setRenderHint(QPainter::Antialiasing, false);

        if (tool_ == Tool::Eraser)
        {
            painter.setCompositionMode(QPainter::CompositionMode_Clear);
        }
        else
        {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(color_, 3, Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
        }

        if (tool_ == Tool::Eraser)
        {
            painter.setPen(QPen(Qt::transparent, 12, Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
        }

        painter.drawLine(lastPoint_, point);
        lastPoint_ = point;
    }

    QImage drawing_;
    QColor color_ = Qt::black;
    Tool tool_ = Tool::Pencil;
    QPoint lastPoint_;
    bool drawingInProgress_ = false;
};

namespace
{
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

    qApp->setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #c0c0c0;
            color: #000000;
        }

        QMenuBar {
            background-color: #c0c0c0;
            border-bottom: 1px solid #808080;
        }

        QMenuBar::item {
            padding: 4px 8px;
            background: transparent;
        }

        QMenuBar::item:selected {
            background-color: #000080;
            color: #ffffff;
        }

        QMenu {
            background-color: #c0c0c0;
            border: 1px solid #000000;
        }

        QMenu::item {
            padding: 4px 24px 4px 20px;
        }

        QMenu::item:selected {
            background-color: #000080;
            color: #ffffff;
        }

        QToolBar {
            background-color: #c0c0c0;
            border: none;
            spacing: 3px;
            padding: 4px;
        }

        QToolButton {
            background-color: #c0c0c0;
            border-top: 2px solid #ffffff;
            border-left: 2px solid #ffffff;
            border-right: 2px solid #404040;
            border-bottom: 2px solid #404040;
            padding: 5px;
            min-width: 62px;
            min-height: 28px;
        }

        QToolButton:pressed,
        QToolButton:checked {
            border-top: 2px solid #404040;
            border-left: 2px solid #404040;
            border-right: 2px solid #ffffff;
            border-bottom: 2px solid #ffffff;
            background-color: #b0b0b0;
        }

        QDockWidget::title {
            background-color: #000080;
            color: #ffffff;
            padding: 4px;
            text-align: left;
        }

        QDockWidget::title:!active {
            background-color: #000080;
            color: #ffffff;
        }

        QTableWidget, QListWidget {
            background-color: #ffffff;
            color: #000000;
            border-top: 2px solid #404040;
            border-left: 2px solid #404040;
            border-right: 2px solid #ffffff;
            border-bottom: 2px solid #ffffff;
            gridline-color: #808080;
        }

        QStatusBar {
            background-color: #c0c0c0;
            border-top: 1px solid #808080;
        }

        QStatusBar::item {
            border: none;
        }
    )");

    canvasWidget_ = new CanvasWidget(this);
    setCentralWidget(canvasWidget_);

    toolStatusLabel_ = new QLabel("Tool: Pencil", this);
    statusBar()->addPermanentWidget(toolStatusLabel_);

    createMenus();
    createToolBar();
    createTimeline();
    createLayersPanel();

    statusBar()->showMessage("Ready | NPT Vision 0.1.0");
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

namespace
{
QIcon makeToolIcon(const QString& name, const QColor& color = Qt::black)
{
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (name == "Pencil")
    {
        painter.setPen(QPen(QColor("#404040"), 1));
        painter.setBrush(QColor("#f4d35e"));
        painter.drawPolygon(QPolygon({
            QPoint(4, 15), QPoint(14, 5),
            QPoint(17, 8), QPoint(7, 18)
        }));

        painter.setPen(QPen(QColor("#404040"), 1));
        painter.drawLine(4, 15, 3, 18);
        painter.drawLine(3, 18, 7, 18);
    }
    else if (name == "Eraser")
    {
        painter.setPen(QPen(QColor("#404040"), 1));
        painter.setBrush(QColor("#f0a0a0"));
        painter.drawPolygon(QPolygon({
            QPoint(3, 12), QPoint(11, 4),
            QPoint(18, 11), QPoint(12, 17),
            QPoint(7, 17)
        }));
        painter.setPen(QPen(Qt::white, 1));
        painter.drawLine(7, 17, 12, 11);
    }
    else if (name == "Color")
    {
        painter.setPen(QPen(QColor("#404040"), 1));
        painter.setBrush(color);
        painter.drawRect(3, 3, 14, 14);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawLine(5, 14, 14, 5);
    }

    return QIcon(pixmap);
}
}

void MainWindow::createToolBar()
{
    auto* toolbar = new QToolBar("Drawing Tools", this);
    toolbar->setObjectName("DrawingTools");
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setOrientation(Qt::Vertical);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setIconSize(QSize(20, 20));

    toolGroup_ = new QActionGroup(this);
    toolGroup_->setExclusive(true);

    auto* pencil = toolbar->addAction(makeToolIcon("Pencil"), "Pencil");
    pencil->setCheckable(true);
    pencil->setChecked(true);
    pencil->setToolTip("Draw using the selected color");

    auto* eraser = toolbar->addAction(makeToolIcon("Eraser"), "Eraser");
    eraser->setCheckable(true);
    eraser->setToolTip("Erase artwork");

    toolGroup_->addAction(pencil);
    toolGroup_->addAction(eraser);

    auto* colorAction = toolbar->addAction(
        makeToolIcon("Color", canvasWidget_->drawingColor()),
        "Color..."
    );
    colorAction->setToolTip("Choose the pencil color");

    connect(pencil, &QAction::triggered, this, [this]()
    {
        canvasWidget_->setTool(CanvasWidget::Tool::Pencil);
        toolStatusLabel_->setText("Tool: Pencil");
        statusBar()->showMessage("Pencil selected");
    });

    connect(eraser, &QAction::triggered, this, [this]()
    {
        canvasWidget_->setTool(CanvasWidget::Tool::Eraser);
        toolStatusLabel_->setText("Tool: Eraser");
        statusBar()->showMessage("Eraser selected");
    });

    connect(colorAction, &QAction::triggered, this, [this, colorAction]()
    {
        const QColor chosen = QColorDialog::getColor(
            canvasWidget_->drawingColor(),
            this,
            "Choose Drawing Color"
        );

        if (!chosen.isValid())
            return;

        canvasWidget_->setDrawingColor(chosen);
        colorAction->setIcon(makeToolIcon("Color", chosen));
        statusBar()->showMessage(
            QString("Drawing color: %1").arg(chosen.name())
        );
    });

    addToolBar(Qt::LeftToolBarArea, toolbar);
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
