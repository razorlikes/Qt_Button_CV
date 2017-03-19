#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraViewfinderSettings>
#include <QPushButton>
#include <QCameraImageCapture>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QVideoWidget>
#include <QRect>
#include <QSlider>
#include <QTime>
#include <QPainter>
#include <QBitmap>
#include <QVideoFrame>
#include <videosurface.h>
#include <QVector>
#include <QCheckBox>
#include <calibration.h>
#include <QEventLoop>
#include <QThread>
#include <worker.h>
#include <QMetaType>
#include <QProcess>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    int _ms = 100;
    int xAvgR = 0, yAvgR = 0, xAvgL = 0, yAvgL = 0, widthR, heightR, widthL, heightL;
    bool convertFinished = false, calibFinished = false, isFieldLocked = false, pressedR = false, pressedL = false;
    double fps1, fps2, fps3;

    QCamera* camera;
    QVideoWidget* viewfinder;
    QCameraImageCapture* imageCapture;

    QPushButton* btnStartCamera;
    QPushButton* btnStopCamera;
    QPushButton* btnLockField;
    QPushButton* btnRestartCalib;
    QPushButton* btnTestLaunchL;
    QPushButton* btnTestLaunchR;

    QCheckBox* ckbB910Fix;

    QComboBox* cmbResolution;

    QLabel* resLabel;
    QLabel* brightnessLabel;
    QLabel* frameInfoLabel;
    QLabel* overlayLabel;
    QLabel* bwImageLabel;
    QLabel* averageLabel;
    QLabel* averageLabelOverlay;

    QSlider* sldBrightness;

    QTime timer;

    QCameraViewfinderSettings viewfinderSettings;

    VideoSurface* surface;

    Worker* worker;

    QThread* workerThread;

    QPainter p;

    QProcess* process;

	QPixmap cFrame;
	QPixmap cOverlay;

    void checkCameras();
    void closeEvent(QCloseEvent *event);
	void paintEvent(QPaintEvent *event);

public slots:
    void startCamera();
	void updateRes();
	void convertFrame(QImage frame);
    void updateUI(QPixmap frame, QPixmap overlay, int xAvgR_t, int yAvgR_t, int xAvgL_t, int yAvgL_t);
    void restartCalib();
    void launchApplicationL();
    void launchApplicationR();
    void fieldLocked(int xAvgR_t, int yAvgR_t, int xAvgL_t, int yAvgL_t, int widthR, int heightR, int widthL_t, int heightL_t);
    void fieldUnLocked();

signals:
    void startThread(QImage frame, int separator, bool B910Fix);
    void resChanged(double resRatioX, double resRatioY);

};

#endif // MAINWINDOW_H
