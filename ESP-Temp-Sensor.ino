#include <Basecamp.hpp>
// Basecamp wird angewiesen einen verschlüsselten Acess-Point zu öffnen. Das Passwort erscheint in der seriellen Konsole.
Basecamp iot{
  Basecamp::SetupModeWifiEncryption::secured
};

// Dieses Beispiel verwendet einen DHT11 als Temperatursensor
// Sie benötigen https://github.com/beegee-tokyo/DHTesp als Bibliothek
#include <DHTesp.h>
DHTesp dht;
const int dhtPin = 17;
String room;
String temp;

String subTopic;
String pubTopic;
String tempTopic;

String getTemp() {

  String temp = String(dht.getTemperature());

  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    temp = "-274";
  }
  iot.configuration.set("my-Temperature", temp);
  iot.configuration.save();
  iot.mqtt.publish("stat/frontdoor/battery", 1, true, "empty" );

  return temp;
}

void setup() {
  dht.setup(dhtPin, DHTesp::DHT11);
  iot.begin();
  iot.mqtt.onConnect(mqttConnected);
  iot.mqtt.onSubscribe(mqttSubscribed);
  iot.mqtt.onMessage(mqttMessage);

  temp = getTemp();
  iot.configuration.set("my-Temperature", temp);
  iot.configuration.save();

  room = iot.configuration.get("my-Room");


  iot.web.addInterfaceElement(
    "tempDisplay",
    "input",
    "Aktuelle Temperatur:",
    "#configform",
    "my-Temperature"
  );
  iot.web.setInterfaceElementAttribute(
    "tempDisplay",
    "readonly",
    "true"
  );
  iot.mqtt.onPublish(mqttPublished);

  iot.web.server.on(
    "/temperature",
    HTTP_GET,
    [](AsyncWebServerRequest * request)
  {
    AsyncWebServerResponse *response =
      request->beginResponse(
        200,
        "text/plain",
        getTemp()
      );
    request->send(response);
  });
}



void mqttConnected(bool sessionPresent) {
  Serial.println("MQTT verbunden!");
  subTopic = iot.hostname + "/sendtemp";
  pubTopic = iot.hostname + "/status";
  tempTopic = iot.hostname + "/temp";
  iot.mqtt.subscribe(subTopic.c_str(), 2);
  iot.mqtt.publish(subTopic.c_str(), 1, true, "online");
};

void mqttSubscribed(uint16_t packetId, uint8_t qos) {
  Serial.println("Abonnement erfolgreich");
};

void mqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Neue MQTT-Nachricht:");
  Serial.print("Topic:");
  Serial.println(topic);
  Serial.print("Payload:");
  Serial.println(payload);

  iot.mqtt.publish(tempTopic.c_str(), 1, true, getTemp().c_str());
};

void mqttPublished(uint16_t packetId) {
  Serial.println("MQTT-Nachricht veröffentlicht");
};


void loop() {
  delay(10000);
  Serial.print("Temperatur: ");
  temp = getTemp();
  Serial.println(temp);
  iot.mqtt.publish(tempTopic.c_str(), 1, true, temp.c_str());
}
