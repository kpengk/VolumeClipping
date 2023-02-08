#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>

// 关闭vtk默认交互
class InteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    static InteractorStyle* New();
    vtkTypeMacro(InteractorStyle, vtkInteractorStyleTrackballCamera);

    InteractorStyle();
    ~InteractorStyle();

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void OnRightButtonDown() override;
    void OnRightButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;

    void setInteractiveEnable(bool enable);

private:
    bool interactive_;// 交互
};
