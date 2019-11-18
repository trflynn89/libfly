import QtQuick 2.13

Image
{
    id: hand

    property alias source: hand.source

    property alias rotation_origin_x: hand_rotation.origin.x
    property alias rotation_origin_y: hand_rotation.origin.y
    property alias rotation_angle: hand_rotation.angle

    transform: Rotation
    {
        id: hand_rotation

        Behavior on angle
        {
            SpringAnimation
            {
                spring: 2
                damping: 0.2
                modulus: 360
            }
        }
    }
}
