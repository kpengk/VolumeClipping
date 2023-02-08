#pragma once
#include <QMouseEvent>
#include <QVTKOpenGLNativeWidget.h>

class InteractorStyle;

// 增加屏幕画多边形的交互
class RenderWidget : public QVTKOpenGLNativeWidget {
    Q_OBJECT

signals:
    void SgnCuttingLine(int type);

public:
    enum State
    {
        Normal,
        DrawCutLine,
        CutLineInside,
        CutLineOutside
    };

public:
    RenderWidget(QWidget* parent = nullptr);

    void drawArea(QList<QPointF>& pf, QPainter& painter);
    QPolygonF cuttingPolygon();
    void setStyleState(const State& state);

protected:
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    vtkSmartPointer<InteractorStyle> style_;
    QList<QPointF> cutting_points_;
    QPointF active_point_;
    State state_;
    bool draw_line_;
};
