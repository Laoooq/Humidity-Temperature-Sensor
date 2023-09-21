#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "WLAN2";
const char* password = "123412341234";
const char* mqtt_server = "192.168.0.1"; 
const int relayPin = 5; 
const int DHT_PIN = 4;  
const int DHT_TYPE = DHT22; 

DHT dht(DHT_PIN, DHT_TYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Подключение к Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Подключено к Wi-Fi");
  Serial.println("IP адрес: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Получено сообщение по теме: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Сообщение: ");
  Serial.println(message);

  if (strcmp(topic, "тема_контроля_реле") == 0) {
    // если ON, включаем реле
    if (message.equals("ON")) {
      digitalWrite(relayPin, HIGH); // включаем
      Serial.println("Реле включено");
    }
    // если OFF то выключаем реле
    else if (message.equals("OFF")) {
      digitalWrite(relayPin, LOW); // выключаем
      Serial.println("Реле выключено");
    }
    else {
      Serial.println("Ошибка: недопустимая команда для реле");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Попытка подключения к MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Подключено");
      client.subscribe("тема_контроля_реле");
    } else {
      Serial.print("Подключение не удалось, ошибка: ");
      Serial.print(client.state());
      Serial.println(" Повторная попытка подключения через 5 секунд");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // изначально реле выключено
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  client.publish("тема_температуры", String(temperature).c_str());
  client.publish("тема_влажности", String(humidity).c_str());
}
