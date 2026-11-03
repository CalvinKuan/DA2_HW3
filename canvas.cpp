#include "canvas.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTextStream>
#include <QLineF>

#include <algorithm>
#include <cmath>
#include <iostream>

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Merge Paths
//////////////////////////////////////////////////////////////////////
// max distance (screen pixels) between two path end points for the paths to be merged
static const double MERGE_TOLERANCE_PIXELS = 15.0;
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Merge Paths
//////////////////////////////////////////////////////////////////////

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
    , _command(None)
    , _orthagonal(false)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void Canvas::setUserRect(const QRect & userRect)
{
    _transform.setUserRect(userRect);
    _transform.setScale(rect().size());
}

QSize Canvas::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize Canvas::sizeHint() const
{
    return QSize(400, 200);
}

bool Canvas::start(CommandType type, bool orthagonal)
{
    if (None != _command)
    {
        return false;
    }

    // iitialize point collection for interative shape creation
    _command = type;
    _orthagonal = orthagonal;
    _pts.clear();

    return true;
}

void Canvas::stop(bool lastPos)
{
    if (lastPos)
    {
        _pts << _pos;
    }

    _command = None;
    update();
}

QVector<QPointF> Canvas::getUserPoints() const
{
    QVector<QPointF> userPoints;
    _transform.screenToUserPts(_pts, userPoints);
    return userPoints;
}

void Canvas::zoomIn()
{
    _transform.zoomIn(rect().size());
    update();
}

void Canvas::zoomOut()
{
    _transform.zoomOut(rect().size());
    update();
}

void Canvas::zoomFit()
{
    _transform.zoomFit(rect().size());
    update();
}

void Canvas::panUp()
{
    _transform.panUp(rect().size());
    update();
}

void Canvas::panDown()
{
    _transform.panDown(rect().size());
    update();
}

void Canvas::panLeft()
{
    _transform.panLeft(rect().size());
    update();
}

void Canvas::panRight()
{
    _transform.panRight(rect().size());
    update();
}

// convert point to orthoganal coordinates
QPoint Canvas::toOrthogonal(const QPoint & pt) const
{
    if (!_pts.empty())
    {
        QPoint thisPt = pt;
        QPoint lastPt = _pts[_pts.size() - 1];
        int32_t distX = abs(lastPt.x() - thisPt.x());
        int32_t distY = abs(lastPt.y() - thisPt.y());

        if (distX > distY)
        {
            thisPt.setY(lastPt.y());
        }
        else
        {
            thisPt.setX(lastPt.x());
        }

        return thisPt;
    }

    return pt;
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    // lets see if a shape can be selected not currently creating a shape
    if (None == _command)
    {
        //////////////////////////////////////////////////////////////////////
        //////////  EXTRA START: Multi-select (Shift+click)
        //////////////////////////////////////////////////////////////////////
        // topmost shape under the cursor
        ShapeObject * hit = nullptr;

        for (QVector<ShapeObject*>::reverse_iterator it = _objects.rbegin(); it != _objects.rend(); it++)
        {
            if ((*it)->select(event->pos(), _transform))
            {
                hit = *it;
                break;
            }
        }

        if (event->modifiers() & Qt::ShiftModifier)
        {
            // shift+click toggles the shape in the selection. Clicking empty space keeps it.
            if (hit)
            {
                const int i = _selection.indexOf(hit);

                if (-1 != i)
                {
                    _selection.remove(i);
                }
                else
                {
                    _selection << hit;
                }
            }
        }
        else
        {
            _selection.clear();

            if (hit)
            {
                _selection << hit;
            }
        }
        //////////////////////////////////////////////////////////////////////
        //////////  EXTRA END: Multi-select (Shift+click)
        //////////////////////////////////////////////////////////////////////

        update();
    }
    // interactive shape creatoin .. collect current point 
    else
    {
        if (_orthagonal)
        {
            _pts << toOrthogonal(event->pos());
        }
        else
        {
            _pts << event->pos();
        }

        // tall the command loop that the mouse button was pressed
        emit ButtonPress();
    }

    QWidget::mousePressEvent(event);
}

void Canvas::keyPressEvent(QKeyEvent * event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key())
    {
        // tall the command loop that the enter/return key was pressed
        emit EnterKeyPress();
    }

    QWidget::keyPressEvent(event);
}

void Canvas::resizeEvent(QResizeEvent *event)
{
    // adjust the transformation based on the new screen size
    _transform.setScale(event->size());
    QWidget::resizeEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    // track the current cursor location and adjust for orthagonal if needed
    if (_orthagonal)
    {
        _pos = toOrthogonal(event->pos());
    }
    else
    {
        _pos = event->pos();
    }

    if (None != _command)
    {
        update();
    }

    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA START: Cursor coordinates
    //////////////////////////////////////////////////////////////////////
    emit CursorMoved(_transform.screenToUser(event->pos()));
    //////////////////////////////////////////////////////////////////////
    //////////  EXTRA END: Cursor coordinates
    //////////////////////////////////////////////////////////////////////

    QWidget::mouseMoveEvent(event);
}

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Mouse wheel zoom
//////////////////////////////////////////////////////////////////////
void Canvas::wheelEvent(QWheelEvent *event)
{
    // points are collected in screen coordinates during interactive creation,
    // so zooming now would move them. Ignore the wheel until the command ends.
    if (None != _command || 0 == event->angleDelta().y())
    {
        event->ignore();
        return;
    }

    // one wheel notch (120 units) scales the view by 0.8x (in) or 1.25x (out)
    const double notches = event->angleDelta().y() / 120.0;
    _transform.zoomAt(event->pos(), std::pow(0.8, notches), rect().size());
    update();
    event->accept();
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Mouse wheel zoom
//////////////////////////////////////////////////////////////////////

void Canvas::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.save();

    /* draw any existing shapes */
    foreach (ShapeObject * s, _objects)
    {
        s->draw(&painter, _transform);
    }

    /* handle interactive shape creation */

    switch (_command)
    {
        case Rectangle:
            RectangleShape::drawInteractive(&painter, _pts, _pos);
            break;
        case Boundary:
            BoundaryShape::drawInteractive(&painter, _pts, _pos);
            break;
        case Path:
            PathShape::drawInteractive(&painter, _pts, _pos);
            break;
        //////////////////////////////////////////////////////////////////////
        //////////  EXTRA START: Move
        //////////////////////////////////////////////////////////////////////
        case Move:
            // preview: the selected shapes follow the cursor, offset from the base point
            if (!_pts.isEmpty())
            {
                painter.save();
                painter.translate(_pos - _pts[0]);

                foreach (ShapeObject * s, _selection)
                {
                    s->drawSelected(&painter, _transform);
                }

                painter.restore();
            }
            break;
        //////////////////////////////////////////////////////////////////////
        //////////  EXTRA END: Move
        //////////////////////////////////////////////////////////////////////
        default:
            break; 
    }

    /* draw the selected shapes */
    foreach (ShapeObject * s, _selection)
    {
        s->drawSelected(&painter, _transform);
    }

    painter.restore();
}

void Canvas::addRectangle(const QPointF & pt1, const QPointF & pt2, uint32_t layer)
{
    _objects << new RectangleShape (pt1, pt2, layer);
}

void Canvas::addBoundary(const QVector<QPointF> & pts, uint32_t layer)
{
    _objects << new BoundaryShape (pts, layer);
}

void Canvas::addPath(const QVector<QPointF> & pts, uint32_t width, uint32_t endStyle, uint32_t layer)
{
    _objects << new PathShape (pts, width, endStyle, layer);
}

void Canvas::clear()
{
    foreach (ShapeObject * s, _objects)
    {
//        delete s;
    }

    _selection.clear();
    _objects.clear();
    update();
}

void Canvas::deleteObject()
{
    if (_selection.isEmpty())
    {
        return;
    }

    foreach (ShapeObject * s, _selection)
    {
        int i = _objects.indexOf(s);

        if (-1 != i)
        {
//            delete s;
            _objects.remove(i);
        }
    }

    _selection.clear();
    update();
}

QString Canvas::query() const
{
    if (_selection.isEmpty())
    {
        return QString("No object selected");
    }

    QString data;
    QTextStream ts(&data);
    ts.setRealNumberNotation(QTextStream::FixedNotation);
    ts.setRealNumberPrecision (3);

    for (int n = 0; n < _selection.size(); n++)
    {
        const ShapeObject * obj = _selection[n];

        //////////////////////////////////////////////////////////////////////
        //////////  EXTRA START: Query multiple objects
        //////////////////////////////////////////////////////////////////////
        // separate objects when several are selected
        if (_selection.size() > 1)
        {
            ts << (n > 0 ? "\n" : "") << "[" << (n + 1) << "/" << _selection.size() << "]\n";
        }
        //////////////////////////////////////////////////////////////////////
        //////////  EXTRA END: Query multiple objects
        //////////////////////////////////////////////////////////////////////

        ts << "Object: " << obj->getTypeString() << "\n";
        ts << "Layer: " << obj->getLayer() << "\n";

        QString att = obj->getAttributes();

        if (att.size())
        {
            ts << att << "\n";
        }

        ts << "User coordinates:\n";

        foreach (const QPointF & pt, obj->getPoints())
        {
            QPoint roundPt = pt.toPoint();
            ts << "(" << roundPt.x() << ", " << roundPt.y() << ")\n";
        }
    }

    return data;
}

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Merge Paths
//////////////////////////////////////////////////////////////////////
// Merge the selected paths into one path. Paths are chained end to end: the closest pair of
// end points (within MERGE_TOLERANCE_PIXELS on screen) is joined, repeated until every path is
// used. If two joined end points do not coincide, an orthogonal corner point is added so the
// merged path stays orthogonal.
bool Canvas::mergeSelectedPaths(QString * error)
{
    QVector<PathShape*> paths;

    foreach (ShapeObject * s, _selection)
    {
        PathShape * p = dynamic_cast<PathShape*>(s);

        if (nullptr == p)
        {
            if (error)
            {
                *error = QString("Only paths can be merged, but the selection contains a %1.").arg(s->getTypeString());
            }

            return false;
        }

        paths << p;
    }

    if (paths.size() < 2)
    {
        if (error)
        {
            *error = QString("Select two or more paths (Shift+click) to merge.");
        }

        return false;
    }

    PathShape * first = paths[0];

    foreach (PathShape * p, paths)
    {
        if (p->getLayer() != first->getLayer() || p->getWidth() != first->getWidth())
        {
            if (error)
            {
                *error = QString("The paths must be on the same layer and have the same width.");
            }

            return false;
        }
    }

    QVector<QPointF> chain = first->getPoints();
    QVector<PathShape*> remaining = paths;
    remaining.remove(0);

    while (!remaining.isEmpty())
    {
        // find the closest pair of end points: chain front/back against path front/back
        int bestPath = -1;
        bool bestChainFront = false;
        bool bestPathFront = false;
        double bestDist = 0;

        for (int i = 0; i < remaining.size(); i++)
        {
            const QVector<QPointF> & pts = remaining[i]->getPoints();

            for (int c = 0; c < 2; c++)
            {
                const bool chainFront = (1 == c);
                const QPointF & chainEnd = chainFront ? chain.first() : chain.last();

                for (int e = 0; e < 2; e++)
                {
                    const bool pathFront = (1 == e);
                    const QPointF & pathEnd = pathFront ? pts.first() : pts.last();
                    const double dist = QLineF(_transform.userToScreen(chainEnd),
                                               _transform.userToScreen(pathEnd)).length();

                    if (-1 == bestPath || dist < bestDist)
                    {
                        bestPath = i;
                        bestChainFront = chainFront;
                        bestPathFront = pathFront;
                        bestDist = dist;
                    }
                }
            }
        }

        if (bestDist > MERGE_TOLERANCE_PIXELS)
        {
            if (error)
            {
                *error = QString("The paths are not connected: the closest end points are %1 pixels apart (limit %2).")
                            .arg(bestDist, 0, 'f', 1).arg(MERGE_TOLERANCE_PIXELS, 0, 'f', 0);
            }

            return false;
        }

        // always attach at the back of the chain, and start from the matched end of the next path
        if (bestChainFront)
        {
            std::reverse(chain.begin(), chain.end());
        }

        QVector<QPointF> next = remaining[bestPath]->getPoints();

        if (!bestPathFront)
        {
            std::reverse(next.begin(), next.end());
        }

        // copy: chain may reallocate when points are appended
        const QPointF a = chain.last();
        const QPointF b = next.first();
        const bool sameX = qAbs(a.x() - b.x()) < 1e-6;
        const bool sameY = qAbs(a.y() - b.y()) < 1e-6;

        if (!sameX && !sameY)
        {
            chain << QPointF(b.x(), a.y());
        }

        // skip b when it is identical to a, to avoid a zero length segment
        for (int j = (sameX && sameY) ? 1 : 0; j < next.size(); j++)
        {
            chain << next[j];
        }

        remaining.remove(bestPath);
    }

    PathShape * merged = new PathShape(chain, first->getWidth(), first->getEndStyle(), first->getLayer());

    foreach (PathShape * p, paths)
    {
        int i = _objects.indexOf(p);

        if (-1 != i)
        {
            _objects.remove(i);
        }
    }

    _objects << merged;
    _selection.clear();
    _selection << merged;
    update();

    return true;
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Merge Paths
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Select All
//////////////////////////////////////////////////////////////////////
void Canvas::selectAll()
{
    _selection = _objects;
    update();
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Select All
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Move
//////////////////////////////////////////////////////////////////////
void Canvas::moveSelection(const QPointF & delta)
{
    foreach (ShapeObject * s, _selection)
    {
        s->translate(delta);
    }

    update();
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Move
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Duplicate
//////////////////////////////////////////////////////////////////////
void Canvas::duplicateSelection()
{
    if (_selection.isEmpty())
    {
        return;
    }

    // offset the copies by 20 screen pixels so they can be told apart from the originals
    const QPointF offset = _transform.screenToUser(QPointF(20, 20)) - _transform.screenToUser(QPointF(0, 0));
    QVector<ShapeObject*> copies;

    foreach (ShapeObject * s, _selection)
    {
        ShapeObject * copy = s->clone();
        copy->translate(offset);
        _objects << copy;
        copies << copy;
    }

    // select the copies so they can be moved or edited right away
    _selection = copies;
    update();
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Duplicate
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Set Layer
//////////////////////////////////////////////////////////////////////
void Canvas::setSelectionLayer(uint32_t layer)
{
    foreach (ShapeObject * s, _selection)
    {
        s->setLayer(layer);
    }

    update();
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Set Layer
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////  EXTRA START: Rotate / Flip
//////////////////////////////////////////////////////////////////////
void Canvas::rotateSelection()
{
    if (_selection.isEmpty())
    {
        return;
    }

    // the whole selection turns about the center of its bounding box
    const QPointF center = selectionCenter();

    foreach (ShapeObject * s, _selection)
    {
        s->rotate90(center);
    }

    update();
}

void Canvas::flipSelection(bool horizontal)
{
    if (_selection.isEmpty())
    {
        return;
    }

    const QPointF center = selectionCenter();

    foreach (ShapeObject * s, _selection)
    {
        s->flip(horizontal, center);
    }

    update();
}

// center of the bounding box of all selected shapes (user coordinates)
QPointF Canvas::selectionCenter() const
{
    double minX = 0;
    double maxX = 0;
    double minY = 0;
    double maxY = 0;
    bool first = true;

    foreach (const ShapeObject * s, _selection)
    {
        const QRectF b = s->boundingRect();

        if (first)
        {
            minX = b.left();
            maxX = b.right();
            minY = b.top();
            maxY = b.bottom();
            first = false;
        }
        else
        {
            minX = qMin(minX, b.left());
            maxX = qMax(maxX, b.right());
            minY = qMin(minY, b.top());
            maxY = qMax(maxY, b.bottom());
        }
    }

    return QPointF((minX + maxX) / 2, (minY + maxY) / 2);
}
//////////////////////////////////////////////////////////////////////
//////////  EXTRA END: Rotate / Flip
//////////////////////////////////////////////////////////////////////
