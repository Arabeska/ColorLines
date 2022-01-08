import QtQuick 2.6
import QtQuick.Window 2.15
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4

Window {
    width  : 1000
    height : 800
    visible: true
    title  : qsTr("Color Lines")

    QtObject {
        id : d
        property int mainMargin : 50
        property int gridSize   : 50

        property bool firstMoveIsComplete : false
        property bool secondMoveIsComplete: false

        property real xFirstMove: 0
        property real yFirstMove: 0
    }

    Rectangle {
        id          : backgroud
        anchors.fill: parent
        color       : "#ffead2"
        Rectangle {
            id          : titleRect
            height      : 150
            width       : 640
            radius      : 20
            border.color: "brown"
            anchors {
                top             : parent.top
                topMargin       : 50
                horizontalCenter: parent.horizontalCenter
            }
            color : "white"

            RoundButton {
                id : newGameButton
                anchors {
                    verticalCenter: parent.verticalCenter
                    left          : parent.left
                    leftMargin    : 20
                }
                width  : 150
                height : 70
                radius : 20

                visible        : true
                font.pixelSize : 20
                text           : "NEW GAME"
                onClicked: {
                    BoardLink.newGame();
                }
            }

            Rectangle {
                id    : finalRect
                width : 210
                height: 120
                color : "white"
                radius: 20

                border {
                    color: "grey"
                    width: 2
                }
                anchors.centerIn: parent
                Text {
                    id              : finalText
                    anchors.centerIn: parent
                    font.pixelSize  : 18
                    text            : qsTr("Game OVER!\nYou scored a "+
                                            BoardLink.currentScore+
                                            " points!\nClick New game")
                }
            }

            Row {
                id            : textScore
                width         : textScore.childrenRect.width
                anchors {
                    verticalCenter: parent.verticalCenter
                    right         : parent.right
                    rightMargin   : 20
                }

                Text {
                    id            : textScoreLine
                    width         : contentWidth
                    text          : "Score: "
                    color         : "black"
                    font.pixelSize: 45
                }

                Text {
                    id            : textScoreInfo
                    width         : contentWidth
                    text          : BoardLink.currentScore
                    color         : "black"
                    font.pixelSize: 45

                    onTextChanged: {
                        seqAnim.start();
                    }

                    SequentialAnimation {
                        id: seqAnim
                        property real yText : textScore.y
                        NumberAnimation {
                            target    : textScoreInfo
                            properties: "opacity"
                            from      : 1
                            to        : 0.3
                            duration  : 200
                        }

                        NumberAnimation {
                            target    : textScoreInfo
                            properties: "opacity"
                            from      : 0.3
                            to        : 1
                            duration  : 1000
                        }
                    }
                }
            }
        }

        GridView {
            id         : gameBoardDesk
            visible    : true
            width      : 450
            height     : 450
            model      : BoardLink
            interactive: false
            anchors {
                top             : titleRect.bottom
                topMargin       : 50
                horizontalCenter: parent.horizontalCenter
            }

            cellWidth          : d.gridSize
            cellHeight         : d.gridSize

            delegate  : Rectangle {
                width : d.gridSize
                height: width
                radius: 7
                color : "white"
                border.color: "brown"
            }
        }

        GridView {
            id     : gameBoard
            visible: true
            width  : 450
            height : 450
            model  : BoardLink
            anchors {
                top : titleRect.bottom
                topMargin: 50
                horizontalCenter: parent.horizontalCenter
            }
            interactive: false

            cellWidth          : d.gridSize
            cellHeight         : d.gridSize
            layoutDirection    : Qt.LeftToRight

            delegate: Cell {
                id               : circle
                width            : d.gridSize
                height           : width
                isVisible        : cellIsBusy
                colorCircle      : cellColor

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        gameBoard.currentIndex = index;
                        if (d.firstMoveIsComplete) {
                            d.secondMoveIsComplete = BoardLink.tryToMakeASecondMove(index);
                            if (d.secondMoveIsComplete) {
                                gameBoard.makeSecondMove();
                            }
                            else {
                                d.firstMoveIsComplete = false;
                            }
                        }
                        else {
                            d.firstMoveIsComplete = BoardLink.tryToMakeAFirstMove(index);
                            if (d.firstMoveIsComplete) {
                                gameBoard.makeFirstMove();
                            }
                        }
                    }
                }
            }

            ParallelAnimation {
                id : animMove

                NumberAnimation {
                    id        : animMoveX
                    target    : gameBoard.currentItem
                    properties: "x"
                    duration  : 1000
                }

                NumberAnimation {
                    id        : animMoveY
                    target    : gameBoard.currentItem
                    properties: "y"
                    duration  : 1000
                }
                onStopped : {
                    BoardLink.endASecondMove(gameBoard.currentIndex);
                }
            }

            function makeFirstMove() {
                d.xFirstMove = gameBoard.currentItem.x;
                d.yFirstMove = gameBoard.currentItem.y;
            }

            function makeSecondMove() {
                animMoveX.from = d.xFirstMove;
                animMoveX.to   = gameBoard.currentItem.x;
                animMoveY.from = d.yFirstMove;
                animMoveY.to   = gameBoard.currentItem.y;

                animMove.start();

                d.firstMoveIsComplete = false;
                d.secondMoveIsComplete = false;
            }
        }

        states: [
            State {
                name: "GameOver"
                when: BoardLink.isFinal
                PropertyChanges {
                    target  : finalRect
                    visible : true
                }
            },
            State {
                name: "GameContinues"
                when: !BoardLink.isFinal
                PropertyChanges {
                    target  : finalRect
                    visible : false
                }
            }
        ]
    }
}
