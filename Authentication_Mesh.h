#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define FLAG_t 0
#define len_key 32
int address;
#include <EEPROM.h>

typedef struct {   
   bool flag;
   uint32_t id;
   uint8_t derivedKey[len_key];
} info_child;

//int _index = 0;
//byte r;
/*
typedef struct {
   int index;
   bool flag;
   uint32_t id;
   uint8_t derivedKey[32];
  // float val_d;
} info_child;
*/
void reset_table_index(info_child child[],int N)
{
    for(int i=0;i<N;i++){
   child[i].flag=FLAG_t;
        
    }
}


    
int store_info_child(info_child child[],uint32_t _id,int NUM,uint8_t derivedKey[])
{   
    //int child_index=0;
    for (int i=0;i<NUM;i++){
        if( child[i].flag==0){
    child[i].flag=1;
    //child[_index].index=_index;
    child[i].id=_id;
    for (int j=0;j<len_key;j++){
        child[i].derivedKey[j]=derivedKey[j];
    }
    return i; //number node in mesh
    }
}
return 0;//false
}



int request_key(info_child child[],int NUM,uint32_t _id,uint8_t *derivedKey)
{   
    //int child_index=0;
for (int i=0;i<NUM;i++){
    if (child[i].flag==1)
    {if (child[i].id==_id){
            for (int j = 0;j<len_key;j++){
                 *derivedKey=child[i].derivedKey[j];
                 derivedKey=derivedKey+1;
        } 
        
        
        return i;
    }
    }
}

return 9999;
}

void show_info_child(info_child child[],int NUM){
    for (int i=0;i<NUM;i++){
    if (child[i].flag==1)
{        printf("\n ---- %d----",i);
        printf("\n index: %d",i);
        printf("\n id: %u",child[i].id);
        printf("\n derivedKey: 0x");
    for(int j=0;j<len_key;j++){    
      printf("%02x",child[i].derivedKey[j]);}

      
        printf("\n ---------------------");

    }

}
printf("----------END--------\n");

}

bool compare_HMAC(uint8_t HMAC1[],char HMAC2[]){
char HMAC1_[100];
sprintf(HMAC1_, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    HMAC1[0],HMAC1[1],HMAC1[2],HMAC1[3],HMAC1[4],HMAC1[5],
    HMAC1[6],HMAC1[7],HMAC1[8],HMAC1[9],HMAC1[10],HMAC1[11],
    HMAC1[12],HMAC1[13],HMAC1[14],HMAC1[15],HMAC1[16],HMAC1[17],
    HMAC1[18],HMAC1[19],HMAC1[20],HMAC1[21],HMAC1[22],HMAC1[23],
    HMAC1[24],HMAC1[25],HMAC1[26],HMAC1[27],HMAC1[28],HMAC1[29],
    HMAC1[30],HMAC1[31]);
    printf("\n--HMAC1---");
    printf(HMAC1_);
    printf("\n---HMAC2---");
    printf(HMAC2);
    printf("------\n");

  if(strcmp(HMAC1_,HMAC2)==0)
    {
          printf("equal\n");
      return 1;
    }
    
  else {
              printf("not equal\n");
    return 0;
  }
}

void store_into_eeprom(uint32_t nodeID, uint8_t derivedKey[32]){
   int address=0;
   int i = 0;  
   printf(" start EEPROM ");      
   byte flag = 0;
   bool FLAG_;
   uint32_t ID;
   uint8_t KEY[32];
   for(int i = 0;i<32;i++){ KEY[i] = derivedKey[i];}
   EEPROM.get(address,  flag);
    while(1){
       if(flag == 0){ 
           EEPROM.put(address, 1); //flag
           EEPROM.get(address, FLAG_);
           address++;  
         
           EEPROM.put(address, nodeID);
           EEPROM.get(address, ID);
           address = address + 4;   
           EEPROM.put(address, KEY);
           EEPROM.get(address, KEY);
           printf("\n genkey index:%d-->",i);
           for(int x = 0;x<32;x++){ 
           printf("%02x|", KEY[x]); 
           }
          printf("\n -----");
           EEPROM.commit(); 
           address = address + 32;  
           break;
        }
         i++;
         address = 37 * i; 
         EEPROM.get(address, flag);
         if (address == 37*100){
          printf("------->>>buffer overload<<<---------");
          break;
         }
       }  
}

bool getTable(info_child child[]){
    int address = 0;
    int i = 0;
    printf("\n start get table");     
    EEPROM.get(address,child[i].flag );
    while (child[i].flag > 0){ 
        address++;
        EEPROM.get(address, child[i].id);
        address = address + 4; 
;
        EEPROM.get(address, child[i].derivedKey);
  
        i=i+1;
        address=37*i;
      //printf("\n -------");
      EEPROM.get(address,child[i].flag );
    }
    printf("\n Get Table success \n"); 
    return 1;
}