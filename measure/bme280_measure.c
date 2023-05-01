#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#define BME280_DEV          "/dev/bme280"
#define MEASUREMENT_LEN     (25)
#define ADDRESS             "tcp://10.0.0.46:1883"
#define CLIENT_ID           "AESD Subscriber"
#define TOPIC               "Measure"
#define MESSAGE             "PBPressed"
#define QOS                 (1)

int bme280_dev_fd; // File descriptor for bme280

int checkMessage(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char measurements[MEASUREMENT_LEN];
    uint8_t num_bytes_read = 0;
    long signed int temperature;
    long unsigned int pressure;

    printf("Message arrived\n");
    printf("Topic: %s\n", topicName);
    printf("Message: %.*s\n", message->payloadlen, (char*)message->payload);

    if(!(memcmp(topicName, TOPIC, sizeof(TOPIC))))
    {
        // Read temperature from BME280 sensor
        num_bytes_read = read(bme280_dev_fd, measurements, MEASUREMENT_LEN);
        if (num_bytes_read < 0) 
        {
            perror("Failed to read temperature from BME280 sensor");
            return -1;
        }

        sscanf(measurements, "%ld %lu", &temperature, &pressure);

        // Print temperature
        printf("Temperature: %ld.%ldC\n", temperature/100, temperature % 100);

        // Print Pressure
        pressure = pressure / 100;
        printf("Pressure: %lu.%luhPa\n", pressure/256, pressure % 256);
    }

    return 0;
}


void connectionLost(void *context, char *cause)
{
    printf("Connection lost due to %s\n", cause);
}


int main() 
{
    int retval = 0;
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    // Open I2C device file
    bme280_dev_fd = open(BME280_DEV, O_CREAT | O_RDWR, 0744);

    if (bme280_dev_fd < 0) 
    {
        perror("Failed to open I2C device file");
        return EXIT_FAILURE;
    }

    // Create the MQTT Client
    retval = MQTTClient_create(&client, ADDRESS, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if(retval != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", retval);
        retval = EXIT_FAILURE;
        goto close_and_exit;
    }

    // Set callbacks for when messages arrive and/or connection is lost
    retval = MQTTClient_setCallbacks(client, NULL, connectionLost, checkMessage, NULL);
    if(retval != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", retval);
        retval = EXIT_FAILURE;
        goto destroy_exit;
    }

    // Connect to the MQTT broker
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    retval = MQTTClient_connect(client, &conn_opts);
    if (retval != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", retval);
        retval = EXIT_FAILURE;
        goto destroy_exit;
    }

    // Subscribe to a topic
    printf("Subscribing to topic %s for client %s\n", TOPIC, CLIENT_ID);
    printf("Enter Q to quit\n");
    retval = MQTTClient_subscribe(client, TOPIC, QOS);
    if (retval != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", retval);
    	retval = EXIT_FAILURE;
    }
    else
    {
    	int ch;
    	do
    	{
        	ch = getchar();
    	} while (ch!='Q' && ch != 'q');

        retval = MQTTClient_unsubscribe(client, TOPIC);
        if (retval != MQTTCLIENT_SUCCESS)
        {
        	printf("Failed to unsubscribe, return code %d\n", retval);
        	retval = EXIT_FAILURE;
        }
    }

    // Disconnect from the broker
    retval = MQTTClient_disconnect(client, 10000);

    if(retval != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to disconnect, return code %d\n", retval);
    	retval = EXIT_FAILURE;
    }


destroy_exit: 
    // Destroy the client
    MQTTClient_destroy(&client);

close_and_exit:
    // Close I2C device file
    close(bme280_dev_fd);

    return retval;
}
