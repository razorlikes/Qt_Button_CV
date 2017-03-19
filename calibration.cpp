#include "calibration.h"

#define RECT_SIZE 40

Calibration::Calibration(QWidget *parent) : QWidget(parent)
{
    resize(780, 500);

    viewfinder = new QCameraViewfinder(this);
    viewfinder->setGeometry(130, 10, 640, 480);

    cSettings.setResolution(QSize(640, 480));

    camera = new QCamera(this);
    camera->setViewfinder(viewfinder);
    camera->setViewfinderSettings(cSettings);

    QPixmap overlay(640, 480);
    overlay.fill(Qt::transparent);
    p.begin(&overlay);
    p.setPen(QPen(Qt::green, 2));
    p.drawLine(320, 0, 320, 480);
    p.setPen(QPen(Qt::darkRed, 2));
    p.setFont(QFont("Sans Serif" , 16, 50, false));
    p.drawRect(1, 1, RECT_SIZE, RECT_SIZE);
    p.drawRect(640 - RECT_SIZE, 1, RECT_SIZE - 1, RECT_SIZE);
    p.drawText(QRect(0, RECT_SIZE + 10, 640, 200), Qt::AlignHCenter | Qt::AlignTop,
               "Please align the two crosses with the two red squares."
               "\nThe crosses should be on the same height and inside the squares!"
               "\n\nThe two buttons should be separated by the green line!");
    p.end();

    lblOverlay = new QLabel(this);
    lblOverlay->setGeometry(130, 10, 640, 480);
    lblOverlay->setPixmap(overlay);

    btnConfirm = new QPushButton(this);
    btnConfirm->setGeometry(10, 10, 100, 30);
    btnConfirm->setText("Confirm calibration");
    connect(btnConfirm, SIGNAL(clicked()), this, SLOT(close()));

    camera->load();
    camera->start();
}

void Calibration::closeEvent(QCloseEvent *event)
{
    camera->stop();
    camera->unload();

    emit formClosing();
}
