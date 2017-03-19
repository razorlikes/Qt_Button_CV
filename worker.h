#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QPixmap>
#include <QPainter>
#include <QVector>
#include <QImage>

typedef QVector< QVector<bool> > matrix;

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

    int firstXR = 0, firstYR = 0, lastXR = 0, lastYR = 0;
    int firstXR_t = -1, firstYR_t = -1, lastXR_t = -1, lastYR_t = -1;  //temporary variables for right button
    int xAvgR = 0, yAvgR = 0;

    int firstXL = 0, firstYL = 0, lastXL = 0, lastYL = 0;
    int firstXL_t = -1, firstYL_t = -1, lastXL_t = -1, lastYL_t = -1;  //temporary variables for left button
    int xAvgL = 0, yAvgL = 0;

    bool isFieldLocked = false;

    matrix image;
    matrix edges;
    matrix average;

    QPainter* p;

signals:
    void convertFinished(QPixmap frame, QPixmap overlay, int xAvgR, int yAvgR, int xAvgL, int yAvgL);
    void fieldLocked(int xAvgR, int yAvgR, int xAvgL, int yAvgL, int widthR, int heightR, int widthL, int heightL);
    void fieldUnLocked();

public slots:
    void convertFrame(QImage frame, int separator, bool B910Fix);
    void switchFieldLock();
    void resChanged(double resRatioX, double resRatioY);
};

#endif // WORKER_H
