#include "worker.h"
#include <QDebug>

#define RECT_SIZE 50

Worker::Worker(QObject *parent) : QObject(parent)
{
    p = new QPainter();
}

void Worker::convertFrame(QImage frame, int separator, bool B910Fix)
{
    QPixmap overlay(frame.size());
    overlay.fill(Qt::transparent);
	
    p->begin(&overlay);
    p->setPen(Qt::red);

    image = matrix(frame.width(), QVector<bool>(frame.height()));
    edges = matrix(frame.width(), QVector<bool>(frame.height()));

    int rectSize = RECT_SIZE / (640 / frame.width());
	for (int y = 0; y < frame.height(); y++)
    {
        QRgb* row = (QRgb*)frame.scanLine(y);
        QRgb* rowU;  //workaround
        QRgb* rowD;  //workaround
        if ((y != frame.height() - 1) && (y != 0))
        {
            rowU = (QRgb*)frame.scanLine(y - 1);
            rowD = (QRgb*)frame.scanLine(y + 1);
        }
        else if (B910Fix && y == frame.height() - 1)  //needed for b910 fix
        {
            rowU = (QRgb*)frame.scanLine(y - 1);
        }

        for (int x = 0; x < frame.width(); x++)
        {
            if (B910Fix && y == frame.height() - 1)  //ask this before all other statements so it WILL run
            {
                row[x] = rowU[x];
                image[x][y] = image[x][y - 1];
                edges[x][y] = edges[x][y - 1];
            }
            else if ((x <= rectSize && y <= rectSize) || (x >= frame.width() - rectSize && y <= rectSize))  //to blend out the two crosses needed for calibration
            {
                row[x] = qRgb(255, 255, 255);
                edges[x][y] = false;
                image[x][y] = false;
            }
            else if (((x != frame.width() - 1) && (x != 0) && (y != frame.height() - 1) && (y != 0)) &&
                    ((qGray(row[x-1]) > separator && qGray(row[x]) <= separator && qGray(row[x+1]) <= separator) ||
                    (qGray(row[x-1]) <= separator && qGray(row[x]) <= separator && qGray(row[x+1]) > separator) ||
                    (qGray(rowD[x]) > separator && qGray(row[x]) <= separator && qGray(rowU[x]) <= separator) ||
                    (qGray(rowD[x]) <= separator && qGray(row[x]) <= separator && qGray(rowU[x]) > separator)))
            {
                row[x] = qRgb(0, 0, 0);
                p->drawPoint(x, y);
                edges[x][y] = true;
                image[x][y] = true;
            }
            else if (((x != frame.width() - 1) && (x != 0) && ((y == frame.height() - 1) || (y == 0))) &&
                     ((qGray(row[x-1]) > separator && qGray(row[x]) <= separator && qGray(row[x+1]) <= separator) ||
                     (qGray(row[x-1]) <= separator && qGray(row[x]) <= separator && qGray(row[x+1]) > separator)))
            {
                row[x] = qRgb(0, 0, 0);
                p->drawPoint(x, y);
                edges[x][y] = true;
                image[x][y] = true;
            }
            else if (((x != frame.width() - 1) && (x != 0)) &&
                     (qGray(row[x]) <= separator && qGray(row[x+1]) <= separator && qGray(row[x-1]) <= separator))
            {
                row[x] = qRgb(0, 0, 0);
                edges[x][y] = false;
                image[x][y] = true;
            }
            else if ((x == 0) && (qGray(row[x]) <= separator && qGray(row[x+1]) <= separator))
            {
                row[x] = qRgb(0, 0, 0);
                edges[x][y] = false;
                image[x][y] = true;
            }
            else if ((x == frame.width() - 1) && (qGray(row[x]) <= separator && qGray(row[x-1]) <= separator))
            {
                row[x] = qRgb(0, 0, 0);
                edges[x][y] = false;
                image[x][y] = true;
            }
            else
            {
                row[x] = qRgb(255, 255, 255);
                edges[x][y] = false;
                image[x][y] = false;
            }
        }
    }

    //  >>>LEFT BUTTON<<<
    firstXL_t = -1, firstYL_t = -1, lastXL_t = -1, lastYL_t = -1;
    for (int x = 0; x < frame.width() / 2; x++)
    {
        for (int y = 0; y < frame.height(); y++)
        {
            if (image[x][y] == true)
            {
                firstXL_t = x;
                break;
            }
        }
        if (firstXL_t != -1)
            break;
    }

    for (int y = 0; y < frame.height(); y++)
    {
        for (int x = 0; x < frame.width() / 2; x++)
        {
            if (image[x][y] == true)
            {
                firstYL_t = y;
                break;
            }
        }
        if (firstYL_t != -1)
            break;
    }

    for (int x = frame.width() / 2; x > 0; x--)
    {
        for (int y = frame.height() - 1; y > 0; y--)
        {
            if (image[x][y] == true)
            {
                lastXL_t = x - 1;
                break;
            }
        }
        if (lastXL_t != -1)
            break;
    }

    for (int y = frame.height() - 1; y > 0; y--)
    {
        for (int x = frame.width() / 2; x > 0; x--)
        {
            if (image[x][y] == true)
            {
                lastYL_t = y - 1;
                break;
            }
        }
        if (lastYL_t != -1)
            break;
    }
    //qDebug() << "Left button scan passed!";

    //  >>>RIGHT BUTTON<<<
    firstXR_t = -1, firstYR_t = -1, lastXR_t = -1, lastYR_t = -1;//reset all temporary variables
    for (int x = frame.width() / 2; x < frame.width(); x++)
    {
        for (int y = 0; y < frame.height(); y++)
        {
            if (image[x][y] == true)
            {
                firstXR_t = x;
                break;
            }
        }
        if (firstXR_t != -1)  //firstXR != -1 to proove that the point has changed even if x == 0
            break;
    }

    for (int y = 0; y < frame.height(); y++)
    {
        for (int x = frame.width() / 2; x < frame.width(); x++)
        {
            if (image[x][y] == true)
            {
                firstYR_t = y;
                break;
            }
        }
        if (firstYR_t != -1)
            break;
    }

    for (int x = frame.width() - 1; x > frame.width() / 2; x--)
    {
        for (int y = frame.height() - 1; y > 0; y--)
        {
            if (image[x][y] == true)
            {
                lastXR_t = x - 1;
                break;
            }
        }
        if (lastXR_t != -1)
            break;
    }

    for (int y = frame.height() - 1; y > 0; y--)
    {
        for (int x = frame.width() - 1; x > frame.width() / 2; x--)
        {
            if (image[x][y] == true)
            {
                lastYR_t = y - 1;
                break;
            }
        }
        if (lastYR_t != -1)
            break;
    }
    //qDebug() << "Right button scan passed!";

    if (firstXR_t == -1 && !isFieldLocked)  //if no pixels are found set to 0 to prevent out of index crash on average calculation
    {
        firstXR = 0;
        firstYR = 0;
        lastXR = 0;
        lastYR = 0;
    }
    else if (firstXL_t == -1 && !isFieldLocked)
    {
        firstXL = 0;
        firstYL = 0;
        lastXL = 0;
        lastYL = 0;
    }
    else if (!isFieldLocked)  //if field is not locked set all coordinates to temporary coordinates
    {
        firstXR = firstXR_t;
        firstYR = firstYR_t;
        lastXR = lastXR_t;
        lastYR = lastYR_t;
        firstXL = firstXL_t;
        firstYL = firstYL_t;
        lastXL = lastXL_t;
        lastYL = lastYL_t;
    }
    //qDebug() << "Variable copy passed!";

    xAvgL = 0, yAvgL = 0;
    int pixNumL = 0;
    for (int y = firstYL; y <= lastYL; y++)
    {
        for (int x = firstXL; x <= lastXL; x++)
        {
            if (image[x][y] == true)
            {
                xAvgL += x;
                yAvgL += y;
                pixNumL++;
            }
        }
    }
    //qDebug() << "Left average calculation passed!";

    xAvgR = 0, yAvgR = 0;
    int pixNumR = 0;
    for (int y = firstYR; y <= lastYR; y++)
    {
        for (int x = firstXR; x <= lastXR; x++)
        {
            if (image[x][y] == true)
            {
                xAvgR += x;
                yAvgR += y;
                pixNumR++;
            }
        }
    }
    //qDebug() << "Right average calculation passed!";

    if (pixNumR > 0 && pixNumL > 0)  //prevent crash when 0 black pixels > divide by 0
    {
        xAvgR = xAvgR / pixNumR;
        yAvgR = yAvgR / pixNumR;
        xAvgL = xAvgL / pixNumL;
        yAvgL = yAvgL / pixNumL;

        if (isFieldLocked)
        {
            p->setPen(Qt::blue);
            p->drawLine(xAvgR, 0, xAvgR, frame.height());
            p->drawLine(frame.width() / 2, yAvgR, frame.width(), yAvgR);
            p->drawLine(xAvgL, 0, xAvgL, frame.height());
            p->drawLine(0, yAvgL, frame.width() / 2, yAvgL);
        }

        p->setPen(Qt::yellow);
        p->drawRect(QRect(QPoint(firstXR, firstYR), QPoint(lastXR, lastYR)));
        p->setPen(Qt::green);
        p->drawRect(QRect(QPoint(firstXL, firstYL), QPoint(lastXL, lastYL)));
    }
    //qDebug() << "Drawing passed!";

    p->end();  //outside of if clause since painter gets started anyway


    emit convertFinished(QPixmap::fromImage(frame), overlay, xAvgR, yAvgR, xAvgL, yAvgL);
}

void Worker::switchFieldLock()
{
   if (!isFieldLocked)
   {
       isFieldLocked = true;
       emit fieldLocked(xAvgR, yAvgR, xAvgL, yAvgL, lastXR - firstXR, lastYR - firstYR, lastXL - firstXL, lastYL - firstYL);
   }
   else
   {
       isFieldLocked = false;
       emit fieldUnLocked();
   }


}

void Worker::resChanged(double resRatioX, double resRatioY)
{
    if (isFieldLocked)
    {
        firstXR = firstXR * resRatioX;
        firstYR = firstYR * resRatioY;
        lastXR = lastXR * resRatioX;
        lastYR = lastYR * resRatioY;
    }
}
