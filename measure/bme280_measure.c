/**
 * @file    bme280_measure.c
 * @brief   Functions and data related to the BME280 driver implementation
 *
 * @author  Ritika Ramchandani
 * @date    2023-04-12
 * 
 * @ref    BME280 GitHub repo by Bosch Sensortec GmbH
 *         BME280 Datasheet 
 *
 */

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
#define ADDRESS             "tcp://broker.hivemq.com:1883"
#define CLIENT_ID           "AESD Subscriber"
#define TOPIC               "Measure"
#define MESSAGE             "PBPressed"
#define QOS                 (1)

int bme280_dev_fd; // File descriptor for bme280

/**
 * @brief   Callback function when message is received by the MQTT Client
 * 
 * @param context A pointer to the <i>context</i> value originally passed to
 * MQTTClient_setCallbacks(), which contains any application-specific context.
 * 
 * @param topicName The topic associated with the received message.
 * 
 * @param topicLen The length of the topic if there are one
 * more NULL characters embedded in <i>topicName</i>, otherwise <i>topicLen</i>
 * is 0. If <i>topicLen</i> is 0, the value returned by <i>strlen(topicName)</i>
 * can be trusted. If <i>topicLen</i> is greater than 0, the full topic name
 * can be retrieved by accessing <i>topicName</i> as a byte array of length
 * <i>topicLen</i>.
 * 
 * @param message The MQTTClient_message structure for the received message.
 * This structure contains the message payload and attributes.
 * 
 * @return This function must return 0 or 1 indicating whether or not
 * the message has been safely received by the client application.
 *
*/
int checkMessage(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char measurements[MEASUREMENT_LEN];
    uint8_t num_bytes_read = 0;
    long signed int temperature;
    long unsigned int pressure;

    printf("Message arrived\n");
    printf("Topic: %s\n", topicName);
    printf("Message: %.*s\n", message->payloadlen, (char*)message->payload);

    if(!(memcmp((char*)message -> payload, MESSAGE, sizeof(MESSAGE))))
    {
        // Read temperature from BME280 sensor
        num_bytes_read = read(bme280_dev_fd, measurements, MEASUREMENT_LEN);
        if (num_bytes_read < 0) 
        {
            perror("Failed to read temperature from BME280 sensor");
            return -1;
        }

        // Extract the values from the user buffer
        sscanf(measurements, "%ld %lu", &temperature, &pressure);

        // Print temperature
        printf("Temperature: %ld.%ldC\n", temperature/100, temperature % 100);

        // Print Pressure
        pressure = pressure / 100;
        printf("Pressure: %lu.%luhPa\n", pressure/256, pressure % 256);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}


/**
 * @brief Callback when connection to the MQTT broker is lost
 * 
 * @param context A pointer to the <i>context</i> value originally passed to
 * MQTTClient_setCallbacks(), which contains any application-specific context.
 * 
 * @param cause The reason for the disconnection.
 * Currently, <i>cause</i> is always set to NULL.
 */
void connectionLost(void *context, char *cause)
{
    printf("Connection lost due to %s\n", cause);
}


/**
 * @brief main application point
*/
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
