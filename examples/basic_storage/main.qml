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
    title: qsTr("Basic example for Computeshader Storage-Buffers")

    //property int dataCount: 10 * 256;

    function initBuffer() {
        const dataSize = 8 * computeItem.dataCount; // 8 entries per point: x pos, y pos, depth, unused, rgba color
        const bufferSize = 4 * dataSize; // four bytes per entry

        let buffer = new ArrayBuffer(bufferSize);
        let view = new Float32Array(buffer);
        let i = 0;
        let coordXY = 0;
        while(i < dataSize) {
            view[i + 0] = Math.random() * 2.0 - 1.0; // x
            view[i + 1] = Math.random() * 2.0 - 1.0; // y
            view[i + 2] = Math.random(); // depth
            view[i + 3] = 0.0; // unused
            // we just add a black particle here, the real particle color is set in the compute shader
            view[i + 4] = 0.0; // r
            view[i + 5] = 0.0; // g
            view[i + 6] = 0.0; // b
            view[i + 7] = 1.0; // a

            i += 8;
        }
        storageBuffer.buffer = buffer;
    }

    ComputeItem {
        id: computeItem

        property real speed: 0.005
        property int dataCount: 10 * 256

        computeShader: ":/shaders/computeshader.comp.qsb"
        dispatchX: computeItem.dataCount / 256

        buffers: [
            StorageBuffer {
                id: storageBuffer
                
                Component.onCompleted: initBuffer()
            }
        ]

        Component.onCompleted: computeItem.compute()
        
    }

    StorageBufferView {
        id: view
        focus: true
        anchors.fill: parent
        computeItem: computeItem
        resultBuffer: storageBuffer
        numberOfPoints: computeItem.dataCount
        pointSize: 2.0
        
        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Plus || event.key === Qt.Key_Right) {
                computeItem.speed = Math.min(computeItem.speed + 0.001, 0.03);
            } else if (event.key === Qt.Key_Minus || event.key === Qt.Key_Left) {
                computeItem.speed = Math.max(computeItem.speed - 0.001, 0.0011);
            }
        }
    }

}
    
