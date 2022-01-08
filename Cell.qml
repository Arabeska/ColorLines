import QtQuick 2.12
import QtGraphicalEffects 1.0

Item {
    id     : container
    width  : container.widthCell
    height : width
    visible: true

    property real widthCell   : 40
    property real widthCircle : 30
    property color colorCircle: "lightpink"
    property color colorBorder: "brown"
    property bool isVisible   : true
    property int  animDuration: 500

    signal clicked(var mouse);

    Rectangle {
        id          : circle
        width       : container.widthCircle
        height      : width
        radius      : width
        color       : container.colorCircle
        border.color: container.colorBorder
        border.width: 1
        opacity     : 1
        anchors.centerIn: parent
    }

    MouseArea {
        anchors.fill: parent
        enabled     : container.enabled
        onClicked   : {
            container.clicked(mouse);
        }
    }

    PropertyAnimation{
        id        : animAppearance
        target    : circle
        properties: "opacity"
        from      : 0
        to        : 1
        duration  : container.animDuration
        onStarted : {
            circle.visible = true;
            circle.opacity = 0;
        }
        onStopped: {
            circle.visible = true;
            circle.opacity = 1;
        }
    }

    PropertyAnimation{
        id        : animFade
        target    : circle
        properties: "opacity"
        from      : 1
        to        : 0
        duration  : container.animDuration
        onStarted : {
            circle.opacity = 0;
            circle.visible = true;
        }
        onStopped: {
            circle.opacity = 1;
            circle.visible = false;
        }
    }

    states: [
        State {
            name: "isVisible"
            when: container.isVisible
            PropertyChanges {
                target      : circle
                border.color: "brown"
            }
        },
        State {
            name: "isNotVisible"
            when: !container.isVisible
            PropertyChanges {
                target      : circle
                border.color: "brown"
            }
        }
    ]

    onStateChanged: {
        if (state == "isVisible") {
            animFade.stop();
            animAppearance.start();
        }
        else {
            animAppearance.stop();
            animFade.start();
        }
    }

    Component.onCompleted: {
        if (container.colorCircle == "#00000000")
            circle.border.color = "#00000000";
    }
}
