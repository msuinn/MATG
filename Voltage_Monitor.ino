#include <avr/sleep.h>
void setup() {
  
  Serial.begin(9600);
  int sleepStatus = 0;
  
}

void loop() {

  int adcVal = analogRead(0);
  float voltage = adcVal * (5.0 / 1023.0);    //convert relative to 5V divider, adjust 5.0 for whatever desired voltage is
  Serial.println(voltage);

  if (voltage < 4.20){   // 4.20 = roughly 9.9V on battery
    Serial.println("low battery, entering sleep mode");
    delay(100);          //required for interrupt to not throw serial error
    sleepNow();
  }
  
  delay(1000);

}

void sleepNow(){
           
   /*    SLEEP_MODE_IDLE         -the least power savings
   *     SLEEP_MODE_ADC
   *     SLEEP_MODE_PWR_SAVE
   *     SLEEP_MODE_STANDBY
   *     SLEEP_MODE_PWR_DOWN     -the most power savings
   */  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode(); //Actual sleep function, puts device to sleep
   
}


