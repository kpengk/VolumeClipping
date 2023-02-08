#include "RenderWidget.hpp"
#include "InteractorStyle.hpp"
#include <QPainter>
#include <QPainterPath>
#include <vtkGenericOpenGLRenderWindow.h>

RenderWidget::RenderWidget(QWidget* parent /*= nullptr*/)
    : QVTKOpenGLNativeWidget(parent)
    , style_{vtkSmartPointer<InteractorStyle>::New()}
    , state_{}
    , draw_line_{} {
    auto render_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    setRenderWindow(render_window);
    render_window->GetInteractor()->SetInteractorStyle(style_);
}

void RenderWidget::paintGL() {
    QVTKOpenGLNativeWidget::paintGL();
    if (state_ == DrawCutLine && !cutting_points_.isEmpty()) {
        QPainter painter(this);
        cutting_points_.append(active_point_);
        drawArea(cutting_points_, painter);
        cutting_points_.removeLast();
    }
    renderWindow()->Render();
}

void RenderWidget::mousePressEvent(QMouseEvent* event) {
    if (state_ == DrawCutLine) {
        if (event->button() == Qt::LeftButton) {
            // 重新框选
            if (!draw_line_) {
                cutting_points_.clear();
            }
            cutting_points_.append(event->position());
            active_point_ = event->position();
            draw_line_ = true;
        } else if (event->button() == Qt::RightButton) {
            if (!cutting_points_.isEmpty()) {
                active_point_ = cutting_points_.last();
            }
            draw_line_ = false;
        }
    }
    QVTKOpenGLNativeWidget::mousePressEvent(event);
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
    if (draw_line_) {
        active_point_ = event->position();
    }
    QVTKOpenGLNativeWidget::mouseMoveEvent(event);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* event) { QVTKOpenGLNativeWidget::mouseReleaseEvent(event); }

void RenderWidget::drawArea(QList<QPointF>& pf, QPainter& painter) {
    if (pf.length() <= 1) {
        return;
    }

    QPainterPath path(pf[0]);
    for (int i = 1; i < pf.size(); ++i) {
        path.lineTo(pf[i]);
    }

    QPen pen;
    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.setBrush(QBrush(Qt::green, Qt::Dense4Pattern));
    painter.drawPath(path);
    painter.drawLine(pf[0], pf.last());
}

QPolygonF RenderWidget::cuttingPolygon() {
    const int h = height();
    QPolygonF out;
    out.reserve(cutting_points_.size());
    for (const auto pt : cutting_points_) {
        out.append(QPointF(pt.x(), h - pt.y())); // VTK二维坐标系原点在左下角，QT坐标系原点在左上角
    }
    return out;
}

void RenderWidget::setStyleState(const State& state) {
    switch (state) {
        case Normal:
            style_->setInteractiveEnable(true);
            state_ = Normal;
            break;
        case DrawCutLine:
            style_->setInteractiveEnable(false);
            state_ = DrawCutLine;
            break;
        case CutLineInside:
        case CutLineOutside:
            style_->setInteractiveEnable(true);
            state_ = Normal;
            emit SgnCuttingLine(state);
            renderWindow()->Render();
            break;
    }
    cutting_points_.clear();
}
