#include <string.h>
#include "myServo.h"
float angleLimit(float angle, float max,float min){
  if (angle > max){
    angle = max; 
  }
  if (angle < min){
    angle = min;
  }
  return angle;
}

//-- type 1
//command, called from main lane
void GotoAngleInMs(SERVO *servo,float angle,long ms){
  (*servo).pendingJob.workmode = 1;
  (*servo).pendingJob.targetAngle = angleLimit(angle,servo->angleMax,servo->angleMin);
  (*servo).pendingJob.countdownInMs = ms;
  (*servo).pendingJob.totalTimeInMs = ms;
  (*servo).hasPendingJob = 1;
}

// cb, called from interrupt lane
void MotionTypeHandler_1(SERVO *servo){
  if (servo->currentJob.countdownInMs == 0){
    (*servo).currentJobDne = 1;
  }else if (servo->angle != servo->currentJob.targetAngle){
    if( servo->currentJob.countdownInMs <= servo->timerPeriod){
      (*servo).angle = servo->currentJob.targetAngle;
      (*servo).currentJob.countdownInMs = 0;
    }else{
      float angle = servo->angle + (servo->currentJob.targetAngle - servo->angle) * servo->timerPeriod / servo->currentJob.countdownInMs;
      (*servo).angle = angleLimit(angle,servo->angleMax,servo->angleMin);
      (*servo).currentJob.countdownInMs = servo->currentJob.countdownInMs - servo->timerPeriod;
    }
  } 
}


//-- type 2
//command, called from main lane
void DoJerkInMs(SERVO *servo,float velocity, float accelerate, float jerk, long ms){
  (*servo).pendingJob.workmode = 2;
  (*servo).pendingJob.velocity = velocity;
  (*servo).pendingJob.accelerate = accelerate;
  (*servo).pendingJob.jerk = jerk;
  (*servo).pendingJob.countdownInMs = ms;
  (*servo).pendingJob.totalTimeInMs = ms;
  (*servo).hasPendingJob = 1;
}
//command, called from main lane
void DoAccelerateInMs(SERVO *servo,float velocity, float accelerate, long ms){
  DoJerkInMs(servo, velocity, accelerate, 0, ms);
}
//command, called from main lane
void DoVelocityInMs(SERVO *servo, float velocity, long ms){
  DoAccelerateInMs(servo, velocity, 0, ms);
}
// cb, called from interrupt lane
void MotionTypeHandler_2(SERVO *servo){
  if (servo->currentJob.countdownInMs == 0 ){
    (*servo).currentJobDne = 1;
  }else{
    long passedTime = servo->timerPeriod;
    if (servo->currentJob.countdownInMs <= servo->timerPeriod){
      passedTime = servo->currentJob.countdownInMs;
    }
    (*servo).currentJob.accelerate = servo->currentJob.accelerate + servo->currentJob.jerk * passedTime / 1000;
    (*servo).currentJob.velocity = servo->currentJob.velocity + servo->currentJob.accelerate * passedTime / 1000;
    float angle = (servo->currentJob.velocity * passedTime / 1000);
    (*servo).angle = angleLimit(angle,servo->angleMax,servo->angleMin);
    (*servo).currentJob.countdownInMs = servo->currentJob.countdownInMs - passedTime;
  }
}

//-- type 3 (extended)
//command, called from main lane
void ExGotoAngleInMs(SERVO *servo, long ms, 
long (*exAngleFunc)(float angle, long ms, long ttlms)){
  (*servo).pendingJob.workmode = 3;
  (*servo).pendingJob.exAngleFunc = exAngleFunc;
  (*servo).pendingJob.countdownInMs = ms;
  (*servo).pendingJob.totalTimeInMs = ms;
  (*servo).hasPendingJob = 1;
}
// cb, called from interrupt lane
void MotionTypeHandler_3(SERVO *servo){
  if (servo->currentJob.countdownInMs == 0 ){
    (*servo).currentJobDne = 1;
  }else{
    long angle = servo->currentJob.exAngleFunc(servo->angle, servo->currentJob.countdownInMs, servo->currentJob.totalTimeInMs);
    (*servo).angle = angleLimit(angle, servo->angleMax, servo->angleMin);
    if (servo->currentJob.countdownInMs <= servo->timerPeriod){
      (*servo).currentJob.countdownInMs = 0;
    }else{
      (*servo).currentJob.countdownInMs = servo->currentJob.countdownInMs - servo->timerPeriod;
    }
  }
}

//-- type 4 (extended)
//command, called from main lane
void ExDoVelocityInMs(SERVO *servo, float velocity, long ms, 
long (*exVelocityFunc)(float angle, float velocity, long ms, long ttlms)){
  (*servo).pendingJob.workmode = 4;
  (*servo).pendingJob.velocity = velocity;
  (*servo).pendingJob.exVelocityFunc = exVelocityFunc;
  (*servo).pendingJob.countdownInMs = ms;
  (*servo).pendingJob.totalTimeInMs = ms;
  (*servo).hasPendingJob = 1;
}
// cb, called from interrupt lane
void MotionTypeHandler_4(SERVO *servo){
  if (servo->currentJob.countdownInMs == 0 ){
    (*servo).currentJobDne = 1;
  }else{
    long angle = servo->currentJob.exVelocityFunc(servo->angle, servo->currentJob.velocity, servo->currentJob.countdownInMs, servo->currentJob.totalTimeInMs);
    (*servo).angle = angleLimit(angle, servo->angleMax, servo->angleMin);
    if (servo->currentJob.countdownInMs <= servo->timerPeriod){
      (*servo).currentJob.countdownInMs = 0;
    }else{
      (*servo).currentJob.countdownInMs = servo->currentJob.countdownInMs - servo->timerPeriod;
    }
  }
}

//-- type 5 (extended)
//command, called from main lane
void ExDoAccelerateInMs(SERVO *servo,float velocity, float accelerate, long ms, 
long (*exAccelerateFunc)(float angle, float velocity, float accelerate, long ms, long ttlms)){
  (*servo).pendingJob.workmode = 5;
  (*servo).pendingJob.velocity = velocity;
  (*servo).pendingJob.accelerate = accelerate;
  (*servo).pendingJob.exAccelerateFunc = exAccelerateFunc;
  (*servo).pendingJob.countdownInMs = ms;
  (*servo).pendingJob.totalTimeInMs = ms;
  (*servo).hasPendingJob = 1;
}
// cb, called from interrupt lane
void MotionTypeHandler_5(SERVO *servo){
  if (servo->currentJob.countdownInMs == 0 ){
    (*servo).currentJobDne = 1;
  }else{
    long angle = servo->currentJob.exAccelerateFunc(
                  servo->angle, servo->currentJob.velocity, servo->currentJob.accelerate, 
                  servo->currentJob.countdownInMs, servo->currentJob.totalTimeInMs);
    (*servo).angle = angleLimit(angle, servo->angleMax, servo->angleMin);
    if (servo->currentJob.countdownInMs <= servo->timerPeriod){
      (*servo).currentJob.countdownInMs = 0;
    }else{
      (*servo).currentJob.countdownInMs = servo->currentJob.countdownInMs - servo->timerPeriod;
    }
  }
}

//-- type 6 (extended)
//command, called from main lane
void ExDoJerkInMs(SERVO *servo,float velocity, float accelerate, float jerk, long ms,
long (*exJerkFunc)(float angle, float velocity, float accelerate, float jerk, long ms, long ttlms)){
  (*servo).pendingJob.workmode = 6;
  (*servo).pendingJob.velocity = velocity;
  (*servo).pendingJob.accelerate = accelerate;
  (*servo).pendingJob.jerk = jerk;
  (*servo).pendingJob.exJerkFunc = exJerkFunc;
  (*servo).pendingJob.countdownInMs = ms;
  (*servo).pendingJob.totalTimeInMs = ms;
  (*servo).hasPendingJob = 1;
}
// cb, called from interrupt lane
void MotionTypeHandler_6(SERVO *servo){
  if (servo->currentJob.countdownInMs == 0 ){
    (*servo).currentJobDne = 1;
  }else{
    long angle = servo->currentJob.exJerkFunc(servo->angle, servo->currentJob.velocity, servo->currentJob.accelerate, servo->currentJob.jerk, servo->currentJob.countdownInMs, servo->currentJob.totalTimeInMs);
    (*servo).angle = angleLimit(angle, servo->angleMax, servo->angleMin);
    if (servo->currentJob.countdownInMs <= servo->timerPeriod){
      (*servo).currentJob.countdownInMs = 0;
    }else{
      (*servo).currentJob.countdownInMs = servo->currentJob.countdownInMs - servo->timerPeriod;
    }
  }
}





//-- type switcher
void ServoTimerCallback(SERVO *servo){
  // 用暫存區和暫存指標的方式來解 critical section
  // 低優先(main)填寫，高優先(int)套用受理
  if(servo->hasPendingJob){
    servo->hasPendingJob    = 0;
    servo->currentJobDne    = 0;
    memcpy((void *)&(servo->currentJob),(void *)&(servo->currentJob),sizeof(JOB));
  }
  if(servo->power == 0){
    (*servo).currentJob.workmode = 0;
    return;
  }
  if(servo->halt == 1){
    return;
  }
  switch(servo->currentJob.workmode){
    case(1):
      MotionTypeHandler_1(servo);
      break;
    case(2):
      MotionTypeHandler_2(servo);
      break;
    case(3):
      MotionTypeHandler_3(servo);
      break;
    case(4):
      MotionTypeHandler_4(servo);
      break;
    case(5):
      MotionTypeHandler_5(servo);
      break;
    case(6):
      MotionTypeHandler_6(servo);
      break;
    default:{
      (*servo).currentJob.workmode = 0;
      (*servo).currentJobDne = 1;
    }
  }
  servo->DoAngle(angleLimit(servo->angle,servo->angleMax,servo->angleMin));
}
