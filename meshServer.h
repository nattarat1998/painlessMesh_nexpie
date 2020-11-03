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
   char area[20];
   char attr[3][10];
} info_child;

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
    if (child[i].flag==1){        
        printf("\n ---- %d----",i);
        printf("\n index: %d",i);
        printf("\n id: %u",child[i].id);
        printf("\n derivedKey: 0x");
        for(int j=0;j<len_key;j++){    
            printf("%02x",child[i].derivedKey[j]);
        }
        printf("\n area: %s ", child[i].area);       
        printf("\n attr(0): %s", child[i].attr[0]);
        printf("\n attr(1): %s", child[i].attr[1]);
        printf("\n attr(2): %s", child[i].attr[2]);
        
    }

}
printf("\n ----------END--------\n");

}

bool compare_HMAC(uint8_t HMAC1[],uint8_t HMAC2[]){
  for (int i=0;i<2;i++){
    if (HMAC1[i]!=HMAC2[i]){
        return 0;
    }
  }           
  return 1;
  
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

void map_attribute(info_child child[],int NUM,uint32_t _id, char attr_[][10], char area_[]){
   for (int i=0;i<NUM;i++){
        if (child[i].flag==1){
            if (child[i].id==_id){               
                for(int j=0;j<3;j++){
                    strcpy(child[i].attr[j],attr_[j]);
                }
                 strcpy(child[i].area, area_);     
            }        
        }
    } 
}




