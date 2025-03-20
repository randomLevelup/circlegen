#include "MainWindow.h"
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    imageLabel = new QLabel(this);
    QPixmap image("path/to/your/image.jpg");
    imageLabel->setPixmap(image);
    setCentralWidget(imageLabel);
    resize(image.size());
    setWindowTitle("Image Viewer");
}

MainWindow::~MainWindow()
{
    delete imageLabel;
}
