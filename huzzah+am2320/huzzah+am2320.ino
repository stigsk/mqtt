#include "Adafruit_AM2320.h";

int sleepTime = 15E6; //60E6 = 60seconds

Adafruit_AM2320 am2320 = Adafruit_AM2320();


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("Setup");

  am2320.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.print("Temp: "); Serial.println(am2320.readTemperature());
  Serial.print(" Hum: "); Serial.println(am2320.readHumidity());
  
  Serial.println("going to sleep...");

  ESP.deepSleep(sleepTime);
  Serial.println("...and this line is never printed.");
}
