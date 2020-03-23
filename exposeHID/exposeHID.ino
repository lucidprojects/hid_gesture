/*
   ExposéHID
   by Jake Sherwood
   created March 14, 2020
   last modified March 23, 2020

   About:
   ExposéHID is a gesture device to control common macOS X Exposé functions.

   ExposéHID uses the Arduino 33 IoT's integrated IMU to control certain modes
   based on the user's gestures.

   ExposéHID has 5 modes: sleep (on/off with capacitive ring button),
   tab through open apps (gesture left), show all open docs in a specific app (gesture up),
   show all open apps and docs (gesture down), and open Launchpad (gesture right).

   In certain modes ExposéHID will switch to mouse mode to allow the user to navigate and
   make a selection with the mouse.

   Libraries:
   ExposéHID uses Keyboard.h & Mouse.h to control computer keyboard and mouse functions
   Arduino Nano 33 IoT's built in IMU with LSM6DS3.h lib for gestures
   Adafruit DRV2605 for haptic feedback
   CapacitiveSensor.h for capacitive button smoothing

   Adapted IMU and threshold code from Atharva Punch Punch Revolution
   https://github.com/atharvapatil/punch-punch-revolution

*/

#include <Arduino_LSM6DS3.h>
#include <Mouse.h>
#include <Keyboard.h>
#include "Adafruit_DRV2605.h"
#include <CapacitiveSensor.h>

CapacitiveSensor Sensor = CapacitiveSensor(9, 12);
long val;  // value from capacitive button
boolean onOff = false;
int capSenstivity = 50000;  // adjust this value to adjust capacitive touch sensitivity

Adafruit_DRV2605 drv;

char ctrlKey = KEY_LEFT_GUI;
float x, y, z, delta, x11, y13 , xL, xR, yU, yD;

boolean tabApps = false;
boolean showDocs = false;
boolean engageMouse = false;
boolean showApps = false;
boolean launchPad = false;
float aSum;
boolean sleep;

//mouse switch -not in use for final but leaving if physical button needed later
//int mouseBtn = 5;
//int mouseBtnPushCounter = 0; // counter for the number of btn presses
//int mouseBtnbuttonState = 0; // current state of the button
//int mouseBtnLastButtonState = 0; //previous state of the button

//RGB led pins
int red = 2; //this sets the red led pin
int green = 6 ; //this sets the green led pin
int blue = 4; //this sets the blue led pin

// LaunchPad left & right booleans
int lpleft = 1;
int lpright = 1;

void setup() {
  Serial.begin(9600);
  // not used in final but left if physical button desired by others
  //  pinMode(mouseBtn, INPUT_PULLUP);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    exit(1);
  }

  Keyboard.begin();
  Mouse.begin();

  Serial.println("DRV test");
  drv.begin();
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);

}

void loop() {


  // not used in final but left if physical button desired by others
  // MOUSE CLICK BUTTON STATE CODE
  // mouseBtnbuttonState = digitalRead(mouseBtn);
  //
  //  if (mouseBtnbuttonState != mouseBtnLastButtonState) {
  //    if (mouseBtnbuttonState == LOW) {
  //      mouseBtnPushCounter++;
  //      Serial.println("on");
  //      Serial.print("number of button pushes: ");
  //      Serial.println(mouseBtnPushCounter);
  //    } else {
  //      Serial.println("off");
  //    }
  //    delay(50);
  //  }
  //
  //  mouseBtnLastButtonState = mouseBtnbuttonState;
  //
  //  if (mouseBtnPushCounter % 2 == 0) {
  //    // digitalWrite(green, HIGH);
  //
  //
  //  } else {
  //    // digitalWrite(green, LOW);
  //  }
  //END MOUSE CLICK CODE

  // Check Capacitve Ring button to toggle Sleep Mode
  val = Sensor.capacitiveSensor(30); // loop for samples parameter - simple lowpass filter

  if (val >= 100 && onOff == false) {
    Serial.print(val);
    Serial.print("  ");
    Serial.println(onOff);
  }

  if (val >= capSenstivity && onOff == false) {
    onOff = true;
    delay(200); // small delay for better capacitve
  } else if (val >= capSenstivity && onOff == true) {
    Serial.println("I'm true");
    onOff = false;
    delay(200); // small delay for better capacitve
  }

  if (onOff == false) {
    digitalWrite(red, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(blue, HIGH);

    sleep = true;
  } else if (onOff == true) {
    digitalWrite(red, HIGH);
    digitalWrite(blue, LOW);
    digitalWrite(green, HIGH);
    sleep = false;
  }

  delta = 0.05; // delta for IMU if statements

  //check for IMU val only if not in Sleep mode
  if (IMU.accelerationAvailable() && sleep == false)  {
    IMU.readAcceleration(x, y, z);
    if (y <= delta && y >= -delta) {
      // Serial.println("flat");
    } else if (y >= 0.50) {
      doKeyBoardMouse(1);   // TAB THROUGH OPEN APPS
    }
    else if (y > delta && y < 1 - delta) {
      Serial.println("tilted to the left");
      Serial.print("left y = ");
      Serial.println(y);
    }
    else if (y >= 1 - delta) {
      Serial.println("left");
    }
    else if (y < -delta && y > delta - 1) {
      Serial.println("tilted to the right");
      Serial.print("right y = ");
      Serial.println(y);
    }
    else {
      Serial.println("right");
      doKeyBoardMouse(3); // LAUNCHPAD
    }
    if (x <= delta && x >= -delta) {
      //   Serial.println("flat");
    }
    else if (x > 0.25 && x < 1 - delta) {
      Serial.println("tilted to the forward");
      doKeyBoardMouse(5); // ALL OPEN APPS ALL WINDOWS
    }
    else if (x >= 1 - delta) {
      Serial.println("forward");
    }
    else if (x < -delta && x > delta - 1) {
      Serial.println("tilted to back");
    }
    else if (x < -0.80) {
      Serial.println("back");
      doKeyBoardMouse(4); // ALL OPEN DOCS IN APP
    }
    else {
      //noop;
    }
    // end if IMU avail
  }
}


void doKeyBoardMouse(int thisVal) {

  switch (thisVal) {
    case 0:
      //noop;
      Serial.println("sent val 0");
      Serial.println("case 0");
      break;

    // TAB THROUGH APPS - both ways
    // adjusted to enter tab thru apps mode with gesture
    // then gesture right / left to select desired app, capactive touch button to select
    // this was to handle the poor reaction time of previous version of tab thru mode (case 2)
    case 1:
      Serial.println("sent val 1");
      Serial.println("cmd tab- case 1");
      doHaptics(64);
      digitalWrite(red, HIGH);
      digitalWrite(blue, HIGH);
      digitalWrite(green, LOW);
      tabApps = true; // boolean used to keep tab thru apps view active
      while (tabApps == true) {
        IMU.readAcceleration(x, y, z);
        Serial.print("my y val in app tab case = ");
        Serial.println(y);
        if (y <= -0.25) {
          IMU.readAcceleration(x, y, z);
          Serial.print("my y val in app tab case = ");
          Serial.println(y);
          Keyboard.release(KEY_LEFT_SHIFT);
          Keyboard.press(ctrlKey);
          delay(200);
          Keyboard.press(KEY_TAB);
          delay(50);
          Keyboard.release(KEY_TAB);
          delay(50);
        }


        if (y >= 0.25) {
          IMU.readAcceleration(x, y, z);
          Serial.print("my y val in app tab case = ");
          Serial.println(y);
          Keyboard.press(ctrlKey);
          delay(200);
          Keyboard.press(KEY_LEFT_SHIFT);
          delay(200);
          Keyboard.press(KEY_TAB);
          delay(50);
          Keyboard.release(KEY_TAB);
          delay(50);
        }

        //check cap val again for selection button
        val = Sensor.capacitiveSensor(30);
        if (val >= capSenstivity) {
          Keyboard.release(ctrlKey);
          delay(50);
          Keyboard.releaseAll();
          tabApps = false;
          doHaptics(119);
          digitalWrite(red, HIGH);
          digitalWrite(blue, LOW);
          digitalWrite(green, HIGH);
          delay(200);
        }

      }
      break;


    // TAB THROUGH APPS - to the right only
    // this version has poor selection reaction times (use case 1)
    case 2:
      Serial.println("sent val 2");
      Serial.println("cmd tab- case 2");
      doHaptics(64);
      digitalWrite(red, HIGH);
      digitalWrite(blue, HIGH);
      digitalWrite(green, LOW);
      while (y >= 0.5) {
        IMU.readAcceleration(x, y, z);
        Serial.print("my y val in app tab case = ");
        Serial.println(y);
        Keyboard.press(ctrlKey);
        delay(200);
        Keyboard.press(KEY_TAB);
        delay(300);
        Keyboard.release(KEY_TAB);  //needed to continue tabbing thru apps otherwise only tabs once to the end
        delay(100); // smaller delays here make the tab thru apps too fast - increasing slows down but doesn't help
      }
      if (y < 0.45 ) { // turning HID back to right releases ALL and selects the desired app
        Keyboard.releaseAll();
        doHaptics(119);
        digitalWrite(red, HIGH);
        digitalWrite(blue, LOW);
        digitalWrite(green, HIGH);
      }
      break;


    //  OPEN LAUNCHPAD
    case 3:
      Serial.println("cmd tab- case 3");
      digitalWrite(red, HIGH);
      digitalWrite(blue, HIGH);
      digitalWrite(green, LOW);
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press(' ');
      Keyboard.releaseAll();
      delay(300);
      Keyboard.print("launchpad");
      delay(500);
      Keyboard.println();
      doHaptics(64);
      launchPad = true; // boolean used to keep launchPad view active
      while (launchPad == true) {
        IMU.readAcceleration(x, y, z);
        if ( y > 0.15 && y < 0.85) {
          if (lpleft < 2) {
            Serial.println("launchpad left+");
            Serial.println(lpleft);
            lpleft++;
            delay(500);
            Keyboard.press(ctrlKey);
            delay(300);
            Keyboard.press(KEY_LEFT_ARROW);
            delay(300);
            Keyboard.releaseAll();
            delay(500);
            lpright = 1;
            lpleft = 1;
          }
        }
        if (y < -0.15 && y > -0.85) {
          if ( lpright < 2) {
            Serial.println("launchpad right-");
            Serial.println(lpright);
            lpright++;
            delay(500);
            Keyboard.press(ctrlKey);
            delay(300);
            Keyboard.press(KEY_RIGHT_ARROW);
            delay(300);
            Keyboard.releaseAll();
            delay(500);
            lpleft = 1;
            lpright = 1;
          }
        }
        if ( x < -0.8) {
          doHaptics(64);
          Serial.println("let's mouse");
          delay(1000);
          engageMouse = true;
          while ( engageMouse == true) {
            doMouseMove();
          }
        }
        //mouse switch -not in use for final but leaving if physical button needed later
        //        if (digitalRead(mouseBtn) == LOW) {
        //          mouseClick();
        //        }

      } // end if while loop
      break;

    //  OPEN SCROLL THRU DOCUMENTS
    case 4:
      Serial.println("cmd tab- case 4");
      digitalWrite(red, HIGH);
      digitalWrite(blue, HIGH);
      digitalWrite(green, LOW);
      if (x <= -0.87) {
        showDocs = true; // boolean used to keep docs view active
        while (showDocs == true) {
          Keyboard.press(KEY_F1);
          delay(200);
          Keyboard.release(96);

          engageMouse = true;
          doHaptics(64);
          while ( engageMouse == true) {
            Serial.print("engageMouse =");
            Serial.println(engageMouse);
            doMouseMove();
          }
          Serial.print("ext while engageMouse = ");
          Serial.println(engageMouse);
        }
      }
      break;

    //  SHOW ALL APPS
    case 5:
      Serial.println(x);
      digitalWrite(red, HIGH);
      digitalWrite(blue, HIGH);
      digitalWrite(green, LOW);
      if (x >= 0.87) {
        showApps = true; // boolean used to keep apps view active
        doHaptics(64);
        while (showApps == true) {
          Serial.println("cmd tab- case 5");
          Keyboard.press(KEY_LEFT_CTRL);
          delay(200);
          Keyboard.press(KEY_UP_ARROW);
          delay(200);
          Keyboard.releaseAll();
          engageMouse = true;
          while ( engageMouse == true) {
            Serial.print("engageMouse =");
            Serial.println(engageMouse);
            doMouseMove();
          }
          Serial.print("ext while engageMouse = ");
          Serial.println(engageMouse);
        }
      }
      break;

  }

}

void moveMouse(int thisX, int thisY) {
  Mouse.move(thisX, thisY);
}



void doMouseMove() {
  //  Serial.println("in doMouseMove");
  //doHaptics(119);
  IMU.readAcceleration(x, y, z);
  if (y <= 1 && y >= 0.25) {
    //    Serial.println("mouse go left");
    xR = 0;  // set right value back to 0 so mouse doesn't get to fast to control
    Mouse.move(xL, 0);
    xL = -2;
  } else if (y >= -1 && y < -0.25) {
    //    Serial.println("mouse go right");
    xL = 0;  // set left value back to 0 so mouse doesn't get to fast to control
    Mouse.move(xR, 0);
    xR = 2;
  } else {
    //noop
  }
  
  if (x <= 1 && x >= 0.25) {
    //    Serial.println("mouse go down");
    yU = 0;  // set up value back to 0 so mouse doesn't get to fast to control
    Mouse.move(0, yD);
    yD = 2;
  } else if (x >= -1 && x < -0.25) {
    //    Serial.println("mouse go up");
    yD = 0;  // set down value back to 0 so mouse doesn't get to fast to control
    Mouse.move(0, yU);
    yU = -2;
  } else {
    //noop
  }

 // sum up all values to find threshold for whip click 
  aSum = fabs(x) + fabs(y) + fabs(z);
  if (aSum > 4.5) {
    mouseClick();
  }
  //mouse switch -not in use for final but leaving if physical button needed later
  //  if (digitalRead(mouseBtn) == LOW) {
  //    mouseClick();
  //  }

}


void mouseClick() {
  Mouse.click();
  Keyboard.releaseAll();
  engageMouse = false;
  tabApps = false;
  showDocs = false;
  showApps = false;
  launchPad = false;
  Serial.println("mouse whip click here - in mouseMove ");
  doHaptics(119);
  digitalWrite(green, LOW);
  digitalWrite(blue, LOW);

}


void doHaptics(int thisEffect) {
  drv.setWaveform(0, thisEffect);  // play effect //liked 17, 47, 64, 119
  drv.setWaveform(1, 0);       // end waveform
  // do haptics
  drv.go();
  // slight delay
  delay(500);

}
