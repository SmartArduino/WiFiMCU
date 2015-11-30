--mqtt demo
print("------mqtt demo------")

clientid = "wifimcu_mqtt_client"
keepalive = 30
username = "doit"
password = "123456"
mqttClt	= mqtt.new(clientid,keepalive,username,password)

server = "test.mosquitto.org"
port = 1883
mqtt.start(mqttClt,server,port)