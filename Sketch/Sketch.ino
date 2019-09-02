#include <ArduinoHttpClient.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <FPS_GT511C3.h>
#include <UIPEthernet.h>
#include <TimeLib.h>
#include <Keypad.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <DS1307RTC.h>
#include <utility/Enc28J60Network.h>

//Hardware setup
/*
  Fingerprint Scanner:
    Reciever    - A8(Green)
	  Transmitter - A9(White)
	LED GREEN : pin 10
	LED RED   : pin 11
	Buzzer    : pin 12
	Button    :	pin 2
  MISO      : pin 50
  MOSI      : pin 51
  SCK       : pin 52
  CS:
    Ethernet    - 3
    SD          - 53
  Keypad    : 22 to 29
*/

// Pin Declaration
const int buttonPin  = 2;
const int buzzPin    = 12;
const int fpstxPin   = 18;
const int fpsrxPin   = 19;
const int gledPin    = 10;
const int rledPin    = 11;
const int etherPin   = 3;
const int sdPin      = 53;

//Variable Declaration
int line = 0;
int port = 8084;
int buttonState = 0;
const byte K_ROWS = 4;
const byte K_COLS = 4;
int waitPeriod = 10000;
unsigned long time_now = 0;
const char password[] = "1234abcd";
char serverAddress[] = "192.168.1.3";
int fyjump = 1, syjump = 1, tyjump = 1;
byte k_rowPins[K_ROWS] = {22, 23, 24, 25};
byte k_colPins[K_COLS] = {26, 27, 28, 29};
uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
char k_keys[K_ROWS][K_COLS] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};

//Function Declarations
char fileId(int id);
int checkPin();
int getClass();
int sync();
int getRoll();
String getTime();
String print2digits(int number);
String print3digits(int number);
void addAdmin();
void attendance();
void authorize();
void buzz(int sec);
void clrscr();
void contReg();
void delClass();
void delClass(int division);
void delDB();
void delMenu();
void delRoll();
void enroll();
void powerSave();
void mainMenu();
void readJumps();
void regMenu();
void renameFile(int id);
void seleReg();
void writeCLog(String text);
void writeJumps();
void writeLog(String text);

//Module Initialization
File myFile;
tmElements_t tm;
//Enc28J60Network eth;
EthernetClient ether;
FPS_GT511C3 fps(A8, A9);
HttpClient client = HttpClient(ether, serverAddress, port);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Keypad keypad = Keypad( makeKeymap(k_keys), k_rowPins, k_colPins, K_ROWS, K_COLS );

//Main Function
void setup()
{
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  pinMode(buzzPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(gledPin, OUTPUT);
  pinMode(rledPin, OUTPUT);
  pinMode(etherPin, OUTPUT);
  pinMode(sdPin, OUTPUT);
  clrscr();
  printline("Project Cerberus");
  printline("The");
  printline("Attndance Initiative");
  buzz(1);
  printline("Loading Ethernet");
  Ethernet.init(etherPin);
  while (!Ethernet.begin(mac))
  {}
  //eth.powerOff();
  line--;
  printline("Loading FPS");
  fps.Open();
  fps.SetLED(false);
  line--;
  printline("Loading SD Card");
  while (!SD.begin(sdPin))
  {}
  line--;
  printline("Reading Data");
  readJumps();
  digitalWrite(gledPin, HIGH);
  line--;
  printline("System Ready");
  delay(1000);
  digitalWrite(gledPin, LOW);
  authorize();
  attendance();
}

void authorize()
{
  fps.SetLED(true);
  int flag = 0;
  clrscr();
  printline("Authorization");
  printline("Required");
  printline("Press Finger");
  do
  {
    if (fps.IsPressFinger())
    {
      clrscr();
      printline("Reading Finger");
      fps.CaptureFinger(false);
      int id = fps.Identify1_N();
      if (id < 200 && id >= 180)
      {
        clrscr();
        printline("Finger Identified");
        printline("");
        printline("Device");
        printline("Authorized");
        digitalWrite(gledPin, HIGH);
        delay(1500);
        flag = 1;
        digitalWrite(gledPin, LOW);
      }
      else
      {
        clrscr();
        printline("Access Denied");
        digitalWrite(rledPin, HIGH);
        digitalWrite(buzzPin, HIGH);
        delay(1000);
        digitalWrite(rledPin, LOW);
        digitalWrite(buzzPin, LOW);
      }
      if (fps.IsPressFinger())
      {
        printline("Remove Finger");
        while (fps.IsPressFinger() == true)
        {}
      }
      if (flag == 0)
      {
        clrscr();
        printline("Authorization");
        printline("Required");
        printline("Press Finger");
      }
    }
  } while (flag == 0);
}

void attendance()
{
  fps.SetLED(true);
  clrscr();
  printline("Fingerprint");
  printline("Attendance");
  printline("Press Finger");
  time_now = millis();
  do
  {
    if (millis() > time_now + waitPeriod)
    {
      fps.SetLED(false);
      buzz(1);
      //int flag = sync();
      clrscr();
      printline("Sync Function");
      printline("is");
      printline("Disabled");
      delay(2000);
      int flag = 1;
      if (flag == 1)
      {

        powerSave();
        time_now = millis();
        clrscr();
        printline("Fingerprint");
        printline("Attendance");
        printline("Press Finger");
      }
      else
      {
        clrscr();
        printline("Server Timeout");
        printline("");
        printline("Automatic Retry");
        printline("Scheduled");
        delay(1500);
        clrscr();
        printline("Fingerprint");
        printline("Attendance");
        printline("Press Finger");
        time_now = millis();
      }
      fps.SetLED(true);
    }
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW)
    {
      if (fps.IsPressFinger())
      {
        clrscr();
        printline("Reading Finger");
        fps.CaptureFinger(false);
        int id = fps.Identify1_N();
        if (id < 180)
        {
          int division;
          clrscr();
          printline("Finger Identified");
          if (id >= 0 && id < 60)
          {
            id = id + fyjump;
            division = 0;
            printline("Class - FY");
          }
          else if (id >= 60 && id < 120)
          {
            id = id - 60 + syjump;
            division = 1;
            printline("Class - SY");
          }
          else if (id >= 120 && id < 180)
          {
            id = id - 120 + tyjump;
            division = 2;
            printline("Class - TY");
          }
          printline("Roll Num : " + print3digits(id));
          String rtc = getTime();
          printline(rtc);
          digitalWrite(gledPin, HIGH);
          String details = String(division) + " " + print3digits(id);
          writeLog(details);
          delay(1500);
        }
        else
        {
          clrscr();
          printline("Finger not found");
          digitalWrite(rledPin, HIGH);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(rledPin, LOW);
          digitalWrite(buzzPin, LOW);
        }
        fps.SetLED(true);
        if (fps.IsPressFinger() == true)
        {
          clrscr();
          printline("Remove Finger");
        }
        while (fps.IsPressFinger() == true)
        {}
        clrscr();
        printline("Fingerprint");
        printline("Attendance");
        printline("Press Finger");
        digitalWrite(gledPin, LOW);
        time_now = millis();
      }
    }
    else
    {
      fps.SetLED(false);
      mainMenu();
      clrscr();
      printline("Fingerprint");
      printline("Attendance");
      printline("Press Finger");
      fps.SetLED(true);
      time_now = millis();
    }
  } while (1);
}

void mainMenu()
{
  clrscr();
  printline("Main Menu");
  buzz(1);
  //int flag = checkPin();
  clrscr();
  printline("CheckPin Function");
  printline("is");
  printline("Disabled");
  delay(2000);
  int flag = 0;
  if (flag == 1)
  {
    clrscr();
    printline("Incorrect Pin");
    buzz(1);
  }
  while (flag == 0)
  {
    clrscr();
    printleft("A- Registration");
    printleft("B- Add Admin");
    printleft("C- Delete Menu");
    printleft("#- Abort");
    char key = ' ';
    key = keypad.waitForKey();
    clrscr();
    switch (key)
    {
      case 'A':
        printline("Registration");
        regMenu();
        break;
      case 'B':
        printline("Add Admin");
        addAdmin();
        break;
      case 'C':
        printline("Delete Menu");
        delMenu();
        break;
      case '#':
        flag = 1;
        break;
      default:
        printline("Invalid Input");
        delay(1500);
    }
  }
}

void regMenu()
{
  buzz(1);
  //Enter pin code
  int flag = 0;
  do
  {
    clrscr();
    printleft("A - ContRegistration");
    printleft("B - SeleRegistration");
    printleft("# - Abort");
    char key = ' ';
    key = keypad.waitForKey();
    clrscr();
    switch (key)
    {
      case 'A':
        flag = 1;
        clrscr();
        printline("Continuous");
        printline("Registration");
        contReg();
        break;
      case 'B':
        flag = 1;
        clrscr();
        printline("Selected Roll No");
        printline("Registration");
        seleReg();
        break;
      case '#':
        flag = 1;
        clrscr();
        break;
      default:
        printline("Invalid Input");
        delay(1500);
    }
  } while (flag == 0);
}

void contReg()
{
  clrscr();
  printline("A - Cont from Roll");
  printline("B - New Database");
  printline("Any Key to Abort");
  int flag = 0;
  int enrollid = 0;
  int roll = 0;
  int division = 0;
  char key = ' ';
  int limit = 0;
  key = keypad.waitForKey();
  clrscr();
  switch (key)
  {
    case 'A':
      flag = 1;
      division = getClass();
      clrscr();
      do
      {
        clrscr();
        printline("Roll No");
        printline("to Continue from");
        delay(2000);
        roll = getRoll();
        enrollid = division * 60;
        if (division == 0)
        {
          if (roll < fyjump || roll >= fyjump + 60)
          {
            clrscr();
            printline("Invalid Roll No");
            printline("Roll !< First Roll)");
            printline("Roll !> Lab Limit)");
            printline("First Roll :" + String(fyjump));
            delay(1500);
            flag = 0;
          }
          else
          {
            enrollid = enrollid + roll - fyjump;
            limit = 59;
          }
        }
        else if (division == 1)
        {
          if (roll < syjump || roll >= syjump + 60)
          {
            clrscr();
            printline("Invalid Roll No");
            printline("Roll !< First Roll)");
            printline("Roll !> Lab Limit)");
            printline("First Roll :" + String(syjump));
            delay(1500);
            flag = 0;
          }
          else
          {
            enrollid = enrollid + roll - syjump;
            limit = 119;
          }
        }
        else if (division == 2)
        {
          if (roll < tyjump || roll >= tyjump + 60)
          {
            clrscr();
            printline("Invalid Roll No");
            printline("Roll !< First Roll)");
            printline("Roll !> Lab Limit)");
            printline("First Roll :" + String(tyjump));
            delay(1500);
            flag = 0;
          }
          else
          {
            enrollid = enrollid + roll - tyjump;
            limit = 179;
          }
        }
      } while (flag != 1);
      break;
    case 'B':
      clrscr();
      printline("!!Warning!!");
      printline("Previous Data");
      printline("will be overwritten");
      buzz(1);
      delay(2000);
      clrscr();
      printline("A - Continue");
      printline("Any Key to Abort");
      key = keypad.waitForKey();
      switch (key)
      {
        case 'A':
          division = getClass();
          delClass(division);
          clrscr();
          printline("Enter");
          printline("First Roll No");
          printline("in");
          printline("Current Lab");
          delay(2000);
          roll = getRoll();
          if (division == 0)
          {
            fyjump = roll;
            limit = 59;
          }
          else if (division == 1)
          {
            syjump = roll;
            limit = 119;
          }
          else if (division == 2)
          {
            tyjump = roll;
            limit = 179;
          }
          writeJumps();
          enrollid = division * 60;
          flag = 1;
          break;
        default :
          flag = 0;
          break;
      }
      break;
    default : flag = 0;
      break;
  }

  if (flag == 1)
  {
    fps.SetLED(true);
    do
    {
      clrscr();
      printline("Roll Number :" + String(roll));
      printline("A- Continue");
      printline("B- Skip    C- Prev");
      printline("Any to Abort");
      char key = ' ';
      key = keypad.waitForKey();
      switch (key)
      {
        case 'A':
          flag = 1;
          break;
        case 'B':
          flag = 2;
          roll++;
          enrollid++;
          break;
        case 'C':
          flag = 2;
          roll--;
          enrollid--;
          break;
        default :
          flag = 0;
          break;
      }
      if (flag == 1)
      {
        if (fps.CheckEnrolled(enrollid))
        {
          clrscr();
          printline("Roll No :" + String(roll));
          printline("Already Enrolled");
          delay(2000);
        }
        else
        {
          clrscr();
          printline("Roll Number :" + String(roll));
          printline("Prss Fingr to Enroll");
          while (fps.IsPressFinger() == false)
          {}
          printline("Reading Finger");
          fps.EnrollStart(enrollid);
          enroll();
          String details = "Enrolled " + String(division) + " " + print3digits(roll);
          writeCLog(details);
          roll++;
          enrollid++;
        }
      }
    } while (flag != 0  && enrollid <= limit );
  }
  if (flag == 0)
  {
    clrscr();
    printline("Exit Cont Registrtn");
    delay(1000);
    clrscr();
  }
}

void seleReg()
{
  clrscr();
  printline("Initialzng Registrtn");
  delay(1000);
  clrscr();
  int flag = 0;
  int roll = 0;
  int division = 0;
  int enrollid = 0;
  String divi = "";
  do
  {
    clrscr();
    printline("Press A to Continue");
    printline("Any Key to Abort");
    char key = ' ';
    key = keypad.waitForKey();
    clrscr();
    switch (key)
    {
      case 'A':
        flag = 1;
        break;
      default :
        flag = 0;
        break;
    }
    if (flag == 1)
    {
      division = getClass();
      clrscr();
      do
      {
        clrscr();
        printline("Enter");
        printline("Roll Number");
        delay(2000);
        roll = getRoll();
        enrollid = division * 60;
        if (division == 0)
        {
          if (roll < fyjump || roll >= fyjump + 60)
          {
            clrscr();
            printline("Invalid Roll No");
            printline("Roll !< First Roll)");
            printline("Roll !> Lab Limit)");
            printline("First Roll :" + String(fyjump));
            delay(1500);
            flag = 0;
            divi = "FY";
          }
          else
          {
            enrollid = enrollid + roll - fyjump;
          }
        }
        else if (division == 1)
        {
          if (roll < syjump || roll >= syjump + 60)
          {
            clrscr();
            printline("Invalid Roll No");
            printline("Roll !< First Roll)");
            printline("Roll !> Lab Limit)");
            printline("First Roll :" + String(syjump));
            delay(1500);
            flag = 0;
            divi = "SY";
          }
          else
          {
            enrollid = enrollid + roll - syjump;
          }
        }
        else if (division == 2)
        {
          if (roll < tyjump || roll >= tyjump + 60)
          {
            clrscr();
            printline("Invalid Roll No");
            printline("Roll !< First Roll)");
            printline("Roll !> Lab Limit)");
            printline("First Roll :" + String(tyjump));
            delay(1500);
            flag = 0;
            divi = "TY";
          }
          else
          {
            enrollid = enrollid + roll - tyjump;
          }
        }
      } while (flag != 1);
      bool usedid = true;
      usedid = fps.CheckEnrolled(enrollid);
      if (usedid == true)
      {
        clrscr();
        printline("Roll No :" + String(roll));
        printline("Already Enrolled");
        flag = 2;
        delay(2000);
      }
      if (flag == 1)
      {
        fps.SetLED(true);
        clrscr();
        printline("Roll Number :" + String(roll));
        printline("Class :" + divi);
        printline("Prss Fingr to Enroll");
        while (fps.IsPressFinger() == false)
        {}
        clrscr();
        printline("Reading Finger");
        fps.EnrollStart(enrollid); //enroll
        enroll();
        String details = "Enrolled " + String(division) + " " + print3digits(roll);
        writeCLog(details);
      }
    }
  } while (flag == 1 || flag == 2);
  if (flag == 0)
  {
    clrscr();
    printline("Exit Sele Registrtn");
    delay(1000);
    clrscr();
  }
}

void enroll()
{
  //bret - Fingerprint Reading Result
  int iret = 0;//Erollment Error Code
  int errCount = 0;
  while (errCount <= 2)
  {
    if (errCount > 0)
    {
      printline("Press Finger");
      clrscr();
      while (fps.IsPressFinger() == false)
      {
      }
    }
    bool bret = fps.CaptureFinger(true);
    if (bret != false)
    {
      errCount = 0;
      fps.Enroll1();
      clrscr();
      printline("Captured Image 1/3");
      printline("Remove Finger");
      while (fps.IsPressFinger() == true)
      {}
      while (errCount <= 2)
      {
        clrscr();
        printline("Press Same Finger");
        while (fps.IsPressFinger() == false)
        {}
        printline("Reading Finger");
        bret = fps.CaptureFinger(true);
        if (bret != false)
        {
          errCount = 0;
          fps.Enroll2();
          clrscr();
          printline("Captured Image 2/3");
          printline("Remove finger");
          while (fps.IsPressFinger() == true)
          {}
          while (errCount <= 2)
          {
            clrscr();
            printline("Press Same Finger");
            while (fps.IsPressFinger() == false)
            {}
            printline("Reading Finger");
            bret = fps.CaptureFinger(true);
            if (bret != false)
            {
              errCount = 0;
              iret = fps.Enroll3();//Returns error code
              clrscr();
              printline("Captured Image 3/3");
              printline("Remove finger");
              while (fps.IsPressFinger() == true)
              {}
              clrscr();
              if (iret == 0)
              {
                printline("Enroll Successfull");
                errCount = 3;
                delay(1000);
              }
              else
              {
                clrscr();
                printline("Error code :" + String(iret));
                switch (iret)
                {
                  case 1 :
                    printline("Error");
                    break;
                  case 2 :
                    printline("Bad finger");
                    break;
                  case 3 :
                    printline("Duplicate Finger");
                    break;
                }
                delay(5000);
                errCount = 4;
              }
            }
            else
            {
              clrscr();
              printline("Failed - 3rd Image");
              printline("Wet/Dry/Dirty Finger");
              printline("Cannot be Imaged");
              printline("Please Try Again");
              errCount++;
              delay(3000);
            }
          }
        }
        else
        {
          clrscr();
          printline("Failed - 2nd Image");
          printline("Wet/Dry/Dirty Finger");
          printline("Cannot be Imaged");
          printline("Please Try Again");
          errCount++;
          delay(3000);
          clrscr();
          printline("Press Finger Agian");
        }
      }
    }
    else
    {
      clrscr();
      printline("Failed - 1st Image");
      printline("Wet/Dry/Dirty Finger");
      printline("Cannot be Imaged");
      printline("Please Try Again");
      errCount++;
      delay(3000);
    }
  }
  if (errCount != 3)
  {
    clrscr();
    printline("Enrollment Failed");
    printline("Failed Capture Image");
    printline("Inform Lab Teacher");
    delay(5000);
  }
}

void addAdmin()
{
  buzz(1);
  int enrollid = 180;
  while (fps.CheckEnrolled(enrollid))
  {
    enrollid++;
  }
  clrscr();
  printline("Admin No : " + String(enrollid - 179));
  printline("Prss Fingr to Enroll");
  fps.SetLED(true);
  while (fps.IsPressFinger() == false)
  {}
  printline("Reading Finger");
  fps.EnrollStart(enrollid);
  enroll();
  String rtc = getTime();
  String details = "Added Admin No :" + String(enrollid - 179);
  writeCLog(details);
}

void delMenu()
{
  buzz(1);
  //Enter pin code
  int flag = 0;
  do
  {
    clrscr();
    printleft("A - Del Roll No");
    printleft("B - Del Class");
    printleft("C - Del Database");
    printleft("# - Abort");
    char key = ' ';
    key = keypad.waitForKey();
    clrscr();
    switch (key)
    {
      case 'A':
        flag = 1;
        clrscr();
        printline("Delete Roll No");
        delRoll();
        break;
      case 'B':
        flag = 1;
        clrscr();
        printline("Delete Class");
        delClass();
        break;
      case 'C':
        flag = 1;
        clrscr();
        printline("Delete Database");
        delDB();
        break;
      case '#':
        flag = 1;
        clrscr();
        break;
      default:
        printline("Invalid Input");
        delay(1500);
    }
  } while (flag == 0);
}

void delRoll()
{
  clrscr();
  printline("!!Warning!!");
  printline("RollNo to be deleted");
  printline("Press A to Continue");
  printline("Any Key to Abort");
  int flag = 0;
  int division = 0;
  int roll = 0;
  char key = ' ';
  key = keypad.waitForKey();
  clrscr();
  switch (key)
  {
    case 'A':
      flag = 1;
      break;
    default :
      flag = 0;
      break;
  }
  if (flag == 1)
  {
    division = getClass();
    roll = getRoll();
    int enrollid = division * 60;
    String divi = "";
    if (division == 0)
    {
      enrollid = enrollid + roll - fyjump - 1;
      divi = "FY";
    }
    else if (division == 1)
    {
      enrollid = enrollid + roll - syjump - 1;
      divi = "SY";
    }
    else if (division == 2)
    {
      enrollid = enrollid + roll - tyjump - 1;
      divi = "TY";
    }
    fps.DeleteID(enrollid);
    clrscr();
    printline("Fingerprint Deleted");
    printline("Roll Number :" + String(roll));
    printline("Class :" + divi);
    buzz(1);
    String details = "Deleted " + String(division) + " " + print3digits(roll);
    writeCLog(details);
    delay(1500);
    clrscr();
  }
}

void delClass()
{
  clrscr();
  printline("!!Warning!!");
  printline("Full Class Deletion");
  printline("Press A to Continue");
  printline("Any Key to Abort");
  int flag = 0;
  int division = 0;
  delay(1000);
  char key = ' ';
  key = keypad.waitForKey();
  clrscr();
  switch (key)
  {
    case 'A':
      flag = 1;
      break;
    default :
      flag = 0;
      break;
  }
  if (flag == 1)
  {
    division = getClass();
    int limit = ((division + 1) * 60) - 1;
    int id = division * 60;
    String divi = "";
    while (id <= limit)
    {
      fps.DeleteID(id);
      clrscr();
      printline("Deleted ID :" + String(id));
      id++;
    }
    if (division == 0)
    {
      fyjump = 1;
      divi = "FY";
    }
    else if (division == 1)
    {
      syjump = 1;
      divi = "SY";
    }
    else if (division == 2)
    {
      tyjump = 1;
      divi = "TY";
    }
    writeJumps();
    clrscr();
    printline("Full Class Deleted");
    printline("Class :" + divi);
    buzz(1);
    String details = "Deleted Class " + divi;
    writeCLog(details);
    delay(1500);
    clrscr();
  }
}

void delClass(int division)
{
  int limit = ((division + 1) * 60) - 1;
  int id = division * 60;
  String divi = "";
  while (id <= limit)
  {
    fps.DeleteID(id);
    clrscr();
    printline("Deleted ID :" + String(id));
    id++;
  }
  if (division == 0)
  {
    fyjump = 1;
    divi = "FY";

  }
  else if (division == 1)
  {
    syjump = 1;
    divi = "SY";
  }
  else if (division == 2)
  {
    tyjump = 1;
    divi = "TY";
  }
  writeJumps();
  printline("Full Class Deleted");
  printline("Class :" + divi);
  String details = "Deleted Class " + divi;
  writeCLog(details);
  delay(1000);
  clrscr();
}

void delDB()
{
  clrscr();
  printline("!!Warning!!");
  printline("Full DB Deletion");
  printline("Press A to Continue");
  printline("Any Key to Abort");
  int flag = 0;
  delay(1000);
  char key = ' ';
  key = keypad.waitForKey();
  switch (key)
  {
    case 'A':
      flag = 1;
      break;
    default :
      flag = 0;
      break;
  }
  if (flag == 1)
  {
    fps.DeleteAll();
    fyjump = 1;
    syjump = 1;
    tyjump = 1;
    writeJumps();
    String details = "Deleted DB";
    writeCLog(details);
    clrscr();
    printline("Full DB Deleted");
    printline("New Admin Needed");
    delay(2000);
    addAdmin();
  }
}

int getClass()
{
  int division;
  int flag = 0;
  do
  {
    clrscr();
    printline("Select Class");
    printline("A - FY");
    printline("B - SY");
    printline("C - TY");
    char key = ' ';
    key = keypad.waitForKey();
    clrscr();
    switch (key)
    {
      case 'A':
        printline("FY Selected");
        delay(1000);
        clrscr();
        flag = 1;
        division = 0;
        //roll = fyjump;
        break;
      case 'B':
        printline("SY Selected");
        delay(1000);
        clrscr();
        flag = 1;
        division = 1;
        //roll = syjump;
        break;
      case 'C':
        printline("TY Selected");
        delay(1000);
        clrscr();
        flag = 1;
        division = 2;
        //roll = tyjump;
        break;
      default:
        printline("Invalid Input");
        delay(1000);
        clrscr();
        flag = 1;
    }
  } while (flag == 0);
  return (division);
}

int getRoll()
{
  int roll = 0;
  String Roll = "";
  int flag = 0;
  do
  {
    clrscr();
    printleft("A - Clear");
    printleft("B - Delete");
    printleft("* - Enter");
    printleft("Roll No: " + Roll);
    char key = ' ';
    key = keypad.waitForKey();
    if (key == 'C' || key == 'D' || key == '#')
    {
      digitalWrite(buzzPin, HIGH);
      delay(500);
      digitalWrite(buzzPin, LOW);
    }
    else if (key == 'A' )
    {
      Roll = "";
    }
    else if (key == 'B' && Roll.length() > 0)
    {
      Roll.remove((Roll.length() - 1));
    }
    else if (key == '*')
    {
      if (roll == 0 || roll > 120 || roll < 0)
      {
        clrscr();
        printline("Invalid Roll Number");
        printline("Enter Roll No Again");
        Roll = "";
        roll = 0;
        delay(1500);
      }
      else
      {
        flag = 1;
      }
    }
    else
    {
      Roll = String(Roll + key);
    }
    roll = Roll.toInt();
  } while (flag == 0);
  return (roll);
}

void powerSave()
{
  clrscr();
  printline("Power Save Mode");
  printline("");
  printline("To Power Up");
  printline("Press Button");
  //eth.powerOff();
  int brightness0;
  int brightness1;
  int fade;
  while (digitalRead(buttonPin) == LOW)
  {
    brightness0 = 0;
    brightness1 = 255;
    fade = 5;
    while (brightness0 <= 255 && brightness1 >= 0 && digitalRead(buttonPin) == LOW)
    {
      analogWrite(gledPin, brightness0);
      analogWrite(rledPin, brightness1);
      brightness1 -= fade;
      brightness0 += fade;
      delay(10);
    }
    if (digitalRead(buttonPin) == LOW)
    {
      brightness0 = 255;
      brightness1 = 0;
    }
    else
    {
      clrscr();
      printline("Reading Finger");
    }
    while (brightness0 >= 0 && brightness1 <= 255 && digitalRead(buttonPin) == LOW)
    {
      analogWrite(gledPin, brightness0);
      analogWrite(rledPin, brightness1);
      brightness1 += fade;
      brightness0 -= fade;
      delay(10);
    }
  }
  if (digitalRead(buttonPin) == HIGH)
  {
    clrscr();
    printline("Unpress Button");
    while (digitalRead(buttonPin) == HIGH)
    {}
  }
  digitalWrite(gledPin, LOW);
  digitalWrite(rledPin, LOW);
}

void clrscr()
{
  lcd.clear();
  line = -1;
}

void printline(String text )
{
  int len = text.length();
  int indent = ((20 - len) / 2);
  for (int i = 0; i < indent; i++)
  {
    text = " " + text;
  }
  for (int i = 0; text.length() < 20; i++)
  {
    text = text + " ";
  }

  line++;
  if (line > 3)
  {
    line = 0;
  }
  lcd.setCursor(0, line);
  lcd.print(text);
}

void printleft(String text )
{
  line++;
  if (line > 3)
  {
    line = 0;
  }
  lcd.setCursor(0, line);
  lcd.print(text);
}

String print2digits(int number)
{
  String num = "";
  if (number >= 0 && number < 10) {
    num = "0";
  }
  num = num + String(number);
  return (num);
}

String print3digits(int number)
{
  String num = "";
  if (number >= 0 && number < 10) {
    num = "00";
  }
  if (number >= 10 && number < 100) {
    num = "0";
  }
  num = num + String(number);
  return (num);
}

String getTime()
{
  RTC.read(tm);
  String rtc =  print2digits(tm.Hour) + ":" + print2digits(tm.Minute) + ":" + print2digits(tm.Second) + " " + tmYearToCalendar(tm.Year) + "/" + print2digits(tm.Month) + "/" + print2digits(tm.Day) ;
  return (rtc);
}

void buzz(int sec)
{
  int totalBuzz = sec * 5; //Calculate no of seconds
  int buzzCount = 0;
  int ledpin = 10;
  while (buzzCount <= totalBuzz)
  {
    if (buzzCount % 2 == 0)
    {
      ledpin = 10;
    }
    else
    {
      ledpin = 11;
    }
    digitalWrite(ledpin, HIGH);
    digitalWrite(buzzPin, HIGH);
    delay(100);
    digitalWrite(ledpin, LOW);
    digitalWrite(buzzPin, LOW);
    delay(100);
    buzzCount++;
  }
}

int checkPin()
{
  String pinCode = "";
  int flag = 0;
  do
  {
    clrscr();
    printline("A - Clear");
    printline("B - Delete");
    printline("* - Enter");
    printline("Pincode : " + pinCode);
    char key = ' ';
    key = keypad.waitForKey();
    if (key == 'C' || key == 'D' || key == '#')
    {
      digitalWrite(buzzPin, HIGH);
      delay(500);
      digitalWrite(buzzPin, LOW);
    }
    else if (key == 'A' )
    {
      pinCode = "";
    }
    else if (key == 'B' && pinCode.length() > 0)
    {
      pinCode.remove((pinCode.length() - 1));
    }
    else if (key == '*')
    {
      flag = 1;
    }
    else
    {
      pinCode = String(pinCode + key);
    }
  } while (flag == 0);
  clrscr();
  printline("Checking Pin");
  //eth.powerOn();
  String contentType = "application/x-www-form-urlencoded ";
  String postData = "password=" + String(password) + "&pin=" + pinCode;
  client.post("/Cerberus/checkPin", contentType, postData);
  int statusCode = client.responseStatusCode();
  int response = (client.responseBody()).toInt();
  //eth.powerOff();
  if (statusCode == 200)
  {
    return (response);
  }
  else
  {
    clrscr();
    printline("Connection");
    printline("Failed");
    delay(1000);
    return (2);
  }
}

int sync()
{
  int data = 0;
  int flag = 0;
  fps.SetLED(false);
  delay(1000);
  for (int i = 1; i <= 6; i++)
  {
    clrscr();
    printline("Syncing Files");
    printline("DO NOT POWER");
    printline("OFF THE DEVICE");
    printline("File " + String (i) + "/6");
    delay(1000);
    char filenamef[13] = "log1full.txt";
    char filenamed[13] = "log1data.txt";
    char filename[9] = "log1.txt";
    filename[3] = fileId(i);
    filenamed[3] = fileId(i);
    filenamef[3] = fileId(i);
    int file = 0;
    if (SD.exists(filenamef))
    {
      myFile = SD.open(filenamef);
      file = 1;
    }
    else if (SD.exists(filenamed))
    {
      myFile = SD.open(filenamed);
      file = 2;
    }
    else if (SD.exists(filename))
    {
      data++;
    }
    if (file == 1 || file == 2)
    {
      String contents = "";
      char c;
      while (myFile.available())
      {
        c = myFile.read();
        contents += c;
      }
      myFile.close();
      String contentType = "application/x-www-form-urlencoded ";
      String postData = "password=" + String(password) + "&log=" + contents;
      //eth.powerOn();
      client.post("/Cerberus/bank", contentType, postData);
      int statusCode = client.responseStatusCode();
      String response = client.responseBody();
      //eth.powerOff();
      if (statusCode == 200)
      {
        if (file == 1)
        {
          SD.remove(filenamef);
        }
        if (file == 2) {
          SD.remove(filenamed);
        }
        myFile = SD.open(filename, FILE_WRITE);
        myFile.close();
        flag = 1;
        clrscr();
        printline("File " + String(i) + " Uploaded");
        printline("Sync Successfull");
        delay(1500);
      }
      else
      {
        flag = 0;
      }
    }
  }
  if (data == 6)
  {
    flag = 1;
    clrscr();
    printline("No data to upload");
    delay(1500);
  }
  fps.SetLED(true);
  return (flag);
}

char fileId(int id)
{
  char c;
  switch (id)
  {
    case 1 :
      c = '1'; break;
    case 2 :
      c = '2'; break;
    case 3 :
      c = '3'; break;
    case 4 :
      c = '4'; break;
    case 5 :
      c = '5'; break;
    case 6 :
      c = '6'; break;
  }
  return (c);
}

void writeLog(String text)
{
  int id = 0;
  char filenamef[13] = "log1full.txt";
  char filenamed[13] = "log1data.txt";
  char filename[9] = "log1.txt";
  String rtc = getTime();
  text = text + " " + rtc;
  String contents;
  int flag = 0;
  for (id = 1; id <= 6 && flag == 0; id++)
  {
    filenamed[3] = fileId(id);
    if (SD.exists(filenamed))
    {
      myFile = SD.open(filenamed, FILE_WRITE);
      myFile.println(text);
      myFile.close();
      myFile = SD.open(filenamed);
      char c;
      int lines = 0;
      contents = "";
      while (myFile.available())
      {
        c = myFile.read();
        if (c == 10)
        {
          lines++;
        }
        contents += c;
      }
      myFile.close();
      if (lines == 40 )
      {
        filenamef[3] = fileId(id);
        myFile = SD.open(filenamef, FILE_WRITE);
        myFile.println(contents);
        myFile.close();
        SD.remove(filenamed);
      }
      flag = 1;
    }
  }
  if (id == 7)
  {
    flag = 0;
    for (id = 1; id <= 6 && flag == 0; id++)
    {
      filename[3] = fileId(id);
      if (SD.exists(filename))
      {
        SD.remove(filename);
        filenamed[3] = fileId(id);
        myFile = SD.open(filenamed, FILE_WRITE);
        myFile.println(text);
        myFile.close();
        flag = 1;
      }
    }
    if (id == 7)
    {
      clrscr();
      printline("Data Overload");
      printleft("Check Connection");
      printline("and perform");
      printline("Manual Restart");
      while (1)
      {
        digitalWrite(rledPin, HIGH);
        delay(1000);
        digitalWrite(rledPin, LOW);
        delay(1000);
      }
    }
  }
}

void writeCLog(String text)
{
  String rtc = getTime();
  text = text + " " + rtc;
  myFile = SD.open("clog.txt", FILE_WRITE);
  myFile.println(text);
  myFile.close();
  delay(1000);
}

void readJumps()
{
  myFile = SD.open("jumps.txt");
  int recNum = 1; // We have read 0 records so far
  while (myFile.available())
  {
    String list = myFile.readStringUntil(',');
    switch (recNum)
    {
      case 1:
        fyjump = list.toInt();
        break;
      case 2:
        syjump = list.toInt();
        break;
      case 3:
        tyjump = list.toInt();
        break;
    }
    recNum++;
  }
  myFile.close();
}

void writeJumps()
{
  SD.remove("jumps.txt");
  myFile = SD.open("jumps.txt", FILE_WRITE);
  myFile.println(String(fyjump) + "," + String(syjump) + "," + String(tyjump));
  myFile.close();
}
