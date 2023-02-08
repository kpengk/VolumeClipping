#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <vtkColorTransferFunction.h>
#include <vtkContourValues.h>
#include <vtkCoordinate.h>
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkNamedColors.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkVolumeProperty.h>
#include <vtkWidgetRepresentation.h>

// 切割模型
void CutingImagedata(vtkSmartPointer<vtkImageData> image_data, vtkSmartPointer<vtkVolume> volume,
                     vtkSmartPointer<vtkRenderer> renderer, const QPolygonF& polygon, const int& type);

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    initButton();

    // 读取数据
    static const QString path = "HeadMRVolume.mhd";
    auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
    reader->SetFileName(path.toLocal8Bit().data());
    reader->Update();

    auto image_data = reader->GetOutput();

    auto mapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
    mapper->SetInputData(image_data);
    mapper->AutoAdjustSampleDistancesOff();
    mapper->SetSampleDistance(0.5);
    mapper->SetBlendModeToIsoSurface();
    mapper->SetCropping(1);
    mapper->SetCroppingRegionPlanes(50, 150, 50, 200, 50, 150);
    mapper->SetCroppingRegionFlags(VTK_CROP_SUBVOLUME);

    vtkNew<vtkColorTransferFunction> color_transfer_fun;
    color_transfer_fun->RemoveAllPoints();
    vtkNew<vtkNamedColors> colors;
    const auto flesh_color = colors->GetColor3d("flesh").GetData();
    const double iso1 = 40.0;
    color_transfer_fun->AddRGBPoint(iso1, flesh_color[0], flesh_color[1], flesh_color[2]);
    vtkNew<vtkPiecewiseFunction> scalarOpacity;
    scalarOpacity->AddPoint(iso1, 0.6);
    vtkSmartPointer<vtkVolumeProperty> volume_property = vtkSmartPointer<vtkVolumeProperty>::New();
    volume_property->ShadeOn();
    volume_property->SetInterpolationTypeToLinear();
    volume_property->SetColor(color_transfer_fun);
    volume_property->SetScalarOpacity(scalarOpacity);
    volume_property->GetIsoSurfaceValues()->SetValue(0, iso1);
    auto volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(mapper);
    volume->SetProperty(volume_property);

    // 渲染
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddVolume(volume);
    renderer->SetBackground(colors->GetColor3d("cornflower").GetData());
    renderer->ResetCamera();
    renderer->ResetCameraClippingRange();

    // 显示 Wiget
    ui->renderWidget->renderWindow()->AddRenderer(renderer);
    auto wid = ui->renderWidget;
    QObject::connect(ui->renderWidget, &RenderWidget::SgnCuttingLine, wid,
                     [wid, renderer, image_data, volume](int type) {
                         CutingImagedata(image_data, volume, renderer, wid->cuttingPolygon(), type);
                     });

    // 包围盒
    auto boxCallback = vtkSmartPointer<CBvtkBoxWidget3DCallback>::New();
    boxCallback->SetVolume(volume);

    box_widget_ = vtkSmartPointer<CBvtkBoxWidget3D>::New();
    box_widget_->SetInteractor(ui->renderWidget->renderWindow()->GetInteractor());
    box_widget_->GetRepresentation()->SetPlaceFactor(1); // Default is 0.5
    box_widget_->GetRepresentation()->PlaceWidget(volume->GetBounds());
    box_widget_->AddObserver(vtkCommand::InteractionEvent, boxCallback);
    box_widget_->Off();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::initButton() {

    static auto change_btn_state = [this](bool enable) {
        ui->cutBtn->setText(enable ? "cancel" : "cut");
        ui->insideBtn->setEnabled(enable);
        ui->outsideBtn->setEnabled(enable);
    };
    change_btn_state(false);

    QObject::connect(ui->boundingBoxBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) {
            box_widget_->On();
        } else {
            box_widget_->Off();
        }
    });

    QObject::connect(ui->cutBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) {
            ui->renderWidget->setStyleState(RenderWidget::DrawCutLine);
            change_btn_state(true);
        } else {
            ui->renderWidget->setStyleState(RenderWidget::Normal);
            change_btn_state(false);
        }
    });

    QObject::connect(ui->insideBtn, &QPushButton::clicked, this, [this] {
        ui->renderWidget->setStyleState(RenderWidget::CutLineInside);
        change_btn_state(false);
    });

    QObject::connect(ui->outsideBtn, &QPushButton::clicked, this, [this] {
        ui->renderWidget->setStyleState(RenderWidget::CutLineOutside);
        change_btn_state(false);
    });
}

void CutingImagedata(vtkSmartPointer<vtkImageData> image_data, vtkSmartPointer<vtkVolume> volume,
                     vtkSmartPointer<vtkRenderer> renderer, const QPolygonF& polygon, const int& type) {
    int img_dims[3]{};
    double img_spacing[3]{};
    double img_origian[3]{};
    image_data->GetDimensions(img_dims);
    image_data->GetSpacing(img_spacing);
    image_data->GetOrigin(img_origian);

    vtkNew<vtkCoordinate> corrdinate;
    corrdinate->SetCoordinateSystemToWorld();
    // 不用vtkImageIterator，迭代器没办法获得空间位置
    // 只是个demo，直接认为vtkImageData 数据是 unsigned char 保存的。如果是其他记得要改。
    // 只是个demo，直接认为bround是从0，0，0开始的，实际工程记得校验。
    for (int k = 0; k < img_dims[2]; ++k) {
        for (int i = 0; i < img_dims[0]; ++i) {
            for (int j = 0; j < img_dims[1]; ++j) {
                if (i < 100 && j < 100) {
                    double word_pos[]{i * img_spacing[0] + img_origian[0], j * img_spacing[1] + img_origian[1],
                                      k * img_spacing[2] + img_origian[2]};
                    corrdinate->SetValue(word_pos);
                    const double* display_pos = corrdinate->GetComputedDoubleDisplayValue(renderer);
                    const bool contains_point =
                        polygon.containsPoint(QPointF(display_pos[0], display_pos[1]), Qt::OddEvenFill);
                    if (type == RenderWidget::CutLineInside && contains_point) {
                        auto pPixel = static_cast<unsigned char*>(image_data->GetScalarPointer(i, j, k));
                        *pPixel = 0;
                    } else if (type == RenderWidget::CutLineOutside && !contains_point) {
                        auto pPixel = static_cast<unsigned char*>(image_data->GetScalarPointer(i, j, k));
                        *pPixel = 0;
                    }
                }
            }
        }
    }
    volume->Update();
    renderer->RemoveVolume(volume);
    renderer->AddVolume(volume);
}
