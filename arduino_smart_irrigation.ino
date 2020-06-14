#include <DHT.h>;
#include <SoftwareSerial.h>
#include <String.h> 
#include <LiquidCrystal.h>

const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define DHTPIN A1     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
int soil_moisture=0;
const int motor=7,mode_sel=6;
bool auto_mode=0;
int ct=0;
String textMessage="";
String buffer,phone_num;

void setup()
{
  Serial.begin(115200);               // the GPRS baud rate   
//  Serial.begin(9600);
	dht.begin();
  pinMode(motor,OUTPUT);
  pinMode(mode_sel,INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.setCursor(0,0); 
  lcd.print("SMART IRRIGATION");
//  Serial.println("<<----Welcome to Smart Irrigation Project--->>");
  delay(8000);
  
  Serial.println("AT");
  delay(500);
  ShowSerialData();
  
  Serial.println("ATE1");
  delay(500);
  ShowSerialData();

  Serial.println("AT+CPIN?");
  delay(500);
  ShowSerialData();
  
  Serial.println("AT+CREG?");
  delay(500);
  ShowSerialData();
  
  Serial.println("AT+CMGD=1,4");
  delay(200);
  Serial.println("AT+CMGF=1");
  delay(200);
  Serial.println("AT+CNMI=1,2,0,0,0");
  delay(200);
  
//  Serial.println("<<---- Setup finished --->>");
//  sendSMS("Hi there!");
  lcd.clear();
}

void loop()
{
    ct++;
    //Read data and store it to variables hum and temp
    buffer = readSIM800();
    if (buffer.startsWith("\r\n+CMT: "))
    {
//        phone_num = get_num(buffer);
        Serial.println("*** RECEIVED SMS *** from " + phone_num);  
        buffer.remove(0, 51); // Remove first 51 characters
        int len = buffer.length();
        buffer.remove(len - 2); // Remove \r\n from tail          
        if(digitalRead(mode_sel)){
          if(buffer=="ON"){
          digitalWrite(motor,HIGH);
          sendSMS("M: Water Pump ON");
          }
          if(buffer=="OFF"){
          digitalWrite(motor,LOW);
          sendSMS("M: Water Pump OFF");
          }
          
        }
//        Serial.println(buffer);
        Serial.println("*** END SMS ***");

    }
    
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    //Print temp and humidity values to serial monitor
    
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("SMART IRRIGATION");
    lcd.setCursor(0,1); 
    lcd.print("M: ");
    lcd.print(soil_moisture);
    soil_moisture = analogRead(A0);
    soil_moisture = map(soil_moisture, 1023, 0, 0, 100);
    if(digitalRead(mode_sel))
    {  // sms mode
    lcd.print("  MANUAL");      
    }
    else{   // auto mode
    lcd.print("  AUTO");      
    if(soil_moisture<50){
      digitalWrite(motor,HIGH);
      delay(2000);
      digitalWrite(motor,LOW);      
      sendSMS("A: Water Pump ON");
    }
    else{
      digitalWrite(motor,LOW);      
      sendSMS("A: Water Pump OFF");
      }
    }


    if(ct>1200){  // 2 mins
    lcd.print(" TS");
    Send2thingspeak();
    ct=0;
    }
    delay(100); //Delay 2 sec.
}

void Send2thingspeak()
{
  Serial.println("AT");
  delay(1000);

  Serial.println("AT+CPIN?");
  delay(1000);
  ShowSerialData();

  Serial.println("AT+CREG?");
  delay(300);
  ShowSerialData();

  Serial.println("AT+CGREG?");
  delay(300);
  ShowSerialData();
  
  Serial.println("AT+CSQ");
  delay(300);
  ShowSerialData();
  
  Serial.println("AT+CGATT?");
  delay(1000);
  ShowSerialData();

  Serial.println("AT+CIPSHUT");
  delay(1000);
  ShowSerialData();

  Serial.println("AT+CIPSTATUS");
  delay(2000);
  ShowSerialData();

  Serial.println("<------ CIPSTATUS ----->");
  
  Serial.println("AT+CIPMUX=0");
  delay(2000); 
  ShowSerialData();
 
  Serial.println("AT+CSTT=\"internet\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  Serial.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  ShowSerialData();
 
  Serial.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  Serial.println("AT+CIPSPRT=0");
  delay(3000);
 
  ShowSerialData();
  
  Serial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);
 
  ShowSerialData();
 
  Serial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  
  String str="GET http://api.thingspeak.com/update?api_key=KTQZFHR5H1HP84BC&field1=" + String(soil_moisture);
  Serial.println(str);//begin send data to remote server
  Serial.println(str);//begin send data to remote server
  delay(4000);
  ShowSerialData();

  Serial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  Serial.println();
 
  ShowSerialData();
  Serial.println("Data Sent");//
 
  Serial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
} 

void ShowSerialData()
{
//  while(Serial.available()!=0)
//    Serial.write(Serial.read());
}

void sendSMS(String message){
  Serial.println("AT+CMGF=1"); 
  delay(200);

  Serial.println("AT+CMGS=\"+919xxxxxxxxx\""); 
  delay(500);
  // Send the SMS
  Serial.println(message); 
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  Serial.println((char)26); 
  delay(100);
  Serial.println();
  // Give module time to send SMS
  delay(3000);  
}

String readSIM800() // Save whole SMS into a string
{
    String buffers;
    char c;
    while (Serial.available())
    {
        c = Serial.read();
        buffers.concat(c);
        delay(10);
    }   
    return buffers;
}
