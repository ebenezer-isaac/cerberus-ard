<h1 align="center">
	<img width="80" src="https://raw.githubusercontent.com/iamkotwala/cerberus/master/logo-circle.png">
	<br>
	C E R B E R U S
	<br>
	Remote Fingerprint Attendance System
</h1>

### Hardware used : ###
* <a href="https://robu.in/product/atmel-mcu-atmega16u2-mega-2560-r3-improved-version-ch340g-cable-arduino-mega-2560-transparent-acrylic-case-arduino-mega-2560/?gclid=Cj0KCQjwtMvlBRDmARIsAEoQ8zQRZXfmCU2xr6cv7uVrZ77PqEXLFcOEKLTSkX0KvOt5glzpWTEVXW4aAsJrEALw_wcB">Arduino Mega 2560 R3</a>
* <a href="https://robu.in/product/fingerprint-scanner-ttl-gt-511c3/">GT511C3</a>
* <a href="https://robu.in/product/serial-2004-20-x-4-iici2ctwi-blue-backlight-lcd-module/?gclid=CjwKCAjwza_mBRBTEiwASDWVvgv5-7Z2ck3JDQexbm28OYU8A0GNDzFmfHsDwZYOSB_mKJHKnKkA3RoCr78QAvD_BwE">2004 Character Display</a>
* <a href="https://robu.in/product/ethernet-module-enc28j60/">ENC28J60</a>
* <a href="https://robu.in/product/tiny-rtc-real-time-clock-ds1307-i2c-iic-module-for-arduino/">DS1307</a>
* <a href="https://robu.in/product/sd-card-reading-writing-module-arduino/?gclid=Cj0KCQjwn8_mBRCLARIsAKxi0GKwAo3eZptNW_0JqorbEiFiDLG7JqhbTHO8yQ2QhS7Gb-Pc-_A4DjAaAudhEALw_wcB">SD Card Module</a>
* <a href="https://robu.in/product/4x4-matrix-keypad-membrane-switch-arduino-arm-mcu/">4x4 Membrane Keypad</a>
* Buzzer</a>
* A Few LEDs</a>

### Pin Configuration ###
* Fingerprint Scanner:
	* Reciever	- A8(Green)
	* Transmitter	- A9(White)
* LED GREEN	:pin 10
* LED RED	: pin 11
* Buzzer	: pin 12
* Button	:	pin 2
* MISO		: pin 50
* MOSI		: pin 51
* SCK		: pin 52
* CS:
	* Ethernet	- 3
	* SD		- 53
* Keypad    : 22 to 29 (4 row pins before 4 column pins)

### Highlights ###
* System has been designed for 3 classrooms each fitted with one arduino system with each classroom having 3 batches(referenced as divisions in the code)
* Attendance starts after identification of admin finger(ids 180 and above)
* Stores Time, Date, Student Roll Number in 6 files(max 40 records each, you can increase as per requirement)
	* This was implemented so that it doesnt overload the sync function(below)
* Sync function sends data to remote server on the network periodically and runs lightshow function when idle
* Fully Functional Enrollment Modules(i.e. functions) with error control, error count and automatic try again
	* Continous Enrollment
	* Selected ID Enrollment
* Fully Functional Delete Modules(i.e. functions) 
	* Delete Selected ID
	* Delete Selected Division
	* Delete Whole Database
* Web Server recieves data, checks timetable for subject and inserts data into respective subject table
* Web Server Modules
	* Login
	* Admin Management
	* Subject Management
	* Timetable Management
	* Student Management
	
### Known Issues ###
* Ethernet and SD Card Modules cannot communicate simultaneously<br>(Hopefully someone solves it)
* The database file got lost during the process, you will have to make your own database

### Architecture of the system ###
<h1 align="center">
	<img width="600" src="Architecture.png">
	<br>
	<br>
</h1>

### Programmatic Flow of the Sketch ###
<h1 align="center">
	<img width="600" src="FlowChart.jpg">
	<br>
	<br>
</h1>

