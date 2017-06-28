import MQTT

led_value = IONode.get_input('in1')['event_data']['value']
send_value = "0"
if led_value == "on":
    send_value = "1"

send_value += "0"

MQTT.publish_event_to_client('esp8266', send_value)
