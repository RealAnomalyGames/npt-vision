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
#include <QRect>
#include <QPixmap>
#include <QMenuBar>
#include <QPainter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QSize>
#include <QPushButton>
#include <QSignalBlocker>

#include <memory>
#include <vector>

class CanvasWidget final : public QWidget
{
public:
    enum class Tool { Pencil, Eraser };

    static constexpr int DrawingWidth = 640;
    static constexpr int DrawingHeight = 360;

    struct Cel
    {
        std::shared_ptr<QImage> image;
    };

    struct Layer
    {
        QString name;
        std::vector<Cel> cels;
    };

    explicit CanvasWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(400, 300);
        setMouseTracking(true);
        addLayer("Layer 1");
    }

    int frameCount() const
    {
        return layers_.empty()
            ? 0 : static_cast<int>(layers_[0].cels.size());
    }

    int layerCount() const
    {
        return static_cast<int>(layers_.size());
    }

    int currentFrame() const { return currentFrame_; }
    int currentLayer() const { return currentLayer_; }

    QString layerName(int index) const
    {
        if (index < 0 || index >= layerCount())
            return {};
        return layers_[index].name;
    }

    bool selectFrame(int index)
    {
        if (index < 0 || index >= frameCount())
            return false;
        currentFrame_ = index;
        drawingInProgress_ = false;
        update();
        return true;
    }

    bool selectLayer(int index)
    {
        if (index < 0 || index >= layerCount())
            return false;
        currentLayer_ = index;
        drawingInProgress_ = false;
        update();
        return true;
    }

    void addFrame()
    {
        for (auto& layer : layers_)
            layer.cels.push_back({makeBlankImage()});

        currentFrame_ = frameCount() - 1;
        drawingInProgress_ = false;
        update();
    }

    bool removeFrame(int index)
    {
        if (frameCount() <= 1 ||
            index < 0 || index >= frameCount())
            return false;

        for (auto& layer : layers_)
            layer.cels.erase(layer.cels.begin() + index);

        if (currentFrame_ >= frameCount())
            currentFrame_ = frameCount() - 1;
        else if (currentFrame_ > index)
            --currentFrame_;

        drawingInProgress_ = false;
        update();
        return true;
    }

    bool isCelLinked(int layerIndex, int frameIndex) const
    {
        if (layerIndex < 0 || layerIndex >= layerCount() ||
            frameIndex < 0 || frameIndex >= frameCount())
            return false;

        return layers_[layerIndex].cels[frameIndex].image.use_count() > 1;
    }

    void addLayer(const QString& name = {})
    {
        Layer layer;
        layer.name = name.isEmpty()
            ? QString("Layer %1").arg(layerCount() + 1)
            : name;

        for (int i = 0; i < qMax(1, frameCount()); ++i)
            layer.cels.push_back({makeBlankImage()});

        layers_.push_back(std::move(layer));
        currentLayer_ = layerCount() - 1;
        update();
    }

    bool removeLayer(int index)
    {
        if (layerCount() <= 1 ||
            index < 0 || index >= layerCount())
            return false;

        layers_.erase(layers_.begin() + index);

        if (currentLayer_ >= layerCount())
            currentLayer_ = layerCount() - 1;
        else if (currentLayer_ > index)
            --currentLayer_;

        update();
        return true;
    }

    bool linkCurrentCelToPrevious()
    {
        if (currentFrame_ <= 0)
            return false;

        auto& cels = layers_[currentLayer_].cels;
        cels[currentFrame_].image = cels[currentFrame_ - 1].image;
        update();
        return true;
    }

    void makeCurrentCelIndependent()
    {
        Cel& cel = layers_[currentLayer_].cels[currentFrame_];
        cel.image = std::make_shared<QImage>(*cel.image);
        update();
    }

    void setTool(Tool tool) { tool_ = tool; }
    void setDrawingColor(const QColor& color) { color_ = color; }
    QColor drawingColor() const { return color_; }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), QColor("#303238"));

        const QRect canvas = canvasRect();
        if (canvas.isEmpty())
            return;

        painter.fillRect(canvas, Qt::white);

        painter.save();
        painter.setClipRect(canvas);
        painter.setPen(QPen(QColor("#e4e4e4"), 1));

        for (int x = canvas.left() + 24; x < canvas.right(); x += 24)
            painter.drawLine(x, canvas.top(), x, canvas.bottom());

        for (int y = canvas.top() + 24; y < canvas.bottom(); y += 24)
            painter.drawLine(canvas.left(), y, canvas.right(), y);

        painter.restore();

        // Composite layers from bottom to top.
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        for (const auto& layer : layers_)
        {
            if (currentFrame_ < static_cast<int>(layer.cels.size()) &&
                layer.cels[currentFrame_].image)
            {
                painter.drawImage(
                    canvas, *layer.cels[currentFrame_].image
                );
            }
        }

        painter.setPen(QPen(QColor("#808080"), 1));
        painter.drawRect(canvas);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton)
            return;

        const QRect canvas = canvasRect();
        if (!canvas.contains(event->position().toPoint()))
            return;

        lastPoint_ = toDrawingPoint(event->position().toPoint());
        drawingInProgress_ = true;
        drawTo(lastPoint_);
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (!drawingInProgress_ ||
            !(event->buttons() & Qt::LeftButton))
            return;

        drawTo(toDrawingPoint(event->position().toPoint()));
        update();
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
            drawingInProgress_ = false;
    }

private:
    static std::shared_ptr<QImage> makeBlankImage()
    {
        auto image = std::make_shared<QImage>(
            DrawingWidth, DrawingHeight,
            QImage::Format_ARGB32_Premultiplied
        );
        image->fill(Qt::transparent);
        return image;
    }

    QRect canvasRect() const
    {
        const int w = qMin(DrawingWidth, width() - 48);
        const int h = qMin(DrawingHeight, height() - 48);
        if (w <= 0 || h <= 0)
            return {};

        return QRect((width() - w) / 2, (height() - h) / 2, w, h);
    }

    QPoint toDrawingPoint(const QPoint& point) const
    {
        const QRect canvas = canvasRect();
        if (canvas.isEmpty())
            return {-1, -1};

        return {
            (point.x() - canvas.left()) * DrawingWidth / canvas.width(),
            (point.y() - canvas.top()) * DrawingHeight / canvas.height()
        };
    }

    void drawTo(const QPoint& point)
    {
        const QRect bounds(0, 0, DrawingWidth, DrawingHeight);
        if (!bounds.contains(point))
        {
            lastPoint_ = point;
            return;
        }

        // Copy-on-write: preserve linked drawings if a cel is edited.
        auto& image = layers_[currentLayer_].cels[currentFrame_].image;
        if (!image.unique())
            image = std::make_shared<QImage>(*image);

        QPainter painter(image.get());
        painter.setRenderHint(QPainter::Antialiasing, false);

        if (tool_ == Tool::Eraser)
        {
            painter.setCompositionMode(QPainter::CompositionMode_Clear);
            painter.setPen(QPen(Qt::transparent, 12, Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
        }
        else
        {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(color_, 3, Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
        }

        painter.drawLine(lastPoint_, point);
        lastPoint_ = point;
    }

    std::vector<Layer> layers_;
    int currentFrame_ = 0;
    int currentLayer_ = 0;

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

    auto* titleBar = new QWidget(dock);
    titleBar->setFixedHeight(24);
    titleBar->setStyleSheet(
        "background-color: #000080;"
    );

    auto* layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(6, 0, 4, 0);
    layout->setSpacing(4);

    auto* label = new QLabel(title, titleBar);
    label->setStyleSheet(
        "color: #ffffff;"
        "background-color: transparent;"
        "font-weight: bold;"
    );

    layout->addWidget(label);
    layout->addStretch();

    dock->setTitleBarWidget(titleBar);

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

    // Connect the menu actions to the dock widgets.
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
    auto* content = new QWidget(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(4, 4, 4, 4);

    auto* controls = new QHBoxLayout();
    auto* addButton = new QPushButton("+ Frame", content);
    auto* removeButton = new QPushButton("- Frame", content);
    auto* holdDrawingButton = new QPushButton("Hold Drawing", content);
    holdDrawingButton->setToolTip(
        "Hold the previous frame's drawing in the selected cel."
    );
    auto* independentButton =
        new QPushButton("Make Independent", content);

    controls->addWidget(addButton);
    controls->addWidget(removeButton);
    controls->addWidget(holdDrawingButton);
    controls->addWidget(independentButton);
    layout->addLayout(controls);

    timelineTable_ = new QTableWidget(content);
    timelineTable_->setObjectName("TimelineTable");
    timelineTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    timelineTable_->setSelectionBehavior(QAbstractItemView::SelectItems);
    timelineTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timelineTable_->verticalHeader()->setDefaultSectionSize(28);
    timelineTable_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    timelineTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    timelineTable_->horizontalHeader()->setDefaultSectionSize(48);

    layout->addWidget(timelineTable_);

    auto* dock = makeDock("Timeline", content, this);
    dock->setObjectName("Timeline");
    dock->setMinimumHeight(180);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

    connect(addButton, &QPushButton::clicked,
            this, &MainWindow::addFrame);
    connect(removeButton, &QPushButton::clicked,
            this, &MainWindow::removeFrame);
    connect(holdDrawingButton, &QPushButton::clicked,
            this, &MainWindow::linkPreviousCel);
    connect(independentButton, &QPushButton::clicked,
            this, &MainWindow::makeCelIndependent);

    connect(timelineTable_, &QTableWidget::currentCellChanged,
            this, [this](int row, int column, int, int)
    {
        if (row < 0 || column < 0)
            return;

        canvasWidget_->selectLayer(row);
        canvasWidget_->selectFrame(column);

        if (layersList_ && layersList_->currentRow() != row)
        {
            const QSignalBlocker blocker(layersList_);
            layersList_->setCurrentRow(row);
        }

        statusBar()->showMessage(
            QString("Layer %1 | Frame %2")
                .arg(row + 1).arg(column + 1)
        );
    });

    refreshTimeline();
}

void MainWindow::refreshTimeline()
{
    if (!canvasWidget_)
        return;

    if (timelineTable_)
    {
        const QSignalBlocker blocker(timelineTable_);

        timelineTable_->clear();
        timelineTable_->setRowCount(canvasWidget_->layerCount());
        timelineTable_->setColumnCount(canvasWidget_->frameCount());

        for (int row = 0; row < canvasWidget_->layerCount(); ++row)
        {
            auto* header = new QTableWidgetItem(
                canvasWidget_->layerName(row)
            );
            timelineTable_->setVerticalHeaderItem(row, header);

            for (int column = 0;
                 column < canvasWidget_->frameCount(); ++column)
            {
                auto* cell = new QTableWidgetItem;
                cell->setTextAlignment(Qt::AlignCenter);
                cell->setBackground(QColor("#d4d0c8"));
                timelineTable_->setItem(row, column, cell);

                if (canvasWidget_->isCelLinked(row, column))
                    cell->setText("↪");
                else if (row == canvasWidget_->currentLayer() &&
                         column == canvasWidget_->currentFrame())
                    cell->setText("●");
            }
        }

        for (int column = 0;
             column < canvasWidget_->frameCount(); ++column)
        {
            timelineTable_->setHorizontalHeaderItem(
                column,
                new QTableWidgetItem(QString::number(column + 1))
            );
        }

        timelineTable_->setCurrentCell(
            canvasWidget_->currentLayer(),
            canvasWidget_->currentFrame()
        );
    }

    if (layersList_)
    {
        const QSignalBlocker blocker(layersList_);
        layersList_->clear();

        for (int row = 0; row < canvasWidget_->layerCount(); ++row)
            layersList_->addItem(canvasWidget_->layerName(row));

        layersList_->setCurrentRow(canvasWidget_->currentLayer());
    }

    canvasWidget_->update();
}

void MainWindow::selectFrame(int frameIndex)
{
    if (!canvasWidget_->selectFrame(frameIndex))
        return;

    refreshTimeline();
}

void MainWindow::addFrame()
{
    canvasWidget_->addFrame();
    refreshTimeline();

    statusBar()->showMessage(
        QString("Added Frame %1")
            .arg(canvasWidget_->currentFrame() + 1),
        3000
    );
}

void MainWindow::removeFrame()
{
    if (canvasWidget_->frameCount() <= 1) {
        statusBar()->showMessage(
            "At least one frame must remain.",
            3000
        );
        return;
    }

    const int removedFrame = canvasWidget_->currentFrame();

    canvasWidget_->removeFrame(removedFrame);
    refreshTimeline();

    statusBar()->showMessage(
        QString("Removed Frame %1").arg(removedFrame + 1),
        3000
    );
}

void MainWindow::addLayer()
{
    canvasWidget_->addLayer();
    refreshTimeline();
}

void MainWindow::removeLayer()
{
    if (canvasWidget_->layerCount() <= 1)
    {
        statusBar()->showMessage(
            "At least one layer must remain."
        );
        return;
    }

    if (!canvasWidget_->removeLayer(canvasWidget_->currentLayer()))
        return;

    refreshTimeline();
}

void MainWindow::selectLayer(int layerIndex)
{
    if (canvasWidget_->selectLayer(layerIndex))
        refreshTimeline();
}

void MainWindow::linkPreviousCel()
{
    if (canvasWidget_->currentFrame() == 0) {
        statusBar()->showMessage(
            "The first frame has no previous drawing to hold.",
            3000
        );
        return;
    }

    canvasWidget_->linkCurrentCelToPrevious();
    refreshTimeline();

    statusBar()->showMessage(
        QString("Holding drawing from Frame %1")
            .arg(canvasWidget_->currentFrame()),
        3000
    );
}

void MainWindow::makeCelIndependent()
{
    canvasWidget_->makeCurrentCelIndependent();
    refreshTimeline();

    statusBar()->showMessage(
        "Selected cel is now independent.",
        3000
    );
}

void MainWindow::createLayersPanel()
{
    auto* content = new QWidget(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(4, 4, 4, 4);

    layersList_ = new QListWidget(content);
    layersList_->setObjectName("LayersList");
    layout->addWidget(layersList_);

    auto* buttons = new QHBoxLayout();
    auto* addButton = new QPushButton("+", content);
    auto* removeButton = new QPushButton("-", content);

    addButton->setToolTip("Add layer");
    removeButton->setToolTip("Remove selected layer");

    buttons->addWidget(addButton);
    buttons->addWidget(removeButton);
    layout->addLayout(buttons);

    auto* dock = makeDock("Layers", content, this);
    dock->setObjectName("Layers");
    dock->setMinimumWidth(180);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    connect(addButton, &QPushButton::clicked,
            this, &MainWindow::addLayer);
    connect(removeButton, &QPushButton::clicked,
            this, &MainWindow::removeLayer);

    connect(layersList_, &QListWidget::currentRowChanged,
            this, [this](int row)
    {
        if (row < 0)
            return;

        canvasWidget_->selectLayer(row);

        if (timelineTable_ &&
            timelineTable_->currentRow() != row)
        {
            const QSignalBlocker blocker(timelineTable_);
            timelineTable_->setCurrentCell(
                row, canvasWidget_->currentFrame()
            );
        }

        canvasWidget_->update();
    });

    refreshTimeline();
}
