// Copyright 2021 - 2023, Ricardo Quesada
// SPDX-License-Identifier: Apache 2.0 or LGPL-2.1-or-later

/*
 * This example shows how to use the Controller API.
 *
 * Supported on boards with NINA W10x. In particular, these boards:
 *  - Arduino MKR WiFi 1010,
 *  - UNO WiFi Rev.2,
 *  - Nano RP2040 Connect,
 *  - Nano 33 IoT,
 *  - Arduino MKR Vidor 4000
 */
#include <Bluepad32.h>
#include "pami_lib.h"

ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

// Arduino setup function. Runs in CPU 1
void setup() {
  // Initialize serial
  Serial.begin(115200);
  beginPami();
  while (!Serial) {
    // Wait for serial port to connect.
    // You don't have to do this in your game. This is only for debugging
    // purposes, so that you can see the output in the serial console.
    ;
  }

  String fv = BP32.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  // To get the BD Address (MAC address) call:
  const uint8_t* addr = BP32.localBdAddress();
  Serial.print("BD Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(addr[i], HEX);
    if (i < 5)
      Serial.print(":");
    else
      Serial.println();
  }

  // BP32.pinMode(27, OUTPUT);
  // BP32.digitalWrite(27, 0);

  // This call is mandatory. It sets up Bluepad32 and creates the callbacks.
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.print("CALLBACK: Controller is connected, index=");
      Serial.println(i);
      myControllers[i] = ctl;
      foundEmptySlot = true;

      // Optional, once the gamepad is connected, request further info about the
      // gamepad.
      ControllerProperties properties = ctl->getProperties();
      char buf[80];
      sprintf(buf,
              "BTAddr: %02x:%02x:%02x:%02x:%02x:%02x, VID/PID: %04x:%04x, "
              "flags: 0x%02x",
              properties.btaddr[0], properties.btaddr[1], properties.btaddr[2],
              properties.btaddr[3], properties.btaddr[4], properties.btaddr[5],
              properties.vendor_id, properties.product_id, properties.flags);
      Serial.println(buf);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
        "CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.print("CALLBACK: Controller is disconnected from index=");
      Serial.println(i);
      myControllers[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
        "CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr gamepad) {
  // There are different ways to query whether a button is pressed.
  // By query each button individually:
  //  a(), b(), x(), y(), l1(), etc...
  float speed = (float)gamepad->throttle();
  if(speed > 40) speed /= 1023.;
  else{
    speed = (float)gamepad->brake() ;
    if(speed > 40) speed /= -1023.;
    else speed = 0;
  }
  avancer(speed, (float)gamepad->axisX() / 512.);
  if(gamepad->dpad() & DPAD_LEFT) trim_left();
  if(gamepad->dpad() & DPAD_RIGHT) trim_right();
  

  // Another way to query the buttons, is by calling buttons(), or
  // miscButtons() which return a bitmask.
  // Some gamepads also have DPAD, axis and more.
  // char buf[256];
  // snprintf(buf, sizeof(buf) - 1,
  //          "idx=%d, dpad: 0x%02x, buttons: 0x%04x, "
  //          "axis L: %4li, %4li, axis R: %4li, %4li, "
  //          "brake: %4ld, throttle: %4li, misc: 0x%02x, "
  //          "gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d, "
  //          "battery: %d",
  //          gamepad->index(),        // Gamepad Index
  //          gamepad->dpad(),         // DPad
  //          gamepad->buttons(),      // bitmask of pressed buttons
  //          gamepad->axisX(),        // (-511 - 512) left X Axis
  //          gamepad->axisY(),        // (-511 - 512) left Y axis
  //          gamepad->axisRX(),       // (-511 - 512) right X axis
  //          gamepad->axisRY(),       // (-511 - 512) right Y axis
  //          gamepad->brake(),        // (0 - 1023): brake button
  //          gamepad->throttle(),     // (0 - 1023): throttle (AKA gas) button
  //          gamepad->miscButtons(),  // bitmask of pressed "misc" buttons
  //          gamepad->gyroX(),        // Gyro X
  //          gamepad->gyroY(),        // Gyro Y
  //          gamepad->gyroZ(),        // Gyro Z
  //          gamepad->accelX(),       // Accelerometer X
  //          gamepad->accelY(),       // Accelerometer Y
  //          gamepad->accelZ(),       // Accelerometer Z
  //          gamepad->battery()       // 0=Unknown, 1=empty, 255=full
  // );
  // Serial.println(buf);

  // You can query the axis and other properties as well. See
  // Controller.h For all the available functions.
}

void processMouse(ControllerPtr mouse) {
  char buf[160];
  sprintf(buf,
          "idx=%d, deltaX:%4li, deltaY:%4li, buttons: 0x%04x, misc: 0x%02x, "
          "scrollWheel: %d, battery=%d",
          mouse->index(),        // Controller Index
          mouse->deltaX(),       // Mouse delta X
          mouse->deltaY(),       // Mouse delta Y
          mouse->buttons(),      // bitmask of pressed buttons
          mouse->miscButtons(),  // bitmask of pressed "misc" buttons
          mouse->scrollWheel(),  // Direction: 1=up, -1=down, 0=no movement
          mouse->battery()       // 0=Unk, 1=Empty, 255=full
  );
  Serial.println(buf);
}

void processBalanceBoard(ControllerPtr balance) {
  char buf[160];
  sprintf(buf,
          "idx=%d, tl:%4i, tr:%4i, bl: %4i, br: %4i, temperature=%d, "
          "battery=%d",
          balance->index(),  // Controller Index
          balance->topLeft(), balance->topRight(), balance->bottomLeft(),
          balance->bottomRight(), balance->temperature(),
          balance->battery()  // 0=Unk, 1=Empty, 255=full
  );
  Serial.println(buf);
}

// Arduino loop function. Runs in CPU 1
void loop() {
  // This call fetches all the controller info from the NINA (ESP32) module.
  // Call this function in your main loop.
  // The controllers' pointer (the ones received in the callbacks) gets updated
  // automatically.
  BP32.update();

  // It is safe to always do this before using the controller API.
  // This guarantees that the controller is valid and connected.
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      if (myController->isGamepad()){
        processGamepad(myController);
        digitalWrite(LED_VERTE_PIN, HIGH);
      }
      else if (myController->isMouse())
        processMouse(myController);
      else if (myController->isBalanceBoard())
        processBalanceBoard(myController);
    }
  }
  delay(150);
}
