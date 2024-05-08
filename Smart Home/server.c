#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"
#include <stdint.h>
#include <stdio.h>
#include <mariadb/mysql.h>

// SQL
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server = "localhost";
	char *user = "duyrasp";
	char *password = "1212"; 
	char *database = "demo_iot";
// MQTT

#define ADDRESS     "tcp://broker.emqx.io:1883"
#define CLIENTID	"mqtt_XYZ"

// SUB ID
#define SUB_TOPIC   "ESP8266/IOT/Bedroom/Sensor"
#define SUB_TOPIC1	"Rasp/IOT/Bedroom/Update_DataBase"

// PUB ID
#define PUBTOPIC1   "Server/IOT/Bedroom/Comand" 
#define PUBTOPIC2   "Server/IOT/Bedroom/Control"

// khai bai bien 
int State_Light, State_Window, Mode_Light,State_Fan, Mode_Window, Percent_Window,Light_Set,People;
int Sensor;

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

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    char sql[200];
    //printf("Received message: %s\n", payload);

    //Update Database
    sscanf(payload,"Light_Update_Auto:%d\n",&State_Light);
    sprintf(sql,"update Light set State = %d",State_Light);
	mysql_query(conn,sql);

    sscanf(payload,"Update_Light:%d\n",&State_Light);
    sprintf(sql,"update Light set State = %d",State_Light);
	mysql_query(conn,sql);
    
    sscanf(payload,"Update_Fan:%d\n",&State_Fan);
	sprintf(sql,"update Fan set State = %d",State_Fan);
	mysql_query(conn,sql);

    sscanf(payload,"Update_Window:%d\n",&Percent_Window);
    sprintf(sql,"update Window set Percent = %d",Percent_Window);
	mysql_query(conn,sql);
    
    sscanf(payload,"Lux:%d:People:%d",&Sensor,&People);
	sprintf(sql,"update Sen set Lux = %d, People = %d",Sensor,People);
    mysql_query(conn,sql);
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;


    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
   
    //Subcribe Topic
    MQTTClient_subscribe(client, SUB_TOPIC, 0);
    MQTTClient_subscribe(client, SUB_TOPIC1, 0);

	char messeger[200];
    conn = mysql_init(NULL);
    mysql_real_connect(conn,server,user,password,database,0,NULL,0);

    while(1) {
        char querry_connect[200]="";
        
        // get data from table Sensor
        sprintf(querry_connect,"select * from Sen ");
        mysql_query(conn,querry_connect);
        res = mysql_store_result(conn);
        while(row = mysql_fetch_row(res)){
            sscanf(row[0],"%d\n",&Sensor);
            sscanf(row[1],"%d\n",&People);
        }
        
        // clear result and close the connection
        mysql_free_result(res);

        // get data from table Light
        sprintf(querry_connect,"select * from Light");
        mysql_query(conn,querry_connect);
        res = mysql_store_result(conn);
        while(row = mysql_fetch_row(res)){
            sscanf(row[0],"%d\n",&Mode_Light);
            sscanf(row[1],"%d\n",&State_Light);
            sscanf(row[2],"%d\n",&Light_Set);
        }

        // clear result and close the connection
        mysql_free_result(res);

        // get data from table Windown
        sprintf(querry_connect,"select * from Window ");
        mysql_query(conn,querry_connect);
        res = mysql_store_result(conn);
        while(row = mysql_fetch_row(res)){
            sscanf(row[0],"%d\n",&Mode_Window);
            sscanf(row[1],"%d\n",&State_Window);
            sscanf(row[2],"%d\n",&Percent_Window);
        }
        
        // clear result and close the connection
        mysql_free_result(res);
       
        // get data from table Fan
        sprintf(querry_connect,"select * from Fan ");
        mysql_query(conn,querry_connect);
        res = mysql_store_result(conn);
        while(row = mysql_fetch_row(res)){
            sscanf(row[0],"%d\n",&State_Fan);
        }
        
        // clear result and close the connection
        mysql_free_result(res);        

        // send data to raspberry    
        
        sprintf(messeger,"Lux:%d:Peo:%d\n",Sensor,People);
        publish(client,PUBTOPIC1,messeger);

        sprintf(messeger,"L_S:%d:L_M:%d:L_Set:%d:W_S:%d:W_M:%d:W_P:%d:F_S:%d\n",State_Light,Mode_Light,Light_Set,State_Window,Mode_Window,Percent_Window,State_Fan);
        publish(client,PUBTOPIC1,messeger);

        //send data to Esp8266 control motor
        sprintf(messeger,"Percent:%d\n",Percent_Window);
        publish(client,PUBTOPIC2,messeger);        

        sleep(1);
    } 
    mysql_close(conn);
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}
