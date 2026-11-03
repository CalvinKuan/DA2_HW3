#ifndef CANVAS_H
#define CANVAS_H

#include "transform.h"
#include "shape.h"

#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QPen>

#include <iostream>

typedef enum {None = 0, Rectangle = 1, Boundary = 2, Path = 3, Move = 4} CommandType;

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
    // merge the selected paths into a single path. returns false and sets error if they can't be merged
    bool mergeSelectedPaths(QString * error = nullptr);

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

signals:
    void ButtonPress();
    void EnterKeyPress();
    // cursor position in user coordinates, emitted on every mouse move
    void CursorMoved(const QPointF & userPos);

private:
    QPoint toOrthogonal(const QPoint & pt) const;
    QPointF selectionCenter() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
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
