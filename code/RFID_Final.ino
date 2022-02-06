#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#define SS_PIN 10 //RX slave select
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

const int NUM_CARDS = 14; //total number of cards

//data base of all card ids.
//write as a string of hex characters
String CARD_ID_DATABASE[NUM_CARDS]= {
  "53a4205",//first UID card    
  "8cde414a",//second UID card
  "e2823e9b",// third UID card 
  "e6d8e6b0",//fourth UID card 
  "b27d359b",//Fifth UID card 
  "adcd9fba",// Sixth UID card
  "92b4f31b",//Seventh UID card
  "1dad7aba",//Eight UID card
  "dff51ba",//Nine  UID card
  "82de3e9",//ten UID card
  "2dd342ba", //eleven UID card 
 
};

//names asscocited to the cards in order
String NAME_DATABASE[NUM_CARDS] = {
  "Jason Lawrence",
  "Candan Luliano",
  "Ashwin Swar", 
  "Cameron Rossi",  
  "Zhou Zhou",   
  "John Batchelor",
  "Brandon Gray", 
  "Nathaniel Klint",
  "Ji Yang Luo",
  "Huyen Pham",
  "Justin Tavares", 
  "Blue Card",
  "White Card"
};

//array to hold the ID of cards already read during the day
String cards_already_read[NUM_CARDS];

int const RedLed=4;
int const GreenLed=5;
int const Buzzer=2;

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  
  //Serial.println("CLEARSHEET"); // clears starting at row 1
  Serial.println("CLEARRANGE,A,1,E,1000");
  Serial.println("LABEL, Date,Time, Student Name, Student Index, Student User ID");// make four columns (Date,Time,[Name:"user name"]line 48 & 52,[Number:"user number"]line 49 & 53)

  pinMode(RedLed,OUTPUT);
  pinMode(GreenLed,OUTPUT);
  pinMode(Buzzer,OUTPUT);
  noTone(Buzzer);
  lcd.begin(16,2); // Turn on the blacklight and print a message.  
}
   
void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {      // new tag is available
    lcd.clear();
    bool card_in_database = false;            //this variable checks if the new card already in the data base
    if (mfrc522.PICC_ReadCardSerial()) {      // NUID has been read
      //MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); //this is to print the type of card
      //Serial.print("RFID/NFC Tag Type: ");
      //Serial.println(rfid.PICC_GetTypeName(piccType));

      String user_id = read_card();           //read the user_id
      int index;                              //index of the uder in the database
      String student_name;                    //name of the student with the user_id

      if(is_already_read(cards_already_read, user_id)==true){
        digitalWrite(GreenLed,LOW);
        digitalWrite(RedLed,HIGH);
        lcd.setCursor(0,0);
        lcd.print("ALREADY");
        lcd.setCursor(0,1);
        lcd.print("REGISTERED");
        digitalWrite(RedLed, HIGH);  
        tone(Buzzer, 200);  
        delay(3000);  
        digitalWrite(RedLed, LOW);  
        noTone(Buzzer);
        delay(3000);
        lcd.clear();
        return;
      }

      //go through the database and check if the card is there
      //if it is then print the results and buzzer
      for(int i=0; i<NUM_CARDS; i++){
        if(CARD_ID_DATABASE[i] == user_id){
          //card found in database
          card_in_database = true;
          index = i;
          student_name = NAME_DATABASE[i];
          //Serial.println("found");
          Serial.print("DATA,DATE,TIME," + student_name); //send the Name to excel
          Serial.print(",");
          Serial.print(index);                //send the Number to excel
          Serial.print(",");
          Serial.println(user_id);            //send the hex string to excel
          lcd.print(student_name);
          lcd.setCursor(0,1);
          lcd.print("Present");
          tone(Buzzer, 500);  
          delay(300);  
          noTone(Buzzer);
          digitalWrite(GreenLed,HIGH);
          digitalWrite(RedLed,LOW);
          digitalWrite(Buzzer,HIGH);
          delay(30);
          digitalWrite(Buzzer,LOW);
          Serial.println("EE193,Names/WorkNames");
        }

      }
      //if the new card is not in database then print in LCD "Not Registered"
      if(!card_in_database){
        digitalWrite(GreenLed,LOW);
        digitalWrite(RedLed,HIGH);
        lcd.setCursor(0,0);
        lcd.print("NOT REGISTERED");
        lcd.setCursor(0,1);
        lcd.print("GO SEE THE DEAN");
        digitalWrite(RedLed, HIGH);  
        tone(Buzzer, 200);  
        delay(3000);  
        digitalWrite(RedLed, LOW);  
        noTone(Buzzer);
      }
      mfrc522.PICC_HaltA(); // halt PICC
      mfrc522.PCD_StopCrypto1(); // stop encryption on PCD
      }
  }
  //if no new card then just wait for new card
  else{
    lcd.setCursor(3,0);  
    lcd.print("SHOW YOUR");  
    lcd.setCursor(4,1);  
    lcd.print("ID CARD");
    return;//got to start of loop if there is no card present
  }
cont:
delay(200);
digitalWrite(GreenLed,LOW);
digitalWrite(RedLed,LOW);
lcd.clear();

//if you want to close the Excel when all card had detected and save Excel file in Names Folder. in my case i have just 2 card (optional)
//if(n==10){
    
  //  Serial.println("FORCEEXCELQUIT");
 //   
 
}



//function to read the card and return a string of the user_id
String read_card(){
  String user_id;
  for (int i = 0; i < mfrc522.uid.size; i++) {
        user_id += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(user_id);
  Serial.println();
  return user_id;
}

//function to check if the user_id is already recorderd for the day
bool is_already_read(String *cards_already_read, String user_id){
  int empty_index = 0;
  for(int i=0; i<NUM_CARDS; i++){
    if(cards_already_read[i] == ""){
      empty_index = i;
      break;
    }
    if(cards_already_read[i] == user_id){
      return true;
    }
  }
  cards_already_read[empty_index] = user_id;
  return false;
}
