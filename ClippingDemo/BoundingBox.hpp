#pragma once
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkVolume.h>
#include <vtkPlanes.h>
#include <vtkBoxWidget2.h>
#include <vtkObjectFactory.h>


// 包围盒
/**
 * @brief The CBvtkBoxWidget3DCallback class
 * vtkBoxWidget2D 的观察者，响应 InteractionEvent 事件，设置当前的Planes到vtkVolume的Planes。
 */
class CBvtkBoxWidget3DCallback final : public vtkCommand {
public:
    static CBvtkBoxWidget3DCallback* New();
    vtkTypeMacro(CBvtkBoxWidget3DCallback, vtkCommand);

    CBvtkBoxWidget3DCallback()
        : transform_(vtkSmartPointer<vtkTransform>::New()) {}
    ~CBvtkBoxWidget3DCallback() = default;
    vtkVolume* GetVolume() const;
    void SetVolume(vtkVolume* volume);
    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override;

private:
    vtkVolume* volume_ = {};
    vtkNew<vtkPlanes> planes_{};
    vtkSmartPointer<vtkTransform> transform_{};
};

class CBvtkBoxWidget3D : public vtkBoxWidget2 {
public:
    static CBvtkBoxWidget3D* New();
    vtkTypeMacro(CBvtkBoxWidget3D, vtkBoxWidget2);
    CBvtkBoxWidget3D();
    ~CBvtkBoxWidget3D();
};
