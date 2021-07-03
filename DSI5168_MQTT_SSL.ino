/*
 * Author: David Lee
 * Create on Jul 03,2021
 * Content: Upload the measured distance to IDEASChain by using MQTT protocol. And make a warn to Line notify by SSL(http) protocal.
 */



#include <WiFi.h>                   //WIFI library
#include <PubSubClient.h>           //MQTT library
char ssid[] = "********";        // SSID:router name      
char pass[] = "********";             // pass:router password
String Linetoken = "atvAG0ZJjjWLdGez6Uk6cenAstRfiRm6Iamc13jdkr1";

int status  = WL_IDLE_STATUS;         // keep connecting
char mqttServer[]     = "iiot.ideaschain.com.tw";          // take ideaschain as server
int mqttPort          = 1883;
char clientId[]       = "********";  // MQTT client ID. Create an unique ID.
char username[]       = "********";      // device access token(change your own access token of IDEASChain)
char password[]       = "";                                // don't need to set up 
char subscribeTopic[] = "v1/devices/me/telemetry";         //fixed topic, do not modify
char publishTopic[]   = "v1/devices/me/telemetry";         //as the same as subscribeTopic
String payload_string;
char publishTopicStr1[]= "sensorDist";
char publishPayload[]="{\"sensorDist\":\"30 \"}";

WiFiSSLClient client1;
char host[] = "notify-api.line.me";//LINE Notify API URL

int trigPin =12;
int echoPin =13;

void callback(char *topic, byte *payload, unsigned int length) {//recept the data from server
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i=0; i<length; i++) {
       Serial.print( (char) payload[i]);                 // convert *byte to string
    }
    
    Serial.println();
    Serial.println("-----------------------------------");
}

WiFiClient wifiClient;
PubSubClient client(wifiClient);                                //define the client's name


void publishData( char*publishTopicStr, float sensorValue){

char sensorDist [30];
sprintf (sensorDist ,"{\"%s \":\"%.2f \"}", publishTopicStr,sensorValue);
Serial.println(sensorDist);

while(!client.connected()){
  Serial.println("Attempting MQTT connection Attempt to connect...");
  if(client.connect (clientId, username, password)){
     Serial.println("MQTT connected");
     client.publish(publishTopic,sensorDist);
     client.subscribe(subscribeTopic);
}
  else{
Serial.print("failed rc= ");
Serial.print(client.state());
Serial.println("try again in 5 seconds ");
delay(5000);
}
}
}


void reconnect() {                            // client connect to the MQTT server
                                          
  while (!client.connected()) {                             //while(disconnect),then run the loop continually
    Serial.println("Attempting MQTT connection...");     
    if (client.connect(clientId, username, password)) {     // try to connect
      Serial.println("MQTT connected");                     //after connected, publish the topic & payload
      client.publish(publishTopic, "payload_string");      
      client.subscribe(subscribeTopic);                     //resubscribe the topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");                                    
      delay(5000);                                          //wating for 5 second to reconnect
    }
  }
}


void setup()                     //set up the pinmode and WIFI
{  
  pinMode(trigPin ,OUTPUT);
  pinMode(echoPin ,INPUT);
  while (status != WL_CONNECTED) {              
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);    //initialize wifi setting    
    delay(10000);                       //wating for WiFi connecting for 10 second
  }
  printWifiData();
  client.setServer(mqttServer, mqttPort);   
  client.setCallback(callback);
  delay(1500);
}

void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop()
{  
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration_us = pulseIn(echoPin, HIGH);//Measure the integral time of pulse triging and receiving.
  float distance_cm = 0.017 * duration_us;   //Change the integral into distance.
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  delay(500);
  if(isnan(duration_us) ){
   Serial.println("Failed to read from sensor!");
   return;
   }
  if (distance_cm <= 50) {
    String message = " <Warning>: The patient is getting out of bed!"; //The content of line message.
    message += "\n The distance between patient and bed head=" + String(((float)distance_cm)) + " cm";
    Serial.println(message);
    if (client1.connect(host, 443)) {
      int LEN = message.length();
      String url = "/api/notify";  //POST header
      client1.println("POST " + url + " HTTP/1.1");
      client1.print("Host: "); client1.println(host);
      //Access token 
      client1.print("Authorization: Bearer "); client1.println(Linetoken);
      client1.println("Content-Type: application/x-www-form-urlencoded");
      client1.print("Content-Length: "); client1.println( String((LEN + 8)) );
      client1.println();      
      client1.print("message="); client1.println(message);
      client1.println();
      delay(2000);
      String response = client1.readString();
      Serial.println(response); //Display the result of responsing
      client1.stop(); //Disconnecting
    }
    else {
      Serial.println("connected fail");
    }
  }
  delay(5000);
 if(!client.connected()){
  reconnect();
  delay(2000);
  }

  client.disconnect();
  client.loop();
  delay(300);
  publishData(publishTopicStr1,distance_cm);
  client.loop();

}


  
