import QtQuick 2.13
import QtQuick.Controls 2.13

import "components" as Components

ApplicationWindow
{
    id: root

    visible: true

    width: 640
    height: 320

    color: "#646464"

    menuBar: MenuBar
    {
        Menu
        {
            title: qsTr("File")

            MenuItem
            {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
    }

    ListView
    {
        id: clock_view

        cacheBuffer: 2000
        anchors.fill: parent

        highlightRangeMode: ListView.ApplyRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem

        delegate: Components.Clock
        {
            city: city_name
            shift: time_shift
        }

        model: ListModel
        {
            ListElement
            {
                city_name: "Boston"
                time_shift: -5
            }
            ListElement
            {
                city_name: "London"
                time_shift: 0
            }
            ListElement
            {
                city_name: "Oslo"
                time_shift: 1
            }
        }
    }
}
