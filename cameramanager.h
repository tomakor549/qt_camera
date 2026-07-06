#pragma once
#include <QCamera>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QObject>
#include <QPixelFormat>
#include <QPointer>
#include <QVideoFrame>
#include <QVideoFrameInput>
#include <QVideoSink>
#include <qbuffer.h>
#include <qelapsedtimer.h>
#include <qpainter.h>

enum class CameraQuality { kNormal, kVeryHigh };

class CameraManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVideoSink *videoSink READ previewVideoSink WRITE
                 setPreviewVideoSink NOTIFY videoSinkChanged)

public:
  explicit CameraManager(CameraQuality quality = CameraQuality::kVeryHigh,
                         QObject *parent = nullptr);

  QVideoSink *previewVideoSink() const;

public slots:
  void setPreviewVideoSink(QVideoSink *sink);
  void start(const QString &deviceName);
  void stop();

  void startRecord(const QString &filePath, QString fileName);
  void stopRecord();

signals:
  void videoSinkChanged();
  void newFrameAvailable(const QVideoFrame &frame);
  void newFrameAsImageAvailable(const QImage &image);
  void cameraErrorOccurred(const QString &errorString);

private slots:
  void onVideoFrameChanged(const QVideoFrame &frame);

private:
  const QSize kRecordFrameSize{kRecordFrameWidth, kRecordFrameHeight};
  CameraQuality _quality;
  QCamera _camera;
  QMediaCaptureSession _main_session;
  QMediaCaptureSession _record_session;

  QVideoSink _sink;
  QMediaRecorder _recorder;
  QVideoFrameInput _frame_input;

  // watermark
  QImage _watermark;
  QSize _watermark_right_down_margins;

  QPointer<QVideoSink> _preview_video_sink = nullptr;

  void setWatermark(QImage &image, qreal opacity = 1.0);
  void initWatermark(const QString &watermarkPath = ":/logo.jpg",
                     int rightEdgeMargin = 20, int downEdgeMargin = 20,
                     int watermarkHeight = kRecordFrameHeight / 8);

  static QCameraFormat findBaseCompatibleFormat(QList<QCameraFormat> formats);
  static QCameraFormat findHighCompatibleFormat(QList<QCameraFormat> formats);

  static constexpr int kRecordFrameWidth = 1080;
  static constexpr int kRecordFrameHeight = 720;

  static constexpr int kMaximumNormalFrameWidth = 1920;
  static constexpr int kMinimumNormalFrameWidth = 1080;
  static constexpr int kMaximumNormalFrameHeight = 1080;
  static constexpr int kMinimumNormalFrameHeight = 720;

  static constexpr int kMaximumHighFrameWidth = 2592;
  static constexpr int kMinimumHighFrameWidth = 2560;
  static constexpr int kMaximumHighFrameHeight = 1944;
  static constexpr int kMinimumHighFrameHeight = 1440;

  static constexpr QVideoFrameFormat::PixelFormat kStandardVideoFormat =
      QVideoFrameFormat::Format_Jpeg;
};