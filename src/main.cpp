#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
int piezoPin = 14;
int len = 0;
bool isOn = false;
// Вставьте сюда свои учетные данные сети
const char* ssid = "HUAWEI-B311-1E6A";
const char* password = "9MND93DGJF0";
long preTime = 0; 
long preTime2 = 0; 
long interval = 20;
long interval2 = 40;
long mainTime = 0; 
long secondTime = 0; 
bool needSet = false; 
// Запускаем бот
#define BOTtoken "5811771675:AAEAIe-9gkrXPr3ZWRC2XrdtTcHiF6rn-VU"  // Вставляем токен бота.
 
// Используйте @myidbot, чтобы получить ID пользователя или группы
// Помните, что бот сможет вам писать только после нажатия
// вами кнопки /start
#define CHAT_ID "-840265905"
 
#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif
 
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
 
// Каждую секунду проверяет новые сообщения
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

 
// Задаем действия при получении новых сообщений 
void handleNewMessages(int numNewMessages) {
  // Serial.println("handleNewMessages");
  // Serial.println(String(numNewMessages));
 
  for (int i=0; i<numNewMessages; i++) {
    // Идентификатор чата запроса
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Вы кто такие, я вас не знаю, идите нахер!", "");
      continue;
    }
    
    // Выводим полученное сообщение
    String text = bot.messages[i].text;
    Serial.println(text);
 
    String from_name = bot.messages[i].from_name;
 
    
    if (text == "/set_interval") {
      bot.sendMessage(chat_id, "установите время: ", "");
      needSet = true;
    }else{
      if(needSet){
        
        len = sizeof(text)/sizeof(int);
        String nm = "";
        for(int i = 1; i < len; ++i){
          nm += text[i];
        }
        Serial.println(nm);
        interval = nm.toInt();
        needSet  = false;
        preTime = millis();
        mainTime = 0;

        bot.sendMessage(chat_id, "интервал установлен на " + String(interval) + " секунд!", "");
      }
      
    }
    
  }
}

void mainCycle(){
  
  if(!digitalRead(13)){
    mainTime = millis() - preTime;
    secondTime = 0;
    preTime2 = millis();
  }
  if(digitalRead(13)){
    mainTime = 0;
    secondTime = millis() - preTime2;
    preTime = millis();
  }
  if(secondTime > (interval2 * 1000)){
    digitalWrite(12, LOW);
    noTone(piezoPin);
    bot.sendMessage(CHAT_ID, "Запайщик отключен уже 40 секунд!", "");
    isOn = false;
  }
  if(mainTime > (interval * 1000)){
    digitalWrite(12, HIGH);
    tone(piezoPin, 1500, 10000);
    bot.sendMessage(CHAT_ID, "Запайщик не используется уже: " + String(mainTime / 1000) + " секунд!", "");
  }else{
    digitalWrite(12, LOW);
    noTone(piezoPin);
  }
}

void setup() {
  Serial.begin(115200);
 
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // получаем всемирное координированное время (UTC) через NTP
    client.setTrustAnchors(&cert); // Получаем сертификат api.telegram.org
  #endif
  pinMode(13, INPUT_PULLUP);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);

  // Подключаемся к Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Добавляем корневой сертификат для api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // Serial.println(WiFi.localIP());
 
  bot.sendMessage(CHAT_ID, "Бот подключен!", "");
  preTime = millis();
}
 
void loop() {
  if(isOn){
    mainCycle();
  }
  if(!isOn && !digitalRead(13)){
    isOn = true;
    bot.sendMessage(CHAT_ID, "Запайщик включен!", "");
    secondTime = 0;
    preTime2 = millis();
    mainTime = 0;
    preTime = millis();
  }
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

}