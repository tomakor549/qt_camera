#include "cameramanager.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
int main(int argc, char *argv[]) {
  // qputenv("QT_FFMPEG_ENCODING_HW_DEVICE_TYPES", "");
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  qmlRegisterType<CameraManager>("MyModule", 1, 0, "CameraManager");

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("QtCamera", "Main");

  return app.exec();
}