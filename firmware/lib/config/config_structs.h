#ifndef CONFIG_STRUCTS_H_
#define CONFIG_STRUCTS_H_
/*
Data Sructs will be stored here. These hold configuration styles.
*/
#include <config.h>

namespace config
{
	struct Ultrasonic
	{
		int echo;
		int trigger;
	};

	struct Stepper
	{
		short int direction;
		short int step;
	};

	struct IR
	{
		short int pin;
	};

	struct MqttTopic
	{
		String topic;
		bool is_subscriber;
	};
}
#endif