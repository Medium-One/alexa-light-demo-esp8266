import Alexa
import Analytics
import MQTT
import Filter
import json

def build_response(title, output):
    return {
        'version': '1.0',
        'sessionAttributes': {},
            'response' : {
                'outputSpeech': {
                    'type': 'PlainText',
                    'text': output
                    },
               'card': {
                   'type': 'Simple',
                   'title': 'SessionSpeechlet - ' + title,
                   'content': 'SessionSpeechlet - ' + output
                  },
              'reprompt': {
                   'outputSpeech': {
                        'type': 'PlainText',
                        'text': ""
                        }
                 },
              'shouldEndSession': True
           }
       }

def switch_light(light_type, state):
    send_list = list("00")
    if light_type == 'light' or light_type == 'the light':
        send_list[1] = '1'
    if state == 'on':
        send_list[0] = '1'
    send_value = "".join(send_list)
    MQTT.publish_event_to_client('esp8266', send_value)

event = IONode.get_event()
request = event['request']
uuid_marker = event.get("uuid", "")
if request['type'] == 'IntentRequest':
    if request['intent']['name'] == 'SensorStatusIntent':
        light_type = request['intent']['slots']['selection']['value']
        state = request['intent']['slots']['on_or_off']['value']
        response_txt = "Turning " + state + " " + light_type
        response_json = build_response(request['intent']['name'], response_txt)
        switch_light(light_type, state)
        Alexa.response(uuid_marker, response_json) 
