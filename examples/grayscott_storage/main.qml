// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick
import QtQuick.Window

import QtQuickComputeItem

Window {
    id: window

    width: 1024
    height: 1024
    visible: true
    color: "black"
    title: qsTr("Gray Scott reaction/diffusion (storage buffers)")

    property int spread: 2
    property int dataCountX: window.width / spread
    property int dataCountY: window.height / spread

    property int dataCount: window.dataCountX * window.dataCountY;

    function randomInt(max) {
        return Math.floor(Math.random() * max);
    }

    function inCenterPoint(x, y, left, right, top, bottom) {
        return (x >= left && x <= right && y >= top && y <= bottom);
    }

    function toSceneX(xCoord, stretch) {
        return (xCoord / window.width) * 2.0 * stretch - 1.0;
    }

    function toSceneY(yCoord, stretch) {
        return (yCoord / window.height) * 2.0 * stretch - 1.0;
    }

    ComputeItem {
        id: computeItem
        computeShader: ":/shaders/computeshader.comp.qsb"

        dispatchX: window.width * 2

        property real du: 0.190
        property real dv: 0.050
        property real feed: 0.060
        property real kill: 0.062

        property int dataCount: window.dataCount
        property int height: window.dataCountY
        property int timestep: 0

        property real mouseX: -5.0
        property real mouseY: -5.0

       /*NumberAnimation on stepSize {
            from: 0.2
            to: 0.8
            loops: Animation.Infinite
            duration: 3000
        }*/

        buffers: [
            StorageBuffer {
                id: storageBufferA
                
                Component.onCompleted: {
                    const dataSize = 8 * window.dataCount; // 8 entries per point: x pos, y pos, u material, v material, rgba color
                    const bufferSize = 4 * dataSize; // four bytes per entry

                    const centerPointSize = 10.0;
                    const left = (window.dataCountX - centerPointSize) / 2.0;
                    const right = (window.dataCountX + centerPointSize) / 2.0;
                    const top = (window.dataCountY - centerPointSize) / 2.0;
                    const bottom = (window.dataCountY + centerPointSize) / 2.0;

                    let buffer = new ArrayBuffer(bufferSize);
                    let view = new Float32Array(buffer);
                    let i = 0;
                    let coordXY = 0;
                    while(i < dataSize) {

                        let xCoord = coordXY % window.dataCountX;
                        let yCoord = Math.floor(coordXY / window.dataCountX);

                        view[i]     = window.toSceneX(xCoord, window.spread);
                        view[i + 1] = window.toSceneY(yCoord, window.spread);

                        if (window.inCenterPoint(xCoord, yCoord, left, right, top, bottom)) {
                            view[i + 2] = 1.0; // u material
                            view[i + 3] = 1.0; // v material
                        } else {
                            view[i + 2] = 1.0; // u material
                            view[i + 3] = 0.0; // v material
                        }


                        view[i + 4] = 0.0; // r
                        view[i + 5] = 0.0; // g
                        view[i + 6] = 0.0; // b
                        view[i + 7] = 1.0; // a

                        i += 8;
                        coordXY++;
                    }
                    storageBufferA.buffer = buffer;
                }
            },
            StorageBuffer {
                id: storageBufferB
                Component.onCompleted: {
                    const size = 8 * 4 * window.dataCount; // same as StorageBufferA
                    let buffer = new ArrayBuffer(size);
                    storageBufferB.buffer = buffer;
                }
            }
        ]

        Component.onCompleted: computeItem.compute()
        
    }

    Timer {
        id: timer
        running: true
        repeat: true
        interval: 1
        onTriggered: computeItem.timestep += 1
    }

    StorageBufferView {
        id: view
        anchors.fill: parent
        computeItem: computeItem
        resultBuffer: storageBufferA
        numberOfPoints: window.dataCount
        pointSize: 2.0

        /*NumberAnimation on pointSize {
            from: 1.0
            to: 8.0
            loops: Animation.Infinite
            duration: 3000
        }

        NumberAnimation on numberOfPoints {
            from: 0
            to: window.dataCount
            loops: Animation.Infinite
            duration: 3000
        }*/
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: (mouse) => {
           if (ma.pressed) {
               computeItem.mouseX = window.toSceneX(mouse.x, 1.0);
               computeItem.mouseY = window.toSceneY(mouse.y, 1.0);
               mouse.accepted = false;
           }
        }
        onReleased: (mouse) => {
           computeItem.mouseX = -5.0;
           computeItem.mouseY = -5.0; 
        }
    }

}
    
