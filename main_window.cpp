#include <QMenuBar>
#include <QStatusBar>
#include <QEventLoop>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>
#include <QPixmap>

#include <iostream>
#include <fstream>

#include "main_window.h"
#include "gdsFileParser.h"

MainWindow::MainWindow()
    : QMainWindow ()
{
    _canvas = new Canvas;

    setCentralWidget (_canvas);

    _fileMenu = menuBar()->addMenu(tr("&File"));
    _editMenu = menuBar()->addMenu(tr("&Edit"));
    _viewMenu = menuBar()->addMenu(tr("&View"));

    _fileToolbar = addToolBar(tr("File"));
    _editToolbar = addToolBar(tr("Edit"));
    _viewToolbar = addToolBar(tr("View"));

    createActions();

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Cursor coordinates
    //////////////////////////////////////////////////////////////////////
    // permanent widget so it does not overwrite the "Create ..." status messages
    _coordLabel = new QLabel(tr("X: -   Y: -"), this);
    _coordLabel->setMinimumWidth(180);
    statusBar()->addPermanentWidget(_coordLabel);
    connect (_canvas, &Canvas::CursorMoved, this, &MainWindow::showCursorPos);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Cursor coordinates
    //////////////////////////////////////////////////////////////////////

    setGeometry (100, 100, 800, 600);
}

void MainWindow::createActions()
{
    QAction * newAct = nullptr;

    newAct = new QAction(tr("&Open..."), this);
    newAct->setIcon(QIcon(":/images/open.png"));
    connect (newAct, &QAction::triggered, this, &MainWindow::open);
    _fileMenu->addAction (newAct);
    _fileToolbar->addAction (newAct);

    newAct = new QAction(tr("&Query Selected"), this);
    newAct->setIcon(QIcon(":/images/digital.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_Q));
    connect (newAct, &QAction::triggered, this, &MainWindow::query);
    _fileMenu->addAction (newAct);
    _fileToolbar->addAction (newAct);

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Save Image
    //////////////////////////////////////////////////////////////////////
    newAct = new QAction(tr("&Save Image..."), this);
    newAct->setShortcut (QKeySequence::Save);
    connect (newAct, &QAction::triggered, this, &MainWindow::saveImage);
    _fileMenu->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Save Image
    //////////////////////////////////////////////////////////////////////

    newAct = new QAction(tr("&Quit"), this);
    _fileMenu->addAction (newAct);
    connect (newAct, &QAction::triggered, this, &MainWindow::quit);

    newAct = new QAction(tr("&Rectangle"), this);
    newAct->setToolTip("Create Rectangle object");
    newAct->setShortcut (QKeySequence(Qt::Key_R));
    connect (newAct, &QAction::triggered, this, &MainWindow::createRectangle);
    _editMenu->addAction (newAct);
    _editToolbar->addAction (newAct);

    newAct = new QAction(tr("&Boundary"), this);
    newAct->setToolTip("Create Boundary object");
    newAct->setShortcut (QKeySequence(Qt::Key_B));
    connect (newAct, &QAction::triggered, this, &MainWindow::createBoundary);
    _editMenu->addAction (newAct);
    _editToolbar->addAction (newAct);

    newAct = new QAction(tr("&Path"), this);
    newAct->setToolTip("Create Path object");
    newAct->setShortcut (QKeySequence(Qt::Key_P));
    connect (newAct, &QAction::triggered, this, &MainWindow::createPath);
    _editMenu->addAction (newAct);
    _editToolbar->addAction (newAct);

    newAct = new QAction(tr("&Delete"), this);
    newAct->setIcon(QIcon(":/images/delete.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_Delete));
    connect (newAct, &QAction::triggered, this, &MainWindow::deleteObject);
    _editMenu->addAction (newAct);
    _editToolbar->addAction (newAct);

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Merge Paths
    //////////////////////////////////////////////////////////////////////
    newAct = new QAction(tr("&Merge Paths"), this);
    newAct->setToolTip("Merge the selected paths (Shift+click to select several)");
    newAct->setShortcut (QKeySequence(Qt::Key_M));
    connect (newAct, &QAction::triggered, this, &MainWindow::mergePaths);
    _editMenu->addAction (newAct);
    _editToolbar->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Merge Paths
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Move
    //////////////////////////////////////////////////////////////////////
    _editMenu->addSeparator();

    newAct = new QAction(tr("Mo&ve"), this);
    newAct->setToolTip("Move the selected objects: click a base point, then a destination point");
    newAct->setShortcut (QKeySequence(Qt::Key_V));
    connect (newAct, &QAction::triggered, this, &MainWindow::moveObjects);
    _editMenu->addAction (newAct);
    _editToolbar->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Move
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Duplicate
    //////////////////////////////////////////////////////////////////////
    newAct = new QAction(tr("D&uplicate"), this);
    newAct->setShortcut (QKeySequence(Qt::CTRL + Qt::Key_D));
    connect (newAct, &QAction::triggered, this, &MainWindow::duplicateObjects);
    _editMenu->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Duplicate
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Set Layer
    //////////////////////////////////////////////////////////////////////
    newAct = new QAction(tr("Set &Layer..."), this);
    newAct->setShortcut (QKeySequence(Qt::Key_L));
    connect (newAct, &QAction::triggered, this, &MainWindow::setLayer);
    _editMenu->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Set Layer
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Rotate
    //////////////////////////////////////////////////////////////////////
    newAct = new QAction(tr("Ro&tate 90\xC2\xB0"), this);
    newAct->setShortcut (QKeySequence(Qt::CTRL + Qt::Key_R));
    connect (newAct, &QAction::triggered, this, &MainWindow::rotateObjects);
    _editMenu->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Rotate
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Flip
    //////////////////////////////////////////////////////////////////////
    newAct = new QAction(tr("Flip &Horizontal"), this);
    newAct->setShortcut (QKeySequence(Qt::CTRL + Qt::Key_H));
    connect (newAct, &QAction::triggered, this, &MainWindow::flipHorizontal);
    _editMenu->addAction (newAct);

    newAct = new QAction(tr("Flip V&ertical"), this);
    newAct->setShortcut (QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H));
    connect (newAct, &QAction::triggered, this, &MainWindow::flipVertical);
    _editMenu->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Flip
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Select All
    //////////////////////////////////////////////////////////////////////
    _editMenu->addSeparator();

    newAct = new QAction(tr("&Select All"), this);
    newAct->setShortcut (QKeySequence::SelectAll);
    connect (newAct, &QAction::triggered, this, &MainWindow::selectAll);
    _editMenu->addAction (newAct);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Select All
    //////////////////////////////////////////////////////////////////////

    newAct = new QAction(tr("Delete &All"), this);
    connect (newAct, &QAction::triggered, this, &MainWindow::clear);
    _editMenu->addAction (newAct);

    newAct = new QAction(tr("&Fit"), this);
    newAct->setIcon(QIcon(":/images/fit.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_F));
    connect (newAct, &QAction::triggered, this, &MainWindow::zoomFit);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);

    newAct = new QAction(tr("&Zoom In"), this);
    newAct->setIcon(QIcon(":/images/zoomin.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_BracketLeft));
    connect (newAct, &QAction::triggered, this, &MainWindow::zoomIn);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);

    newAct = new QAction(tr("&Zoom Out"), this);
    newAct->setIcon(QIcon(":/images/zoomout.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_BracketRight));
    connect (newAct, &QAction::triggered, this, &MainWindow::zoomOut);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);

    newAct = new QAction(tr("&Pan up"), this);
    newAct->setIcon(QIcon(":/images/up.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_Up));
    connect (newAct, &QAction::triggered, this, &MainWindow::panUp);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);

    newAct = new QAction(tr("&Pan down"), this);
    newAct->setIcon(QIcon(":/images/down.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_Down));
    connect (newAct, &QAction::triggered, this, &MainWindow::panDown);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);

    newAct = new QAction(tr("&Pan left"), this);
    newAct->setIcon(QIcon(":/images/left.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_Left));
    connect (newAct, &QAction::triggered, this, &MainWindow::panLeft);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);

    newAct = new QAction(tr("&Pan right"), this);
    newAct->setIcon(QIcon(":/images/right.png"));
    newAct->setShortcut (QKeySequence(Qt::Key_Right));
    connect (newAct, &QAction::triggered, this, &MainWindow::panRight);
    _viewMenu->addAction (newAct);
    _viewToolbar->addAction (newAct);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open GDS file"), ".", tr("GDS file(*.gds)"));

    extern int GdsFileParserMain(const char *);
    GdsFileParserMain(fileName.toLocal8Bit().data());

    // ... add canvas objects ...
    

    // remove existing objects
    _canvas->clear();

    // set sample user coordinate rect, this would come from the GDS file.
    _canvas->setUserRect (QRect(-2000, -2000, 120000, 120000));

    // create sample rectangle object
    _canvas->addRectangle(QPointF(-1000, -1000), QPointF(100000, 100000), 16/*layer*/);

    // create sample boundary object
    QVector<QPointF> bdryPts;
    bdryPts << QPointF(4000, 4000);
    bdryPts << QPointF(110000, 6000);
    bdryPts << QPointF(-2000, 90000);
    bdryPts << bdryPts[0];
    _canvas->addBoundary(bdryPts, 14/*layer*/);

    // create sample path. NOTE points must be orthagonal
    QVector<QPointF> pathPts;
    pathPts << QPointF(3000, 3000);
    pathPts << QPointF(90000, 3000);
    pathPts << QPointF(90000, 70000);
    pathPts << QPointF(0, 70000);
    _canvas->addPath(pathPts, 30/*width*/, 0/*endStyle*/, 17/*layer*/);

    // force a redraw.
    _canvas->update();
}

void MainWindow::query()
{
    QString info = _canvas->query();
    QMessageBox dialog(this);
    dialog.setText(info);
    dialog.exec();
}

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Save Image
//////////////////////////////////////////////////////////////////////
void MainWindow::saveImage()
{
    QString fileName = QFileDialog::getSaveFileName(this,
         tr("Save image"), "layout.png",
         tr("PNG image (*.png);;JPEG image (*.jpg *.jpeg);;Bitmap image (*.bmp)"));

    if (fileName.isEmpty())
    {
        return;
    }

    // QPixmap::save picks the image format from the file suffix
    if (QFileInfo(fileName).suffix().isEmpty())
    {
        fileName += ".png";
    }

    if (_canvas->grab().save(fileName))
    {
        statusBar()->showMessage(tr("Image saved: %1").arg(fileName));
    }
    else
    {
        QMessageBox::warning(this, tr("Save image"), tr("Failed to save image to %1").arg(fileName));
    }
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Save Image
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Cursor coordinates
//////////////////////////////////////////////////////////////////////
void MainWindow::showCursorPos(const QPointF & userPos)
{
    _coordLabel->setText(tr("X: %1   Y: %2").arg(qRound(userPos.x())).arg(qRound(userPos.y())));
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Cursor coordinates
//////////////////////////////////////////////////////////////////////

void MainWindow::quit()
{
    exit(0);
}

void MainWindow::createRectangle()
{
    QEventLoop loop;

    /* start collecting points */
    if (!_canvas->start(Rectangle))
    {
        return;
    }

    /* stop collecting on mouse button press*/
    connect(_canvas, &Canvas::ButtonPress, &loop, &QEventLoop::quit);

    /* get first point */
    statusBar()->showMessage(tr("Create Rectange: Enter first point"));
    loop.exec();

    /* get second point */
    statusBar()->showMessage(tr("Create Rectangle: Enter second point"));
    loop.exec();

    /* stop collecting points */
    _canvas->stop();

    /* get points as user corrdinates */
    QVector<QPointF> userPoints = _canvas->getUserPoints();

    /* add the rectange to the object collection */
    if (userPoints.size() > 1)
    {
        _canvas->addRectangle (userPoints[0], userPoints[1], 19/*layer*/);
    }

    statusBar()->showMessage(tr("Create Rectangle: done"));

}

void MainWindow::createBoundary()
{
    QEventLoop loop;

    /* start collecting points */
    if (!_canvas->start(Boundary))
    {
        return;
    }

    /* stop collecting on enter/return key pressed*/
    connect(_canvas, &Canvas::EnterKeyPress, &loop, &QEventLoop::quit);

    statusBar()->showMessage(tr("Create Boundary: Enter points with mouse button click. Press Enter key when done."));
    loop.exec();

    /* stop collecting points */
    _canvas->stop();

   /* store user coordinates */
   QVector<QPointF> userPoints = _canvas->getUserPoints();
  
    uint32_t count = userPoints.size();

    if (count > 2)
    {
        /* make sure boundary is closed */
        userPoints << userPoints[0];

        _canvas->addBoundary(userPoints, 10/*layer*/);
    }

    statusBar()->showMessage(tr("Create Boundary: done"));
}

void MainWindow::createPath()
{
    QEventLoop loop;

    /* start collecting orthagonal points */
    if (!_canvas->start(Path, true))
    {
        return;
    }

    /* stop collecting on enter/return key pressed*/
    connect(_canvas, &Canvas::EnterKeyPress, &loop, &QEventLoop::quit);

    statusBar()->showMessage(tr("Create Path: Enter points with mouse button click. Press Enter key when done."));
    loop.exec();

    /* stop collecting points. Add the current cursor postion to point list */
    _canvas->stop(true);

   /* store user coordinates */
   QVector<QPointF> userPoints = _canvas->getUserPoints();
  
    uint32_t count = userPoints.size();

    if (count > 1)
    {
        _canvas->addPath(userPoints, INTERACTIVE_PATH_WIDTH/*width*/, INTERACTIVE_PATH_ENDSTYLE/*endStyle*/, 18/*layer*/);
    }

    statusBar()->showMessage(tr("Create Path: done"));
}

void MainWindow::zoomIn()
{
    _canvas->zoomIn();
}

void MainWindow::zoomOut()
{
    _canvas->zoomOut();
}

void MainWindow::zoomFit()
{
    _canvas->zoomFit();
}

void MainWindow::panUp()
{
    _canvas->panUp();
}

void MainWindow::panDown()
{
    _canvas->panDown();
}

void MainWindow::panLeft()
{
    _canvas->panLeft();
}

void MainWindow::panRight()
{
    _canvas->panRight();
}

void MainWindow::clear()
{
    _canvas->clear();
}

void MainWindow::deleteObject()
{
    _canvas->deleteObject();
}

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Edit operations helper
//////////////////////////////////////////////////////////////////////
bool MainWindow::requireSelection(const QString & command)
{
    if (_canvas->selection().isEmpty())
    {
        statusBar()->showMessage(tr("%1: select one or more objects first").arg(command));
        return false;
    }

    return true;
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Edit operations helper
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Move
//////////////////////////////////////////////////////////////////////
void MainWindow::moveObjects()
{
    if (!requireSelection(tr("Move")))
    {
        return;
    }

    QEventLoop loop;

    /* start collecting points */
    if (!_canvas->start(Move))
    {
        return;
    }

    /* stop collecting on mouse button press*/
    connect(_canvas, &Canvas::ButtonPress, &loop, &QEventLoop::quit);

    /* get base point */
    statusBar()->showMessage(tr("Move: click the base point"));
    loop.exec();

    /* get destination point */
    statusBar()->showMessage(tr("Move: click the destination point"));
    loop.exec();

    /* stop collecting points */
    _canvas->stop();

    /* move the selection by the distance between the two points */
    QVector<QPointF> userPoints = _canvas->getUserPoints();

    if (userPoints.size() > 1)
    {
        _canvas->moveSelection(userPoints[1] - userPoints[0]);
    }

    statusBar()->showMessage(tr("Move: done"));
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Move
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Duplicate
//////////////////////////////////////////////////////////////////////
void MainWindow::duplicateObjects()
{
    if (requireSelection(tr("Duplicate")))
    {
        _canvas->duplicateSelection();
        statusBar()->showMessage(tr("Duplicate: done. The copies are selected"));
    }
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Duplicate
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Set Layer
//////////////////////////////////////////////////////////////////////
void MainWindow::setLayer()
{
    if (!requireSelection(tr("Set Layer")))
    {
        return;
    }

    const int maxLayer = int(ShapeObject::layerCount()) - 1;
    const int current = int(_canvas->selection().first()->getLayer());
    bool ok = false;

    const int layer = QInputDialog::getInt(this, tr("Set Layer"),
         tr("Layer (0 - %1):").arg(maxLayer), current, 0, maxLayer, 1, &ok);

    if (ok)
    {
        _canvas->setSelectionLayer(layer);
        statusBar()->showMessage(tr("Set Layer: done"));
    }
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Set Layer
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Rotate
//////////////////////////////////////////////////////////////////////
void MainWindow::rotateObjects()
{
    if (requireSelection(tr("Rotate")))
    {
        _canvas->rotateSelection();
    }
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Rotate
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Flip
//////////////////////////////////////////////////////////////////////
void MainWindow::flipHorizontal()
{
    if (requireSelection(tr("Flip")))
    {
        _canvas->flipSelection(true);
    }
}

void MainWindow::flipVertical()
{
    if (requireSelection(tr("Flip")))
    {
        _canvas->flipSelection(false);
    }
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Flip
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Select All
//////////////////////////////////////////////////////////////////////
void MainWindow::selectAll()
{
    _canvas->selectAll();
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Select All
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Merge Paths
//////////////////////////////////////////////////////////////////////
void MainWindow::mergePaths()
{
    QString error;

    if (_canvas->mergeSelectedPaths(&error))
    {
        statusBar()->showMessage(tr("Merge Paths: done"));
    }
    else
    {
        QMessageBox::information(this, tr("Merge Paths"), error);
    }
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Merge Paths
//////////////////////////////////////////////////////////////////////
