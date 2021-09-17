/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../../../common"

Column {
    id: root

    property QtObject endHookType: null
    property QtObject thickness: null
    property QtObject hookHeight: null

    property alias possibleEndHookTypes: lineTypeButtonList.model

    width: parent.width

    spacing: 12

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Line type")
        propertyItem: root.endHookType

        RadioButtonGroup {
            id: lineTypeButtonList

            height: 30
            width: parent.width

            delegate: FlatRadioButton {
                ButtonGroup.group: lineTypeButtonList.radioButtonGroup

                iconCode: modelData["iconRole"]
                checked: root.endHookType && !root.endHookType.isUndefined ? root.endHookType.value === modelData["typeRole"]
                                                                           : false
                onToggled: {
                    root.endHookType.value = modelData["typeRole"]
                }
            }
        }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        InspectorPropertyView {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.thickness

            IncrementalPropertyControl {
                isIndeterminate: root.thickness ? root.thickness.isUndefined : false
                currentValue: root.thickness ? root.thickness.value : 0

                onValueEdited: {
                    root.thickness.value = newValue
                }
            }
        }

        InspectorPropertyView {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Hook height")
            propertyItem: root.hookHeight

            IncrementalPropertyControl {
                enabled: root.hookHeight ? root.hookHeight.isEnabled : false
                isIndeterminate: root.hookHeight && enabled ? root.hookHeight.isUndefined : false
                currentValue: root.hookHeight ? root.hookHeight.value : 0

                onValueEdited: {
                    root.hookHeight.value = newValue
                }
            }
        }
    }
}
