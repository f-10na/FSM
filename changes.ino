#include <Adafruit_RGBLCDShield.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//#define DEBUG_MAIN_FUNCTIONS
//#define DEBUG_BUTTONS


#ifdef DEBUG_MAIN_FUNCTIONS
#define debug_main_println(x) Serial.println(x);
#else
#define debug_main_println(x) do {} while(0);
#endif

#ifdef DEBUG_BUTTONS
#define debug_buttons_println(x) Serial.println(x);
#else
#define debug_buttons_println(x) do {} while(0);
#endif

//leave all the global variables here

uint8_t buttons; //used to read buttons
const int MAX_DEVICES = 12;
int countAdd=0;
int count = 0;
int counter = 0;
const int bufferSize PROGMEM= 16;
char outputBuffer[bufferSize];
int currentDevice = 0;
bool selectPressed = false;
unsigned long currentTime=0;
unsigned long previousTime=0;
unsigned long lastUpdated = 0;
unsigned const long updateInterval PROGMEM= 2000; // update every 2 second

//code below allows you to access the FREERAM in the program
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}


//the function below returns a boolean based on whether there is a serial input that is 'X' or not

bool checkString()
  {
   if (Serial.available()>0)
    {
      String start = Serial.readString();
      start.trim(); 
     if (strcmp(start.c_str(), "X") == 0)//looks at first byte of the user's input to check if it is an 'X' and returns true if it does
      {
       return true;
      }
      else
       {
        return false;
       }
     }
     return false;
   }

// Defines the possible states of the finite state machine and uses a switch case to transition between the different states

enum State{
  SYNC,
  MAIN,
  SHOW_ID
}currentState;

//EXTENSIONS

//UDCHARS
//the ones correspond to the shape or character you are trying to make

const byte upChar[8]= {
  B00000,
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00000
};

const byte downChar[8] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
  B00000
};

const byte degreesChar[8] = {
  B00110,
  B01001,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000
};

// Define the device's structure
class Device
{
  private:
    String id;
    String location;
    String deviceType;
    String deviceState;
    String devicePower;

    
public:
    // Getter for id
    String getId() {
        return id;
    }

    // Setter for id
    void setId(String newId) {
        id = newId;
    }

    // Getter for location
    String getLocation() {
        return location;
    }

    // Setter for location
    void setLocation(String newLoc) {
        location = newLoc;
    }

    // Getter for deviceType
    String getDeviceType() {
        return deviceType;
    }

    // Setter for deviceType
    void setDeviceType(String newType) {
        deviceType = newType;
    }

    // Getter for deviceState
    String getDeviceState() {
        return deviceState;
    }

    // Setter for deviceState
    void setDeviceState(String newState) {
        deviceState = newState;
    }

    // Getter for devicePower
    String getDevicePower() {
        return devicePower;
    }

    // Setter for devicePower
    void setDevicePower(String newPower) 
    {
        devicePower = newPower;
    }

    // Function to sort the Devices array by ID in alphabetic order using bubble sort algorithm with strcmp()
    //the function iterates through the array and compares it with every other element of the arrayand swaps the current element ID  with the element it is being compared to if its ID is greater
    //c_str() coverts the std::string to a C-style string so it can use strcmp()
    
    void sortDevices(Device Devices[], int deviceCount) {
      for (int i = 0; i < deviceCount - 1; i++) {
        for (int j = i + 1; j < deviceCount; j++) {
          if (strcmp(Devices[i].getId().c_str(), Devices[j].getId().c_str()) > 0) {
            Device temp = Devices[i];
            Devices[i] = Devices[j];
            Devices[j] = temp;
          }
        }
      }
    }

   //function takes array of device objects and prints the current device's information on the lcd screen
   //it first gets the unique characters from the program memory and then loops through each device in the devices array and uses sprintf to format it
   //it checks if the device is on or off to decide the backlight 
   //if there are more devices below the current device it prints a down arrow if there are devices above it, it prints an up arrow. 

    void printDevices(Device Devices[], int deviceCount, char* outputBuffer,int currentDevice)
    { 
      //Serial.print(F("DISPLAY DEVICE FUNCTION ENTERED"));
      char buffersize = pgm_read_word(&bufferSize);
      lcd.createChar(0,upChar);
      lcd.createChar(1,downChar);
      lcd.createChar(2, degreesChar);
      int stringLen = 0;
      int i=0;
      for(;i<deviceCount;i++){
         // Ignore objects with empty ID values
        if (Devices[i].getId() == "\0") {
            continue;
        }
        stringLen += sprintf(outputBuffer + stringLen,"%s,%s,%s,%s,%s,%d\n",
                             Devices[i].getId().c_str(),
                             Devices[i].getLocation().c_str(),
                             Devices[i].getDeviceType().c_str(),
                             Devices[i].getDeviceState().c_str(),
                             Devices[i].getDevicePower().c_str());
        stringLen += sprintf(outputBuffer + stringLen, "\n");
        stringLen =0;      
      
        //Serial.println(Devices[currentDevice].getDeviceState());
        if(Devices[currentDevice].getDeviceState()=="ON"){
           lcd.setBacklight(2);
           lcd.setCursor(3, 1);
           lcd.print(" " + Devices[currentDevice].getDeviceState());
              }
        if(Devices[currentDevice].getDeviceState()=="OFF"){
           lcd.setBacklight(3);
           lcd.setCursor(3, 1);
           lcd.print(Devices[currentDevice].getDeviceState());
              }
        if((currentDevice<deviceCount-1)){
          lcd.setCursor(0, 1);
          lcd.write(byte(1));
        }
        if((currentDevice>0)){
          lcd.setCursor(0,0);
          lcd.write(byte(0));
        }
        lcd.setCursor(1, 0);
        lcd.print(Devices[currentDevice].getId());
        if (Devices[currentDevice].getLocation().length()>11){
          lcd.setCursor(5, 0);
          lcd.print(Devices[currentDevice].getLocation().substring(0,11));
        }else{
          lcd.setCursor(5, 0);
          lcd.print(Devices[currentDevice].getLocation());
        }
        lcd.setCursor(1, 1);
        lcd.print(Devices[currentDevice].getDeviceType());
        if (Devices[currentDevice].getDeviceType()=="S" or Devices[currentDevice].getDeviceType()=="L"){
           lcd.setCursor(10,1);
           lcd.print('%');
           lcd.setCursor(7, 1);
           lcd.print(Devices[currentDevice].getDevicePower());
        }else if (Devices[currentDevice].getDeviceType()=="T"){
           lcd.setCursor(10,1);
           lcd.write(byte(2));
           lcd.setCursor(11,1);
           lcd.print("C");
           lcd.setCursor(8, 1);
           lcd.print(Devices[currentDevice].getDevicePower());
        }else{
           lcd.print("\0");
            }

    }
    }

      void removeDevice(Device devices[],int currentDevice) {
      devices[currentDevice].setId("\0");
      devices[currentDevice].setLocation("\0");
      devices[currentDevice].setDeviceType("\0");
      devices[currentDevice].setDevicePower("\0");
      devices[currentDevice].setDeviceState("\0");
      }

    bool hasDevicesBelow(Device devices[], int deviceCount, int currentIndex) {
      // Check if the current index is the last index in the array
      if (currentIndex == deviceCount - 1) {
        return false;
      }
      
      // Check if there are any devices below the current device
      for (int i = currentIndex + 1; i < deviceCount; i++) {
        if (devices[i].getId() != "") {
          return true;
        }
      }
      
      return false;
    }

      bool hasDevicesAbove(Device devices[], int deviceCount, int currentIndex) {
        // Check if the current index is the first index in the array
        if (currentIndex == 0) {
          return false;
        }
        
        // Check if there are any devices above the current device
        for (int i = currentIndex - 1; i >= 0; i--) {
          if (devices[i].getId() != "") {
            return true;
          }
        }
        
        return false;
      }

};

Device device;
Device Devices[MAX_DEVICES];

//function is called in the synchronisation state and prints the string inside

void sync()
  {
  lcd.setBacklight(7);
  Serial.println(F("UDCHARS,FREERAM"));
  delay(100);
  lcd.clear();
  }

void removeArrayElement(int arraySize,int pos){
  //shifts all the elements after the index you want to delete to the left essentially deleting it
  for(int i = pos; i< arraySize -1;i++){
      Devices[i]=Devices[i+1]; 
      Serial.println(Devices[i+1].getId());   
  }
}
 
//VALIDITY CHECK FUNCTIONS

//used in all functions to check if the device id is already i the devices array and returns the position in the array if it is 
int checkID(String s) {
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (Devices[i].getId() == s) {
      return i;
    }
  }
  return -1;
}

//function below makes sure that a string only contains alphabetic characters by iterating through each character and works with dashreplace variables to make sure numbers are only where they are supposed to be

bool checkNum(String str) {
    for (int i = 0; i < str.length(); i++) {
        if (!isalpha(str[i])) {
            return false; // contains a non-letter character (a number or a symbol)
        }
    }
    return true; // contains only letters
}

bool checkNumAndSpace(String str) {
    for (int i = 0; i < str.length(); i++) {
        if (!isalpha(str[i])) {
            if (isdigit(str[i])) {
                return true; // contains a number
            } else if (isspace(str[i])) {
                return false; // contains a whitespace
            }
        }
    }
    return true; // contains only letters
}


// checks if the input string contains digits of (0-9)

bool checkPowerOutput(String str) {
    int i = 0;
    while (i<str.length()) {
      for(;i<str.length();){
        if (isDigit(str[i])==0) {
            return false;
        }
        i++;
      }
    }
    return true;
}

//function below is used in the write power output functionaity to ensure only the right devices are having their power written

bool checkDeviceType(String s){
  for(int i=0;i<=MAX_DEVICES;i++){
    //Serial.println(Devices[i].getId());
    if (Devices[i].getId()==s.substring(2,5)){
      String type = Devices[i].getDeviceType();
      if (type == "S"){
        return true;
        }
      else if (type == "L"){
        return true;
        }
      else if (type == "T"){
        return true;
        }     
        else{
      return false;
    }
  }
}
}

//function below makes sure you can only add a device of a specific type 

bool checkAddDT(String s){
  String type = s.substring(6,7);
      if (type == "S"){
        return true;
        }
      else if (type == "L"){
        return true;
        }
      else if (type == "T"){
        return true;
        }
      else if (type == "O"){
        return true;
        }
       else if (type == "C"){
        return true;    
      }else{
      return false;
        }
}

//this function counts how many hyphens are in a given string and is used to verify the format 

int countChar(String s){
  int charIncrement = 0;
  s.trim();
  for(int i = 0; i < s.length(); i++){
    if(s.charAt(i) == '-'){
      charIncrement++;
    }
     
   }
   return charIncrement;
}

//function below checks if the string has the hyphens in the write place for the add 

bool checkCharAdd(String s){
  if(s.substring(1,2)=="-"){
    if(s.substring(5,6)=="-"){
      if(s.substring(7,8)=="-"){
        return true;
      }
    }
  }
  return false;
}

//function below checks if the string the hyphens in the write places for indicate state and write power

bool checkChar(String s){
  if(s.substring(1,2)=="-"){
    if(s.substring(5,6)=="-"){
        return true;
      }
    }
  return false;
}

//function below checks if the string has the hyphen in the write place for remove

bool checkCharRemove(String s){
  if(s.substring(1,2)=="-"){
        return true;
      }
  return false;
}

//function below validates the volume input for the write power functionality

bool checkVolume(int i){
  if((i>=0) && (i<=100)){
    return true;
    }
    return false;
}

//function below validates the brightness input for the write power functionality

bool checkBrightness(int i){
  if((i>=0)&& (i<=100)){
    return true;
    }
    return false;
}

//function below validates the temperature input for the write power functionality

bool checkTemperature(int i){
  if((i>=9 )&& (i<=32)){
    return true;
    }
    return false;
}

//function below only allows userInput to change state to ON or OFF

bool isValidIndicateState(String s){
      if (s == "ON"){
        return true;
        }
      else if (s == "OFF"){
        return true;
        }
      else{
        return false;
    }    
}

//function below checks to see if the user is trying to change the state to soemthing it already is

bool checkRepeat(String s,int i){
  if (Devices[i].getDeviceState()!=s){
    return true;
  }return false;
}

bool checkSame(String s,int i){
  if ((s.substring(6,7)==Devices[i].getDeviceType()) && (s.substring(8,s.length())==Devices[i].getLocation())){
    return false;
    //Serial.println(Devices[i].getDeviceType());
    //Serial.println(Devices[i].getLocation());
  }else if(s.substring(6,7)==Devices[i].getDeviceType() && s.substring(8,s.length())!=Devices[i].getLocation()&&s.substring(8,s.length())!="\0"){
    return true;
    //Serial.println(Devices[i].getDeviceType());
    //Serial.println(Devices[i].getLocation());
  }else if(s.substring(6,7)!=Devices[i].getDeviceType() && s.substring(8,s.length())==Devices[i].getLocation()){
    return true;
    //Serial.println(Devices[i].getDeviceType());
    //Serial.println(Devices[i].getLocation());
  }else{
  return true;
  }
}

// function below checks for button presses

bool buttonPressed(uint8_t pressedButtons){
  pressedButtons = lcd.readButtons();
  if(pressedButtons){
    return true;
  }
 return false;
}

/*
 the function below reads the state of the buttons from the LCD screen and based on which button is pressed 
 does different actions
 */
 

void checkButtons(uint8_t pressedButtons,int i){
  pressedButtons= lcd.readButtons();
  if(pressedButtons != 0){
    debug_buttons_println("button pressed");
    //Serial.println(currentDevice);
    if(pressedButtons & BUTTON_DOWN){ //if down button pressed the current device is incremented and displayed on lcd
      debug_buttons_println("down");
      if (device.hasDevicesBelow(Devices,i,currentDevice)){
        debug_buttons_println("down if");
        lcd.clear();
          if (currentDevice < i - 1 && currentDevice != i) {
            currentDevice++;
          // display next device in the array
            device.printDevices(Devices,countAdd,outputBuffer,currentDevice);
        }   
      }
  }else if(pressedButtons & BUTTON_UP ){ //if up button is pressed the current device is decremented and displayed on the lcd
      debug_buttons_println("up if");
      if (device.hasDevicesAbove(Devices,i,currentDevice)){
        lcd.clear();
          if (currentDevice > 0 && currentDevice != 0) {
            currentDevice--;
    // display previous device in the array
            device.printDevices(Devices,countAdd,outputBuffer,currentDevice);
      }
      }
      selectPressed = false;
  }else if(pressedButtons & BUTTON_LEFT){ //left over code from HCI attempt
   
      for(int i=0;i<counter;i++){
        if(Devices[i].getDeviceState()=="OFF"){
          device.printDevices(Devices,countAdd,outputBuffer,currentDevice);
        }
      } 
      selectPressed = false;    
  }else if(pressedButtons & BUTTON_RIGHT){ // left over code from HCI attempt
      for(int i=0;i<counter;i++){
        if(Devices[i].getDeviceState()=="ON"){
          device.printDevices(Devices,countAdd,outputBuffer,currentDevice);
        }
      }
      selectPressed = false;
  }else if(pressedButtons & BUTTON_SELECT){ // clears lcd and sets currentState to SHOW_ID
    if (!selectPressed){
      lcd.clear();
    }
      debug_buttons_println("select button"); 
      //Serial.println(currentState);
      //Serial.println(selectPressed);     
      currentState = SHOW_ID;
      selectPressed=true;
      //Serial.println(currentState);
    }
  }
}

void setup(){
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16,2); 
  lcd.clear();
  //Serial.println(currentState);
  currentState = SYNC; //sets the 1st state to immediately run the synchronisation phase
  //Serial.println(currentState);

}

//function updates LCD display to sho my student ID and how much free ram is left

void updateLCD() {
  unsigned long interval = pgm_read_dword(&updateInterval);
  if (millis() - lastUpdated >= interval) {
    lcd.setBacklight(5);
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("F223966");
    lastUpdated = millis();
  }
}





void loop(){
  // put your main code here, to run repeatedly:

  switch (currentState) 
  { 
    case SYNC:
    {
        while (!checkString()) //constantly prints 'Q' unless an X is inputted then breaks out of while loop
          {
            lcd.setBacklight(5);
            Serial.print(F("Q")); 
            delay(1000); 
            }
            //Serial.print("before sync");
        sync(); 
        //Serial.print("after sync");
        //Serial.println(currentState);
        currentState = MAIN;
        //Serial.println(currentState);
    }
        break;

      
//this state has three sub states get a user input, get button requests or display the devices in array 
    case MAIN:
    {
      buttons = lcd.readButtons();
      if (Serial.available()>0){
      String userInput; 
      userInput = Serial.readStringUntil('\n'||'r');
      userInput.trim();
      userInput.toUpperCase();
      userInput.replace("\r","");
      //Serial.println(userInput);
      String dashReplace1 = userInput.substring(0,8);
      dashReplace1.replace('-','F');
      String dashReplace2 = userInput.substring(0,6);
      dashReplace2.replace('-','F');
      //checkSame(userInput);
      //(checkNum(dashReplace2));
      //(checkPowerOutput(userInput.substring(6,10)));
      //(checkID(userInput.substring(2,5)));
      if ((userInput[0]=='A')&&(checkID(userInput.substring(2,5)))==-1&&checkNum(dashReplace1)&&checkNumAndSpace(userInput.substring(8,userInput.length()+1))&&(checkCharAdd(userInput))&&(countChar(userInput)==3)&&userInput.length()>8)
        { 
          //the first if checks whether it is the add request,the id is not already in the array,there are only numbers in the location string and that only 3 dashes are in the request
          //Serial.println("A recieved");
           if (checkAddDT(userInput)&&counter<MAX_DEVICES)//checks it is a valid device type and if array is full or not
                {
                 countAdd++;
                 for(int i=0;i<MAX_DEVICES;i++){
                     if(i==countAdd-1){
                        Devices[countAdd-1].setId(userInput.substring(2,5));
                        Devices[countAdd-1].setLocation(userInput.substring(8,userInput.length())); 
                        Devices[countAdd-1].setDeviceType(userInput.substring(6,7)); 
                        Devices[countAdd-1].setDeviceState("OFF");
                        if (){
                          Devices[countAdd-1].setDevicePower();
                        }else if (){
                        }else if(){
                            
                          }else {
                            continue;
                        }
                        debug_main_println("currentdevice being added if statement");
                     }else if (i<countAdd-1){
                        continue;
                        debug_main_println("ignore devices current device being added else if block");
                     }else{
                        Devices[i].setId("\0");
                        Devices[i].setLocation("\0"); 
                        Devices[i].setDeviceType("\0"); 
                        Devices[i].setDeviceState("\0");
                        debug_main_println("devices after device being added set to null values");
                           }                                   
                     }
                        counter++;
                        debug_main_println("device should be displaying on lcd if i am printed"); 
                        Serial.println(F("DONE ADD!"));
                        device.sortDevices(Devices,countAdd);
                        device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
                        //Serial.println(currentDevice);  
                        //Serial.println(countAdd);              
                    }else{
                      Serial.println(F("INVALID DEVICE TYPE OR MAX DEVICES REACHED"));
                      }
        }else if((userInput[0]=='A')&&(checkID(userInput.substring(2,5)))>-1&&(checkSame(userInput,checkID(userInput.substring(2,5))))&&(checkAddDT(userInput))&&checkNum(dashReplace1)&&checkNumAndSpace(userInput.substring(8,userInput.length()+1))&&(checkCharAdd(userInput))&&countChar(userInput)==3&&userInput.length()>8){
                 for(int i=0;i<MAX_DEVICES;i++){
                     if(i==checkID(userInput.substring(2,5))){
                        Devices[i].setLocation(userInput.substring(8,userInput.length())); 
                        Devices[i].setDeviceType(userInput.substring(6,7)); 
                        debug_main_println("currentdevice being added if statement");
                        Serial.println(F("DONE UPDATE!"));
                        lcd.clear();
                        device.sortDevices(Devices,countAdd);
                        device.printDevices(Devices, countAdd,outputBuffer,currentDevice);            
                    }
                 }
        }else if ((userInput[0] == 'S' )&&((checkID(userInput.substring(2,5)))!=-1)&&(checkNum(dashReplace2))&&countChar(userInput)==2) //checks if the function is indicate state and if the id is already in the array and right number of dashes and no numbers 
          {
          if((checkChar(userInput))&& !userInput.substring(6,9).equals(" ")){ //checks if dashes in the right place and the state you are trying to add is not an empty string
            //checks if ON or OFF and accepts nothing else
            //Serial.println(checkChar(userInput));
                      if (isValidIndicateState(userInput.substring(6,9)))
                        {
                        //Serial.println(isValidIndicateState(userInput.substring(6,9)));
                        int pos = checkID(userInput.substring(2,5)); //returns index of the array element we want to change
                        //Serial.println(pos);
                            if (checkRepeat(userInput.substring(6,9),pos)){ 
                              //Serial.println(pos);        
                              Devices[pos].setDeviceState(userInput.substring(6,9));
                              debug_main_println("DONE");
                              lcd.clear();
                              device.sortDevices(Devices,countAdd);
                              device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
                              Serial.println(F("DONE CHANGE STATE"));
                              }
                              else{
                                Serial.println("error:"+userInput);
                              }
                      }else{
                        Serial.println("error:"+userInput);
                        }
                }else{
                  Serial.println("error:"+userInput);
                }
         }else if ((userInput[0]=='P')&&((checkID(userInput.substring(2,5)))!=-1)&&(checkNum(dashReplace2))&&checkPowerOutput(userInput.substring(6,10))&&countChar(userInput)==2) // 
            {
          //checks if the function is to write power,if the id already exists in the array,if there are no numbers in the id,if the right number of dashes and if the power we are trying to write is actually a number
          debug_main_println("writing power if block");
                if(checkChar(userInput)&& !userInput.substring(6,9).equals(" ")){ //checks if the dashes are in the right places and that the power we are trying to write is not empty 
                //Serial.println(checkChar(userInput));
                //Serial.println(checkDeviceType(userInput))
                      if(checkDeviceType(userInput))
                //checks the device type can have its power written
                        {
                          debug_main_println("CHECK DEVICE")
                            if (Devices[(checkID(userInput.substring(2,5)))].getDeviceType()=="S"){
                              debug_main_println("DONE");
                                  if (checkVolume(userInput.substring(6,9).toInt())) //validity checks for speaker
                                    {
                                    debug_main_println("CHECK SPEAKER STUFF")
                                    Devices[(checkID(userInput.substring(2,5)))].setDevicePower(userInput.substring(6,9)); //sets the power
                                    debug_main_println("DONE ");
                                    lcd.clear();
                                    device.sortDevices(Devices,countAdd);
                                    device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
                                    Serial.println(F("DONE POWER"));
                                    }
                            }else if (Devices[(checkID(userInput.substring(2,5)))].getDeviceType()=="L"){
                                  if (checkBrightness(userInput.substring(6,9).toInt())){ //validity checks for light
                                  debug_main_println("CHECK LIGHT STUFF")
                                    Devices[(checkID(userInput.substring(2,5)))].setDevicePower(userInput.substring(6,9));
                                    lcd.clear();
                                    device.sortDevices(Devices,countAdd);
                                    device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
                                    debug_main_println("DONE ");
                                    Serial.println(F("DONE POWER"));                                   
                                  }
                            }else if (Devices[(checkID(userInput.substring(2,5)))].getDeviceType()=="T"){
                                  if (checkTemperature(userInput.substring(6,9).toInt())){ //validity checks for thermostat
                                  debug_main_println("CHECK THERMOSTAT STUFF")
                                    Devices[(checkID(userInput.substring(2,5)))].setDevicePower(userInput.substring(6,9));//need to try and add unique char to do degree sign
                                    debug_main_println("DONE");
                                    lcd.clear();
                                    device.sortDevices(Devices,countAdd);
                                    device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
                                    Serial.println(F("DONE POWER"));
                                      }
                                      else{
                                         Serial.println("error:"+userInput);
                                      }
                                }
                            else{
                              debug_main_println("incorrect device type else block")
                              Serial.println("error:"+userInput);
                            }
                            
                }else{
                  Serial.println("error:"+userInput); 
                }
                }else{
                  Serial.println("error:"+userInput);
                }
        }else if ((userInput[0]=='R')&&((checkID(userInput.substring(2,5)))!=-1)&&(checkNum(dashReplace2))&&countChar(userInput)==1) 
          {
          //checks to see if the function is to remove and checks to see if device is stored in the array,checks to make sure no numbers,checks number of dashes
        debug_main_println(checkID(userInput.substring(2,5)));
            if (checkCharRemove(userInput))
              {
                //checks the dash in the right place
                //Serial.println(checkCharRemove(userInput));
                device.removeDevice(Devices,checkID(userInput.substring(2,5)));
                lcd.clear();
                device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
                debug_main_println("DONE");
                Serial.println();
                Serial.println("DONE REMOVE");
            }else{
              debug_main_println("can't remove because of faulty remove request");
              Serial.println("error:"+userInput);
            } 
        }else{
          Serial.println("error:"+userInput);
        }
      }else if(buttonPressed(buttons)){ //listens for button requests
      checkButtons(buttons,counter);
      }
    else{ //print devices in the array
       device.sortDevices(Devices,countAdd);
       device.printDevices(Devices, countAdd,outputBuffer,currentDevice);
       
      }
    }
 
      break;
      
    case SHOW_ID:
    {
      updateLCD();
      lcd.setCursor(0,1);
      lcd.print("FREE RAM:");
      lcd.print(freeMemory());
      buttons = lcd.readButtons();
      //the code belows ensures that once select button is released the lcd screen is cleared and the main state is reentered
      if (buttons != BUTTON_SELECT){
        lcd.clear();
        lcd.setBacklight(7);
        //Serial.println(currentState);
        currentState = MAIN;
        //Serial.println(currentState);
        }
      }
      break;   
  }
  count++;
}