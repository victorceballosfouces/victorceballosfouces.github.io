#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_BNO08x.h>

// Ajusta según tu hotspot
const char* ssid = "POCO X6 Pro 5G";  
const char* password = "bebitobollito";

// Servidor y WebSocket en puerto 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// BNO08x
Adafruit_BNO08x bno08x;
#define REPORT_INTERVAL_US 20000 // 50 Hz => 20ms

// Cuaterniones
float qw = 1.0f, qx = 0.0f, qy = 0.0f, qz = 0.0f;

void onWsEvent(AsyncWebSocket * server, 
               AsyncWebSocketClient * client,
               AwsEventType type, 
               void * arg, 
               uint8_t *data, 
               size_t len)
{
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Cliente WebSocket conectado. ID=%u\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Cliente WebSocket desconectado. ID=%u\n", client->id());
  }
}

void setup(){
  Serial.begin(115200);

  // 1) Conectar a la WiFi del móvil
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Conectando al hotspot...");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP de la ESP32: ");
  Serial.println(WiFi.localIP());  // Ej: 192.168.142.x

  // 2) Inicializar BNO08x
  if (!bno08x.begin_I2C()) {
    Serial.println("No se detectó BNO08x. Revisa conexiones I2C.");
    while(true) { delay(10); }
  }
  // Activar reporte de rotación (Rotation Vector)
  bno08x.enableReport(SH2_ROTATION_VECTOR, REPORT_INTERVAL_US);

  // 3) Configurar WebSocket y servidor
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
  Serial.println("WebSocket listo en puerto 80, ruta /ws");
}

void loop(){
  // Leer BNO08x
  sh2_SensorValue_t sensorValue;
  while(bno08x.getSensorEvent(&sensorValue)){
    if(sensorValue.sensorId == SH2_ROTATION_VECTOR){
      qw = sensorValue.un.rotationVector.real; // real part
      qx = sensorValue.un.rotationVector.i;
      qy = sensorValue.un.rotationVector.j;
      qz = sensorValue.un.rotationVector.k;
    }
  }

  // Enviar cuaterniones ~cada 50ms
  static unsigned long lastSend = 0;
  if(millis() - lastSend >= 50){
    lastSend = millis();
    char buff[80];
    snprintf(buff, sizeof(buff),
      "{\"qw\":%.3f,\"qx\":%.3f,\"qy\":%.3f,\"qz\":%.3f}",
      qw, qx, qy, qz
    );
    // Enviar a todos los clientes WebSocket
    ws.textAll(buff);
  }
}
