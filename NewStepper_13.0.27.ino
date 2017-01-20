
    /*changelog:
     * TRANSMOGRIFIER! THE STEREO ENCABULATOR
     * 
     * DONT FORGET TO CHANGE CODE ON LINE 151 OR SO FOR NEW FIRMWARE #!*
    12.6.27 adding changleog to codefixing libraries adding 3 presets for buffer delay (RAW, JPEG or OTHER) buffer delay modes, which can be adjutsed in BufferDelay menu added EEPROM values for 6,7,8 to remeber new settings on power cycle.
    12.7.27 attempt at bugfix in 12.6.27 about final count and reset on count up but made further bugs so abandoned and moved to 12.8
    12.8.27 fixed final count error.  took xmas pix with it.
    12.9.27 further fix timing of capture complete function and reset at complete
    13.0.27 update to chopper driver hardware protoboard (any firmare below 13.0.27 WILL NOT WORK with Chopper Driver hardware.  fix in photo array position some work on capture complete
    
    */
    #include <AccelStepper.h>
    // Define a stepper and the pins it will use  
// 27:1 gearing 200 step.  so 5400 is full rev

AccelStepper myStepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
    
    #include <EEPROM.h>
    #include <Wire.h>
  
    //LCD shield//
   // 
    #include <Adafruit_RGBLCDShield.h>
    #include <utility/Adafruit_MCP23017.h>
    int stepperSteps=200.00;//steps of stepper motor installed
  
    //**********
   //ints  

int numOfPhotos[]={50,40,32,25,20,16,12,10,8,4};//8 positions for numbers(0-9) 
int shutterSpeed[]={125,60,30,15,8,4,2,1,2,3,4,5,6,7,8}; //15 positions (0-14) be careful changing vals in array. check code...//
int photosArrayPosition=EEPROM.read(0);//for numOfPhotos  make EEPROM 0 (make 0 intitial load before EEPROM is programmed)
int shutterArrayPosition=EEPROM.read(1); // make EEPROM 1 make initial value 1
int gearBoxRatio =27.00;// gearbox ratiuo of gearbox attached to stepper
int delayPeriod = EEPROM.read (2) *50; //3100;//  make EEPROM 2 minimum delay beween shutter firings (make intitial EEPROM something like 60.
int delayPeriodII = EEPROM.read (6)*50;// =3000;//change EEPROM Value once loaded
int delayPeriodIII = EEPROM.read (7)*50; // =3000;//change EEPROM value once loaded
int delayMode= EEPROM.read(8);// =1// change to EEPROM after initial load

//EEPROM VALUES//
// 0 photosArrayPosition
//1 shutterArrayPosition
//2delayPeriod
//3 countDown
//4 endofTravel
//5 clockwise
//6 delayPeriodII
//7 delayPeriodIII
//8 delayMode

//int stepofMotor = 400;// assumes 100;1 gear, 200 step motor
float stepofMove = (gearBoxRatio*stepperSteps)/(numOfPhotos[photosArrayPosition]);//calc stepofMotor how many steps per move
int stepsInterval = 0;// how many total steps taken 20000= 1 rev
int pictureCount =0;//how many picture pairs
int countDown = EEPROM.read (3);//countdown before start make EEPROM 3 make initial value 5
int shutterPin = 12;//trigger for camera//
int piezoPin = 9 ;//buzzer//
int threeSixtyVal= gearBoxRatio*stepperSteps;// total steps for 360 move
int endofTravel=  EEPROM.read (4)* 25 ;//(gearBoxRatio*stepperSteps*1.25);// how far it rotates. in this case 1.25  make EEPROM4//
int menuWait=15000; //delay in sub menus 
float degreesPerStep= (360.0/threeSixtyVal); //360 degrees divided by total steps in revolution gives you degree per step
boolean clockwise=EEPROM.read(5);
//long eepromCount= 1;//EEPROM.read(6);
//int eepromCountMem=6; // 6, 7, 8,9 used for storing long value of use of EEPROM

//int previousMillis=0;

unsigned long lastInput = 0;//from sous vide for timer in menu
//*********/
//lcd stuff
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7
   
   
  
  
//**********SETUP*******//

    void setup(){
    { TWBR = ((F_CPU /400000l) - 16) / 2; // Change the i2c clock to 400KHz
    //  AFMS.begin(); // Start the shield

     myStepper.setMaxSpeed(4000);// steps per second (fastest at 16mhz is 4000 steps per sec)
    myStepper.setAcceleration(1500.0);// steps per second per second
     
  }
//***********
pinMode (shutterPin, OUTPUT);
digitalWrite (shutterPin, LOW);
pinMode (piezoPin, OUTPUT);
digitalWrite (piezoPin, LOW);

//************
//lcd stuff

//lcd commands
lcd.begin(16, 2);
//not sure what code below is.  for LCD buttons?
uint8_t i=0;
Serial.begin(9600);
     
lcd.setCursor (0,0);
   lcd.print (" Transmogrifier ");
   lcd.setCursor (0,1);
   
   
   
   lcd.print ("Firmware 13.0.27");
 delay (800); 
 //second page of splash screen
   lcd.setCursor (0,0);
   lcd.print ("   The Stereo   ");
   lcd.setCursor (0,1);
   
   
  
   lcd.print ("   Encabulator   ");
 delay (1300); 
 
;
 
 }
//***************

    void loop()
    //wait for SELECT before start//
  //  lcd.clear();
 // lcd.print ("SELECT to Start");
  
   {
     //lcd.clear();
 // start instructions
 lcd.setCursor (0,0);
   lcd.print ("SELECT to Start ");
  lcd.setCursor (0,1);
  lcd.print(numOfPhotos [photosArrayPosition]);
 // lcd.setCursor (4,1);
  lcd.print (" pix  ");
 // lcd.setCursor (9,1);
  lcd.print (degreesPerStep*stepofMove);// degress per 360
  //lcd.setCursor (13,1);
  lcd.print("deg ");
  myStepper.disableOutputs();
 //above LCD commands are for coming out of submenus
     
     uint8_t buttons = lcd.readButtons();
     //troubleshooting menus
     if (buttons & BUTTON_LEFT) 
     {
     EndofTravelFunction();
    }
    if (buttons & BUTTON_RIGHT) 
    {
      PixPer360Function();
    }
    if (buttons & BUTTON_DOWN) {
      PixPer360Function();
 
 }   
 
  if (buttons & BUTTON_UP) {
    PixPer360Function();
  
  }
   
if (buttons & BUTTON_SELECT) 

{
  lcd.clear ();
  // do something, i.e. the whole app//
  do  {
      
      
// intial countdown sequence before start rotation.
while (countDown > 0){

lcd.setCursor (0,0);
lcd.print ("   start in ");
//lcd.setCursor (10,);
lcd.print (countDown);
delay (300);
countDown --;
 tone(piezoPin, 4000, 300);
   lcd.clear();
delay (700);

//long beep before start//
 if (countDown == 0){tone (piezoPin, 4000,1500);
 //myMotor-> onestep (FORWARD,DOUBLE);// turn on motor engage// comment out for chopper
// myMotor move(1);

//NEED TO PUT ACTIVATE MOTOR COMMAND//
myStepper.enableOutputs();


 
 delay (1600);
 }
}
  unsigned long startTime=millis();
  lcd.setCursor (0, 1);
  lcd.print ("pix:");
  lcd.setCursor(7,1);
  lcd.print ("Deg:");
 //motor interval counter advance
 stepsInterval = stepsInterval + stepofMove;
  //bug of humans die//
  lcd.setCursor (0,0);
lcd.print("All Humans Die! ");
//delay after move before take picture
 delay (10);
 //activate shutter
 digitalWrite (shutterPin, HIGH);
 
 //update display
 
 pictureCount = pictureCount + 1;
  lcd.setCursor (4,1);
  lcd.print (pictureCount);
 lcd.setCursor (11,1);
  lcd.print (stepsInterval*degreesPerStep); //360 degree arc/20,000 steps= 0.018
  //Serial.print(stepsInterval * (360/gearBoxRatio*stepperSteps));
  
  //delay then turn off shutter trigger
 delay (30);
 digitalWrite (shutterPin, LOW);

 
 // freaining delay so no blurred shots
// Serial.print ("SHUTTER DELAY: ");
 //Serial.print (1000/shutterSpeed [shutterArrayPosition]);

 
 if (stepsInterval >=((threeSixtyVal)/100 * endofTravel))
  //old code  if (stepsInterval >=((threeSixtyVal-stepofMove)/100 * endofTravel))
  {CaptureCompleteFunction();
   //stepsInterval=0;
  // pictureCount=0;
   //may need to remove last two lines
  }
 
 
if (shutterArrayPosition <=6){delay (1000/shutterSpeed[shutterArrayPosition]);}//1000 milliseconds divided by shutter speed = how many milliseconds to wait.  1/30= 33mm
else {delay (shutterSpeed[shutterArrayPosition]*1000);}// delay in seconds rather than fractions of seconds
//buzzer for 360 coverage//
/*if (stepsInterval == threeSixtyVal) { 
tone(piezoPin, 4000, 700);
}*/

// erase all humans die bug
lcd.setCursor (0,0);
 lcd.print ("Transmogrifier");
 

    /*   //early fast code 50 revs in 20 sec 500 steps/sec
      stepper1.runSpeed();
      myMotor-> quickstep (FORWARD);
      delayMicroseconds(1025);
      */
    
     //ramp//
     //stepper1.setCurrentPosition(0);
       if (clockwise==false) {  myStepper.runToNewPosition(stepsInterval);}
       else {myStepper.runToNewPosition(-stepsInterval);}
         //counting time for buffer
        unsigned long currentTime=millis();
         unsigned long elapsedTime=currentTime-startTime;
         Serial.print ("ELAPSED");
         Serial.print (elapsedTime);
         
  

switch (delayMode) {
  case 1:
         if (elapsedTime<delayPeriod) 
        {delay ((delayPeriod- elapsedTime)/3*2 ); //delay 2/3 the delay
        lcd.setCursor (0,0);
lcd.print("All Humans Die! ");
delay ((delayPeriod- elapsedTime)/3  );// delay 1/3 the delay 
Serial.print ("  DELAY:");
    Serial.print (delayPeriod-elapsedTime);
}
break;


case 2:
if (elapsedTime<delayPeriodII) 
        {delay ((delayPeriodII- elapsedTime)/3*2 ); //delay 2/3 delay
        lcd.setCursor (0,0);
lcd.print("All Humans Die! ");
delay ((delayPeriodII- elapsedTime)/3  ); // delay 1/3 the delay
Serial.print ("  DELAY:");
    Serial.print (delayPeriodII-elapsedTime);
}
break;


case 3:
if (elapsedTime<delayPeriodIII) 
        {delay ((delayPeriodIII- elapsedTime)/3*2 ); //delay 2/3 the delay
        lcd.setCursor (0,0);
lcd.print("All Humans Die! ");
delay ((delayPeriodIII- elapsedTime)/3  );// delay 1/3 the delay
Serial.print ("  DELAY:");
    Serial.print (delayPeriodIII-elapsedTime);
}
break;
}
        
    
      
         
         
      

    }
    //matches do command at head of loop//
    while (stepsInterval <((threeSixtyVal)/100 * endofTravel));// the /100x is to compensate for avoiding floating point math. i think it is important where you put it or blows data types.
    }
   /*old code: while (stepsInterval <((threeSixtyVal-stepofMove)/100 * endofTravel));// the /100x is to compensate for avoiding floating point math. i think it is important where you put it or blows data types.
    }*/
    //end of capture commands:
 /* if (stepsInterval >=((threeSixtyVal)/100 * endofTravel))
  //old code  if (stepsInterval >=((threeSixtyVal-stepofMove)/100 * endofTravel))
  {CaptureCompleteFunction();
   stepsInterval=0;
   pictureCount=0;
   //may need to remove last two lines
  }
 */
   }
   
   //
   //********FUNCTIONS**********
   
  //******* PixPer360Function
   //****** CountDownFunction
     //****** BufferDelayFunction (avail via DelayModeFunction)
     //****** BufferDealyFunctionII
     //****** BufferDelayFunctionIII
   //****** DelayModeFunction
   //****** ShutterSpeedFunction
   //clockwiseFunction
   //****** EndofTravelFunction
   //CaptureCompleteFunction
   
   /**********************************************
*********PixPer360Function ******************
**********************************************/
   
void PixPer360Function ()
 {
  do
  {
//lcd.clear();
lcd.setCursor (0,0);
   lcd.print ("  Pix Per 360   ");
  lcd.setCursor (0,1);
  lcd.print(numOfPhotos[photosArrayPosition]);
  
  lcd.print(" pix ");
  lcd.print (stepofMove *degreesPerStep); 
  lcd.print (" deg ");
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_DOWN) {
 photosArrayPosition++;
 if (photosArrayPosition >9) {photosArrayPosition=0;}//if I add array positions change value
   stepofMove = (gearBoxRatio*stepperSteps)/(numOfPhotos[photosArrayPosition]);
 }   
 
  if (buttons & BUTTON_UP) {
   photosArrayPosition--;
   if (photosArrayPosition<0) {photosArrayPosition=9;} //if I add array positions, change value
 stepofMove = (gearBoxRatio*stepperSteps)/(numOfPhotos[photosArrayPosition]);//assign new value for stepofmotor
 
  }
   if (buttons & BUTTON_LEFT) {
 EEPROM.write(0, photosArrayPosition);// save # photos per 360 between power cylces
      //eeprom increment count
      /* long eepromCount= EEPROMReadlong(eepromCountMem);
       EEPROMWritelong(eepromCountMem, eepromCount+1);
       Serial.println (EEPROMReadlong(eepromCountMem));
    //  {
   //  eepromCount=eepromCount+1;
    //   Serial.println (eepromCount);
    */
      EndofTravelFunction();
      break;
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write(0, photosArrayPosition); //save # photos per 360 between power cycles
     /* //eeprom increment count
       long eepromCount= EEPROMReadlong(eepromCountMem);
       EEPROMWritelong(eepromCountMem, eepromCount+1);
       Serial.println (EEPROMReadlong(eepromCountMem));
   //   eepromCount=eepromCount+1;
   //   EEPROM.write (6, eepromCount);
    //   Serial.println (eepromCount);
    */
     CountDownFunction();
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write(0, photosArrayPosition); //save photos per 360 between power cycles
       /*//eeprom increment count
       long eepromCount= EEPROMReadlong(eepromCountMem);
       EEPROMWritelong(eepromCountMem, eepromCount+1);
       Serial.println (EEPROMReadlong(eepromCountMem));
       // Serial.println (eepromCount);
       */
     break;
   }
  }
  }
  while 
  //(menuEntryMillis + menuWait>= millis());
  (menuWait>0);
 }

 
 ///
/**********************************************
*********CountDownFunction ******************
**********************************************/ 
 
 void CountDownFunction ()
 {
  do
  {
 // lcd.clear();
 lcd.setCursor (0,0);
    lcd.print ("   Countdown    ");
    lcd.setCursor (0,1);
    lcd.print ("   ");
    lcd.print(countDown);
    lcd.print (" seconds    ");
    delay (20);
uint8_t buttons = lcd.readButtons();
if (buttons) {
  if (buttons & BUTTON_UP) {
    countDown++;
   }
  if (buttons & BUTTON_DOWN) {
   countDown--;
   if (countDown <0) {countDown =0;}
    }
   if (buttons & BUTTON_LEFT) {
     EEPROM.write (3,countDown);
      PixPer360Function();
      break;
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write (3,countDown);
     DelayModeFunction();
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write (3,countDown);
     break;
   }
  }
  }
  while 
   (menuWait>0);
 }

 /**********************************************
*********BufferDelayFunction ******************
**********************************************/

 void BufferDelayFunction()
 {
  do
  {
lcd.setCursor (0,0);
   lcd.print (" Buffer Delay 1 ");
  lcd.setCursor (0,1);
  //lcd.print ("  ");
  lcd.print (delayPeriod);
  lcd.print(" millisecs    ");
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_UP) 
  {
 delayPeriod= (delayPeriod +50);
   }
  if (buttons & BUTTON_DOWN) 
  {
   delayPeriod= (delayPeriod -50);
   if (delayPeriod<0) {delayPeriod=0;} //make sure does not go below 0
    }
   if (buttons & BUTTON_LEFT) {
     EEPROM.write(2, delayPeriod/50);
     
      BufferDelayFunctionIII();
      break;  
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write(2, delayPeriod/50);
     BufferDelayFunctionII();
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write(2, delayPeriod/50);
       DelayModeFunction();
     break;
   }
  }
  }
  while 
   (menuWait>0);
 }
 

  /**********************************************
*********BufferDelayFunctionII ******************
**********************************************/

 void BufferDelayFunctionII()
 {
  do
  {
lcd.setCursor (0,0);
   lcd.print (" Buffer Delay 2 ");
  lcd.setCursor (0,1);
  //lcd.print ("  ");
  lcd.print (delayPeriodII);
  lcd.print(" millisecs    ");
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_UP) 
  {
 delayPeriodII= (delayPeriodII +50);
   }
  if (buttons & BUTTON_DOWN) 
  {
   delayPeriodII= (delayPeriodII -50);
   if (delayPeriodII<0) {delayPeriodII=0;} //make sure does not go below 0
    }
   if (buttons & BUTTON_LEFT) {
     EEPROM.write(6, delayPeriodII/50);
     
      BufferDelayFunction();
      break;  
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write(6, delayPeriodII/50);
     BufferDelayFunctionIII();
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write(6, delayPeriodII/50);
       DelayModeFunction();
     break;
   }
  }
  }
  while 
   (menuWait>0);
 }

  /**********************************************
*********BufferDelayFunctionIII ******************
**********************************************/

 void BufferDelayFunctionIII()
 {
  do
  {
lcd.setCursor (0,0);
   lcd.print (" Buffer Delay 3 ");
  lcd.setCursor (0,1);
  //lcd.print ("  ");
  lcd.print (delayPeriodIII);
  lcd.print(" millisecs    ");
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_UP) 
  {
 delayPeriodIII= (delayPeriodIII +50);
   }
  if (buttons & BUTTON_DOWN) 
  {
   delayPeriodIII= (delayPeriodIII -50);
   if (delayPeriodIII< 0) {delayPeriodIII= 0;} //make sure does not go below 0
    }
   if (buttons & BUTTON_LEFT) {
     EEPROM.write(7, (delayPeriodIII/50));
     
      BufferDelayFunctionII();
     
      break;  
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write(7, (delayPeriodIII/50));
     BufferDelayFunction();
     
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write(7, (delayPeriodIII/50));
       DelayModeFunction();
     break;
   }
  }
  }
  while 
   (menuWait>0);
 }
 /**********************************************
*********DelayModeFunction ******************
**********************************************/ 
 
 void DelayModeFunction ()
 {
  do
  {
 // lcd.clear();
 lcd.setCursor (0,0);
    lcd.print ("BufferDelay Mode");
    lcd.setCursor (0,1);
    lcd.print ("Preset:"); 
    lcd.print(delayMode);
    if (delayMode== 1) lcd.print (" (RAW)  ");
    if (delayMode ==2) lcd.print (" (JPEG) ");
    if (delayMode==3) lcd.print (" (OTHER)");
    
    delay (20);
uint8_t buttons = lcd.readButtons(); 
if (buttons) {
  if (buttons & BUTTON_UP) {
    delayMode++;
     if (delayMode >3) {delayMode =3;}
   
   }
  if (buttons & BUTTON_DOWN) {
   delayMode--;
   if (delayMode <1) {delayMode =1;}
    }
   if (buttons & BUTTON_LEFT) {
     EEPROM.write (8,delayMode);
      CountDownFunction();
      break;
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write (8,delayMode);
     ShutterSpeedFunction();
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write (8,delayMode);
       if (delayMode==1) BufferDelayFunction();
       if (delayMode ==2) BufferDelayFunctionII();
       if (delayMode ==3) BufferDelayFunctionIII();
       
     break;
   }
  }
  }
  while 
   (menuWait>0);
 }
/*********************************************
************ShutterSpeedFunction***************
*********************************************/
void ShutterSpeedFunction ()
 {
  do
  {
//lcd.clear();
lcd.setCursor (0,0);
   lcd.print ("  Shutter Speed ");
  lcd.setCursor (0,1);
  if (shutterArrayPosition <=6) {
  lcd.print ("1/");
lcd.print(shutterSpeed[shutterArrayPosition]);
lcd.print(" (or ");
  lcd.print(1000/shutterSpeed[shutterArrayPosition]);
  lcd.print ("ms)  ");
}
else
{
  lcd.print(shutterSpeed[shutterArrayPosition]);
  lcd.print ("sec");
  
  lcd.print(" (or ");
  lcd.print(1000*shutterSpeed[shutterArrayPosition]);
  lcd.print ("ms)  ");
  }
  
  
  
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_DOWN) {
 shutterArrayPosition++;
 if (shutterArrayPosition >14) {shutterArrayPosition=14;}//if I add array positions change value
   
 }   
 
  if (buttons & BUTTON_UP) {
   shutterArrayPosition--;
   if (shutterArrayPosition<0) {shutterArrayPosition=0;} //if I add array positions, change value

 
  }
   if (buttons & BUTTON_LEFT) {
     EEPROM.write(1,shutterArrayPosition);
      DelayModeFunction();
      break;
    }
   if (buttons & BUTTON_RIGHT) {
     EEPROM.write(1,shutterArrayPosition);
     ClockwiseFunction();
     break;
   }
    if (buttons & BUTTON_SELECT) {
      EEPROM.write(1,shutterArrayPosition);
     break;
   }
  }
  }
  while 
  //(menuEntryMillis + menuWait>= millis());
  (menuWait>0);
 }
 
 /*********************************************
*********** Define ReadButtons*****************
**********************************************/

 //not sure where this goes //for getting out of menu
 uint8_t ReadButtons()
{
  uint8_t buttons = lcd.readButtons();
  if (buttons != 0)
  {
    lastInput = millis();
  }
  return buttons;
}

/*********************************************
************ClockwiseFunction*****************
*********************************************/
void ClockwiseFunction ()
 {
  do
  {
lcd.setCursor (0,0);
   lcd.print ("   Rotation     ");
  lcd.setCursor (0,1);
 if (clockwise==false) {lcd.print("Counterclockwise");
 Serial.print ("counter  ");}
  
  
  else {lcd.print("   Clockwise    ");
Serial.print ("clockwise  ");}
 delay (20);
 
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_UP) {
    clockwise=false; 
    Serial.print ("FALSE");
  }
   
  if (buttons & BUTTON_DOWN) {
   clockwise= true;
  Serial.print ("TRUE");
}
   if (buttons & BUTTON_LEFT) {
     EEPROM.write(5,clockwise);
     ShutterSpeedFunction();
      break;
    }
   if (buttons & BUTTON_RIGHT) {
     EEPROM.write(5,clockwise);
     EndofTravelFunction();
     break;
   }
    if (buttons & BUTTON_SELECT) {
      EEPROM.write(5,clockwise);
     break;
   }
  }
  }
  
  while
  (menuWait>0);
 }
 

/**********************************************
*********EndofTravelFunction ******************
**********************************************/


void EndofTravelFunction ()
 {
  do
  {
lcd.setCursor (0,0);
   lcd.print (" X of Rotation  ");
  lcd.setCursor (0,1);
  lcd.print("    "); //4 spaces
  lcd.print( float(endofTravel)/100);//(gearBoxRatio*stepperSteps)); //200 is steps of motor
  lcd.print ("x       ");//7 spaces
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_UP) {
    endofTravel = (endofTravel +25); //(gearBoxRatio*stepperSteps* 0.25));
     if (endofTravel > 300) {endofTravel =300;}//gearBoxRatio*stepperSteps*3) {endofTravel= gearBoxRatio*stepperSteps*3;}//keeps rtation down to 3X and inside int top number
 }
   
  if (buttons & BUTTON_DOWN) {
   endofTravel = (endofTravel- 25);//(gearBoxRatio*stepperSteps*0.25);
   if (endofTravel < 100) {endofTravel =100;}//(gearBoxRatio*stepperSteps )) {endofTravel= (gearBoxRatio*stepperSteps);}//keeps it at 1x rotation
}
   if (buttons & BUTTON_LEFT) {
     EEPROM.write(4,endofTravel/25);
     ClockwiseFunction();
      break;
    }
   if (buttons & BUTTON_RIGHT) {
      EEPROM.write(4,endofTravel/25);
     PixPer360Function();
     break;
   }
    if (buttons & BUTTON_SELECT) {
       EEPROM.write(4,endofTravel/25);
     break;
   }
  }
  }
  
  while
  (menuWait>0);
 }
 /*****************************************
 *********CaptureCompleteFunction**********
 ******************************************/
 void CaptureCompleteFunction ()
 {
  do
  {
    
    // myMotor-> release();// comment out for chopper
//lcd.clear();
lcd.setCursor (0,0);
   lcd.print ("CAPTURE COMPLETE");
   lcd.setCursor (0,1);
   lcd.print (pictureCount);
   lcd.print ("pix ");
   lcd.print (stepsInterval*degreesPerStep);
   lcd.print ("deg.   ");
    myStepper.setCurrentPosition(0); //set current position as 0
     myStepper.disableOutputs();
   //void release (void); //release stepper//
  
  delay (20);
uint8_t buttons = lcd.readButtons();

 if (buttons) {
  if (buttons & BUTTON_DOWN) {
    countDown = EEPROM.read (3);
    stepsInterval=0;
    pictureCount=0;
     EndofTravelFunction ();
     lcd.clear();
     break;
 
 }   
 
  if (buttons & BUTTON_UP) {
    countDown = EEPROM.read (3);
    stepsInterval=0;
    pictureCount=0;
     EndofTravelFunction ();
     lcd.clear();
     break;
  
  }
   if (buttons & BUTTON_LEFT) {
    countDown = EEPROM.read (3);
    stepsInterval=0;
    pictureCount=0;
     EndofTravelFunction ();
     lcd.clear();
     break;
    }
   if (buttons & BUTTON_RIGHT) {
    countDown = EEPROM.read (3);
    stepsInterval=0;
    pictureCount=0;
    PixPer360Function ();
    lcd.clear();
    break;
   }
    if (buttons & BUTTON_SELECT) {
      countDown = EEPROM.read (3);
       stepsInterval=0;
   pictureCount=0;
   lcd.clear();
     break;
   }
  }
  }
  while 
  //(menuEntryMillis + menuWait>= millis());
  (menuWait>0);
 }
 
 
 //This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

