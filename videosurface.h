#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QList>

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit VideoSurface(QObject *parent = 0);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
    bool present(const QVideoFrame &frame);

signals:
    void frameAvailable(QImage frame);

public slots:
};

#endif // VIDEOSURFACE_H
