#define BLYNK_PRINT Serial
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Servo.h> 
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "DHT.h"
#define DHTPIN A8
#include "TimerObject.h"
#define DHTTYPE DHT11
char auth[] = "e1ce112ee984488499ec195da8c4f62fe1ce112ee984488499ec195da8c4f62f";

#define W5100_CS  10
#define SDCARD_CS 4
DHT dht(DHTPIN, DHTTYPE);
const int ledAzul = 32;      //refere-se ao pino digital 9.
const int ledVerde = 33;     //refere-se ao pino digital 8.
const int ledVermelho = 34; //refere-se ao pino digital 7.
int leds[] = {22,23,24,25,26,28,29,30,31,35, 32, 33, 34};
String comodos[] = {"Garagem","Sala","Banheiro","Cozinha","Varanda","Suíte","SalaTv","Quarto1","Quarto2","Jardim", "Vermelho","Verde","Azul" };
Servo microservo;  
int pos = 0; 
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   
//byte ip[] = { 192, 168, 1, 150};                      
//byte gateway[] = { 192, 168, 1, 1 };                   // internet access via router
//byte subnet[] = { 255, 255, 255, 0 };                  //subnet mask
IPAddress server_addr(177, 234, 153, 2);
EthernetServer server(80);                             //server port     
String readString;
bool garagemAberta = false;
bool piscinaLigada = false;

TimerObject *timer1 = new TimerObject(600000); //will call the callback in the interval
TimerObject *timer2 = new TimerObject(4000); //will call the callback in the interval

char user[] = "u338669747_ardui";
char password[] = "cacete12";
char INSERT_SQL[] = "INSERT INTO u338669747_ardui.sensor (humidity, temperature) VALUES ('%s', '%s')";
long counter;
char query[128];
char temperatura[10];
char humidade[10];
EthernetClient client;
MySQL_Connection conn((Client *)&client);

void setup() {
  Serial.begin(9600);

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card

  Blynk.begin(auth);
 // Open serial communications and wait for port to open:
 dht.begin();
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
         for(int ledAtual = 0 ;ledAtual < sizeof(leds);ledAtual++){
            pinMode(leds[ledAtual],OUTPUT);
           }
  microservo.attach(7);
  // start the Ethernet connection and the server:
//  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  timer1->setOnTimer(&takeReading);
  timer1->Start(); //start the thread.

  pinMode(ledAzul,OUTPUT);   
  pinMode(ledVerde,OUTPUT);   
  pinMode(ledVermelho,OUTPUT);

        digitalWrite(ledAzul,HIGH);     
      digitalWrite(ledVerde,HIGH);
       digitalWrite(ledVermelho,HIGH);
  
}


void loop() {
  Blynk.run();

int h = dht.readHumidity();
int t = dht.readTemperature();
    timer1->Update(); //will check the Timer and if necessary, will run it.
  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
     
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
         }
         //if HTTP request has ended
         if (c == '\n') {          
           Serial.println(readString); //print to serial monitor for debuging
           client.println("HTTP/1.1 200 OK"); //send new page
           client.println("Content-Type: text/html");
           client.println();     
           client.println("<HTML>");
           client.println("<HEAD>");
           client.println("<meta charset='UTF-8'>");
           client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
           client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
           client.println("<meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'>");
           client.println("<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'>");
            client.println("<link rel='stylesheet' type='text/css' href='https://randomnerdtutorials.com/ethernetcss.css' />");
           client.println("<TITLE>Automação Residencial com Arduino</TITLE>");
           client.println("</HEAD>");
           client.println("<BODY>");
           client.println("<div class='container-fluid'>");
           client.print("<div class='col-sm-12 col-md-12 col-lg-12 col-xl-12'>");  
           client.print("<h1> Temperatura </h1>");  
           client.print(t);
                      client.print("ºC");
           client.print("<h1> Humidade </h1>");  
           client.print(h);
                      client.print("%");
           client.print("</div>"); 

     //  client.println(readString.substring(8,10).toInt()); 
            for(int count = 0 ;count < 10;count++){
                                     client.print("<div class='row'>");
                         client.print("<div class='col-sm-12 col-md-12 col-lg-12 col-xl-12'>");  
                         client.print("<button class='botaomeu'><a href='?on" + String(count) + "\'>Ligar "+comodos[count]+"</a></button> <button class='botaomeu'><a href='?of" + String(count) + "\'>Desligar "+comodos[count]+"</a></button>");
                         client.print("</div>"); 
                                   client.println("</div>"); 

           }
          client.print("<div class='col-sm-12 col-md-12 col-lg-12 col-xl-12'>");  
          client.print("<button class='botaomeu'><a href=\"/?button2\"\">Portão</a></button>");
          client.print("<h2> Controles da piscina </h2>"); 
            for(int count = 10 ;count < 13;count++){
                         client.print("<div class='col-sm-12 col-md-12 col-lg-12 col-xl-12'>");  
                         client.print("<button class='botaomeu'><a href='?of" + String(count) + "\'>Ligar "+comodos[count]+"</a></button><button class='botaomeu'><a href='?on" + String(count) + "\'>Desligar "+comodos[count]+"</a></button>");
                         client.print("</div>"); 



                         
           }
          client.print("<button class='botaomeu'><a href=\"/?button3\"\">Desligar todos</a></button>");
          client.print("</div>"); 
          client.println("</div>"); 
          client.println("</BODY>");
          client.println("</HTML>");
           delay(10);
           //stopping client
           client.stop();
           //controls the Arduino if you press the buttons
          if (readString.indexOf("?on") >0){
             digitalWrite(leds[readString.substring(8,10).toInt()], HIGH);
          }
          if (readString.indexOf("?of") >0){
            digitalWrite(leds[readString.substring(8,10).toInt()], LOW);
          }


            if (readString.indexOf("?button2") >0){
             
                if(garagemAberta){
                  digitalWrite(leds[0], HIGH);
                           for(pos = 123; pos > 0; pos --)  // goes from 0 degrees to 180 degrees 
                  {                                  // in steps of 1 degree 
                     microservo.write(pos);              // tell servo to go tble 'pos' 
                    delay(10);                       // waits 15ms for the servo to reach the position o position in varia
                 } 
           
                }else{
                  
                   for(pos = 0; pos < 123; pos ++)  // goes from 0 degrees to 180 degrees 
                  {                                  // in steps of 1 degree 
                     microservo.write(pos);              // tell servo to go tble 'pos' 
                    delay(10);                       // waits 15ms for the servo to reach the position o position in varia
                 } 
                 digitalWrite(leds[0], LOW);

                }
              garagemAberta = !garagemAberta;
           }

  


            if (readString.indexOf("?button3") >0){
  
  digitalWrite(ledAzul,HIGH);     
  digitalWrite(ledVerde,HIGH);
  digitalWrite(ledVermelho,HIGH);
              
           }
            //clearing string for next read
            readString="";  
           
         }
       }
    }
}

}

void takeReading(){
float h = dht.readHumidity();
float t = dht.readTemperature();
Serial.print("Humidity: ");
Serial.print(humidade);
Serial.print(" %\t");
Serial.print("Temperature: ");
Serial.print(temperatura);
Serial.println(" *C ");
//Ethernet.begin(mac, ip);
Serial.println("Connecting...");
if (conn.connect(server_addr, 3306, user, password)) {
delay(1000);
MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
//sprintf(INSERT_SQL);
Serial.println("Recording data...");
dtostrf(t, 1, 1, temperatura);
dtostrf(h, 1, 1, humidade);
sprintf(query, INSERT_SQL, humidade, temperatura);
cur_mem->execute(query);
delete cur_mem;
}

}
