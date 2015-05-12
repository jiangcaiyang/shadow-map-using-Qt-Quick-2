#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>
#include "Cube.h"
#include "Plane.h"
#include "TexturedCube.h"
#include "View.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<View>( "QtProblem", 1, 0, "TexturedCubeView" );
    qmlRegisterType<TexturedCube>( "QtProblem", 1, 0, "TexturedCube" );
    qmlRegisterType<Cube>( "QtProblem", 1, 0, "Cube" );
    qmlRegisterType<Plane>( "QtProblem", 1, 0, "Plane" );

    // 注册一些类
    QSurfaceFormat defaultFormat;
    defaultFormat.setSamples( 4 );
    QSurfaceFormat::setDefaultFormat( defaultFormat );

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec( );
}
