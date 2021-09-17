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

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

InspectorPropertyView {
    id: root

    titleText: qsTrc("inspector", "Position")

    RadioButtonGroup {
        id: positionButtonList

        height: 30
        width: parent.width

        model: [
            { textRole: qsTrc("inspector", "Above"), valueRole: CommonTypes.PLACEMENT_TYPE_ABOVE },
            { textRole: qsTrc("inspector", "Below"), valueRole: CommonTypes.PLACEMENT_TYPE_BELOW }
        ]

        delegate: FlatRadioButton {
            ButtonGroup.group: positionButtonList.radioButtonGroup

            text: modelData["textRole"]
            checked: root.propertyItem && !root.propertyItem.isUndefined ? root.propertyItem.value === modelData["valueRole"]
                                                                         : false
            onToggled: {
                root.propertyItem.value = modelData["valueRole"]
            }
        }
    }
}
