// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define VULKAN

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuick/QQuickView>
#include <QSurfaceFormat>

#include <QLoggingCategory>

int main(int argc, char **argv)
{
    QSurfaceFormat f;
    f.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(f);

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Metal);
#elif defined(VULKAN)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);
#endif

    QGuiApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QLatin1String("qt.rhi.*=true"));

    QQmlApplicationEngine engine("qrc:/main.qml");
    return app.exec();
}