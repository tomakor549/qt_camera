#include "cameramanager.h"
#include <QDebug>
#include <QDir>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <qbuffer.h>
#include <qmediaformat.h>
#include <qmetaobject.h>
#include <qurl.h>

CameraManager::CameraManager(CameraQuality quality, QObject *parent)
    : QObject(parent) {
  _watermark.load(":/logo.jpg");
  _watermark = _watermark.scaled(300, 100, Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
  _watermark.isNull() ? qDebug() << "Watermark is not loaded"
                      : qDebug() << "Watermark loaded";
  _record_session.setRecorder(&_recorder);
  _main_session.setVideoSink(&_sink);
  _main_session.setCamera(&_camera);

  _quality = quality;

  _record_session.setVideoFrameInput(&_frame_input);

  connect(&_sink, &QVideoSink::videoFrameChanged, this,
          &CameraManager::onVideoFrameChanged);

  connect(&_recorder, &QMediaRecorder::errorChanged, [this]() {
    qDebug() << "Recording error:" << _recorder.error()
             << _recorder.errorString();
  });
}

QVideoSink *CameraManager::previewVideoSink() const {
  return _preview_video_sink.get();
}

void CameraManager::setPreviewVideoSink(QVideoSink *sink) {
  if (_preview_video_sink.get() == sink) {
    return;
  }

  _preview_video_sink = sink;
  emit videoSinkChanged();
}

void CameraManager::start(const QString &cameraId) {
  _camera.stop();

  QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
  QCameraDevice selected;

  if (cameraId.isEmpty()) {
    emit cameraErrorOccurred("Source camera is not selected");
    return;
  }

  auto it = std::ranges::find_if(cameras, [&](const QCameraDevice &camera) {
    return camera.id().contains(cameraId.toUtf8());
  });

  if (it == cameras.end()) {
    qDebug() << "Camera not found:" << cameraId;
    emit cameraErrorOccurred("Camera not found");
    return;
  }

  selected = *it;
  qDebug() << "Selected camera:" << selected.description();

  _camera.setCameraDevice(selected);

  QCameraFormat format =
      _quality == CameraQuality::kVeryHigh
          ? findHighCompatibleFormat(selected.videoFormats())
          : findBaseCompatibleFormat(selected.videoFormats());

  qDebug() << "Set camera format: Resolution:" << format.resolution()
           << "| Max FPS:" << format.maxFrameRate();

  _camera.setCameraFormat(format);
  _camera.start();
}

void CameraManager::stop() {
  stopRecord();
  _camera.stop();
  if (_preview_video_sink) {
    _preview_video_sink->setVideoFrame(QVideoFrame());
  }
}

void CameraManager::startRecord(const QString &directory, QString fileName) {
  if (_recorder.recorderState() == QMediaRecorder::RecordingState ||
      _camera.cameraFormat().isNull()) {
    return;
  }

  QMediaFormat format;
  format.setFileFormat(QMediaFormat::MPEG4);
  format.setVideoCodec(QMediaFormat::VideoCodec::H265);
  fileName += ".mp4";

  const QString filePath = QDir(directory).filePath(fileName);
  _recorder.setOutputLocation(QUrl::fromLocalFile(filePath));

  _recorder.setMediaFormat(format);
  _recorder.setQuality(_quality == CameraQuality::kVeryHigh
                           ? QMediaRecorder::VeryHighQuality
                           : QMediaRecorder::NormalQuality);

  ////for QMediaFormat::VideoCodec::MPEG4
  // _recorder.setEncodingMode(QMediaRecorder::AverageBitRateEncoding);
  // _recorder.setVideoBitRate(
  //     _quality == CameraQuality::kVeryHigh ? kHighBitRate : kNormalBitRate);

  _recorder.record();
  qDebug() << "Recording started:" << filePath;
}

void CameraManager::stopRecord() {
  if (_recorder.recorderState() == QMediaRecorder::RecordingState) {
    _recorder.stop();
    qDebug() << "Recording stop";
  }
}

void CameraManager::onVideoFrameChanged(const QVideoFrame &frame) {
  if (!frame.isValid()) {
    return;
  }

  emit newFrameAvailable(frame);

  // Format_RGB32 - alpha not needed
  QImage image = frame.toImage();
  emit newFrameAsImageAvailable(image);

  if (_recorder.recorderState() == QMediaRecorder::RecordingState) {
    setWatermark(image, 0.8);

    QVideoFrame frame_with_watermark(image);
    frame_with_watermark.setStartTime(frame.startTime());
    frame_with_watermark.setEndTime(frame.endTime());
    frame_with_watermark.setStreamFrameRate(frame.streamFrameRate());

    _record_session.videoFrameInput()->sendVideoFrame(frame_with_watermark);
  }
  if (_preview_video_sink) {
    _preview_video_sink->setVideoFrame(frame);
  }
}

void CameraManager::setWatermark(QImage &image, qreal opacity) {
  if (!_watermark.isNull()) {
    QPainter painter(&image);
    painter.setOpacity(opacity);

    int x = image.width() - _watermark.width() - 20;
    int y = image.height() - _watermark.height() - 20;

    painter.drawImage(x, y, _watermark);
    painter.end();
  }
}

QCameraFormat
CameraManager::findBaseCompatibleFormat(const QList<QCameraFormat> formats) {
  QCameraFormat bestFormat;
  int bestArea = 0;

  for (const auto &f : formats) {
    const int width = f.resolution().width();
    const int height = f.resolution().height();
    const int area = width * height;

    if (f.pixelFormat() == kStandardVideoFormat &&
        width >= kMinimumNormalFrameWidth &&
        width <= kMaximumNormalFrameWidth &&
        height >= kMinimumNormalFrameHeight &&
        height <= kMaximumNormalFrameHeight) {

      if (area > bestArea) {
        bestFormat = f;
        bestArea = area;
      }
    }
  }

  return bestFormat;
}

QCameraFormat
CameraManager::findHighCompatibleFormat(QList<QCameraFormat> formats) {
  QCameraFormat bestFormat;
  int bestArea = 0;

  for (const auto &f : formats) {
    const int width = f.resolution().width();
    const int height = f.resolution().height();
    const int area = width * height;

    if (f.pixelFormat() == kStandardVideoFormat &&
        width >= kMinimumHighFrameWidth && width <= kMaximumHighFrameWidth &&
        height >= kMinimumHighFrameHeight &&
        height <= kMaximumHighFrameHeight) {

      if (area > bestArea) {
        bestFormat = f;
        bestArea = area;
      }
    }
  }

  return bestFormat;
}