import sys
import paho.mqtt.client as mqtt
from sense_hat import SenseHat

# Initialize Sense HAT
sense = SenseHat()

# Initialize MQTT broker details
hostname = sys.argv[1]
port = int(sys.argv[2])
mqtt_topic = "base/baseA/signal"

# Define the threshold
threshold = 0.5

# Function to check gyroscope data and publish message if any axis value exceeds the threshold
def check_gyroscope_and_publish():
    # Get gyroscope data
    gyro_data = sense.get_gyroscope_raw()
    
    # Check if any axis value is greater than the threshold
    if abs(gyro_data['x']) > threshold or abs(gyro_data['y']) > threshold or abs(gyro_data['z']) > threshold:
    # Publish message to MQTT broker
        print("signal sent")
        client.publish(mqtt_topic, "true")
    

# MQTT event callbacks
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code " + str(rc))
    # Subscribe to the MQTT topic
    client.subscribe(mqtt_topic)


def on_message(client, userdata, msg):
    print("Received message: " + msg.payload.decode())

# Initialize MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT broker
client.connect(hostname, port, 60)

# Start MQTT loop in a non-blocking way
client.loop_start()

# Main loop to continuously check gyroscope data
while True:
    check_gyroscope_and_publish()
