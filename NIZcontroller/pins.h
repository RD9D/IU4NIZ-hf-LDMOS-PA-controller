//TEMP SENSORS PINS
#define TEMP_SENSOR1_PIN 9 // Pin del sensore 1
#define TEMP_SENSOR2_PIN 8 // Pin del sensore 2
#define TEMP_SENSOR3_PIN 7 // Pin del sensore 3

//PWM FOR FANS
#define FAN_PWM_PIN 12 // Pin PWM per il driver delle ventole
#define AUX1_PWM 11 // Pin PWM ausiliario
#define AUX2_PWM 10 // Pin PWM ausiliario

//ALARMS INPUTS
#define ALARM_PWR_PIN 37 // Ingresso allarme PWR protection board
#define ALARM_SWR_PIN 39 // Ingresso allarme SVR protection board
#define ALARM_CURR_PIN 41 // Ingresso allarme CURRENT protection board

//INPUT/OUTPUT FROM PANEL
#define ANT_SWITCH_PIN 32 // Ingresso pulsante antenna
#define BAND_SWITCH_PIN 24 // Ingresso pulsante banda
#define OPR_SWITCH_PIN 28 // Ingresso pulsante operate
#define RESET_SWICTH_PIN 36 // Ingresso pulsante reset
#define BUZZER_PIN 13 // Buzzer pin
#define ANT_SW_LED 34 //Led pulsante antenna
#define BAND_SW_LED 26 // Led pulsante band
#define OPR_SW_LED 30 //Led pulsante operate
#define RESET_SW_LED 38 //Led pulsante reset

//ANTENNA AND RESET RELE
#define ANT_SEL_RELE_PIN 45 // Uscita per cambio antenna -> RXTX Board Ant+
#define RESET_RELE_PIN 47 // Uscita per protection board -> Reset switch

//INTERLOCKS
#define VCC50_LOCK_RELE_PIN 49 // Uscita per interlock 50v
#define BIAS_LOCK_RELE_PIN 51 // Uscita per interlock bias 12v
#define PTT_LOCK_RELE_PIN 53 // Uscita per interlock PTT

//LPF RELE OPTO-SWITCH
#define LPF_6M_PIN 40 //Uscita per relè LPF banda 6M
#define LPF_10M_PIN 42 //Uscita per relè LPF banda 10M
#define LPF_15M_PIN 44 //Uscita per relè LPF banda 15M
#define LPF_20M_PIN 46 //Uscita per relè LPF banda 20M
#define LPF_40M_PIN 48 //Uscita per relè LPF banda 40M
#define LPF_80M_PIN 50 //Uscita per relè LPF banda 80M
#define LPF_160M_PIN 52 //Uscita per relè LPF banda 160M

//ANALOGS SENSE
#define POWER_FWD_SENSOR_PIN A7 //Sensore rilevamento potenza uscita
#define SWR_SENSOR_PIN A6 //Sensore rilevamento SWR
#define CURRENT_RF_SENSOR_PIN A5 //Sensore rilevamento corrente su pallet
#define AUX1_ANALOG_SENSOR_PIN A4 //Ingresso analogico aiusiliario
#define AUX2_ANALOG_SENSOR_PIN A3 //Ingresso analogico aiusiliario
#define AUX3_ANALOG_SENSOR_PIN A2 //Ingresso analogico aiusiliario

//DIGITAL INPUT
#define TX_SIGNAL_INPUT_PIN 5 //Segnale di TX
#define RX_SIGNAL_INPUT_PIN 6 //Segnale di RX
#define DIGI_AUX1_IN_PIN 4 //Ingresso digitale ausiliario
#define DIGI_AUX2_IN_PIN 3 //Ingresso digitale ausiliario
#define DIGI_AUX3_IN_PIN 2 //Ingresso digitale ausiliario

//ABCD BAND DATA
#define BAND_DATA_A 16
#define BAND_DATA_B 17
#define BAND_DATA_C 18
#define BAND_DATA_D 19
