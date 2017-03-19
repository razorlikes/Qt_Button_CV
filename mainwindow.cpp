#include "mainwindow.h"
#include <QDebug>

#define STANDARD_RES "320x240"
#define X_TRIG_PERCENTAGE 15
#define Y_TRIG_PERCENTAGE 10

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setGeometry(200, 200, 780, 500);  //x 1500

    surface = new VideoSurface();
    connect(surface, SIGNAL(frameAvailable(QImage)), this, SLOT(convertFrame(QImage)));

    camera = new QCamera;
    camera->setCaptureMode(QCamera::CaptureVideo);
    camera->setViewfinder(surface);

    frameInfoLabel = new QLabel(this);
    frameInfoLabel->setStyleSheet("QLabel { color: red; } ");
    frameInfoLabel->setGeometry(140, 11, 300, 12);

    resLabel = new QLabel(this);
    resLabel->setGeometry(10, 5, 100, 12);
    resLabel->setText("Camera resolution");

    cmbResolution = new QComboBox(this);
    cmbResolution->setGeometry(10, 20, 100, 20);
    connect(cmbResolution, SIGNAL(currentTextChanged(QString)), this, SLOT(updateRes()));

    brightnessLabel = new QLabel(this);
    brightnessLabel->setGeometry(10, 50, 100, 12);
    brightnessLabel->setText("Brightness separator");

    sldBrightness = new QSlider(Qt::Horizontal, this);
    sldBrightness->setGeometry(10, 65, 100, 20);
    sldBrightness->setMaximum(255);
    sldBrightness->setMinimum(0);
    sldBrightness->setValue(50);
    sldBrightness->setToolTip(QString::number(sldBrightness->value()));

    ckbB910Fix = new QCheckBox(this);
    ckbB910Fix->move(10, 90);
    ckbB910Fix->setText("B910 Fix");

    btnStartCamera = new QPushButton(this);
    btnStartCamera->setGeometry(10, 130, 100, 30);
    btnStartCamera->setText("Start Camera");
    connect(btnStartCamera, SIGNAL(clicked()), this, SLOT(startCamera()));

    btnStopCamera = new QPushButton(this);
    btnStopCamera->setGeometry(10, 130, 100, 30);
    btnStopCamera->setText("Stop Camera");
    btnStopCamera->hide();
    connect(btnStopCamera, SIGNAL(clicked()), camera, SLOT(stop()));
    connect(btnStopCamera, SIGNAL(clicked()), frameInfoLabel, SLOT(hide()));
    connect(btnStopCamera, SIGNAL(clicked()), btnStartCamera, SLOT(show()));
    connect(btnStopCamera, SIGNAL(clicked()), btnStopCamera, SLOT(hide()));
    connect(btnStartCamera, SIGNAL(clicked()), btnStopCamera, SLOT(show()));  //switch buttons since they cant do anything at the same time
    connect(btnStartCamera, SIGNAL(clicked()), btnStartCamera, SLOT(hide()));

    worker = new Worker;
    workerThread = new QThread;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()), Qt::QueuedConnection);
    connect(this, SIGNAL(startThread(QImage,int,bool)), worker, SLOT(convertFrame(QImage,int,bool)), Qt::QueuedConnection);
    connect(worker, SIGNAL(convertFinished(QPixmap,QPixmap,int,int,int,int)), this, SLOT(updateUI(QPixmap,QPixmap,int,int,int,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(resChanged(double,double)), worker, SLOT(resChanged(double,double)));
    connect(worker, SIGNAL(fieldLocked(int,int,int,int,int,int,int,int)), this, SLOT(fieldLocked(int,int,int,int,int,int,int,int)));
    connect(worker, SIGNAL(fieldUnLocked()), this, SLOT(fieldUnLocked()));
    workerThread->start();

    btnLockField = new QPushButton(this);
    btnLockField->setGeometry(10, 165, 100, 30);
    btnLockField->setText("Un-/Lock Field");
    connect(btnLockField, SIGNAL(clicked()), worker, SLOT(switchFieldLock()));

    btnRestartCalib = new QPushButton(this);
    btnRestartCalib->setGeometry(10, 200, 100, 30);
    btnRestartCalib->setText("Restart Calibration");
    connect(btnRestartCalib, SIGNAL(clicked()), this, SLOT(restartCalib()));
    connect(btnRestartCalib, SIGNAL(clicked()), btnStartCamera, SLOT(show()));
    connect(btnRestartCalib, SIGNAL(clicked()), btnStopCamera, SLOT(hide()));

    btnTestLaunchL = new QPushButton(this);
    btnTestLaunchL->setGeometry(10, 460, 48, 30);
    btnTestLaunchL->setText("Test L");
    connect(btnTestLaunchL, SIGNAL(clicked()), this, SLOT(launchApplicationL()));

    btnTestLaunchR = new QPushButton(this);
    btnTestLaunchR->setGeometry(62, 460, 48, 30);
    btnTestLaunchR->setText("Test R");
    connect(btnTestLaunchR, SIGNAL(clicked()), this, SLOT(launchApplicationR()));

    camera->load();
    for (int i = 0; i < camera->supportedViewfinderResolutions().size(); i++)
    {
        QString resolutionW = QString::number(camera->supportedViewfinderResolutions()[i].width());
        QString resolutionH = QString::number(camera->supportedViewfinderResolutions()[i].height());

        cmbResolution->addItem(resolutionW + "x" + resolutionH, camera->supportedViewfinderResolutions()[i]);

        if (resolutionW + "x" + resolutionH == STANDARD_RES)
            cmbResolution->setCurrentIndex(i);
    }
    camera->unload();

    checkCameras();

    Calibration* calib = new Calibration();
    calib->show();
    QEventLoop evLoop;
    connect(calib, SIGNAL(formClosing()), &evLoop, SLOT(quit()));
    evLoop.exec();
}

void MainWindow::checkCameras()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

    if (cameras.isEmpty())
        qDebug() << "No cameras available!";
    else
    {
        qDebug() << "Available cameras:";
        foreach (const QCameraInfo &cameraInfo, cameras)
        {
            qDebug() << cameraInfo.description() << " ID:" << cameraInfo.deviceName();
            if ( cameraInfo.description() == "Logitech B910 HD Webcam")
                ckbB910Fix->setChecked(true);
        }
    }  
}

void MainWindow::startCamera()
{
    updateRes();
    camera->start();
    qDebug() << "Camera started with resolution:" << viewfinderSettings.resolution();
    frameInfoLabel->show();
    convertFinished = true;
}

void MainWindow::updateRes()
{
    int oldX = viewfinderSettings.resolution().width();  //needed to recalculate position of button field when locked and resolution changes
    int oldY = viewfinderSettings.resolution().height();
    int newX = cmbResolution->currentData().toSize().width();
    int newY = cmbResolution->currentData().toSize().height();
    double resRatioX = (double)newX / (double)oldX;
    double resRatioY = (double)newY / (double)oldY;
    emit resChanged(resRatioX, resRatioY);

    viewfinderSettings.setResolution(cmbResolution->currentData().toSize());
    camera->setViewfinderSettings(viewfinderSettings);

    qDebug() << "New resolution:" << viewfinderSettings.resolution() << " Resolution ratio to old resolution:" << resRatioX << "X" << resRatioY << "Y";
}

void MainWindow::convertFrame(QImage frame)
{
    if (convertFinished)  //handle this in main thread so no delay builds up
    {
        convertFinished = false;
        timer.restart();

        frame = frame.mirrored(false, true);

        emit startThread(frame, sldBrightness->value(), ckbB910Fix->isChecked());
    }
}

void MainWindow::updateUI(QPixmap frame, QPixmap overlay, int xAvgR_t, int yAvgR_t, int xAvgL_t, int yAvgL_t)
{
    cFrame = frame;  //copy pixmaps from worker to temporary global pixmaps
	cOverlay = overlay;

    repaint();  //use qpainter instead of qlabels for images since its faster and keeps ui more responsive

    _ms = timer.elapsed();
    fps3 = fps2;
    fps2 = fps1;
    fps1 = 1 / (double)_ms * 1000;

    double fps = (fps1 + fps2 + fps3) / 3;


    if (!pressedR && isFieldLocked && (((xAvgR != 0) && ((xAvgR_t - xAvgR >= (widthR / 100.0 * X_TRIG_PERCENTAGE)) || (xAvgR_t - xAvgR <= (widthR / 100.0 * -X_TRIG_PERCENTAGE)))) ||
                          ((yAvgR != 0) && ((yAvgR_t - yAvgR >= (heightR / 100.0 * Y_TRIG_PERCENTAGE)) || (yAvgR_t - yAvgR <= (heightR / 100.0 * -Y_TRIG_PERCENTAGE))))))
    {
        qDebug() << "RIGHT average coordinates differed more than " << widthR / 100.0 * X_TRIG_PERCENTAGE << "X and" << heightR / 100.0 * Y_TRIG_PERCENTAGE << "Y  at " << xAvgR_t << "X" << yAvgR_t << "Y";
        qDebug() << "Launch triggered!";
        launchApplicationR();  //launch application when average values differ more than the given percentage
        pressedR = true;
    }
    else if ((xAvgR_t >= xAvgR - 1 && xAvgR_t <= xAvgR + 1) && (yAvgR_t >= yAvgR - 1 && yAvgR_t <= yAvgR + 1))  //reset when average coordinates go back to initial averages +/-1
        pressedR = false;

    if (!pressedL && isFieldLocked && (((xAvgL != 0) && ((xAvgL_t - xAvgL >= (widthL / 100.0 * X_TRIG_PERCENTAGE)) || (xAvgL_t - xAvgL <= (widthL / 100.0 * -X_TRIG_PERCENTAGE)))) ||
                          ((yAvgL != 0) && ((yAvgL_t - yAvgL >= (heightL / 100.0 * Y_TRIG_PERCENTAGE)) || (yAvgL_t - yAvgL <= (heightL / 100.0 * -Y_TRIG_PERCENTAGE))))))
    {
        qDebug() << "LEFT average coordinates differed more than " << widthL / 100.0 * X_TRIG_PERCENTAGE << "X and" << heightL / 100.0 * Y_TRIG_PERCENTAGE << "Y  at " << xAvgL_t << "X" << yAvgL_t << "Y";
        qDebug() << "Launch triggered!";
        launchApplicationL();
        pressedL = true;
    }
    else if ((xAvgL_t >= xAvgL - 1 && xAvgL_t <= xAvgL + 1) && (yAvgL_t >= yAvgL - 1 && yAvgL_t <= yAvgL + 1))
        pressedL = false;


    frameInfoLabel->setText("Frametime: " + QString::number(_ms) + "ms" + "  FPS: " + QString::number((int)fps));

    convertFinished = true;
}

void MainWindow::restartCalib()
{
    camera->stop();
    frameInfoLabel->hide();

    Calibration* calib = new Calibration();
    calib->show();
    QEventLoop evLoop;
    connect(calib, SIGNAL(formClosing()), &evLoop, SLOT(quit()));
    evLoop.exec();
}

void MainWindow::launchApplicationL()
{

}

void MainWindow::launchApplicationR()
{
    //QProcess::startDetached("notepad");
}

void MainWindow::fieldLocked(int xAvgR_t, int yAvgR_t, int xAvgL_t, int yAvgL_t, int widthR_t, int heightR_t, int widthL_t, int heightL_t)
{
    xAvgR = xAvgR_t;  //assign final average values to average variables
    yAvgR = yAvgR_t;
    xAvgL = xAvgL_t;
    yAvgL = yAvgL_t;

    widthR = widthR_t;
    heightR = heightR_t;
    widthL = widthL_t;
    heightL = heightL_t;

    isFieldLocked = true;

    qDebug() << "Fields locked with size" << widthR << "x" << heightR << "R  and " << widthL << "x" << heightL << "L";
}

void MainWindow::fieldUnLocked()
{
    isFieldLocked = false;
    pressedL = false;
    pressedR = false;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    p.begin(this);
    p.drawPixmap(130, 10, 640, 480, cFrame, 0, 0, cFrame.width(), cFrame.height());
    p.drawPixmap(130, 10, 640, 480, cOverlay, 0, 0, cOverlay.width(), cOverlay.height());

    if (pressedL)
        p.setBrush(Qt::green);
    else
        p.setBrush(Qt::gray);
    p.drawRect(10, 240, 100, 100);

    if (pressedR)
        p.setBrush(Qt::yellow);
    else
        p.setBrush(Qt::gray);
    p.drawRect(10, 345, 100, 100);

    p.drawText(QRect(10, 240, 100, 100), Qt::AlignCenter, "LEFT Button");
    p.drawText(QRect(10, 345, 100, 100), Qt::AlignCenter, "RIGHT Button");
    p.end();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    workerThread->exit();  //end worker thread so no error message occurs
}










//  ====================================
//  ==            Old Code            ==
//  ====================================

//bwImageLabel->setPixmap(frame);
//overlayLabel->setPixmap(overlay);
//averageLabel->setPixmap(averageImage);
//averageLabelOverlay->setPixmap(overlay);

/*    for (int y = 0; y < grayscale.height(); y++)
    {
        for (int x = 0; x < grayscale.width(); x++)
        {
            if (grayscale.pixelColor(x, y).red() >= separator)
                grayscale.setPixelColor(x, y, Qt::white);
            else if (grayscale.pixelColor(x, y).red() < separator)
                grayscale.setPixelColor(x, y, Qt::black);
        }
    }*/

/*QPixmap MainWindow::captureGrayscale(int separator)
{
    timer.restart();

    QImage grayscale = QPixmap(this->grab(QRect(130, 10, 640, 480))).toImage();

    for (int y = 0; y < grayscale.height(); y++)
    {
        QRgb* row = (QRgb*)grayscale.scanLine(y);
        //QRgb* rowU = (QRgb*)grayscale.scanLine(y-1);  implement later
        //QRgb* rowD = (QRgb*)grayscale.scanLine(y+1);
        for (int x = 0; x < grayscale.width(); x++)
        {
            if ((qGray(row[x]) > separator && qGray(row[x-1]) > separator) || (qGray(row[x]) > separator && qGray(row[x+1]) > separator)// && qGray(rowU[x]) > separator && qGray(rowD[x]) > separator)
                row[x] = qRgb(255, 255, 255);
            else if ((x == 0) && (qGray(row[x]) > separator && qGray(row[x+1]) > separator))
                row[x] = qRgb(255, 255, 255);
            else if ((x == grayscale.width() - 1) && (qGray(row[x]) > separator && qGray(row[x-1]) > separator))
                row[x] = qRgb(255, 255, 255);
            else
                row[x] = qRgb(0, 0, 0);
        }
    }

    return(QPixmap::fromImage(grayscale));
}*/

/*        for (int y = 0; y < frame.height(); y++)
        {
                QRgb* row = (QRgb*)frame.scanLine(y);
                if (y > 0)
                    QRgb* rowU = (QRgb*)frame.scanLine(y-1);
                if (y < (frame.height() - 1))
                    QRgb* rowD = (QRgb*)frame.scanLine(y+1);
            for (int x = 0; x < frame.width(); x++)
            {
                if (qGray(row[x]) > separator && qGray(row[x+1]) > separator && qGray(rowD[x]) > separator)  //B BWW W
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(rowU[x]) > separator && qGray(row[x]) > separator && qGray(row[x+1]) > separator)  //W BWW B
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(rowU[x]) > separator && qGray(row[x-1]) > separator && qGray(row[x]) > separator)  //W WWB B
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(row[x-1]) > separator && qGray(row[x]) > separator && qGray(rowD[x]) > separator)  //B WWB W
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(rowU[x]) > separator && qGray(row[x]) > separator && qGray(row[x+1]) > separator && qGray(rowD[x]) > separator)  //W BWW W
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(rowU[x]) > separator && qGray(row[x-1]) > separator && qGray(row[x]) > separator && qGray(row[x+1]) > separator)  //W WWW B
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(rowU[x]) > separator && qGray(row[x-1]) > separator && qGray(row[x]) > separator && qGray(rowD[x]) > separator)  //W WWB W
                    row[x] = qRgb(255, 255, 255);
                else if (qGray(row[x-1]) > separator && qGray(row[x]) > separator && qGray(row[x+1]) > separator && qGray(rowD[x]) > separator)  //B WWW W
                    row[x] = qRgb(255, 255, 255);
                else
                    row[x] = qRgb(0, 0, 0);
            }
        }*/

/*void MainWindow::captureTmr(QImage bwImage)
{
    QPixmap overlay = QPixmap::fromImage(bwImage);

    p->begin(&overlay);
    p->setPen(Qt::red);

    for (int y = 0; y < bwImage.height(); y++)
    {
        QRgb* row = (QRgb*)bwImage.scanLine(y);
        for (int x = 0; x < bwImage.width(); x++)
        {
            if (((x != 0 || x != bwImage.width()) && (y != 0 || y != bwImage.height())) &&
                    ((qGray(row[x]) == 255 && qGray(row[x-1]) == 255 && qGray(row[x+1]) == 0) ||
                     (qGray(row[x]) == 255 && qGray(row[x-1]) == 0 && qGray(row[x+1]) == 255)))
                p->drawPoint(x, y);
        }
    }
    p->end();

    overlayLabel->setPixmap(overlay);

    _ms = timer.elapsed();
    fps3 = fps2;
    fps2 = fps1;
    fps1 = 1 / (double)_ms * 1000;

    double fps = (fps1 + fps2 + fps3) / 3;

    qDebug() << "Frametime:" << _ms << "ms" << "FPS:" << fps;
    frameInfoLabel->setText("Frametime: " + QString::number(_ms) + "ms" + "  FPS: " + QString::number((int)fps));
    convertFinished = true;

    //grayscaleLabel->setPixmap(QPixmap::fromImage(bwImage));
}*/

/*void MainWindow::convertFrame(QImage frame)
{
    if (convertFinished)
    {
        convertFinished = false;
        timer.restart();
        int separator = sldBrightness->value();
        frame = frame.mirrored(false, true);

        for (int y = 0; y < frame.height(); y++)
        {
                QRgb* row = (QRgb*)frame.scanLine(y);
                if (y > 0)
                    QRgb* rowU = (QRgb*)grayscale.scanLine(y-1);
                if (y < (frame.height() - 1))
                    QRgb* rowD = (QRgb*)grayscale.scanLine(y+1);
            for (int x = 0; x < frame.width(); x++)
            {
                if ((qGray(row[x]) > separator && qGray(row[x-1]) > separator) ||(qGray(row[x]) > separator && qGray(row[x+1]) > separator))
                    row[x] = qRgb(255, 255, 255);
                else if (x == 0 && qGray(row[x]) > separator && qGray(row[x+1]) > separator)
                    row[x] = qRgb(255, 255, 255);
                else if (x == frame.width() && qGray(row[x]) > separator && qGray(row[x-1]) > separator)
                    row[x] = qRgb(255, 255, 255);
                else
                    row[x] = qRgb(0, 0, 0);
            }
        }

        QPixmap overlay = QPixmap::fromImage(frame);

        p->begin(&overlay);
        p->setPen(Qt::red);

        for (int y = 0; y < frame.height(); y++)
        {
            QRgb* row = (QRgb*)frame.scanLine(y);
            for (int x = 0; x < frame.width(); x++)
            {
                if (((x != 0 && x != frame.width()) && (y != 0 && y != frame.height())) &&
                        ((qGray(row[x]) == 255 && qGray(row[x-1]) == 255 && qGray(row[x+1]) == 0) ||
                         (qGray(row[x]) == 255 && qGray(row[x-1]) == 0 && qGray(row[x+1]) == 255)))
                    p->drawPoint(x, y);
            }
        }
        p->end();

        overlayLabel->setPixmap(overlay);

        _ms = timer.elapsed();
        fps3 = fps2;
        fps2 = fps1;
        fps1 = 1 / (double)_ms * 1000;

        double fps = (fps1 + fps2 + fps3) / 3;

        //qDebug() << "Frametime:" << _ms << "ms" << "FPS:" << fps;
        frameInfoLabel->setText("Frametime: " + QString::number(_ms) + "ms" + "  FPS: " + QString::number((int)fps));
        convertFinished = true;
    }
}*/
