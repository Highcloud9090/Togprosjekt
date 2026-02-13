import paho.mqtt.client as mqtt
import json
import csv
import os
from datetime import datetime

# MQTT credentials
MQTT_BROKER = "10.22.21.95"
MQTT_PORT = 1883
MQTT_USER = "mjm"
MQTT_PASSWORD = "solutions"
MQTT_TOPIC = "esp32/topic"

# CSV file name
CSV_FILE = "sensor_data.csv"

# Create CSV file with headers if it doesn't exist
def initialize_csv():
    if not os.path.exists(CSV_FILE):
        with open(CSV_FILE, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(["ts", "acc x", "acc y", "acc z", "gyro x", "gyro y", "gyro z"])
        print(f"Created CSV file: {CSV_FILE}")
    else:
        print(f"Using existing CSV file: {CSV_FILE}")

# Append data to CSV
def append_to_csv(timestamp, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z):
    with open(CSV_FILE, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([timestamp, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z])

# Callback when connected to MQTT broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker successfully!")
        client.subscribe(MQTT_TOPIC)
        print(f"Subscribed to topic: {MQTT_TOPIC}")
    else:
        print(f"Failed to connect, return code {rc}")

# Callback when a message is received
def on_message(client, userdata, msg):
    try:
        # Decode and parse JSON message
        message = msg.payload.decode()
        print(f"\nReceived message:")
        print(f"Topic: {msg.topic}")
        print(f"Raw: {message}")
        
        # Parse JSON
        data = json.loads(message)
        
        # Extract measurements
        if "målinger" in data and len(data["målinger"]) > 0:
            for måling in data["målinger"]:
                timestamp = måling.get("timestamp", "")
                acc = måling.get("aksel", {})
                gyro = måling.get("gyro", {})
                
                acc_x = acc.get("x", 0)
                acc_y = acc.get("y", 0)
                acc_z = acc.get("z", 0)
                gyro_x = gyro.get("x", 0)
                gyro_y = gyro.get("y", 0)
                gyro_z = gyro.get("z", 0)
                
                # Print extracted data
                print(f"Timestamp: {timestamp}")
                print(f"Accelerometer - X: {acc_x}, Y: {acc_y}, Z: {acc_z}")
                print(f"Gyroscope - X: {gyro_x}, Y: {gyro_y}, Z: {gyro_z}")
                
                # Append to CSV
                append_to_csv(timestamp, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z)
                print(f"✓ Data saved to {CSV_FILE}")
        else:
            print("No 'målinger' data found in message")
        
        print("-------------------")
        
    except json.JSONDecodeError as e:
        print(f"JSON decode error: {e}")
        print("-------------------")
    except Exception as e:
        print(f"Error processing message: {e}")
        print("-------------------")

# Callback when disconnected
def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection. Reconnecting...")

# Main execution
if __name__ == "__main__":
    # Initialize CSV file
    initialize_csv()
    
    # Create MQTT client
    client = mqtt.Client()
    
    # Set username and password
    client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    
    # Assign callbacks
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect
    
    # Connect to broker
    try:
        print(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        
        # Start the loop to process received messages
        client.loop_forever()
        
    except KeyboardInterrupt:
        print("\n\nDisconnecting...")
        client.disconnect()
        print(f"Data saved in: {CSV_FILE}")
        
    except Exception as e:
        print(f"Error: {e}")
