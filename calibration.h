#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinderSettings>
#include <QCameraViewfinder>
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPen>
#include <QFont>

class Calibration : public QWidget
{
    Q_OBJECT
public:
    explicit Calibration(QWidget *parent = 0);
    void closeEvent(QCloseEvent *event);

    QCamera* camera;
    QCameraViewfinder* viewfinder;
    QCameraViewfinderSettings cSettings;

    QPainter p;
    QLabel* lblOverlay;

    QPushButton* btnConfirm;


signals:
	void formClosing();

public slots:
};

#endif // CALIBRATION_H
