#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //for display
#include <LittleFS.h>           //for storage
#include <WiFiManager.h>        //for wifi
#include <OneWire.h>            //for temperature
#include <DallasTemperature.h>  //for temperature
#define ONE_WIRE_BUS 0          //Data wire is plugged to GPIO0
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
const int buttonPin = 2;
const int ledPin = 12;
LiquidCrystal_I2C lcd(0x27, 16, 2);
int buttonState = 0;
int lastButtonState = 0;
int counter = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long previousMillis = 0;
unsigned long interval = 10000;
unsigned long currentMillis = millis();

void setup() {
  Serial.begin(115200);
  sensors.begin();
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the backlight
  // Set the cursor to column 0, line 0


  pinMode(buttonPin, INPUT);       // Initialize the pushbutton pin as an input
  pinMode(ledPin , OUTPUT);         // Initialize the led pin as output

  if (!LittleFS.begin()) {         // Initialize LittleFS
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  counter = readCounterFromLittleFS();// Read the last saved counter value from LittleFS
  lcd.setCursor(0, 0);
  lcd.print("SettingTimer");
  displayCounter();

  while ( millis() <= interval) {
    Serial.println(millis());
    pressButton();
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SavingCustomTimer");
  connect_wifi();
  delay(2000);
  




}

void loop() {
if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected.");
    
//  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sended Data");
 
  sensors.requestTemperatures(); // Send the command to get temperatures
  float temperatureC = sensors.getTempCByIndex(0);
  lcd.setCursor(0, 1);
  lcd.print("Temp");
  lcd.setCursor(5, 1);
  lcd.print(temperatureC);
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");
  delay(100 * counter);
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin, LOW);

  } else {
    Serial.println("Not connected.");
//    connect_wifi();
    
    lcd.setCursor(0, 0);
    lcd.print("NotConnected.");
    delay(3000);
  
  }
  

  

}
void pressButton() {


  int reading = digitalRead(buttonPin);   // Read the state of the pushbutton value
  // Check if the button state has changed
  if (reading != lastButtonState) {
    // Reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;

      // Only increment the counter if the new button state is LOW
      if (buttonState == LOW) {
        counter++;
        Serial.println("BTN Pressed");
        if (counter > 100) {
          counter = 1;
        }
        displayCounter();
        delay(100);                 // Display the updated counter value on the LCD
        saveCounterToLittleFS();   // Save the counter value to LittleFS
      }
    }
  }

  // Save the reading. Next time through the loop, it'll be the lastButtonState
  lastButtonState = reading;
}

void displayCounter() {

  lcd.setCursor(0, 1);             // Set the cursor to column 0, line 1
  lcd.print("                ");   // Clear the previous counter value
  lcd.setCursor(0, 1);             // Set the cursor to column 0, line 1 again
  lcd.print(counter);              // Print the counter value
}

void saveCounterToLittleFS() {
  File file = LittleFS.open("/counter.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(counter)) {
    Serial.println("File write Successful!");
  } else {
    Serial.println("Write Failed");
  }
  file.close();
}

int readCounterFromLittleFS() {
  File file = LittleFS.open("/counter.txt", "r");
  if (!file) {
    //Read the file data and display it on LCD
    Serial.println("No Saved Data!");
    lcd.clear();
    lcd.print("No Saved Data!");
    return 0;
  }
  String counterString = file.readStringUntil('\n');
  file.close();
  return counterString.toInt();
}

void configModelCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered a config Mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
void connect_wifi()
{
  WiFiManager wifiManager;
  // wifiManager.resetSettings();   ///if you don't wanna save the wifi settings
  wifiManager.setTimeout(120);
  wifiManager.setAPCallback(configModelCallback);
  //  wifiManager.addParameter(&custom_blynk_token);
  Serial.print("Connecting WIFI ...");
  Serial.print(".");
  //for lcd to display the wifi Data
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ConnectingWIFI ...");
  if (!wifiManager.autoConnect("HelloWIFI", "12345678"))
  {

    Serial.println("failed to connect an hit timeout");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fail WIFI ...");
    ESP.restart();
    delay(1000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected WIFI ...");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.SSID());
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  digitalWrite(ledPin, HIGH);
  delay(2000);
  digitalWrite(ledPin, LOW);
}
