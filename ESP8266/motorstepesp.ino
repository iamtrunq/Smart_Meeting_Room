#include <AccelStepper.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define motorPin1  5
#define motorPin2  4
#define motorPin3  0
#define motorPin4  2

#define ssid "E1-303"
#define password "12345678"
#define mqtt_server "broker.emqx.io"


const uint16_t mqtt_port = 1883; //Port của CloudMQTT TCP
WiFiClient espClient;
PubSubClient client(espClient);

int dem = 1;
int Mode,Num,Percent;
AccelStepper mystepper(AccelStepper::HALF4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {
  Serial.begin(9600);
  mystepper.setMaxSpeed(1000);
  mystepper.setAcceleration(500);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port); 
  client.setCallback(callback);
}

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

char mess[30];
String receveData = "";
// Hàm call back để nhận dữ liệu
void callback(char* topic, byte* payload, unsigned int length) 
{
  receveData = "";
  Serial.print("Co tin nhan moi tu topic:");
  Serial.println(topic);
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
    receveData += (char)payload[i];
  }
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
      client.subscribe("Server/IOT/Bedroom/Control"); //đăng kí nhận dữ liệu từ topic IoT47_MQTT_Test
    }
    else 
    {
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(3000);
    }
  }
}

int rounds = 0;
int temp = rounds;
int limit = 10;
int vong = 0;
int domo= 0;



void loop() {
  if (!client.connected())// Kiểm tra kết nối
    reconnect();
  client.loop();

    sscanf(receveData.c_str(),"Percent:%d\n",&Percent);
//  if (Percent!=domo){
//    Serial.print("Per: ");
//    Serial.println(Percent);
//    domo = Percent;
//  }

  domo= Percent;
//neu muon thay doi so vong/buoc thi nhân lên tương ứng
if (dem <= (4 * abs(rounds))) {
    mystepper.setCurrentPosition(0);
    if (dem % 4 == 0) {
      vong = vong + 1;
      Serial.print("    vong: ");
      Serial.println(vong);
    }
  }
  if (rounds < 0) {
    
    while (mystepper.currentPosition() != -1020) {
      mystepper.setSpeed(-700);
      mystepper.runSpeed();
    }
  } else if (rounds > 0) {
    while (mystepper.currentPosition() != 1020) {
      mystepper.setSpeed(700);
      mystepper.runSpeed();
    }
  } else {
    mystepper.setSpeed(0);
    mystepper.runSpeed();
    dem=0;
    vong=0;
  }
  dem++;
  Serial.print("    dem: ");
  Serial.println(dem);

  if (dem > (4 * abs(rounds))) {
  
  Serial.print("    Độ mở: ");
  Serial.println(domo);

  if (domo < 0) domo=0;
  else if (domo>limit) domo = limit;
      rounds = domo - temp;
      temp = domo;
        dem = 1;
        vong=0;

      Serial.print("Số rounds : ");
        Serial.println(rounds);
      Serial.print("vị trí hiện tại : ");
        Serial.println(temp);

      

  }
    

  
  }
