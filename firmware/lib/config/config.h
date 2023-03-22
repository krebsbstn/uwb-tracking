#ifndef CONFIG_H_
#define CONFIG_H_
/*
This Space is for defining important Variables like GPIO Pins.
*/
#include <array>
#include <Arduino.h>
#include <config_structs.h>

#define POSITION_THRESHOLD 			50
#define ROTATION_RESOLUTION			5
#define METER_TO_STEPS				1000

#define TASK_PRIORITY_DRIVER			3	//has To be the Highest because of Time critical calculations
#define TASK_PRIORITY_POSHANDLER		1
#define TASK_PRIORITY_MQTT				1
#define TASK_PRIORITY_COLLISION			4

#define TASK_CORE_DRIVER				0
#define TASK_CORE_POSHANDLER			0
#define TASK_CORE_MQTT					1
#define TASK_CORE_COLLISION				1
//Ultrasonic Sensors
#define MAX_ULTRASONIC_DISTANCE 		200
#define ULTRASONIC_SENSOR_COUNT 		1
#define ULTRASONIC_MEASSUREMENT_COUNT	3
#define ULTRASONIC_COLLISION_DISTANCE	15
//IR Sensors
#define IR_SENSOR_COUNT 				1
//Orientation
#define ORIENTATION_THRESHOLD 			5
#define ORIENTATION_I2C_ADDRESS			0x28
//Stepper Motors
#define MOTOR_COUNT 					2
#define STEPPERMOTOR_MAXSPEED 			250
#define STEPPERMOTOR_ACCELERATION 		200
#define STEPPERMOTOR_TURNSPEED			150	
//Brushless Motor
#define MIN_SPEED_BRUSHLESS				1200
#define MAX_SPEED_BRUSHLESS				1600
#define BRUSHLESS_DATEN_PIN 			4
//MQTT
#define MQTT_TOPIC_COUNT 				3
#define CURRENTPOS_TOPIC 	"currentPos"
#define REQUESTPOS_TOPIC 	"requestPos"
#define INPUTPOS_TOPIC 		"inputPos"

namespace config
{
	const std::array<Ultrasonic,ULTRASONIC_SENSOR_COUNT> UltrasonicPins
	{
		Ultrasonic{echo: 33,trigger:  32}
	};

	const std::array<Stepper,MOTOR_COUNT> MotorPins
	{
		Stepper{direction: 15,step:  2},
		Stepper{direction: 18,step:  5}
	};

	const std::array<IR,IR_SENSOR_COUNT> IrPins
	{
		IR{pin: 26}
	};

	// MQTT Settings
	const char wifi_ssid[] = "o2-WLAN91";
	const char wifi_pwd[] = "4KH9E3UD7896488M";
	const char mqtt_server[] = "192.168.1.176";
	const unsigned int mqtt_port = 1883;

	const std::array<MqttTopic,MQTT_TOPIC_COUNT> MqttTopics
	{
		MqttTopic{topic: CURRENTPOS_TOPIC, is_subscriber: false},
		MqttTopic{topic: INPUTPOS_TOPIC, is_subscriber: true},
		MqttTopic{topic: REQUESTPOS_TOPIC, is_subscriber: false}
	};
}
#endif