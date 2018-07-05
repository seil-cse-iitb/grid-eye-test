#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>

using namespace std;

const char* ssid = "SEIL";
const char* password = "deadlock123";

const char* mqtt_server = "10.129.149.9";
//const char* mqtt_server = "10.129.28.158";
const char* mqtt_username = "<MQTT_BROKER_USERNAME>";
const char* mqtt_password = "<MQTT_BROKER_PASSWORD>";

const char* mqtt_topic = "test/nodemcu/grid_eye_data";
const char* mqtt_topic_publish = "nodemcu/pub";
const char* client_id = "grid_eye_data";

Adafruit_AMG88xx amg;
WiFiClient espClient;
PubSubClient client(espClient);

uint8_t pixels[AMG88xx_PIXEL_ARRAY_SIZE];
//uint8_t* pixels;

char readGridEye = 'f';

vector <byte*> grid_eye_vector;

int row_count = 0;
int col_count = 0;

bool published = false;
bool recorded = false;

int record_count = 0;

int byteIndex = 0;

byte byteArray[100][64];

void grid_eye_read()
{
  amg.readPixels(pixels, (uint8_t)AMG88xx_PIXEL_ARRAY_SIZE);

  String mqtt_data_string = "";

  for (int i = 0; i < AMG88xx_PIXEL_ARRAY_SIZE; i++) {
    mqtt_data_string += String(pixels[i]) + ",";
  }
  Serial.println();

  char mqtt_data[mqtt_data_string.length()];

  mqtt_data_string.toCharArray(mqtt_data, mqtt_data_string.length());
  //  mqtt_data_string.toCharArray(mqtt_data, 80);

  memcpy(byteArray[byteIndex], pixels, 64);

  //    byte* byteArray[byteIndex];
  //    byteArray[byteIndex] = (byte*)pixels;

  //    grid_eye_vector.push_back(byteArray[byteIndex]);

  unsigned int arraySize = sizeof(byteArray[byteIndex]);
  bool published = false;

  Serial.print("Recording Data with size ");
  Serial.print(sizeof(byteArray));
  Serial.print(" : ");
  Serial.println(mqtt_data_string.length());

  Serial.print("[");
  Serial.print(grid_eye_vector.size());
  Serial.println("]");
  //    published  = client.publish(mqtt_topic, byteArray, sizeof(pixels));

  //    if (published) Serial.println("Data Published");
  //    else Serial.println("Unable to publish the data");

  //  Serial.println(mqtt_data);
  //delay a 100 mili-second
  recorded  = true;
  record_count++;
  byteIndex++;
}

void grid_eye_publish()
{

  Serial.println("-------------------------");
  Serial.print("Number of records: ");
  Serial.println(record_count);
  Serial.println("-------------------------");
  delay(500);

  int valid_count = 0;
  Serial.println("Sending Data over mqtt");

  //      for (int vector_index = 0; vector_index < grid_eye_vector.size(); vector_index++)
  for (int i = 0; i < record_count; i++)
  {
    //        published  = client.publish(mqtt_topic, grid_eye_vector[vector_index], sizeof(pixels));
    published = client.publish(mqtt_topic, byteArray[i], sizeof(pixels));
    if (published) Serial.println("Published");
    else Serial.println("Unable to publish");

    valid_count++;
  }

  if (record_count == valid_count) Serial.println("BullsEye Behbee");
  else Serial.println("Some serious issues with count");

  delay(500);
  record_count = 0;
  recorded = false;
  grid_eye_vector.clear();
  byteIndex = 0;
}

void setupWiFi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi with IP");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT...");

    if (client.connect(client_id))
    {
      Serial.println("Connected");
    }
    else
    {
      Serial.print("Failed to connect ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  //  Serial.println(F("AMG88xx pixels"));


  setupWiFi();

  client.setServer(mqtt_server, 1883);

  bool status;

  // default settings
  status = amg.begin(0x68);
  if (!status) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }

  amg.write8(AMG88xx_FPSC, AMG88xx_FPS_10);
  amg.setMovingAverageMode(false);
  //  Serial.println("-- Pixels Test --");

  //  Serial.println();

  delay(100); // let sensor boot up
}

void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  while (Serial.available() > 0)
  {
    readGridEye = Serial.read();
  }

  if (readGridEye == 't')
  {
    grid_eye_read();
  }
  else
  {
    Serial.println("Stop Reading[put 't' for the read");
    if (recorded)
    {
      grid_eye_publish();
    }
  }
  delay(100);
}
