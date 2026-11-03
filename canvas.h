#ifndef CANVAS_H
#define CANVAS_H

#include "transform.h"
#include "shape.h"

#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QPen>

#include <iostream>

typedef enum {None = 0, Rectangle = 1, Boundary = 2, Path = 3,
//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Move command
//////////////////////////////////////////////////////////////////////
    Move = 4
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Move command
//////////////////////////////////////////////////////////////////////
} CommandType;

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(QWidget *parent = 0);

    void setUserRect(const QRect & rect);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    bool start(CommandType type, bool orthagonal = false);
    void stop(bool lastPos = false);
    QVector<QPointF> getUserPoints() const;
    void addRectangle (const QPointF & pt1, const QPointF & pt2, uint32_t layer = 0);
    void addBoundary(const QVector<QPointF> & pts, uint32_t layer = 0);
    void addPath(const QVector<QPointF> & pts, uint32_t width, uint32_t endStyle, uint32_t layer = 0);
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void panUp();
    void panDown();
    void panLeft();
    void panRight();
    void clear();
    void deleteObject();
    QString query() const;
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Merge Paths
    //////////////////////////////////////////////////////////////////////
    // merge the selected paths into a single path. returns false and sets error if they can't be merged
    bool mergeSelectedPaths(QString * error = nullptr);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Merge Paths
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Move / Duplicate / Set Layer / Rotate / Flip / Select All
    //////////////////////////////////////////////////////////////////////
    // currently selected shapes
    const QVector<ShapeObject*> & selection() const
    {
        return _selection;
    }

    // edit operations on the selected shapes
    void selectAll();
    void moveSelection(const QPointF & delta);
    void duplicateSelection();
    void setSelectionLayer(uint32_t layer);
    void rotateSelection();
    void flipSelection(bool horizontal);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Move / Duplicate / Set Layer / Rotate / Flip / Select All
    //////////////////////////////////////////////////////////////////////

signals:
    void ButtonPress();
    void EnterKeyPress();
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Cursor coordinates
    //////////////////////////////////////////////////////////////////////
    // cursor position in user coordinates, emitted on every mouse move
    void CursorMoved(const QPointF & userPos);
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Cursor coordinates
    //////////////////////////////////////////////////////////////////////

private:
    QPoint toOrthogonal(const QPoint & pt) const;
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Rotate / Flip
    //////////////////////////////////////////////////////////////////////
    QPointF selectionCenter() const;
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Rotate / Flip
    //////////////////////////////////////////////////////////////////////

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Mouse wheel zoom
    //////////////////////////////////////////////////////////////////////
    void wheelEvent(QWheelEvent *event) override;
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Mouse wheel zoom
    //////////////////////////////////////////////////////////////////////
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVector<QPoint> _pts;
    QPoint _pos;
    QVector<ShapeObject*> _objects;
    QVector<ShapeObject*> _selection;
    CommandType _command;
    Transform _transform;
    bool _orthagonal;
};

#endif // RENDERAREA_H
