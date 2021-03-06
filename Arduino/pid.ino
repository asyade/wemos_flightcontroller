
#define yawRate 90
#define rollPitchRate 40

static int8_t P_Level_PID = 30;  // P8
static int8_t I_Level_PID = 15;  // I8
static int8_t D_Level_PID = 10;  // D8

void PID_write()
{
  EEPROM.begin(15);
  EEPROM.write(6,  P_PID[0]);
  EEPROM.write(7,  P_PID[1]);
  EEPROM.write(8,  P_PID[2]);
  EEPROM.write(9,  I_PID[0]);
  EEPROM.write(10, I_PID[1]);
  EEPROM.write(11, I_PID[2]);
  EEPROM.write(12, D_PID[0]);
  EEPROM.write(13, D_PID[1]);
  EEPROM.write(14, D_PID[2]);
  EEPROM.commit();
}

void PID_read()
{
  EEPROM.begin(15);
  P_PID[0] = EEPROM.read(6);
  P_PID[1] = EEPROM.read(7);
  P_PID[2] = EEPROM.read(8);
  I_PID[0] = EEPROM.read(9);
  I_PID[1] = EEPROM.read(10);
  I_PID[2] = EEPROM.read(11);
  D_PID[0] = EEPROM.read(12);
  D_PID[1] = EEPROM.read(13);
  D_PID[2] = EEPROM.read(14);
}

static int16_t axisPID[3];
static int16_t lastError[3] = {0,0,0};
static float errorGyroI[3] = {0,0,0};

//----------PID controller----------
// Angle is Level_P
// Horizon is Level_I
// Level_D is unused
// P 3.3, 0.010, 13
// L 4.5, 0.010, 100
    
#define GYRO_I_MAX 50000.0

int plotct;

void pid()
{
  uint8_t axis;
  int16_t errorAngle;
  int16_t PTerm,ITerm,DTerm;
  int16_t delta,deltaSum;
  static int16_t delta1[3],delta2[3];
  int16_t AngleRateTmp, RateError;


  lastAngleRate[0] = angle[0];
  lastAngleRate[1] = angle[1];
  lastAngleRate[2] = angle[2];
      //----------PID controller----------
      for(axis=0;axis<3;axis++) 
      {
        //-----Get the desired angle rate depending on flight mode
        if (axis == 2) 
        {//YAW is always gyro-controlled 
          AngleRateTmp = (((int32_t) yawRate * rcCommand[YAW]) >> 5);
          RateError = AngleRateTmp - gyroData[axis];
        } 
        else 
        {
          if (flightmode != GYRO) 
          { 
            // calculate error and limit the angle to 45 degrees max inclination
            errorAngle = constrain(rcCommand[axis]/2,-225,+225) - angle[axis]; //16 bits is ok here           
            //it's the ANGLE mode - control is angle based, so control loop is needed
            AngleRateTmp = ((int32_t) errorAngle * P_Level_PID)>>4;
            RateError = AngleRateTmp;
            delta = - gyroData[axis]; 
            DTerm = ((int32_t)delta*D_Level_PID)>>7; //@ 7
          }
          else 
          {//control is GYRO based (ACRO - direct sticks control is applied to rate PID
            AngleRateTmp = ((int32_t) rollPitchRate * rcCommand[axis]) >> 4;
            RateError = AngleRateTmp - gyroData[axis];
            //-----calculate D-term
            delta           = RateError - lastError[axis];  // 16 bits is ok here, the dif between 2 consecutive gyro reads is limited to 800
            lastError[axis] = RateError;
            //add moving average here to reduce noise
            deltaSum       = delta1[axis]+delta2[axis]+delta;
            delta2[axis]   = delta1[axis];
            delta1[axis]   = delta;        
            DTerm = (deltaSum*D_PID[axis])>>6;
          } 
        }
        //-----calculate P component scaled to 4ms cycle time
        PTerm = ((int32_t) RateError * P_PID[axis])>>7;
        //-----calculate I component
        errorGyroI[axis]  += RateError * I_PID[axis];
        //limit maximum integrator value to prevent WindUp - accumulating extreme values when system is saturated.
        //I coefficient (I8) moved before integration to make limiting independent from PID settings
        errorGyroI[axis]  = constrain(errorGyroI[axis], -GYRO_I_MAX, +GYRO_I_MAX);
        ITerm = 0; //errorGyroI[axis] * 0.0001;
        //-----calculate total PID output
        axisPID[axis] =  PTerm + ITerm + DTerm;
      }
}
