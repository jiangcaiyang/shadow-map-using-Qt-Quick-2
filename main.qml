import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtProblem 1.0

Window
{
    visible: true
    width: 320
    height: 480

    TexturedCubeView
    {
        objectName: "view"
        anchors.fill: parent

        Cube
        {
            objectName: "biscuit cube"
            source: "image/biscuit.jpg"
            length: 2
            translate: Qt.vector3d( -4, -3.9, 4 )
        }

        Cube
        {
            objectName: "wood cube"
            source: "image/wood.jpg"
            length: 2
            translate: Qt.vector3d( 4, -3.9, 4 )
        }

        Cube
        {
            objectName: "spiral cube"
            source: "image/spiral.jpg"
            length: 2
            translate: Qt.vector3d( 4, -3.9, -4 )
        }

        Cube
        {
            objectName: "shining cube"
            source: "image/shining.jpg"
            length: 2
            translate: Qt.vector3d( -4, -3.9, -4 )
        }

        Cube
        {
            objectName: "BIG cube"
            source: "image/color_line.jpg"
            length: 3
            translate: Qt.vector3d( 0, 0, 0 )
        }

        Plane
        {
            objectName: "plane"
            source: "image/color_line.jpg"
            length: 20
            translate: Qt.vector3d( 0, -5, 0 )
        }

//        TexturedCube
//        {
//            objectName: "textureCube_1"
//            source: "image/stone.jpg"
//            length: 10
//        }

//        TexturedCube
//        {
//            objectName: "textureCube_2"
//            source: "image/fruit.jpg"
//            length: 3

//            translate: Qt.vector3d( 0, 15, 0 )
//        }

        position: Qt.vector3d( 0, 4, -12 )
        lookAt: Qt.vector3d( 0, 0, 0 )
        up: Qt.vector3d( 0, 1, 0 )
        fieldOfView: 90
        aspectRatio: 0.6667
        nearPlane: 1
        farPlane: 100

        // 按照Y轴对光源的位置进行旋转
        property real theta: 0.0
        lightPosition: Qt.vector3d( 5.0 * Math.sin( theta ),
                                    9.0,
                                    5.0 * Math.cos( theta ) )

        NumberAnimation on theta
        {
            from: 0
            to: 2.0 * Math.PI
            duration: 4000
            loops: Animation.Infinite
        }

//        NumberAnimation on angle
//        {
//            from: 0
//            to: 2 * Math.PI
//            duration: 20000
//            loops: Animation.Infinite
//            running: true
//        }
    }

//    Label
//    {
//        anchors.centerIn: parent
//        horizontalAlignment: Text.AlignHCenter
//        text: "This example shows\n my problem on flickering the screen.\n" +
//              "(Solved) Because viewMatrix updated in main GUI thread while\n" +
//              "Used in rendering thread."
//        color: "white"
//    }
}
