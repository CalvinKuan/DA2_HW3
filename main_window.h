#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "canvas.h"

#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    void quit();
    void createRectangle();
    void createBoundary();
    void createPath();
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void panUp();
    void panDown();
    void panLeft();
    void panRight();
    void clear();
    void deleteObject();
    void mergePaths();
    void moveObjects();
    void duplicateObjects();
    void setLayer();
    void rotateObjects();
    void flipHorizontal();
    void flipVertical();
    void selectAll();
    void query();
    void saveImage();
    void showCursorPos(const QPointF & userPos);

private:
    void createActions();
    // true if something is selected, otherwise tells the user to select first
    bool requireSelection(const QString & command);

private:
    Canvas * _canvas;
    QMenu * _fileMenu;
    QMenu * _editMenu;
    QMenu * _viewMenu;
    QToolBar * _fileToolbar;
    QToolBar * _editToolbar;
    QToolBar * _viewToolbar;
    QLabel * _coordLabel;
};

#endif // MAIN_WINDOW_H
