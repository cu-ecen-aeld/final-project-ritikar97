#include <mosquitto.h>

int main() 
{
    // Creating a mosquite client struct
    struct mosquitto *mosq;

    // Initializing the moswuitto library
    mosquitto_lib_init();

    // Creating a new client for the struct
    mosq = mosquitto_new(NULL, true, NULL);

    // Connect tot he broker using the default port
    mosquitto_connect(mosq, "10.0.0.176", 1883, 60);

    // Subscriber to topic on the MQTT broker
    mosquitto_subscribe(mosq, NULL, "topic", 0);

    // Stay here until a message is received
    mosquitto_loop_forever(mosq, -1, 1);

    // Cleanup and exit
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
