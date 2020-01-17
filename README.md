# ZumoRobot project with C

#### Only contains files that were made by the group
* Line following `Main.c Line Follow Group 2.c`
* "Sumo fighting" `Main.c Zumo Fight Group 2.c`

### Line following principle
The robot follows a curvy line as fast as possible and sends the time it took via MQTT connection

![6](https://user-images.githubusercontent.com/45162563/72589989-69bf8400-3905-11ea-954e-80ab471352a7.png)

1. Waits at the starting point for a IR-command
2. When IR-command is given, the robot starts the track
3. Stops at the end point and sends the time
#### Example MQTT print
`Zumo039/ready line`

`Zumo039/start 1210`

`Zumo039/stop 54290`

`Zumo039/time 53080`

### Zumo fighting principle
Four robots are placed on the straight lines and start to fight when a IR-command is given. Last one that stays inside the ring wins. When the robots collide, timestamp is send via MQTT

![7](https://user-images.githubusercontent.com/45162563/72589990-69bf8400-3905-11ea-95aa-5fa49986ec86.png)
#### Example MQTT print
`Zumo039/ready zumo`

`Zumo039/start 2301`

`Zumo039/hit 3570`

`Zumo039/hit 4553`

`Zumo039/hit 4988`

`Zumo039/stop 5210`

`Zumo039/time 2909`

