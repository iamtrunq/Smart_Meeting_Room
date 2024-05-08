#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"
#include <time.h>
#include <wiringPi.h>
#include <stdint.h>

//max and min openwindow
#define MAX_WINDOW  10
#define MIN_WINDOW  -1

//config GPIO PORT OUTPUT
#define GPIO_Light 28
#define GPIO_Fan 29

//config GPIO PORT INPUT
#define GPIO_Light_ON_OFF 2
#define GPIO_Window_UP 3
#define GPIO_Window_DOWN 0
#define GPIO_Fan_ON_OFF 4

unsigned char GPIO_OUT[2] ={28,29};
unsigned char GPIO_IN[4]={0,2,3,4};

// Declare Varible
int Light_Mode, Light_State, Light_Sen,Light_Set, Window_Mode, Window_State, Window_Percent, Fan_State;
int People,Mode;

// Declace Varible FLAG
uint8_t FLAG_FAN=0,FLAG_LIGHT=0,FLAG_Window_UP=0,FLAG_Window_DOWN=0, FLAG_Window = 0, Window_Previous=-1;

// MQTT Broker
#define ADDRESS     "tcp://broker.emqx.io:1883"
#define CLIENTID	"mqtt_XY"

// PUB ID
#define PUBTOPIC2   "Rasp/IOT/Bedroom/Update_DataBase"

//SUB ID
#define SUBTOPIC1   "Server/IOT/Bedroom/Comand"

// Init wiringPi an config GPIO
void init_wiringPi(){
	wiringPiSetup();
	for(uint8_t i = 0; i < 2; i++){
		pinMode(GPIO_OUT[i],OUTPUT);
		digitalWrite(GPIO_OUT[i],LOW);
	}
	for(uint8_t i = 0; i < 4;i++){
		pinMode(GPIO_IN[i],INPUT);
	}
}

// Publisher
void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
}

// Subcribe messeger from topic
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received message: %s\n",payload);
    sscanf(payload,"L_S:%d:L_M:%d:L_Set:%d:W_S:%d:W_M:%d:W_P:%d:F_S:%d\n",&Light_State,&Light_Mode,&Light_Set,&Window_State,&Window_Mode,&Window_Percent,&Fan_State);
    sscanf(payload,"Lux:%d:Peo:%d\n",&Light_Sen,&People);

	if(Window_Percent!=Window_Previous){
		FLAG_Window=1;
		Window_Previous=Window_Percent;
	}
	
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Function turn up FLAG for Light
void Light_INT(){
	FLAG_LIGHT=1;
}

// Function turn up FLAG for Window up
void Window_UINT(){
	if(Mode==0){
		if(Window_Percent <= MAX_WINDOW && Window_Percent >= MIN_WINDOW){
			FLAG_Window_UP=1;
			Window_Percent++;
			if(Window_Percent==11) Window_Percent = 10;
		}
	}
}

// Function turn up FLAG for Windown down
void Window_DINT(){
	if(Mode==0){
		if(Window_Percent <= MAX_WINDOW && Window_Percent >= MIN_WINDOW){
			FLAG_Window_DOWN=1;
			Window_Percent--;
			if(Window_Percent==-1) Window_Percent = 0;
		}
	}

} 

// Function turn up FLAG for Fan
void Fan_INT(){
	FLAG_FAN=1;
}

int main(int argc, char* argv[]) {	
	
	//Declare and config MQTT
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);
    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    //Sign up topic subcribe
    MQTTClient_subscribe(client, SUBTOPIC1, 0);

	// init WiringPi
	init_wiringPi();
	printf("Hello\n");
	
	char msg [200];
    
	// interrupt callback
	wiringPiISR(GPIO_Light_ON_OFF,INT_EDGE_RISING, &Light_INT);
    wiringPiISR(GPIO_Window_UP, INT_EDGE_RISING, &Window_UINT);
    wiringPiISR(GPIO_Window_DOWN, INT_EDGE_RISING, &Window_DINT);
    wiringPiISR(GPIO_Fan_ON_OFF, INT_EDGE_RISING, &Fan_INT);
    
    // Turn off Device
    digitalWrite(GPIO_Light,HIGH);
    digitalWrite(GPIO_Fan,HIGH);
	

    while(1) {
		// Select Mode
		if(Light_Mode==1 && Window_Mode==1) Mode = 1;
		else if(Light_Mode==0 && Window_Mode == 0) Mode = 0;
		printf("Mode:%d\n",Mode);
		printf("Window Percent%d\n",Window_Percent);

		switch(Mode){
			// Auto Mode
			case 1:
				if(People==1){
					//Fan control
					if(Fan_State==0){
						digitalWrite(GPIO_Fan,HIGH);
					}
					else{ 
						digitalWrite(GPIO_Fan,LOW);
					}

					if(FLAG_FAN==1){
						if(Fan_State==0)
							sprintf(msg,"Update_Fan:%d\n",1);
						else{
							sprintf(msg,"Update_Fan:%d\n",0);
						}
						publish(client,PUBTOPIC2,msg);
						printf("Updated Data Fan");
						FLAG_FAN=0;
					}
					// Window up
					if(Light_Sen < Light_Set-20){
						Window_Percent++;
						if (Window_Percent >= MAX_WINDOW) Window_Percent=10;
						if(Window_Percent == 10){
							digitalWrite(GPIO_Light,LOW);
							sprintf(msg,"Light_Update_Auto:%d\n",1);
							publish(client,PUBTOPIC2,msg);
                            printf("Max\n");
						}else{
							printf("Window Opening\n");
							sprintf(msg,"Update_Window:%d\n",Window_Percent);
							publish(client,PUBTOPIC2,msg);
						} 

					// Window down	                       
					}else if(Light_Sen > Light_Set+20){
                        Window_Percent--;
						if (Window_Percent <= MIN_WINDOW) Window_Percent=0;
						if(Window_Percent == 0){
							digitalWrite(GPIO_Light,HIGH);
							sprintf(msg,"Light_Update_Auto:%d\n",0);
							publish(client,PUBTOPIC2,msg);
						}else{
							printf("Window Closing\n");
							sprintf(msg,"Update_Window:%d\n",Window_Percent);
							publish(client,PUBTOPIC2,msg);
							printf("Min\n");
						}
                    // Window stop    
					}else if(Light_Sen > Light_Set-20 && Light_Sen < Light_Set + 20){
                        printf("Lux is oke\n");
                    }
                    printf("Present:%d\n",Window_Percent);
				}else{
					// Turn off all device and update database
					digitalWrite(GPIO_Light,HIGH);
					digitalWrite(GPIO_Fan,HIGH);
					sprintf(msg,"Light_Update_Auto:%d\n",0);
					publish(client,PUBTOPIC2,msg);
					sprintf(msg,"Update_Fan:%d\n",0);
					publish(client,PUBTOPIC2,msg);
				}
				break;
			case 0:
				// Manual Control
				//Fan control
				if(Fan_State==0){
					digitalWrite(GPIO_Fan,HIGH);
				}
				else{ 
					digitalWrite(GPIO_Fan,LOW);
				}

				if(FLAG_FAN==1){
					if(Fan_State==0)
						sprintf(msg,"Update_Fan:%d\n",1);
					else{
						sprintf(msg,"Update_Fan:%d\n",0);
					}
					publish(client,PUBTOPIC2,msg);
					printf("Updated Data Fan");
					FLAG_FAN=0;
				}
				// Control Window
				if(FLAG_Window == 1 && (FLAG_Window_DOWN == 0 || FLAG_Window_UP == 0)){
				 	FLAG_Window = 0 ;						
				 	sprintf(msg,"Update_Window:%d\n",Window_Percent);
				 	publish(client,PUBTOPIC2,msg);
				}

				if(FLAG_Window_UP==1){
					sprintf(msg,"Update_Window:%d\n",trangthai);
					publish(client,PUBTOPIC2,msg);
					FLAG_Window_UP = 0;
				}
				if(FLAG_Window_DOWN==1){	
					sprintf(msg,"Update_Window:%d\n",trangthai);
					publish(client,PUBTOPIC2,msg);
					FLAG_Window_DOWN = 0;					
				}
				
				//Control Light
				if(Light_State==0){
					digitalWrite(GPIO_Light,HIGH);
				}else if(Light_State==1){
					digitalWrite(GPIO_Light,LOW);
				}
				if(FLAG_LIGHT==1){
					if(Light_State==0){
						sprintf(msg,"Update_Light:%d\n",1);
					}else{
						sprintf(msg,"Update_Light:%d\n",0);
					}
					publish(client,PUBTOPIC2,msg);
					printf("Sent Data");
					FLAG_LIGHT=0;
				}
				break;
		}
        sleep(1);
    } 
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}
