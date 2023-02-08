#include "BoundingBox.hpp"
#include <vtkBoxRepresentation.h>
#include <vtkAbstractVolumeMapper.h>

vtkStandardNewMacro(CBvtkBoxWidget3DCallback);

vtkVolume* CBvtkBoxWidget3DCallback::GetVolume() const { return volume_; }

void CBvtkBoxWidget3DCallback::SetVolume(vtkVolume* volume) { volume_ = volume; }

void CBvtkBoxWidget3DCallback::Execute(vtkObject* caller, unsigned long, void*) {
    auto* const boxWidget = vtkBoxWidget2::SafeDownCast(caller);
    auto* const boxRepresentation = vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());
    boxRepresentation->SetInsideOut(1);
    boxRepresentation->GetPlanes(planes_);
    volume_->GetMapper()->SetClippingPlanes(planes_);
}

vtkStandardNewMacro(CBvtkBoxWidget3D);

CBvtkBoxWidget3D::CBvtkBoxWidget3D() {
    CreateDefaultRepresentation();
    GetRepresentation()->SetPlaceFactor(1);
}

CBvtkBoxWidget3D::~CBvtkBoxWidget3D() {}
