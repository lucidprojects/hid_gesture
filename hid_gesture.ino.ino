#include <Arduino_LSM6DS3.h>
#include <Mouse.h>
#include <Keyboard.h>
//#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;

char ctrlKey = KEY_LEFT_GUI;
int  doIt = 0;
float x, y, z, delta, x11, y13 , xL, xR, yU, yD;

boolean showDocs = false;
boolean engageMouse = false;
boolean showApps = false;
boolean launchPad = false;
float aSum;

int mouseBtn = 2;  //mouse switch

int mouseBtnPushCounter = 0; // counter for the number of btn presses
int mouseBtnbuttonState = 0; // current state of the button
int mouseBtnLastButtonState = 0; //previous state of the button

int led1 = 9;

int lpleft = 1;
int lpright = 1;

void setup() {
  Serial.begin(9600);

  pinMode(mouseBtn, INPUT);
  pinMode(led1, OUTPUT);

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
  delta = 0.05;

  // MOUSE CLICK BUTTON STATE CODE
  mouseBtnbuttonState = digitalRead(mouseBtn);

  if (mouseBtnbuttonState != mouseBtnLastButtonState) {
    if (mouseBtnbuttonState == LOW) {
      mouseBtnPushCounter++;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(mouseBtnPushCounter);
    } else {
      Serial.println("off");
    }
    delay(50);
  }

  mouseBtnLastButtonState = mouseBtnbuttonState;

  if (mouseBtnPushCounter % 2 == 0) {
    digitalWrite(led1, HIGH);

  } else {
    digitalWrite(led1, LOW);

  }
  //END MOUSE CLICK CODE



  if (IMU.accelerationAvailable())  {

    IMU.readAcceleration(x, y, z);

    if (y <= delta && y >= -delta) {
      // Serial.println("flat");

    } else if (y >= 0.5 && y <= 0.75) {
      doKeyBoardMouse(8);   // TAB THROUGH OPEN APPS

    }
    else if (y > delta && y < 1 - delta) {
      Serial.println("tilted to the left");
      Serial.print("left y = ");
      Serial.println(y);
    }

    else if (y >= 1 - delta) {
      Serial.println("left");
      //  doKeyBoardMouse(12);
    }

    else if (y < -delta && y > delta - 1) {
      Serial.println("tilted to the right");
      Serial.print("right y = ");
      Serial.println(y);
      //  doKeyBoardMouse(9);
    }

    else {
      Serial.println("right");
      doKeyBoardMouse(9); // LAUNCHPAD

    }
    if (x <= delta && x >= -delta) {
      //   Serial.println("flat");
      //    } else if (x > delta && x < 1 - delta) {
    } else if (x > 0.25 && x < 1 - delta) {
      Serial.println("tilted to the forward");
      doKeyBoardMouse(11); // ALL OPEN APPS ALL WINDOWS
    }
    else if (x >= 1 - delta) {
      Serial.println("forward");
      //  doKeyBoardMouse(13);
    }
    else if (x < -delta && x > delta - 1) {
      Serial.println("tilted to back");

    }
    else if (x < -0.80) {
      Serial.println("back");
      doKeyBoardMouse(10); // ALL OPEN DOCS IN APP
    }else{
      
    }
    
    // end if IMU avail
  }




};


void doKeyBoardMouse(int thisVal) {

  switch (thisVal) {
    case 0:
      //noop;

      Serial.println("sent val 0");
      Serial.println("case 0");

      break;

    // TAB THROUGH APPS
    case 8:
      Serial.println("sent val 8");
      Serial.println("cmd tab- case 8");
      doHaptics(64);
      digitalWrite(led1, HIGH);
      while (y >= 0.5) {
        IMU.readAcceleration(x, y, z);
        Serial.print("my y val in app tab case = ");
        Serial.println(y);
        Keyboard.press(ctrlKey);
        delay(200);
        Keyboard.press(KEY_TAB);
        delay(300);
        Keyboard.release(KEY_TAB);
        delay(100);

      }

      if (y < 0.45 ) {

        Keyboard.releaseAll();
        doHaptics(119);
        digitalWrite(led1, LOW);
        // delay(200);

      }

      break;


    //  OPEN LAUNCHPAD
    case 9:
      Serial.println("cmd tab- case 9");
      digitalWrite(led1, HIGH);
      // doHaptics(64);
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

        //    Serial.print(x);
        //    Serial.print('\t');
        //    Serial.print(y);
        //    Serial.print('\t');
        //    Serial.println(z);

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
            //doHaptics(64);
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
            // doHaptics(64);
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
        if (digitalRead(mouseBtn) == HIGH) {
          mouseClick();

        }

      } // end if while loop

      break;

    //  OPEN SCROLL THRU DOCUMENTS
    case 10:
      Serial.println("cmd tab- case 10");
      digitalWrite(led1, HIGH);
      //      doHaptics(64);
      if (x <= -0.87) {
        showDocs = true; // boolean used to keep docs view active
        doHaptics(64);
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
    case 11:
      Serial.println(x);
      digitalWrite(led1, HIGH);
      //      doHaptics(64);
      if (x >= 0.87) {
        showApps = true; // boolean used to keep apps view active
        doHaptics(64);
        while (showApps == true) {
          Serial.println("cmd tab- case 11");
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

  } else if (y >= -0.55 && y < -0.25) {
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
    //
  } else if (x >= -1 && x < -0.25) {
    //    Serial.println("mouse go up");
    yD = 0;  // set down value back to 0 so mouse doesn't get to fast to control
    Mouse.move(0, yU);
    yU = -2;

  } else {
    //noop
  }

  aSum = fabs(x) + fabs(y) + fabs(z);

  if (aSum > 4.5) {
    mouseClick();
  }

  if (digitalRead(mouseBtn) == HIGH) {
    mouseClick();
  }

}



void mouseClick() {
  Mouse.click();
  Keyboard.releaseAll();
  engageMouse = false;
  showDocs = false;
  showApps = false;
  launchPad = false;
  Serial.println("mouse whip click here - in mouseMove ");
  doHaptics(119);
  digitalWrite(led1, LOW);

}


void doHaptics(int thisEffect) {
  drv.setWaveform(0, thisEffect);  // play effect //liked 17, 47, 64, 119
  drv.setWaveform(1, 0);       // end waveform

  // play the effect
  drv.go();

  // wait a bit
  delay(500);

}
