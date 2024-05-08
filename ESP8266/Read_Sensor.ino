#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <i2cdetect.h>

const int Detect_People = D6;

BH1750 lightMeter;
uint8_t People,People_temp;
int lux;
char msg[100];

// Thông tin về wifi
#define ssid "E1-303"
#define password "12345678"
//broker.emqx.io


#define mqtt_server "broker.emqx.io"
const uint16_t mqtt_port = 1883; //Port của CloudMQTT TCP 1883

WiFiClient espClient;
PubSubClient client(espClient);

void setup() 
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port); 
  client.setCallback(callback);
  //
  pinMode(Detect_People, INPUT);

  // BH1750
  Wire.begin(D2,D1);
  lightMeter.begin();
  i2cdetect();
  delay(2000);
	
}
// Hàm kết nối wifi
void setup_wifi() 
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// Hàm call back để nhận dữ liệu
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Co tin nhan moi tu topic:");
  Serial.println(topic);
  for (int i = 0; i < length; i++) 
    Serial.print((char)payload[i]);
  Serial.println();
}
// Hàm reconnect thực hiện kết nối lại khi mất kết nối với MQTT Broker
void reconnect() 
{
  while (!client.connected()) // Chờ tới khi kết nối
  {
    // Thực hiện kết nối với mqtt user và pass
    if (client.connect("ESP8266_id1","ESP_offline",0,0,"ESP8266_id1_offline"))  //kết nối vào broker
    {
      Serial.println("Đã kết nối:");
      client.subscribe("rasptoserver"); //đăng kí nhận dữ liệu từ topic IoT47_MQTT_Test
    }
    else 
    {
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(5000);
    }
  }
}
unsigned long t;
void loop() 
{
  lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");


  if (!client.connected())// Kiểm tra kết nối
    reconnect();
  client.loop();
  People_temp=digitalRead(Detect_People);
  People=1-People_temp;
  Serial.println("Home");
  Serial.println(People);

  sprintf(msg,"Lux:%d:People:%d",lux,People);
  Serial.println(msg);

  Serial.println("You recently pusblished messenger");
  client.publish("ESP8266/IOT/Bedroom/Sensor",msg);
  delay(2000);

}