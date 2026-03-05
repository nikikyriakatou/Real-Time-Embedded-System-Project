import sys
import paho.mqtt.client as mqtt
import json
from firebase import firebase


firebase_app = firebase.FirebaseApplication("https://tzatziki-5d081-default-rtdb.europe-west1.firebasedatabase.app/", None)


def on_connect(client, userdata, flags, rc):
    print("Connected.")
    client.subscribe("base/baseA/cords")
  

def on_message(client, userdata, msg):
    separator = msg.payload.find(b'++')
    message = msg.payload[:separator].decode('utf-8')
    print(message)
    payload_dict = json.loads(message)
    send_msg(payload_dict)


def send_msg(payload_dict):
    # Send message to Firebase
    response = firebase_app.post('/SpeedMessageRecord', payload_dict)
    # Print Firebase response
    print("Firebase Response:", response)


hostname = sys.argv[1]
port = int(sys.argv[2])



client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(hostname, port, 60)
client.loop_start()  # Start a background thread to handle the MQTT communication

response = firebase_app.post('/Status', "true")

while True:
    pass
