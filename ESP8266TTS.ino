#if defined(ESP32)
    #include <WiFi.h>
    #include "AudioOutputI2S.h"
    #define ESPMODELNAME "ESP32"
#else
    #include <ESP8266WiFi.h>
    #include "AudioOutputI2SNoDAC.h"
    #define ESPMODELNAME "ESP8266"
#endif
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include <google-tts.h>

const char* ssid     = "ssid";
const char* password = "pass";

AudioGeneratorMP3 *mp3;
AudioFileSourceICYStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2SNoDAC *out;

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode;
  char s1[32], s2[64];
  strncpy_P(s1, type, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  strncpy_P(s2, string, sizeof(s2));
  s2[sizeof(s2)-1]=0;
  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
  Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}
void setup() {
  Serial.begin(115200); //serial starting
  Serial.println("");

  WiFi.mode(WIFI_STA);  //WIFI
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  generatemessage("Hello world! Im a ESPRESSIFF chip", "en");
}

void generatemessage(String text, String language) {
  TTS tts; //TTS initialization

  String PreURL = tts.getSpeechUrl(text, language); //Generating an URL 
  PreURL.replace("https", "http"); //Use HTTP instead of HTTPS
  Serial.println(PreURL);

  file = new AudioFileSourceICYStream(PreURL.c_str());
  file->RegisterMetadataCB(MDCallback, (void*)"ICY"); //Debug (Can be commented)
  buff = new AudioFileSourceBuffer(file, 2048);
  buff->RegisterStatusCB(StatusCallback, (void*)"buffer"); //Debug (Can be commented)
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3"); //Debug (Can be commented)
  mp3->begin(buff, out);
}

void loop() {
  static int lastms = 0;

  if (mp3->isRunning()) { //MP3 Controller
    if (millis()-lastms > 1000) {
      lastms = millis();
      Serial.printf("Running for %d ms...\n", lastms);
      Serial.flush();
     }
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.printf("MP3 done\n");
    delay(1000);
  }
}