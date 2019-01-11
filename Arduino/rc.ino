int cp = 2000;

#ifdef ESP32
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#endif

void mix()
{
  if (stateRotor > 0)
  {

    servo[stateRotor - 1] = 2000;
    if (cp-- == 0)
    {
      cp = 2000;
      stateRotor = stateRotor < 4 ? stateRotor  + 1 : -1;
    }
  }

  if (armed & (rcValue[THR] > MINTHROTTLE))
  {
    servo[0] = constrain(rcValue[THR] - axisPID[ROLL] + axisPID[PITCH] - axisPID[YAW], 1000, 2000);
    servo[1] = constrain(rcValue[THR] - axisPID[ROLL] - axisPID[PITCH] + axisPID[YAW], 1000, 2000);
    servo[2] = constrain(rcValue[THR] + axisPID[ROLL] + axisPID[PITCH] + axisPID[YAW], 1000, 2000);
    servo[3] = constrain(rcValue[THR] + axisPID[ROLL] - axisPID[PITCH] - axisPID[YAW], 1000, 2000);
    Serial.print(servo[0]); Serial.print(" ");
    Serial.print(servo[1]); Serial.print(" ");
    Serial.print(servo[2]); Serial.print(" ");
    Serial.print(servo[3]); Serial.println("");
    //servo[0] = constrain(rcValue[THR],MINTHROTTLE,2000);
    //servo[1] = constrain(rcValue[THR],MINTHROTTLE,2000);
    //servo[2] = constrain(rcValue[THR],MINTHROTTLE,2000);
    //servo[3] = constrain(rcValue[THR],MINTHROTTLE,2000);
  }
  else
  {
    axisPID[0] = 0; axisPID[1] = 0; axisPID[2] = 0;
    servo[0] = 1000; servo[1] = 1000; servo[2] = 1000; servo[3] = 1000;
  }

}

uint8_t  pwmActChan = 0;
uint32_t pwmServo[4] = {80000, 80000, 80000, 80000};
uint32_t next;

void  PWM_ISR(void)
{
  next += pwmServo[pwmActChan];
#ifndef ESP32
  timer0_write(next);
#else
  timerAlarmWrite(timer, next, true);
#endif
  switch (pwmActChan)
  {
    case 0:
      digitalWrite(pwmpin4, LOW);
      digitalWrite(pwmpin1, HIGH);
      pwmActChan = 1;
      break;
    case 1:
      digitalWrite(pwmpin1, LOW);
      digitalWrite(pwmpin2, HIGH);
      pwmActChan = 2;
      break;
    case 2:
      digitalWrite(pwmpin2, LOW);
      digitalWrite(pwmpin3, HIGH);
      pwmActChan = 3;
      break;
    case 3:
      digitalWrite(pwmpin3, LOW);
      digitalWrite(pwmpin4, HIGH);
      pwmActChan = 0;
      break;
  }
}

void writeServo()
{
  pwmServo[0] = servo[0] * 80;
  pwmServo[1] = servo[1] * 80;
  pwmServo[2] = servo[2] * 80;
  pwmServo[3] = servo[3] * 80;
}

void initServo()
{
  pinMode(pwmpin1, OUTPUT);
  pinMode(pwmpin2, OUTPUT);
  pinMode(pwmpin3, OUTPUT);
  pinMode(pwmpin4, OUTPUT);
  noInterrupts();
#ifndef ESP32
  timer0_isr_init();
  timer0_attachInterrupt(PWM_ISR);
  next = ESP.getCycleCount() + 100000;
  timer0_write(next);
#else
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &PWM_ISR, true);
  next = ESP.getCycleCount() + 100000;
  timerAlarmWrite(timer, next, true);
  timerAlarmEnable(timer);

#endif
  interrupts();
}
