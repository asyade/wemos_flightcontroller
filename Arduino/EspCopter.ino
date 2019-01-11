#ifdef ESP32
  #include <WiFi.h>
#endif
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#include <WiFiUdp.h>
#include <Wire.h>
#include <EEPROM.h>


//#define ESP32
#define ESP8266

//#define APMODE
#ifndef APMODE
# define ROUTER_SSID "Livebox-C33A"
# define ROUTER_PASSWD "123581321"
#else
 IPAddress local_IP(192,168,4,22);
 IPAddress gateway(192,168,4,9);
 IPAddress subnet(255,255,255,0);
#endif


#define CONT_STACKSIZE (8192*2)

#include "RC.h"

volatile boolean recv;

#define ACCRESO 4096
#define CYCLETIME 4
#define MINTHROTTLE 1050
#define THRCORR 19

#define MPU_SMOOTHING_DEPTH 4

#define P1 14
#define P2 12
#define P3 13
#define P4 15

#define pwmpin1 P4
#define pwmpin2 P1
#define pwmpin3 P3
#define pwmpin4 P2

#define LOG_CMD 2

#define ROL 0
#define PIT 1
#define THR 2
#define RUD 3
#define AU1 4
#define AU2 5
#define GET_PID      7
#define SET_PID      8
#define STATE_ROTORS 9
#define STATE_CMD    12
#define CALIB_CMD    6
#define GYRO_CMD     32

#define GYRO     0
#define STABI    1
#define RTH      2

WiFiUDP Udp;
unsigned int localUdpPort = 4242;  // local port to listen on
char incomingPacket[64];

enum ang { ROLL, PITCH, YAW };

static int8_t P_PID[3] = { 20, 20, 20 };     // P8
static int8_t I_PID[3] = { 10, 10, 10 };     // I8
static int8_t D_PID[3] = { 10, 10,  0 };     // D8

static int16_t gyroADC[3];
static int16_t accADC[3];
static int16_t gyroData[3];
static uint16_t servo[4];
static float angle[2]    = {0, 0};
extern int calibratingA;
int lastAngleRate[3];

static int16_t rcCommand[] = {0, 0, 0};

int stateRotor = -1;

static int8_t flightmode;
static int8_t oldflightmode;

boolean armed = true;
uint8_t armct = 0;
bool loaded = false;

uint32_t rxt; // receive time, used for falisave
int steep = 1;

void log(char *str)
{
  if (!loaded)
    return;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nLoading ...\n");
  
  MPU6050_init();
  MPU6050_readId(); // must be 0x68, 104dec
  PID_read();
  
  ACC_Read();
  initServo();
  
  #ifndef APMODE
    WiFi.begin(ROUTER_SSID, ROUTER_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
  #else
    WiFi.scanNetworks();
    Serial.println(WiFi.softAP("ESPsoftAP_01") ? "Ready" : "Failed!");
    Serial.println(WiFi.softAPIP());
  #endif
  
  delay(500);
  init_telem();
}


void loop()
{
  if (recv)
  {
    recv = false;
    if      (rcValue[AU1] < 1300)   flightmode = GYRO;
    else if (rcValue[AU1] > 1700)   flightmode = RTH;
    else                            flightmode = STABI;
    if (oldflightmode != flightmode)
      oldflightmode = flightmode;
  }
  
  gyro_getADC();
  ACC_getADC();
  get_estimated_attitude();

  pid();
  mix();
  writeServo();
  recv_tcp();
}
