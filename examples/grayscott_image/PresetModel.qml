// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick


// presets fromhttps://github.com/neozhaoliang/pywonderland/blob/master/src/shader-playground/grayscott.py
ListModel {
    id: presets
    ListElement {
        text: "Unstable" 
        du: 0.210
        dv: 0.105
        feed: 0.018
        kill: 0.051
    }
    ListElement {
        text: "Coral" 
        du: 0.160
        dv: 0.080
        feed: 0.060
        kill: 0.062
    }
    ListElement {
        text: "Fingerprint" 
        du: 0.190
        dv: 0.050
        feed: 0.060
        kill: 0.062
    }
    ListElement {
        text: "Bacteria" 
        du: 0.140
        dv: 0.060
        feed: 0.035
        kill: 0.065
    }
    ListElement {
        text: "Worms" 
        du: 0.160
        dv: 0.080
        feed: 0.050
        kill: 0.065
    }
    ListElement {
        text: "Zebrafish" 
        du: 0.160
        dv: 0.080
        feed: 0.035
        kill: 0.060
    }
    ListElement {
        text: "Net" 
        du: 0.210
        dv: 0.105
        feed: 0.039
        kill: 0.058
    }
    ListElement {
        text: "Worms and Loops" 
        du: 0.210
        dv: 0.105
        feed: 0.082
        kill: 0.060
    }
    ListElement {
        text: "Waves" 
        du: 0.210
        dv: 0.105
        feed: 0.014
        kill: 0.045
    }
    ListElement {
        text: "Moving spots" 
        du: 0.210
        dv: 0.105
        feed: 0.014
        kill: 0.054
    }
    ListElement {
        text: "Pulsating Solitons" 
        du: 0.210
        dv: 0.105
        feed: 0.025
        kill: 0.060
    }

}