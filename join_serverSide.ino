#include "painlessMesh.h"
#include <TypeConversion.h>
#include <Crypto.h>
#include "Authentication_Mesh.h"
#include <EEPROM.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define interruptPin D2
#define NUM 20
#define verifyPin D3
#define notifyLED D4
#define notifyLED2 D5

void ICACHE_RAM_ATTR joinDevice();
void ICACHE_RAM_ATTR Verify();

info_child child[NUM];
namespace TypeCast = experimental::TypeConversion;
Scheduler     userScheduler; 
painlessMesh  mesh;

constexpr char masterKey[] PROGMEM = "secretKey";
bool FLAG_j = false;
size_t nodeID;
uint8_t key[32];
void receivedCallback( uint32_t from, String &msg );

Task logServerTask(30000, TASK_FOREVER, []() {
#if ARDUINOJSON_VERSION_MAJOR==6
        DynamicJsonDocument jsonBuffer(1024);
        JsonObject msg = jsonBuffer.to<JsonObject>();
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& msg = jsonBuffer.createObject();
#endif
    msg["topic"] = "logServer";
    msg["nodeId"] = mesh.getNodeId();

    String str;
#if ARDUINOJSON_VERSION_MAJOR==6
    serializeJson(msg, str);
#else
    msg.printTo(str);
#endif
    mesh.sendBroadcast(str);

    // log to serial
#if ARDUINOJSON_VERSION_MAJOR==6
    serializeJson(msg, Serial);
#else
    msg.printTo(Serial);
#endif
    Serial.printf("\n");
});

void setup() {
  Serial.begin(115200);
  EEPROM.begin(4096);  
  getTable(child);
  mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection([](size_t nodeId) {
    Serial.printf("New Connection %u\n", nodeId);
  });
  mesh.onDroppedConnection([](size_t nodeId) {
    Serial.printf("Dropped Connection %u\n", nodeId);
  });

  
  userScheduler.addTask(logServerTask);
  logServerTask.enable();
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(verifyPin, INPUT_PULLUP);
  pinMode(notifyLED, OUTPUT);
  pinMode(notifyLED2, OUTPUT);
  attachInterrupt(interruptPin, joinDevice, RISING);
  attachInterrupt(verifyPin, Verify, RISING);
}

void loop() {
  
  mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) {  
     
    DynamicJsonDocument data(1024);    
    deserializeJson(data, msg); 
    String str; 
    Serial.printf("logServer: Received from %u msg=%s\n", from, msg.c_str());
    nodeID = data["nodeId"];
  
    if(String("join").equals(data["mode"].as<String>()) && FLAG_j == true){ 
    
      Serial.print("==============RECV PACKAGE [ JOIN ] >>>> ");
      
      using namespace experimental::crypto;      
      uint8_t derivedKey[ENCRYPTION_KEY_LENGTH] { 0 };
      uint8_t hkdfSalt[16] { 0 };
      Serial.print(" GENERATING KEY . . . >>>>>");
      getNonceGenerator()(hkdfSalt, sizeof hkdfSalt);
      HKDF hkdfInstance(FPSTR(masterKey), (sizeof masterKey) - 1, hkdfSalt, sizeof hkdfSalt); // (sizeof masterKey) - 1 removes the terminating null value of the c-string
      hkdfInstance.produce(derivedKey, 32);     

      
      
      store_info_child(child, nodeID, 20, derivedKey);
      Serial.print("STORE INFO CHILD >>>>>");
      
      Serial.print(" SHOW INFO CHILD ==================");
      show_info_child(child, 20);
      Serial.print("derivedKey : ");  for(int j=0;j<32;j++){Serial.printf("%02X", derivedKey[j] );} 
      Serial.println();
      Serial.println("===================================== SENDING .... =======================================");
      DynamicJsonDocument Info(1024); 
      Info["topic"] = "logServer";
      Info["ID"] = mesh.getNodeId();
      for(int k=0;k<32;k++) {Info["KEY"][k] = derivedKey[k];}
      memcpy(key, derivedKey, sizeof(key));
      serializeJson(Info, str);
      mesh.sendSingle(nodeID, str);
      Serial.print("str ---------> "); Serial.println(str);          
      digitalWrite(notifyLED2, HIGH);
      
      FLAG_j = false;          
      Serial.println("================= SET FLAG --------------------------->[ FALSE ]========================");
    } 
    
  
}

void joinDevice(){  
  FLAG_j = true; 
  Serial.println("================= SET FLAG --------------------------->[ TRUE ]========================");
  digitalWrite(notifyLED, HIGH);   
}

void Verify(){
  store_into_eeprom(nodeID, key);
  Serial.println("================================== verify success ========================================");
  digitalWrite(notifyLED, LOW);
  digitalWrite(notifyLED2, LOW);
}
