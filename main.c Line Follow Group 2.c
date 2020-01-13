/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/


/*
    ---PROJECT---
    Group 2
    Line follow
*/
void zmain(void) {
    TickType_t start,end,lapStart,lapEnd,missTick,lineTick; // TickTypes used for time
    struct sensors_ ref; // Structs used for reflectance sensor data
    struct sensors_ dig;
    int i = 0; // Variable used for number of lines
    long startTime,lapTime,stopTime,missTime,lineTime; // Variables used for time
    bool check = true; // Boolean variable used for line checking
    bool irCheck = true; // Boolean variable used to check if the the IR command is given
    bool stop = true; // Boolean variable used for the track loop
    bool creep = true; // Boolean variable used for going slowly
    bool miss = true; // Boolean variable used for line misses
    uint8 button; // User button
    button = SW1_Read(); // Reads the userbutton
    IR_Start(); // Starts the IR sensor
    motor_start(); // Starts the motor
    start = xTaskGetTickCount(); // Gets the starting time of the code
    
    // Infinite loop while the user button is not pressed
    while(button == 1) {
        button = SW1_Read();
    }
    reflectance_start(); // Starts the reflectance sensors
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    
    // Loop used for the track
    while(stop) {
        // Drives slowly forward when entering the track
        if (creep) {
            motor_forward(100,0);   
        }
        // Stops at the first line and waits for IR command
        if (i == 1) {
            if (irCheck) {
                motor_forward(0,0);
                print_mqtt("Zumo039/ready","line"); //MQTT print
                end = xTaskGetTickCount(); // Gets the ending time of the code
                startTime = end - start; // Start time of the code
                print_mqtt("Zumo039/start","%ld",startTime); // MQTT print
                IR_flush(); // Clears the IR data
                IR_wait(); // Waits for IR command
                motor_forward(200,250); // Drives forward for 0.25 seconds after IR command is given
                creep = false;
                irCheck = false;
                lapStart = xTaskGetTickCount(); // Gets the lap starting time
            }
        }
        reflectance_read(&ref); // read raw sensor values
        printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3); // print out each period of reflectance sensors
        reflectance_digital(&dig); // read digital values that are based on threshold. 0 = white, 1 = black
        printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3); //print out 0 or 1 according to results of reflectance period
        // The robot doesn't work without the printf's
        
        // Goes forward if middle sensors are on black
        if (dig.l1 == 1 && dig.r1 == 1) {
            motor_forward(225,0);   
        }
        // Line miss
        if (dig.l1 == 0 && dig.r1 == 0 && miss) {
            missTick = xTaskGetTickCount();
            missTime = missTick - lapStart;
            print_mqtt("Zumo039/miss","%ld",missTime); // MQTT print
            miss = false;
        }
        // Back on line
        if (dig.l1 == 1 && dig.r1 == 1 && !miss) {
            lineTick = xTaskGetTickCount();
            lineTime = lineTick - lapStart;
            print_mqtt("Zumo039/line","%ld",lineTime); // MQTT print
            miss = true;
        }
        // Intersection checker
        if (check && dig.l3 == 1 && dig.r3 == 1) {
            i++; // Number of lines crossed
            check = false;
        }
        // Turn statements
        if (dig.l2 == 1 && dig.l1 == 1) {
            motor_turn(125,240,10); // Right turn
        } else if (dig.r2 == 1 && dig.r1 == 1) {
            motor_turn(240,125,10); // Left turn
        } else if (dig.l3 == 1 && dig.l2 == 1) {
            motor_turn(0,225,10); // Hard right turn
        } else if (dig.r3 == 1 && dig.r2 == 1) {
            motor_turn(225,0,10); // Hard left turn
        } else if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1) {
            motor_turn(0,240,10); // Extra hard right turn
        } else if (dig.r3 == 1 && dig.r2 == 1 && dig.r1 == 1 && dig.l1 == 1) {
            motor_turn(240,0,10); // Extra hard left turn
        } else if (dig.l3 == 1 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0) {
            superTurn(160,160,10,0); // Extra hard left turn with superTurn, which turns the robot while staying still 0 = left
        } else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 1) {
            superTurn(160,160,10,1); // Extra hard right turn with superTurn, which turns the robot while staying still 1 = right
        }
        // Used for stopping at the first and last line
        if (dig.l3 == 0 && dig.r3 == 0) {
            if (check == false) {
                check = true;
            }
        }
        // Stops at the last line and sends the time over mqtt
        if (i == 3 && !check) {
            lapEnd = xTaskGetTickCount();
            lapTime = lapEnd - lapStart;
            stopTime = lapTime - startTime;
            print_mqtt("Zumo039/stop","%ld",lapTime); // MQTT print
            print_mqtt("Zumo039/time","%ld",stopTime); // MQTT print
            stop = false;
            motor_forward(0,0);
        }
    }  
}

/* [] END OF FILE */
