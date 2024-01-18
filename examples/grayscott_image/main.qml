// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import QtQuickComputeItem

Window {
    id: window

    property int controlPanelWidth: 300
    property int contentWidth: 1024
    property real stretch: 1.0 / 2.0
    property size simulationSize: Qt.size(window.contentWidth * window.stretch, window.height * window.stretch)

    width: window.contentWidth + window.controlPanelWidth
    height: 1024
    visible: true
    color: "black"
    title: qsTr("Gray Scott reaction/diffusion (image input/output)")

    function restart() {

        computeItem.stop();
        timer.running = false;
        computeItem.timestep = 0;

        const pixels = simulationSize.width * simulationSize.height;
        const bufferSize = 16 * pixels; // 16 bytes per pixel (rgba, one 4 byte float per channel)

        const centerPointSize = 10.0;
        const left = (simulationSize.width - centerPointSize) / 2.0;
        const right = (simulationSize.width + centerPointSize) / 2.0;
        const top = (simulationSize.height - centerPointSize) / 2.0;
        const bottom = (simulationSize.height + centerPointSize) / 2.0;

        let buffer = new ArrayBuffer(bufferSize);
        let view = new Float32Array(buffer);
        let i = 0;
        let coordXY = 0;
        while(i < bufferSize) {

            let xCoord = coordXY % simulationSize.width;
            let yCoord = Math.floor(coordXY / simulationSize.width);

            if (window.inCenterPoint(xCoord, yCoord, left, right, top, bottom)) {
                view[i + 0] = 1.0; // u material
                view[i + 1] = 1.0; // v material
            } else {
                view[i + 0] = 1.0; // u material
                view[i + 1] = 0.0; // v material
            }


            view[i + 2] = 0.0; // unused
            view[i + 3] = 1.0; // unused

            i += 4;
            coordXY++;
        }
        imageBufferA.buffer = buffer;

        // buffer b
        const bufferBSize = 16 * simulationSize.width * simulationSize.height; // same as StorageBufferA
        let bufferB = new ArrayBuffer(bufferBSize);
        imageBufferB.buffer = bufferB;

        // result buffer
        const resultBufferSize = 4 * simulationSize.width * simulationSize.height; // 4 bytes per pixel
        let resultBuffer = new ArrayBuffer(resultBufferSize);
        result.buffer = resultBuffer;

        computeItem.compute();
        timer.running = true;
    }

    function randomInt(max) {
        return Math.floor(Math.random() * max);
    }

    function inCenterPoint(x, y, left, right, top, bottom) {
        return (x >= left && x <= right && y >= top && y <= bottom);
    }

    ComputeItem {
        id: computeItem
        computeShader: ":/shaders/computeshader.comp.qsb"

        dispatchX: simulationSize.width / 16
        dispatchY: simulationSize.height / 16

        property real du: duRate.value
        property real dv: dvRate.value
        property real feed: feedRate.value
        property real kill: killRate.value

        property bool usePalette: paletteCheckBox.checked

        property int timestep: 0

        property real mouseX: -5.0
        property real mouseY: -5.0


        buffers: [
            ImageBuffer {
                id: imageBufferA
                imageSize: simulationSize
                textureFormat: ImageBuffer.RGBA32F
            },
            ImageBuffer {
                id: imageBufferB
                imageSize: simulationSize
                textureFormat: ImageBuffer.RGBA32F
            },
            ImageBuffer {
                id: result
                imageSize: simulationSize
                textureFormat: ImageBuffer.RGBA8
            }
        ]

        Component.onCompleted: window.restart();
        
    }

    Timer {
        id: timer
        running: true
        repeat: true
        interval: 15
        onTriggered: computeItem.timestep += 1
    }

    Item {
        id: content
        anchors.fill: parent

        Rectangle {
            id: controlPanelWidth
            width: window.controlPanelWidth

            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }

            color: "#203133"

            Rectangle {
                id: separator
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                }
                width: 2
                color: "black"
                opacity: 0.3
            }

            Column {
                id: column

                anchors {
                    fill: parent
                    leftMargin: 16
                    rightMargin: 16
                    topMargin: 32
                }

                spacing: 16

                Label {
                    width: parent.width
                    wrapMode: Label.Wrap
                    horizontalAlignment: Qt.AlignLeft
                    text: qsTr("Presets:")
                }

                ComboBox {
                    model: PresetModel {
                        id: presets
                    }
                    width: parent.width
                    textRole: "text"
                    currentIndex: 2 // Fingerprint

                    onActivated: (index) => {
                        let preset = presets.get(index);
                        feedRate.value = preset.feed;
                        killRate.value = preset.kill;
                        duRate.value = preset.du;
                        dvRate.value = preset.dv;
                    }
                }

                Item { width: 1;  height:column.spacing }

                Label {
                    width: parent.width
                    wrapMode: Label.Wrap
                    horizontalAlignment: Qt.AlignLeft
                    text: qsTr("Feed Rate: ") + feedRate.value.toFixed(4)
                }

                Item {
                    width: parent.width
                    height: 30

                    Slider {
                        id: feedRate
                        anchors.fill: parent
                        from: 0.01
                        value: 0.060
                        to: 0.1
                        live: true
                    }
                }

                Label {
                    width: parent.width
                    wrapMode: Label.Wrap
                    horizontalAlignment: Qt.AlignLeft
                    text: qsTr("Kill Rate: ") + killRate.value.toFixed(4)
                }

                Item {
                    width: parent.width
                    height: 30

                    Slider {
                        id: killRate
                        anchors.fill: parent
                        from: 0.01
                        value: 0.062
                        to: 0.1
                        live: true
                    }
                }

                Label {
                    width: parent.width
                    wrapMode: Label.Wrap
                    horizontalAlignment: Qt.AlignLeft
                    text: qsTr("Diffusion Rate U: ") + duRate.value.toFixed(4)
                }

                Item {
                    width: parent.width
                    height: 30

                    Slider {
                        id: duRate
                        anchors.fill: parent
                        from: 0.01
                        value: 0.190
                        to: 0.3
                        live: true
                    }
                }

                Label {
                    width: parent.width
                    wrapMode: Label.Wrap
                    horizontalAlignment: Qt.AlignLeft
                    text: qsTr("Diffusion Rate V: ") + dvRate.value.toFixed(4)
                }

                Item {
                    width: parent.width
                    height: 30

                    Slider {
                        id: dvRate
                        anchors.fill: parent
                        from: 0.01
                        value: 0.062
                        to: 0.3
                        live: true
                    }
                }

                CheckBox {
                    id: postprocessingCheckBox
                    checked: false
                    text: qsTr("Postprocessing")

                }

                CheckBox {
                    id: paletteCheckBox
                    checked: true
                    text: qsTr("Use Palette")
                }

                Button {
                    text: qsTr("Reset")
                    onClicked: window.restart()
                }

            }

        }

        
        ImageBufferView {
            id: imageBufferView
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom 
            }
            width: window.contentWidth
            computeItem: computeItem
            resultBuffer: result

            layer.enabled: postprocessingCheckBox.checked
            layer.samplerName: "source"
            layer.effect: ShaderEffect {
                property real contentWidth: imageBufferView.width
                property real contentHeight: imageBufferView.height
                fragmentShader: "shaders/postprocessing.frag.qsb"
            }

        }

        MouseArea {
            id: ma
            anchors.fill: imageBufferView
            hoverEnabled: true
            onPositionChanged: (mouse) => {
                if (ma.pressed) {
                    computeItem.mouseX = mouse.x * window.stretch;
                    computeItem.mouseY = mouse.y * window.stretch;
                    mouse.accepted = false;
                }
            }
            onReleased: (mouse) => {
                computeItem.mouseX = -5.0;
                computeItem.mouseY = -5.0; 
            }
        }

    }


}
    
