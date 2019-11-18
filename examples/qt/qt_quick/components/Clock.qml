import QtQuick 2.13

Item
{
    id: clock

    width: ListView.view.width / 3
    height: ListView.view.height

    property alias city: city_label.text
    property real shift

    property int hours
    property int minutes
    property int seconds

    function timeChanged()
    {
        var date = new Date();

        hours = date.getUTCHours() + Math.floor(clock.shift);
        minutes = date.getUTCMinutes();
        seconds = date.getUTCSeconds();
    }

    Timer
    {
        interval: 100
        running: true
        repeat: true
        onTriggered: clock.timeChanged()
    }

    Item
    {
        width: 200
        height: 240

        anchors.centerIn: parent

        Image
        {
            id: background

            source:
            {
                if ((hours > 6) && (hours < 20))
                {
                    return "/images/clock_day.png";
                }
                else
                {
                    return "/images/clock_night.png";
                }
            }
        }

        Hand
        {
            source: "/images/hour.png"

            x: 92.5
            y: 27

            rotation_origin_x: 7.5
            rotation_origin_y: 73
            rotation_angle: (clock.hours * 30) + (clock.minutes * 0.5)
        }

        Hand
        {
            source: "/images/minute.png"

            x: 93.5
            y: 17

            rotation_origin_x: 6.5
            rotation_origin_y: 83
            rotation_angle: clock.minutes * 6
        }

        Hand
        {
            source: "/images/second.png"

            x: 97.5
            y: 20

            rotation_origin_x: 2.5
            rotation_origin_y: 80
            rotation_angle: clock.seconds * 6
        }

        Image
        {
            source: "/images/center.png"

            anchors.centerIn: background
        }

        Text
        {
            id: city_label

            y: 210

            anchors.horizontalCenter: parent.horizontalCenter

            color: "white"
            style: Text.Raised
            styleColor: "black"

            font.family: "Helvetica"
            font.bold: true
            font.pixelSize: 16
        }
    }
}
