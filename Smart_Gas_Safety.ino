#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <SimpleTimer.h>
SimpleTimer alterscantimer;

#include <ESP8266WiFi.h>           // Use this for WiFi instead of Ethernet.h
#include <ESP8266HTTPClient.h>

char ssid[] = "project";         // your SSID
char pass[] = "project1234";     // your SSID Password

const char* host = "http://microembeddedtech.com/appinventor";
String get_host = "http://microembeddedtech.com/appinventor";

WiFiServer server(80);  // open port 80 for server connection
String tablename="msrgasair";


int gaspin = D3;
int airpin= D4;

int fanpin=D5;
int servopin = D6;
int buzzerpin=D7;

int gasval;
int airval;
String rgasvalue;
String rairvalue;
String gasservostatus;
String fanstatus;

int operationmode = 0; // 0- AUTO MODE, 1 - MANUAL MODE
int gasbitvalue=0;

String keyone="mode";
String keytwo="gasbit";

String l1value;
String l2value;

int servostatus=0;


#include <Servo.h>
 
Servo servo;

void setup() {
  Serial.begin(9600);



  pinMode(fanpin, OUTPUT);
  pinMode(buzzerpin, OUTPUT);
  pinMode(gaspin, INPUT);
  pinMode(airpin, INPUT); 

  lcd.begin();                      // initialize the lcd 

  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  WELCOME TO ");
  lcd.setCursor(0,1);
  lcd.print(" PROJECT ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting Wifi..");

  alterscantimer.setInterval(5000, sensormon);
  // Begin WiFi section
  Serial.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // print out info about the connection:
  Serial.println("\nConnected to network");
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
  //starting the server
  server.begin();
  delay(2000);
  servoopen();

}

void loop() {
 alterscantimer.run();

}

void sensormon() {


  gasval = digitalRead(gaspin);
  if (gasval==1){
    rgasvalue="NORMAL";
  }
  else{
    rgasvalue="DETECTED";
     lcd.clear();
     lcd.print("GAS DETECTED");
     buzzering();
  }
  Serial.print(F("GAS : ")); Serial.print(gasval); Serial.print("----");; Serial.println(rgasvalue);

   airval = digitalRead(airpin);
  if (airval==1){
    rairvalue="NORMAL";
    fanturnoff();
  }
  else{
    rairvalue="BAD";
    fanturnon();
    lcd.clear();
    lcd.print("BAD AIR");
    buzzering();
  }
  Serial.print(F("AIR : ")); Serial.print(airval); Serial.print("----");; Serial.println(rairvalue);

  userupdate_status(tablename,rgasvalue,rairvalue);
   lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GAS:");
  lcd.print(rgasvalue);
   lcd.print(" ");
  lcd.print(gasservostatus);
  lcd.setCursor(0,1);
  lcd.print("AIR:");
  lcd.print(rairvalue);
  lcd.print(" ");
  lcd.print(fanstatus);
  delay(1000);
  getloadsdata();
}

void fanturnon() {
  fanstatus="ON";
 digitalWrite(fanpin, 1);
}
void fanturnoff() {
    fanstatus="OFF";
 digitalWrite(fanpin, 0);
}

void buzzering(){
digitalWrite(buzzerpin, 1);
delay(1000);
digitalWrite(buzzerpin, 0);
}

void servoopen(){
  gasservostatus="OPEN";
 servo.attach(servopin);  
 delay(1000);
servo.write(180);
delay(1000);
servo.detach();
}

void servoclose(){
 gasservostatus="CLOSE";
servo.attach(servopin); 
delay(1000);
servo.write(180);
 delay(1000);
servo.detach();
}



void get_device_status(String table_name, String require_text) {

  WiFiClient client = server.available();

  HTTPClient http;
  String url = get_host+"/smartdripdinget.php?table_name="+table_name+"&task_key="+require_text;

  //Serial.println(url);

  http.begin(client,url);

  //GET method
  int httpCode = http.GET();
  String payload = http.getString();
  payload.trim();
  //Serial.print("PAYLOAD:");Serial.println(payload);

  if(require_text==keyone){
    l1value=payload;
    operationmode=l1value.toInt();
    Serial.print("MODE VALUE:");Serial.println(l1value);
  }
  else if(require_text==keytwo){
    l2value=payload;
    gasbitvalue=l2value.toInt();
    Serial.print("GAS BIT VALUE:");Serial.println(l2value);
  }

 
  http.end();


  
  delay(1000);
  checkstatus();
  delay(1000);
}

void checkstatus(){
if (operationmode == 1) {

  if (gasbitvalue == 1) {
        if (servostatus == 0) {
          Serial.println("*** MANUAL TURN ON ********");
          servoopen();
          servostatus = 1;
        }
    } 
    else if (gasbitvalue == 0) {
        
        if (servostatus == 1) {
          Serial.println("*** MANUAL TURN OFF ********");
          servoclose();
          servostatus = 0;
        }
      }


    }
else
    
    {
       if(gasval==0){
         if (servostatus == 1 ) {  
          Serial.println("****AUTO TURN OFF *******");
          servoclose();
          servostatus = 0;
        }
       }
       else{
         if (servostatus == 0) { 
          Serial.println("*** AUTO TURN ON ********");
          servoopen();
          servostatus = 1;
        }
       }
    }

/***********************/

  
 
}

void getloadsdata(){
  get_device_status("msrgasair", keyone);
  delay(1000);
  get_device_status("msrgasair", keytwo);
}


void userupdate_status(String table_name, String ugas,String uair) {
//http://microembeddedtech.com/appinventor/cloningattackupdate.php?table_name=arduinoencryption&updatebit=t2bit&bitval=1
  WiFiClient client = server.available();
 String Ss1= '"'+ugas+'"';
  String Ss2='"'+uair+'"';
  

  HTTPClient http;
  String url = get_host+"/msrairgas.php?table_name="+table_name+"&gasval="+Ss1+"&airval="+Ss2;

  Serial.println(url);

  http.begin(client,url);

  //GET method
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  http.end();
  delay(1000);
}