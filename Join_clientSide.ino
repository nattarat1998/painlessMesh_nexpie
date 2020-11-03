#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <TypeConversion.h>
#include <Crypto.h>
#include "meshClient.h"
#include "painlessMesh.h"
#define NUM 1

info_child child[NUM];

int interruptPin = D2; 
int interruptPin1 = D3; 
int LED1 = D6; 
int LED2 = D7;
int LED3 = D8; 

void ICACHE_RAM_ATTR joinMesh();
void ICACHE_RAM_ATTR restore();

namespace TypeCast = experimental::TypeConversion;
int flag_j=0;
uint8_t derivedKey[32] { 0 };

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler     userScheduler; 
painlessMesh  mesh;


void receivedCallback( uint32_t from, String &msg );

size_t logServerId = 0;
int time_send = 10000;


Task myLoggingTask(time_send, TASK_FOREVER, []() {
    namespace TypeCast = experimental::TypeConversion;
    using namespace experimental::crypto;
    uint8_t resultArray[SHA256::NATURAL_LENGTH] { 0 };
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject msg = jsonBuffer.to<JsonObject>();
    String str;
    uint8_t res_array [32];
    if (flag_j ==0 ){     //flag_j = 0 คือ send data mode  
        msg["nodeId"] = mesh.getNodeId();
        msg["topic"] = "sensor";
        msg["value"] = random(0, 180);

        serializeJson(msg, str); // convert json ----> string
        Serial.println();
        SHA256::hmac(str.c_str(), str.length(), derivedKey, sizeof derivedKey, resultArray, sizeof resultArray);  
        for (int i=0;i<32;i++){ 
        msg["HMAC"][i] = resultArray[i];
        }
        Serial.println(str);
        String strD;
        serializeJson(msg, strD); 
        if (child[0].flag == 1) {
            mesh.sendSingle(logServerId, strD);
            Serial.println("send data mode");
            DynamicJsonDocument jsonBuffer(1024);
            JsonObject msg1 = jsonBuffer.to<JsonObject>();
            msg1["MODE"] = "intro";
            msg1["AREA"] = "oai/layer4/nexpie";
            msg1["ATTR"][0] = "led";
            msg1["ATTR"][1] = "temp";
            msg1["ATTR"][2] = "humid";
            String str1;
            serializeJson(msg1, str1); // convert json ----> string
            SHA256::hmac(str1.c_str(), str1.length(), derivedKey, sizeof derivedKey, resultArray, sizeof resultArray);          
            for (int i=0;i<32;i++){ 
            msg1["HMAC"][i] = resultArray[i];
            }
            String strD1;
            serializeJson(msg1, strD1); 
            mesh.sendSingle(logServerId, strD1);
            Serial.println(strD1);

        
        }
        else if(child[0].flag == 0){                
            Serial.println("NOT YET JOIN TO SERVER");
        }
    }        
    if (flag_j == 1){ 
        msg["nodeId"] = mesh.getNodeId();
        msg["topic"] = "sensor";  
        msg["mode"] = "join";
        serializeJson(msg, str);
        mesh.sendBroadcast(str);
        Serial.println("join mode");
        
    }
    
}

);

void check_status_join(){
  uint8_t errorKey[32] { 0 };
  if (child[0].flag==1){
      int count=0;
  for (int i=0;i<32;i++){
     if (child[0].derivedKey[i] == errorKey[i]){
        count = count+1;
      }
       }
      if (count==32){
        Serial.println("join : ERROR ");
        digitalWrite(LED2, HIGH);
      }
      else {
        Serial.print(child[0].id);
        Serial.println(":join success");
        digitalWrite(LED1, LOW);   

      }
        }
      else {
        Serial.println("join : ERROR ");
        digitalWrite(LED2, HIGH); 
            
      }
}



void setup() {
  Serial.begin(115200);  
  EEPROM.begin(4096);

  reset_table_index(child, NUM);
  getTable(child);

  logServerId=child[0].id;
  for (int i=0;i<32;i++){derivedKey[i]=child[0].derivedKey[i];}
  show_info_child(child, NUM);
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);  
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);  
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  attachInterrupt(interruptPin, joinMesh, RISING);
  attachInterrupt(interruptPin1, restore, RISING);
  check_status_join();
  userScheduler.addTask(myLoggingTask);
  myLoggingTask.enable();
}

void loop() {
  
  mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) { 
    namespace TypeCast = experimental::TypeConversion;
    using namespace experimental::crypto;
    uint8_t resultArray[SHA256::NATURAL_LENGTH] { 0 }; 
#if ARDUINOJSON_VERSION_MAJOR==6
  DynamicJsonDocument jsonBuffer(1024 + msg.length());
  DeserializationError error = deserializeJson(jsonBuffer, msg);
  if (error) {
    Serial.printf("DeserializationError\n");
    return;
  }
  JsonObject root = jsonBuffer.as<JsonObject>();
#else
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
#endif
    if (flag_j==1){
        if (root.containsKey("TOPIC")) {
            if (String("logServer").equals(root["TOPIC"].as<String>())) {
                // check for on: true or false
                logServerId = root["ID"];
            for (int i=0;i<32;i++){
                derivedKey[i] = root["KEY"][i];
                }
                child[0].flag=1;
                Serial.printf("logServer detected!!!\n");
            }
            store_into_eeprom(logServerId,derivedKey);
            getTable(child);
            if (child[0].flag==1){
                Serial.print(child[0].id);
                Serial.println(":join : finish");
                digitalWrite(LED1, LOW);     
                } 
            else {
                Serial.println("join : finish ");
                digitalWrite(LED1, LOW);     

            }
            flag_j=0;
            time_send = 10000;      
            Serial.printf("join finished");
        }
    }     
    else {                
            String push;
            String Req;  
            String strRoot;         
            uint8_t keyTask[32];
            bool check; 
                      
            for (int i=0;i<32;i++){ //keep hmac of root in uint8_t
                keyTask[i] = root["HMAC"][i];
            }
            serializeJson(root, strRoot); 
            Serial.print("strRoot --- > "); Serial.println(strRoot);
            DynamicJsonDocument require(1024); //define jsonObject named required for creating hmac of data recieved
            require["AREA"] = root["AREA"];           
            require["ATTR"] = root["ATTR"];
            require["TASK"] = root["TASK"];
            serializeJson(require, Req);
            Serial.print("Req ---> "); Serial.println(Req);
            for(int q=0;q<32;q++){
                derivedKey[q] = child[0].derivedKey[q];                
            } //pull key from struct
            
            SHA256::hmac(Req.c_str(), Req.length(), derivedKey, sizeof derivedKey, resultArray, sizeof resultArray);
            for(int p=0;p<32;p++){
            Serial.print(resultArray[p]);  Serial.print(" | ");
            }            
            Serial.println();
            
            for(int q=0;q<32;q++){
            Serial.print(keyTask[q]);  Serial.print(" | ");
            }            
            Serial.println();
            check = compare_HMAC(resultArray, keyTask);
            if(check == 1){
                JsonObject response = jsonBuffer.as<JsonObject>();
                if(String("monitor").equals(root["TASK"].as<String>())){        
                    if(String("temp").equals(root["ATTR"].as<String>())){
                        Serial.println("=========================== MONITOR TEMP RESPONSE ===================");
                        response["VALUE"] = random(20, 30);
                        response["ATTR"] = "temp";
                        response["AREA"] = root["AREA"];           
                    }
                    else if(String("humid").equals(root["ATTR"].as<String>())){
                        Serial.println("=========================== MONITOR HUMID RESPONSE ===================");
                        response["VALUE"] = random(50, 80);
                        response["ATTR"] = "humid";
                        response["AREA"] = root["AREA"];
                    }
                    serializeJson(response, push);
                    mesh.sendSingle(logServerId, push);      
            }
                else if(String("control").equals(root["TASK"].as<String>())){
                    if(String("led").equals(root["ATTR"].as<String>())){ 
                        Serial.println("=========================== CONTROL LED RESPONSE ===================");    
                        bool stateLED3 = digitalRead(LED3);
                        digitalWrite(LED3, !stateLED3);
                        stateLED3 = digitalRead(LED3);
                        if(stateLED3 == 1){
                            response["VALUE"] = "on";
                        }
                        else if(stateLED3 == 0){
                            response["VALUE"] = "off";
                        }
                        response["ATTR"] = "led";
                        response["AREA"] = root["AREA"];
                        serializeJson(response, push);
                        mesh.sendSingle(logServerId, push);
                    }
                }       
            }
            else if(check == 0){
                Serial.printf("check ( 0 ) : Received from %u msg=%s\n", from, msg.c_str());
            }          
        
    } 
    
}




void joinMesh()
{       
  Serial.println("----------------------join mode------------------------");
  flag_j = 1;
  time_send = 1000; 
  digitalWrite(LED1, HIGH);     
}

void restore()
{       
  Serial.println("----------------------restore mode------------------------");
  EEPROM.begin(4096);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 4096; i++) {
    EEPROM.write(i, 0);
  }    
    EEPROM.end();
  
    Serial.println("----------------------restore finishrd------------------------");    
}
