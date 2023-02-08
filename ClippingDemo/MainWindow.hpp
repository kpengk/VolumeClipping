#pragma once
#include <QMainWindow>
#include "BoundingBox.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
};
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void initButton();

private:
    Ui::MainWindow* ui;
    vtkSmartPointer<CBvtkBoxWidget3D> box_widget_;
};
