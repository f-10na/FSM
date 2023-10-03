// Define the possible states of the finite state machine
enum State {
  INIT,
  ADD_DEVICE,
  INDICATE_STATE,
  OUTPUT_POWER,
  REMOVE_DEVICE,
};

// Define the device struct
struct Device {
  String id;
  String location;
  char deviceType;
  String deviceState;
  String extraInfo;
};

// Initialize the state machine
State currentState = INIT;

// Define the maximum number of devices
const int MAX_DEVICES = 10;

// Initialize the devices array
Device smartDevices[MAX_DEVICES];

// Initialize the number of devices to 0
int numDevices = 0;

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);
  lcd.begin(16,2)

//initialise button inputs
  pinMode(upButton, INPUT);	
  pinMode(downButton, INPUT);	
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);


  /*while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  */
  
  while 
}

void loop() {

  if (micros()<1000){
      lcd.print("Q");
      lcd.setBacklight

  }
  // Handle the current state
  switch (currentState) {
    case INIT:
      Serial.println("Welcome to the device storage program.");
      currentState = LIST_DEVICES;
      break;
      
    case ADD_DEVICE:
      // Check if there is room for another device
      if (numDevices < MAX_DEVICES) {
        // Prompt the user for the device details
        Serial.println("Enter device id:");
        while (Serial.available() == 0) {}
        String id = Serial.readStringUntil('\n');
        while (Serial.available() == 0) {}
        String location = Serial.readStringUntil('\n');
        while (Serial.available() == 0) {}
        String deviceType = Serial.readStringUntil('\n');
        while (Serial.available() == 0) {}
        String deviceState = Serial.readStringUntil('\n');
        while (Serial.available() == 0) {}
        String extraInfo = Serial.readStringUntil('\n');
        
        // Add the device to the devices array
        smartDevices[numDevices] = {id, location, deviceType, deviceState, extraInfo};
        numDevices++;
        
        Serial.println("Device added.");
        currentState = LIST_DEVICES;
      } else {
        Serial.println("No more room for devices.");
        currentState = LIST_DEVICES;
      }
      break;
      
    case REMOVE_DEVICE:
      // Check if there are any devices to remove
      if (numDevices > 0) {
        // Prompt the user for the device to remove
        Serial.println("Enter the position of the device to remove:");
        while (Serial.available() == 0) {}
        int index = Serial.parseInt();
        
        // Shift the devices after the removed device down in the array
        for (int i = index; i < numDevices - 1; i++) {
          devices[i] = devices[i + 1];
        }
        numDevices--;
        
        Serial.println("Device removed.");
        currentState = LIST_DEVICES;
      } else {
        Serial.println("Don't need to remove any devices");
        currentState = LIST_DEVICES;
      }
      break;
      
    case LIST_DEVICES:
      // Print the devices
      Serial.println("Devices:");
      for (int i = 0; i < numDevices; i++) {
          lcd.print("^");
          lcd.setCursor(1,0);
          lcd.print(devices[i].id);
          lcd.setCursor(5,0);
          lcd.print(devices[i].location);
          lcd.setCursor(0,1);
          lcd.print(devices[i].deviceType);
          lcd.setCursor(3,1);
          lcd.print(devices[i].deviceState);
          lcd.setCursor(7,1);
          lcd.print(devices[i].extraInfo);
        }

      }
      
      // Prompt the user for the next state
      Serial.println("Enter 'A' to add a device, 'S' to indicate the state, 'P' to outout the power where appropriate or 'R' to remove a device");
      while (Serial.available() == 0) {}
      char input = Serial.read();
      switch (input) {
        case 'A':
          currentState = ADD_DEVICE;
          break;
          
        case 'S':
          currentState = INDICATE_STATE;
          break;
          
        case 'P':
          currentState = OUTPUT_POWER;
          break;

        case 'R':
          currentState = REMOVE_DEVICE;
          break;
          
        default:
          Serial.println("Invalid input, no function corresponds to input");
          currentState = LIST_DEVICES;
          break;
      }
      break;
  }

/* how to read buttons
  int upState = digitalRead(upButton);
*/

}
