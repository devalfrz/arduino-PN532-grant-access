/**************************************************************************/
/*! 
    @file     Mifare_grant_access.pde
    @author   Alfredo Rius
    @license  BSD

    This program is based on the readMifare.pde from Adafruit. It takes the
    first 4 bytes of a Mifare card uid and compares it to flash memory. Since
    it is saved to the flash, it will remember all cards even if the arduino
    is disconnected.

    If the card is registered it will grant access to "something".

    Buttons: (buttons are set to have a pullup resistor so they ar inverted)
      - SLOT_SELECT: Selects a memory slot. LED will blink n times depending
          on the slot that has been selected.
            
      - MEMORY_BUTTON: If it's pressed for 3 seconds while a card is read,
          it will either override the selected slot or delete the card if it
          was already registered.
          

*/
/**************************************************************************/

/**** INPUTS/OUTPUTS ****/
// Adafruit PN532 breakout board:
#define PN532_SCK  2
#define PN532_MOSI 3
#define PN532_SS   4
#define PN532_MISO 5
// Buttons
#define SLOT_SELECT 10
#define MEMORY_BUTTON 9
#define LED 13


/**** Other definitions ****/
#define TOTAL_SLOTS 4 // Arbitrary number of slots available
#define SLOT_SIZE 4
//#define DEBUG // Will show output on terminal



#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <EEPROM.h>


// Adafruit PN532 init SPI
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);


int memory_slot = 0;


void setup(void) {
  pinMode(MEMORY_BUTTON,INPUT_PULLUP);
  pinMode(SLOT_SELECT,INPUT_PULLUP);
  pinMode(LED,OUTPUT);
  
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    #ifdef DEBUG
    Serial.print("Didn't find PN53x board");
    #endif
    digitalWrite(LED,HIGH);
    while (1); // halt
  }
  #ifdef DEBUG
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  #endif
  
  // configure board to read RFID tags
  nfc.SAMConfig();
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;    // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  int i,j;
  boolean save_delete = 0;
  boolean access_granted = 0;
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,100);
  
  save_delete = (success && !digitalRead(MEMORY_BUTTON)) ? 1 : 0;
  
  if (success) {
    digitalWrite(LED,HIGH);
    delay(100);
    digitalWrite(LED,LOW);
    #ifdef DEBUG
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    #endif
    for(i=0;i<TOTAL_SLOTS;i++){ //Check all memory slots
      if(check_card(uid,i))
        access_granted = 1;
    }
    if(save_delete){
      access_granted = 0; //Reset Access
      #ifdef DEBUG
      Serial.println("Continue pressing to save or delete card");
      #endif
      delay(3000);
      if(!digitalRead(MEMORY_BUTTON)){
        if(save_delete_card(uid,memory_slot)){
          #ifdef DEBUG
          Serial.print("Save Card Slot: ");
          Serial.println(memory_slot);
          #endif
          blink_n_times(memory_slot+1,LED);
        }else{
          #ifdef DEBUG
          Serial.println("Delete Card");
          #endif
          digitalWrite(LED,HIGH);
          delay(2000);
          digitalWrite(LED,LOW);
        }
      }
    }
    if(access_granted){
      #ifdef DEBUG
      Serial.println("Access Granted");
      #endif
      





      // DO SOMETHING ELSE
      digitalWrite(LED,HIGH);
      delay(2000);
      digitalWrite(LED,LOW);
      delay(100);

      
     

      
      
      
      
      
      
      for(i=0;i<TOTAL_SLOTS;i++){ // Usefull to know the slot in which the card
                                  //  is saved :) 
        if(check_card(uid,i))
          blink_n_times(i+1,LED);
      }
    }
    else{
      #ifdef DEBUG
      Serial.println("Access Denied");
      #endif
    }
    access_granted = 0; //Reset Access
  }
  if(!digitalRead(SLOT_SELECT)){// Select slot
    while(!digitalRead(SLOT_SELECT));
    memory_slot = (memory_slot < TOTAL_SLOTS - 1) ? memory_slot + 1 : 0;
    #ifdef DEBUG
    Serial.print("Slot selected: ");
    Serial.println(memory_slot);
    #endif
    blink_n_times(memory_slot+1,LED);
  }
  
}

boolean save_delete_card(uint8_t *uid,int addr){
  /*
   * If card doesn't exist in any slot, then it is saved in "addr", 
   *   else it is deleted from memory.
   */
  int i;
  int j;
  int memory_offset = SLOT_SIZE*addr;
  boolean save = 1;
  
  for(i=0;i<TOTAL_SLOTS;i++){ //Check if card is saved in any slot
    if(check_card(uid,i)){
      save = 0; //Delete if found
    }
  }
  if(save){
    for(i=0;i<SLOT_SIZE;i++){
      EEPROM.write(memory_offset+i,uid[i]);
    }
    return(1);
  }else{
    for(i=0;i<TOTAL_SLOTS;i++){ //Delete card from all slots
      if(check_card(uid,i)){
        memory_offset = SLOT_SIZE*i;
        for(j=0;j<SLOT_SIZE;j++){
          EEPROM.write(memory_offset+j,0);
        }
      }
    }
    return(0);
  }
}
boolean check_card(uint8_t *uid,int addr){
  /* 
   *  Checks if card (uid) is contained in the designed address (addr)
   */
  int i;
  int memory_offset = SLOT_SIZE*addr;
  for(i=0;i<SLOT_SIZE;i++){
    if(EEPROM.read(memory_offset+i)!=uid[i]) return 0;
  }
  return 1;
}
void blink_n_times(int n,int led_pin){
  /*
   * Blink n times
   */
  int i;
  for(i=0;i<n;i++){
    digitalWrite(led_pin,HIGH);
    delay(250);
    digitalWrite(led_pin,LOW);
    delay(250);
  }
}
