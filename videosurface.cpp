#include "videosurface.h"
#include <QDebug>

VideoSurface::VideoSurface(QObject *parent) : QAbstractVideoSurface(parent)
{

}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;  //format of webcam video frames
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    QImage convertedFrame;
    QVideoFrame cFrame(frame);

    cFrame.map(QAbstractVideoBuffer::ReadOnly);
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(cFrame.pixelFormat());

    convertedFrame = QImage(cFrame.bits(), cFrame.width(), cFrame.height(), cFrame.bytesPerLine(), imageFormat);
    //qDebug() << "Frame format:" << cFrame.pixelFormat() << " Image format:" << imageFormat;

    emit frameAvailable(convertedFrame);
    cFrame.unmap();

    return true;
}
