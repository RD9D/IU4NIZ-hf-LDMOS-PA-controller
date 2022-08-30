/* 
 *  Firmware Author: Lucio Agnelli IU4NIZ
 *  data: 24.02.2022
*/

/*
Banda 1: 6m
Banda 2: 10-12m
Banda 3: 15-17m
Banda 4: 20-30m
Banda 5: 40-60m
Banda 6: 80m
Banda 7: 160m 
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>
#include <EEPROM.h>
//#include <DueFlashStorage.h>
#include "EasyNextionLibrary.h" // Include EasyNextionLibrary
#include "pins.h"
#include "EEPROM.h"

EasyNex myNex(Serial3);
OneWire oneWireTempS1(TEMP_SENSOR1_PIN); // Imposta la connessione OneWire Sensore 1
OneWire oneWireTempS2(TEMP_SENSOR2_PIN); // Imposta la connessione OneWire Sensore 2
OneWire oneWireTempS3(TEMP_SENSOR3_PIN); // Imposta la connessione OneWire Sensore 3
DallasTemperature tempSens1TMP(&oneWireTempS1); // Dichiarazione dell'oggetto sensore 1
DallasTemperature tempSens2TMP(&oneWireTempS2); // Dichiarazione dell'oggetto sensore 2
DallasTemperature tempSens3TMP(&oneWireTempS3); // Dichiarazione dell'oggetto sensore 3
NonBlockingDallas tempSens1(&tempSens1TMP);    //Create a new instance of the NonBlockingDallas class
NonBlockingDallas tempSens2(&tempSens2TMP);    //Create a new instance of the NonBlockingDallas class
NonBlockingDallas tempSens3(&tempSens3TMP);    //Create a new instance of the NonBlockingDallas class

//Antenna 1 = Antenna A - Antenna 2 =  Antenna B
int antenna = 1; //Seleziono per default l'antenna A
int band = 2; //Seleziono per default la banda dei 10m
boolean operate = false;
boolean errorTemp = false; //Errore temperatura non attivato per default
const int REFRESH_TIME = 10;
unsigned long refresh_timer = millis();
int nexGreenColor = 6112;
int nexRedColor = 63488;
float celsiusTempS1 = 0;
float celsiusTempS2 = 0;
float celsiusTempS3 = 0;
float powerFW = 0;
float vswr = 0;
float currentPallet = 0;
float vccMain = 0;
float vccAux = 0;
int velMinInt = 0;
int velMaxInt = 150;
int velMinExt= 0;
int velMaxExt = 255;
int tempMin = 0;
int tempMax = 75;
int multi = 0;  
int fanSpeed = 0;
int fanMode = 1;
int currentPage = 0;
String tempGlobalTxt = "";
String tempRFTxt = "";
boolean alarmSWR = false;
boolean alarmPWR = false;
boolean alarmIPA = false;
boolean masterAlarm = false;
int counterAlarmSWR = 0;
int counterAlarmPWR = 0;
int counterAlarmIPA = 0;
int bandDec = 0;
#define TEMPERATURE_TIME_INTERVAL 1500 //Intervallo lettura sensori temperatura

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;

void setup(void)
{
  delay(15000);
  Serial.begin(250000); // Inizializzazione della seriale di servizio
  myNex.begin(250000); // Inizializzazione seriale display Nextion
  startMillis = millis();  //initial start time
  

  //FIRST STARTUP EEPROM FILLING
  if(EEPROM.read(0) == 255){
    EEPROM.write(0,1); //Segno che è stato effettuato il primo avvio
    EEPROM.write(1,1); //Salvo l'antenna 
    EEPROM.write(2,2); //Salvo la banda
    EEPROM.write(3,0); //Salvo il band decoder disabilitato

    EEPROM.write(4,fanMode); //FAN MODE: 1 Normal | 2 DX | 3 Contest
    
    //DEFAULT VALUE FANMODE1 - NORMAL
    EEPROM.write(10,velMinInt); //Salvo la velocità minima ventole interne
    EEPROM.write(15,velMaxInt); //Salvo la velocità massima ventole interne
    EEPROM.write(20,velMinExt); //Salvo la velocità minima ventole esterne
    EEPROM.write(25,velMaxExt); //Salvo la velocità massima ventole esterne
    EEPROM.write(30,tempMax); //Salvo temperatura di allarme

    //DEFAULT VALUE FANMODE2 - DX
    EEPROM.write(35,20); //Salvo la velocità minima ventole interne
    EEPROM.write(40,190); //Salvo la velocità massima ventole interne
    EEPROM.write(45,50); //Salvo la velocità minima ventole esterne
    EEPROM.write(50,255); //Salvo la velocità massima ventole esterne
    EEPROM.write(55,70); //Salvo temperatura di allarme

    //DEFAULT VALUE FANMODE3 - CONTEST
    EEPROM.write(60,100); //Salvo la velocità minima ventole interne
    EEPROM.write(65,190); //Salvo la velocità massima ventole interne
    EEPROM.write(70,100); //Salvo la velocità minima ventole esterne
    EEPROM.write(75,255); //Salvo la velocità massima ventole esterne
    EEPROM.write(80,75); //Salvo temperatura di allarme
  }
  

  //TEMPERATURE SENSOR SETUP
  tempSens1.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, TEMPERATURE_TIME_INTERVAL); // Inizializzazione del sensore 1
  tempSens2.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, TEMPERATURE_TIME_INTERVAL); // Inizializzazione del sensore 2
  tempSens3.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, TEMPERATURE_TIME_INTERVAL); // Inizializzazione del sensore 3
  tempSens1.onIntervalElapsed(readTemp1);
  tempSens2.onIntervalElapsed(readTemp2);
  tempSens3.onIntervalElapsed(readTemp3);
  tempSens1.requestTemperature();
  tempSens2.requestTemperature();
  tempSens3.requestTemperature();

  //PIN IN OUT DEFINITION
  pinMode(FAN_PWM_PIN,OUTPUT);
  pinMode(AUX1_PWM,OUTPUT);
  pinMode(ALARM_PWR_PIN,INPUT);
  pinMode(ALARM_SWR_PIN,INPUT);
  pinMode(ALARM_CURR_PIN,INPUT);

  pinMode(ANT_SWITCH_PIN,INPUT);
  pinMode(BAND_SWITCH_PIN,INPUT);
  pinMode(OPR_SWITCH_PIN,INPUT);
  pinMode(RESET_SWICTH_PIN,INPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(ANT_SW_LED,OUTPUT);
  pinMode(BAND_SW_LED,OUTPUT);
  pinMode(OPR_SW_LED,OUTPUT);
  pinMode(RESET_SW_LED,OUTPUT);

  pinMode(ANT_SEL_RELE_PIN,OUTPUT);
  pinMode(RESET_RELE_PIN,OUTPUT);

  pinMode(VCC50_LOCK_RELE_PIN,OUTPUT);
  pinMode(BIAS_LOCK_RELE_PIN,OUTPUT);
  pinMode(PTT_LOCK_RELE_PIN,OUTPUT);

  pinMode(LPF_6M_PIN,OUTPUT);
  pinMode(LPF_10M_PIN,OUTPUT);
  pinMode(LPF_15M_PIN,OUTPUT);
  pinMode(LPF_20M_PIN,OUTPUT);
  pinMode(LPF_40M_PIN,OUTPUT);
  pinMode(LPF_80M_PIN,OUTPUT);
  pinMode(LPF_160M_PIN,OUTPUT);

  pinMode(TX_SIGNAL_INPUT_PIN,INPUT);
  pinMode(RX_SIGNAL_INPUT_PIN,INPUT);
  pinMode(DIGI_AUX1_IN_PIN,INPUT);
  pinMode(DIGI_AUX2_IN_PIN,INPUT);
  pinMode(DIGI_AUX3_IN_PIN,INPUT);

  pinMode(BAND_DATA_A,INPUT);
  pinMode(BAND_DATA_B,INPUT);
  pinMode(BAND_DATA_C,INPUT);
  pinMode(BAND_DATA_D,INPUT);
  
  digitalWrite(BAND_DATA_A,LOW);
  digitalWrite(BAND_DATA_B,LOW);
  digitalWrite(BAND_DATA_C,LOW);
  digitalWrite(BAND_DATA_D,LOW);
  
  digitalWrite(ANT_SEL_RELE_PIN,LOW);
  digitalWrite(RESET_RELE_PIN,LOW);

  lockPTT(1);
  lockVCC50(1);
  lockBIAS(1);

  myNex.writeStr("page main");
  digitalWrite(BUZZER_PIN,HIGH);
  digitalWrite(ANT_SW_LED,HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN,LOW);
  digitalWrite(BAND_SW_LED,HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN,HIGH);
  digitalWrite(OPR_SW_LED,HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN,LOW);
  digitalWrite(RESET_SW_LED,HIGH);
  delay(50);
  digitalWrite(ANT_SW_LED,LOW);
  digitalWrite(BAND_SW_LED,LOW);
  digitalWrite(OPR_SW_LED,LOW);
  digitalWrite(RESET_SW_LED,LOW);

  //myNex.writeStr("statePA.txt","RX");
  //myNex.writeNum("statePA.pco",6112);
  //myNex.writeStr("statePTT.txt","LOCK");
  //myNex.writeNum("statePTT.pco",63488);

  digitalWrite(ANT_SW_LED,HIGH);
  digitalWrite(BAND_SW_LED,HIGH);
  digitalWrite(OPR_SW_LED,LOW);

  //RECALL EEPROM ON STARTUP
  if(EEPROM.read(0)==1){
  //Primo avvio già effettuato
    fanMode = EEPROM.read(4);
    antenna = EEPROM.read(1);
    checkAntenna(antenna);
    band = EEPROM.read(2);
    bandDec = EEPROM.read(3);
    if(bandDec == 0){
      selectBand(band);
    }
    
    //STAMPO IN SERIALE LA EEPROM
    /*
    Serial.println("----------------------------------- EEPROM SETTINGS -----------------------------------------------");
    Serial.println("SETUP\tANTENNA\tBAND\tDECODER\tMODE");
    Serial.print(EEPROM.read(0));
    Serial.print("\t");
    Serial.print(EEPROM.read(1));
    Serial.print("\t");
    Serial.print(EEPROM.read(2));
    Serial.print("\t");
    Serial.print(EEPROM.read(3));
    Serial.print("\t");
    Serial.println(EEPROM.read(4));
    Serial.println("---------------------------[ NORMAL SETTINGS]---------------------------------");
    Serial.println("Vel Min int\tVel Max Int\tVel Min Ext\tVel Max Ext\tMax Temp");
    Serial.print(EEPROM.read(10));
    Serial.print("\t");
    Serial.print(EEPROM.read(15));
    Serial.print("\t");
    Serial.print(EEPROM.read(20));
    Serial.print("\t");
    Serial.print(EEPROM.read(25));
    Serial.print("\t");
    Serial.println(EEPROM.read(30));
    Serial.println("---------------------------[ DX SETTINGS]---------------------------------");
    Serial.println("Vel Min int\tVel Max Int\tVel Min Ext\tVel Max Ext\tMax Temp");
    Serial.print(EEPROM.read(35));
    Serial.print("\t");
    Serial.print(EEPROM.read(40));
    Serial.print("\t");
    Serial.print(EEPROM.read(45));
    Serial.print("\t");
    Serial.print(EEPROM.read(50));
    Serial.print("\t");
    Serial.println(EEPROM.read(55));
    Serial.println("---------------------------[ CONTEST SETTINGS]---------------------------------");
    Serial.println("Vel Min int\tVel Max Int\tVel Min Ext\tVel Max Ext\tMax Temp");
    Serial.print(EEPROM.read(60));
    Serial.print("\t");
    Serial.print(EEPROM.read(65));
    Serial.print("\t");
    Serial.print(EEPROM.read(70));
    Serial.print("\t");
    Serial.print(EEPROM.read(75));
    Serial.print("\t");
    Serial.println(EEPROM.read(80));
    */
    //checkFans(true);
    //delay(3000);
    //checkFans(false);
  }
}

void loop()
{  
    //READING TEMPERATURE FROM SENSOR
    tempSens1.update();
    tempSens2.update();
    tempSens3.update();
    
    //PREPARE VARS FOR DISPLAY TEMPERATURE TEXT
    tempGlobalTxt = String(celsiusTempS3)+"C";
    tempRFTxt = String((celsiusTempS2+celsiusTempS1)/2)+"C";

    //UPDATE BAND FROM EEPROM
    band = EEPROM.read(2);

    //LOGGING TEMP
    /*
    Serial.println("-------------------------------------------------------------------------------------------");
    Serial.print("Sens1:");
    Serial.print(celsiusTempS1);
    Serial.print(" |\t");
    Serial.print("Sens2:");
    Serial.print(celsiusTempS2);
    Serial.print(" |\t");
    Serial.print("Sens3:");
    Serial.print(celsiusTempS3);
    Serial.print(" |\t");
    Serial.print(" FAN Vel:");
    Serial.print(fanSpeed);
    Serial.print(" |\t");
    */
    //UPDATE NEXTION PAGE ID---------------------------------------------
    currentPage = myNex.readNumber("dp");
    //currentPage =  1;

    //UPDATE PAGES----------------------------------------------
    updateMainData(); //Main dashboard
    systemMonitorUpdate(); //System Monitor data update
    
    //Antenna Soft Switch Trigger----------------------------------------
    if(currentPage == 1){

      if(operate == true){
        myNex.writeStr("b3.txt","OPERATE");
        myNex.writeNum("b3.pic",16);
        myNex.writeNum("b3.pic2",16);         
      }else{
        myNex.writeStr("b3.txt","STANDBY");
        myNex.writeNum("b3.pic",17);
        myNex.writeNum("b3.pic2",17);
      }

      
      if(fanMode == 1){
        myNex.writeStr("labelFanmode.txt","NORMAL");
        myNex.writeNum("labelFanmode.bco",2047);
      }else if(fanMode == 2){
        myNex.writeStr("labelFanmode.txt","DX");
        myNex.writeNum("labelFanmode.bco",65412);
      }else if(fanMode == 3){
        myNex.writeStr("labelFanmode.txt","CONTEST");
        myNex.writeNum("labelFanmode.bco",63488);
      }
      
      if(myNex.readNumber("antPressed.val")==1  && operate==false){
        if(antenna == 1){
          antenna = 2;
          checkAntenna(antenna);
        }else if(antenna == 2){
          antenna = 1;
          checkAntenna(antenna);
        }
        
      }
      if(myNex.readNumber("operatePressed.val")==1){
        if(operate == false){
          operate = true;
          checkOperate(operate);
        }else if(operate == true){
          operate = false;
          checkOperate(operate);
        }
      }
      //Check the RXTX STATE-----------------------------------------------
      checkStatePA();
    }

    //Band soft switchs
    if(currentPage == 2){
      if(myNex.readNumber("bandSel.val")>0  && operate==false){
        if(myNex.readNumber("bandSel.val")==6){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 1;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==10){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 2;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==10){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 2;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==15){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 3;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==20){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 4;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==40){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 5;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==80){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 6;
          selectBand(band);
        }else if(myNex.readNumber("bandSel.val")==160){
          if(EEPROM.read(3) == 1){
            EEPROM.write(3,0);
          }
          band = 7;
          selectBand(band);
        }
      }
      if(myNex.readNumber("bandSel.val")==99){
          EEPROM.write(3,1);
          buzzConfirm();
          digitalWrite(BAND_SW_LED,LOW);
          delay(50);
          digitalWrite(BAND_SW_LED,HIGH);
          delay(50);
          digitalWrite(BAND_SW_LED,LOW);
          delay(50);
          digitalWrite(BAND_SW_LED,HIGH);
          delay(50);
          if(currentPage!=1){
            myNex.writeStr("page main");
          }
      }
    }

    //Settings page
    if(currentPage == 3){

      
      
      if(myNex.readNumber("loadsettings.val")==1){
        myNex.writeNum("loadsettings.val",0);
        checkFanMode();
      }
      if(myNex.readNumber("changeFanMode.val")==1){
        if(myNex.readStr("fanModeTxt.txt")=="NORMAL"){
          fanMode = 1;
          EEPROM.write(4,fanMode);
          myNex.writeNum("changeFanMode.val",0);
          buzzConfirm();
        }else if(myNex.readStr("fanModeTxt.txt")=="DX"){
          fanMode = 2;
          EEPROM.write(4,fanMode);
          myNex.writeNum("changeFanMode.val",0);
          buzzConfirm();
        }else if(myNex.readStr("fanModeTxt.txt")=="CONTEST"){
          fanMode = 3;
          EEPROM.write(4,fanMode);
          myNex.writeNum("changeFanMode.val",0);
          buzzConfirm();
        }
        checkFanMode();
      }
      
      //SAVE SETTINGS
      
      if(myNex.readNumber("savestatus.val")==1){
        Serial.println("[SAVE SETTINGS]");
        int velminint_val = myNex.readNumber("velminint.val");
        int velmaxint_val = myNex.readNumber("velmaxint.val");
        int velminext_val = myNex.readNumber("velminext.val");
        int velmaxext_val = myNex.readNumber("velmaxext.val");
        int tempmax_val = myNex.readNumber("tempmax.val");
        
        
        if(fanMode == 1){
          EEPROM.write(10,velminint_val);
          EEPROM.write(15,velmaxint_val);
          EEPROM.write(20,velminext_val);
          EEPROM.write(25,velmaxext_val);
          EEPROM.write(30,tempmax_val);
        }else if(fanMode == 2){
          EEPROM.write(35,velminint_val);
          EEPROM.write(40,velmaxint_val);
          EEPROM.write(45,velminext_val);
          EEPROM.write(50,velmaxext_val);
          EEPROM.write(55,tempmax_val);
        }else if(fanMode == 3){
          EEPROM.write(60,velminint_val);
          EEPROM.write(65,velmaxint_val);
          EEPROM.write(70,velminext_val);
          EEPROM.write(75,velmaxext_val);
          EEPROM.write(80,tempmax_val);
        }
        myNex.writeNum("savestatus.val",0);
        buzzConfirm();
      }
    }

    if(currentPage == 9){
      if(myNex.readNumber("rstConfirm.val")==1){
        factoryReset();
        myNex.writeStr("page alertReboot");
        operate = false;
        checkOperate(operate);
      }
    }

    if(currentPage == 10){
      buzzConfirm();
    }
    
    //When band is auto
    if(EEPROM.read(3)==1){
      bandAutoSelect();
      if(currentPage == 1){
        myNex.writeStr("labelBandDec.txt","AUTO");
        myNex.writeNum("labelBandDec.bco",2016);
      }
    }else{
      if(currentPage == 1){
        myNex.writeStr("labelBandDec.txt","MANUAL");
        myNex.writeNum("labelBandDec.bco",65412);
      }
    }
    
    //Band fisical switch
    if(digitalRead(BAND_SWITCH_PIN)==LOW && band!=0 && operate==false){
      if(band>0 && band<7){
        band++;
        EEPROM.write(3,0);
        selectBand(band);
      }else if(band==7){
        band=1;
        EEPROM.write(3,0);
        selectBand(band);
      }
    }
    //Check temperatures
    tempCheck();

    //Ant fisical switch
    if(digitalRead(ANT_SWITCH_PIN)==LOW && operate==false){
      if(antenna == 1){
        antenna = 2;
        checkAntenna(antenna);
      }else if(antenna == 2){
        antenna = 1;
        checkAntenna(antenna);
      }
      
    }
    //Operate fisical switch
    if(digitalRead(OPR_SWITCH_PIN)==LOW){
      if(operate == false){
        operate = true;
        checkOperate(operate);
      }else if(operate == true){
        operate = false;
        checkOperate(operate);
      }
    }

    checkAlarms();
    if(masterAlarm){
      //Reset fisical switch
      if(digitalRead(RESET_SWICTH_PIN)==LOW){
        doReset();
      }
      if(alarmSWR){
        myNex.writeStr("page errorSWR"); 
      }else if(alarmPWR){
        myNex.writeStr("page errorPWR"); 
      }else if(alarmIPA){
        myNex.writeStr("page errorIPA"); 
      }
    }
    
    
    //myNex.NextionListen();
    /*
    Serial.println();

    Serial.println("============================================================================");
    Serial.print("BAND DATA A: ");
    Serial.print(digitalRead(BAND_DATA_A));
    Serial.print("\tBAND DATA B: ");
    Serial.print(digitalRead(BAND_DATA_B));
    Serial.print("\tBAND DATA C: ");
    Serial.print(digitalRead(BAND_DATA_C));
    Serial.print("\tBAND DATA D: ");
    Serial.print(digitalRead(BAND_DATA_D));
    Serial.println();
    Serial.println("============================================================================");
    */
    
    delay(10);
}


//Invoked at every sensor reading (TIME_INTERVAL milliseconds)
void readTemp1(float temperature, bool valid, int deviceIndex){
  celsiusTempS1 = temperature;
}


//Invoked at every sensor reading (TIME_INTERVAL milliseconds)
void readTemp2(float temperature, bool valid, int deviceIndex){
  celsiusTempS2 = temperature;
}

//Invoked at every sensor reading (TIME_INTERVAL milliseconds)
void readTemp3(float temperature, bool valid, int deviceIndex){
  celsiusTempS3 = temperature;
}
void checkAlarms(){
  if(digitalRead(ALARM_PWR_PIN) == HIGH && operate == true){
    counterAlarmPWR++;
    if(counterAlarmPWR >= 15){
      alarmPWR = true;
      masterAlarm = true;
      lockPTT(1);
      lockVCC50(1);
      lockBIAS(1);
      digitalWrite(BUZZER_PIN,HIGH);
      digitalWrite(RESET_SW_LED,HIGH);
      Serial.println(counterAlarmPWR);
    }
  }else if(digitalRead(ALARM_SWR_PIN) == HIGH && operate == true){
    counterAlarmSWR++;
    if(counterAlarmSWR >= 15){
      alarmSWR = true;
      masterAlarm = true;
      lockPTT(1);
      lockVCC50(1);
      lockBIAS(1);
      digitalWrite(BUZZER_PIN,HIGH);
      digitalWrite(RESET_SW_LED,HIGH);
      Serial.println(counterAlarmSWR);
    }
  }else if(digitalRead(ALARM_CURR_PIN) == HIGH && operate == true){
    counterAlarmIPA++;
    if(counterAlarmIPA >= 15){
      alarmIPA = true;
      masterAlarm = true;
      lockPTT(1);
      lockVCC50(1);
      lockBIAS(1);
      digitalWrite(BUZZER_PIN,HIGH);
      digitalWrite(RESET_SW_LED,HIGH);
      Serial.println(counterAlarmIPA);
    }
  }else{
    alarmSWR = false;
    alarmPWR = false;
    alarmIPA = false;
    masterAlarm = false;
    digitalWrite(RESET_SW_LED,LOW);
    digitalWrite(BUZZER_PIN,LOW);
    counterAlarmPWR = 0;
    counterAlarmSWR = 0;
    counterAlarmIPA = 0;
  }
}
void checkFanMode(){
  if(fanMode == 1){
    myNex.writeStr("fanModeTxt.txt","NORMAL");
    myNex.writeNum("fanModeTxt.bco",2047);
    myNex.writeNum("velminint.val",EEPROM.read(10));
    myNex.writeNum("velmaxint.val",EEPROM.read(15));
    myNex.writeNum("velminext.val",EEPROM.read(20));
    myNex.writeNum("velmaxext.val",EEPROM.read(25));
    myNex.writeNum("tempmax.val",EEPROM.read(30));
    
  }else if(fanMode == 2){
    myNex.writeStr("fanModeTxt.txt","DX");
    myNex.writeNum("fanModeTxt.bco",65412);
    myNex.writeNum("velminint.val",EEPROM.read(35));
    myNex.writeNum("velmaxint.val",EEPROM.read(40));
    myNex.writeNum("velminext.val",EEPROM.read(45));
    myNex.writeNum("velmaxext.val",EEPROM.read(50));
    myNex.writeNum("tempmax.val",EEPROM.read(55));
    
  }else if(fanMode == 3){
    myNex.writeStr("fanModeTxt.txt","CONTEST");
    myNex.writeNum("fanModeTxt.bco",63488);
    myNex.writeNum("velminint.val",EEPROM.read(60));
    myNex.writeNum("velmaxint.val",EEPROM.read(65));
    myNex.writeNum("velminext.val",EEPROM.read(70));
    myNex.writeNum("velmaxext.val",EEPROM.read(75));
    myNex.writeNum("tempmax.val",EEPROM.read(80));
    
  }
}
void doReset(){
  if(operate == true){
    operate = false;
    alarmSWR = false;
    alarmPWR = false;
    alarmIPA = false;
    masterAlarm = false;
    checkOperate(operate);
    myNex.writeStr("page main");
  }
  digitalWrite(RESET_RELE_PIN,HIGH);
  digitalWrite(BUZZER_PIN,HIGH);
  delay(1000);
  digitalWrite(RESET_RELE_PIN,LOW);
  digitalWrite(BUZZER_PIN,LOW);
}

void factoryReset(){
  EEPROM.write(0,255);
}
void tempCheck(){
  //TEMPERATURE/VELOCITY CALCULATION  ---------------------------------------------------------------------

    //READ FAN MODE
    if(fanMode == 1){
      velMinInt = EEPROM.read(10);
      velMaxInt = EEPROM.read(15);
      velMinExt = EEPROM.read(20);
      velMaxExt = EEPROM.read(25);
      tempMax = EEPROM.read(30);
    }else if(fanMode == 2){
      velMinInt = EEPROM.read(35);
      velMaxInt = EEPROM.read(40);
      velMinExt = EEPROM.read(45);
      velMaxExt = EEPROM.read(50);
      tempMax = EEPROM.read(55);
    }else if(fanMode == 3){
      velMinInt = EEPROM.read(60);
      velMaxInt = EEPROM.read(65);
      velMinExt = EEPROM.read(70);
      velMaxExt = EEPROM.read(75);
      tempMax = EEPROM.read(80);
    }
    
    
    int rfTemp = ((celsiusTempS1+celsiusTempS2)/2);
    int multiRFFan = (velMaxInt-velMinInt)/(tempMax-tempMin);
    int multiExtFan = (velMaxExt-velMinExt)/(tempMax-tempMin);
    
    if(celsiusTempS3 > rfTemp){
      tempMin = 20;
    }else{
      tempMin = celsiusTempS3;
    }
    fanSpeed = ((rfTemp-tempMin)*multiRFFan)+velMinInt;
    int fanExtSpeed = ((rfTemp-tempMin)*multiExtFan)+velMinExt;
    
    if(fanSpeed<0){
      fanSpeed = 0;
    }
    if(fanExtSpeed<0){
      fanExtSpeed = 0;
    }
    //-------------------------------------------------------------------------------------------------------
  if(errorTemp == false){
    if(operate == true){
      analogWrite(FAN_PWM_PIN,fanSpeed);
      analogWrite(AUX1_PWM,fanExtSpeed);
    }else{
      analogWrite(FAN_PWM_PIN,0);
      analogWrite(AUX1_PWM,0);
    }
  }else{
    myNex.writeStr("page errorTemp");
    digitalWrite(FAN_PWM_PIN,HIGH);
    digitalWrite(AUX1_PWM,HIGH);
    operate = false;
    checkOperate(operate);
    
    digitalWrite(BUZZER_PIN,HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN,LOW);
    delay(500);
    if((rfTemp-celsiusTempS3)<10){
      errorTemp = false;
      myNex.writeStr("page main");
    }
  }
  //Errore temperatura attivato
  
  if(rfTemp>tempMax){
    errorTemp = true;
  }
}

void systemMonitorUpdate(){
  //Serial.print("Page id:");
  //Serial.print(currentPage);
  //Serial.print(" |\t");
  if(currentPage == 4){
    //Monitor
    myNex.writeNum("tempGlobalVar.val",map(celsiusTempS2,0,255,0,100));
    myNex.writeNum("tempRFVar.val",map(celsiusTempS1,0,255,0,100));
    myNex.writeNum("drainVoltVar.val",map(vccMain,0,255,0,100));
    myNex.writeNum("biasVoltVar.val",map(vccAux,0,255,0,100));
    myNex.writeNum("ipaVar.val",map(currentPallet,0,255,0,100));
    myNex.writeNum("powerVar.val",map(powerFW,0,255,0,100));
    myNex.writeNum("fanSpeedVar.val",map(fanSpeed,0,255,0,100));
  }
}

void updateMainData(){
  //MAIN WINDOW DATA
  if(currentPage == 1){
    myNex.writeStr("tmp1Var.txt", tempGlobalTxt);
    myNex.writeStr("tmp2Var.txt", tempRFTxt);
  
    powerFW = map(analogRead(POWER_FWD_SENSOR_PIN),0,1024,0,100);
    
    myNex.writeNum("pwrBarVar.val", powerFW);
  
    vswr = map(analogRead(SWR_SENSOR_PIN),0,1024,0,100);
    myNex.writeNum("vswrBarVar.val", vswr);
  
    currentPallet = map(analogRead(CURRENT_RF_SENSOR_PIN),0,1024,0,100);
    myNex.writeNum("ipaBarVar.val", currentPallet);
  
    vccMain = map(analogRead(AUX1_ANALOG_SENSOR_PIN),0,1024,0,53);
    vccAux = map(analogRead(AUX2_ANALOG_SENSOR_PIN),0,1024,0,12);
    
    myNex.writeStr("vccMainVar.txt", String(int(vccMain))+"v");
    myNex.writeStr("vccAuxVar.txt", String(int(vccAux))+"v");
    
    if(band == 1){
      myNex.writeStr("band.txt","6m");
    }else if(band == 2){
      myNex.writeStr("band.txt","10m");
    }else if(band == 3){
      myNex.writeStr("band.txt","15m");
    }else if(band == 4){
      myNex.writeStr("band.txt","20m");
    }else if(band == 5){
      myNex.writeStr("band.txt","40m");
    }else if(band == 6){
     myNex.writeStr("band.txt","80m");
    }else if(band == 7){
      myNex.writeStr("band.txt","160m");
    }
    
    if(EEPROM.read(1)==1){
      myNex.writeStr("antennaOut.txt","A");
    }else if(EEPROM.read(1)==2){
      myNex.writeStr("antennaOut.txt","B");
    }

    if(operate){
      myNex.writeStr("statePTT.txt", "READY");
      myNex.writeNum("statePTT.pco",6112);
    }else{
      myNex.writeStr("statePTT.txt", "LOCK");
      myNex.writeNum("statePTT.pco",63488);
    }
  }
}

void buzzConfirm(){
  digitalWrite(BUZZER_PIN,HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN,LOW);
  delay(50);
}

void selectBand(int b){
  digitalWrite(LPF_6M_PIN,LOW);
  digitalWrite(LPF_10M_PIN,LOW);
  digitalWrite(LPF_15M_PIN,LOW);
  digitalWrite(LPF_20M_PIN,LOW);
  digitalWrite(LPF_40M_PIN,LOW);
  digitalWrite(LPF_80M_PIN,LOW);
  digitalWrite(LPF_160M_PIN,LOW);
  
  if(b==1){
    digitalWrite(LPF_6M_PIN,HIGH);
    myNex.writeStr("band.txt","6m");
  }else if(b == 2){
    digitalWrite(LPF_10M_PIN,HIGH);
    myNex.writeStr("band.txt","10m");
  }else if(b == 3){
    digitalWrite(LPF_15M_PIN,HIGH);
    myNex.writeStr("band.txt","15m");
  }else if(b == 4){
    digitalWrite(LPF_20M_PIN,HIGH);
    myNex.writeStr("band.txt","20m");
  }else if(b == 5){
    digitalWrite(LPF_40M_PIN,HIGH);
    myNex.writeStr("band.txt","40m");
  }else if(b == 6){
    digitalWrite(LPF_80M_PIN,HIGH);
    myNex.writeStr("band.txt","80m");
  }else if(b == 7){
    digitalWrite(LPF_160M_PIN,HIGH);
    myNex.writeStr("band.txt","160m");
  }
  
  EEPROM.write(2,b); //Salvo banda in eeprom
  buzzConfirm();
  digitalWrite(BAND_SW_LED,LOW);
  delay(50);
  digitalWrite(BAND_SW_LED,HIGH);
  delay(50);
  digitalWrite(BAND_SW_LED,LOW);
  delay(50);
  digitalWrite(BAND_SW_LED,HIGH);
  delay(50);
  if(currentPage!=1){
    myNex.writeStr("page main");
  }
}
//BAND DEC AUTO
void bandAutoSelect(){
  
  int bandA = digitalRead(BAND_DATA_A);
  int bandB = digitalRead(BAND_DATA_B);
  int bandC = digitalRead(BAND_DATA_C);
  int bandD = digitalRead(BAND_DATA_D);
  if(bandA == HIGH && bandB == LOW && bandC == LOW && bandD == LOW){
    band = 7;//160
  }else if(bandA == LOW && bandB == HIGH && bandC == LOW && bandD == LOW){
    band = 6; //80
  }else if(bandA == HIGH && bandB == HIGH && bandC == LOW && bandD == LOW ){
    band = 5; //40
  }else if(bandA == LOW && bandB == LOW && bandC == HIGH && bandD == LOW ){
    band = 4; //30
  }else if(bandA == HIGH && bandB == LOW && bandC == HIGH && bandD == LOW ){
    band = 4; //20
  }else if(bandA == LOW && bandB == HIGH && bandC == HIGH && bandD == LOW ){
    band = 3; //17
  }else if(bandA == HIGH && bandB == HIGH && bandC == HIGH && bandD == LOW ){
    band = 3; //15
  }else if(bandA == LOW && bandB == LOW && bandC == LOW && bandD == HIGH ){
    band = 2; //12
  }else if(bandA == HIGH && bandB == LOW && bandC == LOW && bandD == HIGH ){
    band = 2; //10
  }else if(bandA == LOW && bandB == HIGH && bandC == LOW && bandD == HIGH ){
    band = 1; //6
  }

  if(band != EEPROM.read(2)){
    selectBand(band);
  }
}
void checkAntenna(int antenna){
  if(antenna == 1){
    digitalWrite(ANT_SEL_RELE_PIN,LOW);
    myNex.writeStr("antennaOut.txt","A");
    EEPROM.write(1,1);
    buzzConfirm();
    digitalWrite(ANT_SW_LED,LOW);
    delay(50);
    digitalWrite(ANT_SW_LED,HIGH);
    delay(50);
    digitalWrite(ANT_SW_LED,LOW);
    delay(50);
    digitalWrite(ANT_SW_LED,HIGH);
    delay(50);
    
  }else if(antenna == 2){
    digitalWrite(ANT_SEL_RELE_PIN,HIGH);
    myNex.writeStr("antennaOut.txt","B");
    EEPROM.write(1,2);
    buzzConfirm();
    digitalWrite(ANT_SW_LED,LOW);
    delay(50);
    digitalWrite(ANT_SW_LED,HIGH);
    delay(50);
    digitalWrite(ANT_SW_LED,LOW);
    delay(50);
    digitalWrite(ANT_SW_LED,HIGH);
    delay(50);
  }
}
void checkOperate(boolean operate){
  if(operate == false){
    myNex.writeStr("b3.txt","STANDBY");
    myNex.writeNum("b3.pic",17);
    myNex.writeNum("b3.pic2",17);
    myNex.writeStr("statePTT.txt", "LOCK");
    myNex.writeNum("statePTT.pco",63488);
    lockPTT(1);
    lockVCC50(1);
    lockBIAS(1);
    buzzConfirm();
    digitalWrite(OPR_SW_LED,LOW);
    Serial.print("[STANDBY]");
    Serial.print(" |\t");
  }else if(operate == true){
    myNex.writeStr("b3.txt","OPERATE");
    myNex.writeNum("b3.pic",16);
    myNex.writeNum("b3.pic2",16);
    myNex.writeStr("statePTT.txt", "READY");
    myNex.writeNum("statePTT.pco",6112);
    lockPTT(0);
    lockVCC50(0);
    lockBIAS(0);
    buzzConfirm();
    digitalWrite(OPR_SW_LED,LOW);
    delay(50);
    digitalWrite(OPR_SW_LED,HIGH);
    delay(50);
    digitalWrite(OPR_SW_LED,LOW);
    delay(50);
    digitalWrite(OPR_SW_LED,HIGH);
    delay(50);
    Serial.print("[OPERATE]");
    Serial.print(" |\t");
  }
  delay(250);
}
void checkStatePA(){
  if(digitalRead(TX_SIGNAL_INPUT_PIN)==HIGH && digitalRead(RX_SIGNAL_INPUT_PIN)==LOW){
    myNex.writeStr("statePA.txt","TX");
    myNex.writeNum("statePA.pco",63488);
  }else if(digitalRead(TX_SIGNAL_INPUT_PIN)==LOW && digitalRead(RX_SIGNAL_INPUT_PIN)==HIGH){
    myNex.writeStr("statePA.txt","RX");
    myNex.writeNum("statePA.pco",6112);
  }else{
    myNex.writeStr("statePA.txt","");
  }
}
void lockPTT(int lock){
  if(lock == 1){
    digitalWrite(PTT_LOCK_RELE_PIN,LOW);
  }else if(lock == 0){
    digitalWrite(PTT_LOCK_RELE_PIN,HIGH);
  }
}

void lockVCC50(int lock){
  if(lock == 1){
    digitalWrite(VCC50_LOCK_RELE_PIN,LOW);
  }else if(lock == 0){
    digitalWrite(VCC50_LOCK_RELE_PIN,HIGH);
  }
}
void lockBIAS(int lock){
  if(lock == 1){
    digitalWrite(BIAS_LOCK_RELE_PIN,LOW);
  }else if(lock == 0){
    digitalWrite(BIAS_LOCK_RELE_PIN,HIGH);
  }
}
void checkFans(boolean start){
  if(start){
    digitalWrite(FAN_PWM_PIN,HIGH);
    digitalWrite(AUX1_PWM,HIGH);
  }else{
    digitalWrite(FAN_PWM_PIN,LOW);
    digitalWrite(AUX1_PWM,LOW);
  }
}
