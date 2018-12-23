/**
 * This software is writen under GPL v3
 * https://www.gnu.org/licenses/gpl-3.0.en.thml
 * Christian Görtz 2018
 */
//#define LCD16X4 //character lcd
#define GLCD // KS0108 grafic lcd

#include <Arduino.h>
#include <stdlib.h>
#include <SPI.h>

#ifdef LCD16X4
#include <LiquidCrystal.h>
#define LCD_D4 A8
#define LCD_D5 A9
#define LCD_D6 A10
#define LCD_D7 A11
#define LCD_EN A12
#define LCD_RS A13
#endif
#ifdef GLCD
#include <U8g2lib.h>
#define GLCD_ROTATION 0
#define GLCD_D0 36
#define GLCD_D1 34
#define GLCD_D2 32
#define GLCD_D3 30
#define GLCD_D4 28
#define GLCD_D5 26
#define GLCD_D6 24
#define GLCD_D7 22
#define GLCD_CS0 27
#define GLCD_CS1 29
#define GLCD_CS2
#define GLCD_RST 31
#define GLCD_EN 23
#define GLCD_DC 25
#include <icons.h>
#endif
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <RFID.h>
#include <systemClock.h>

#define RFID_SS_PIN A0
#define RFID_RST_PIN A1

#define BTN_IN 49
#define BTN_OUT 47
#define LED_IN 43
#define LED_OUT 45
#define LED_BUSY 53
#define RFIDTAG_LENGHT 5
#define MODE_IN 1
#define MODE_OUT 2
#define MODE_INFO 3
#define BAUD 115200

RFID rfid(RFID_SS_PIN, RFID_RST_PIN);
#ifdef LCD16X4
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
#endif
#ifdef GLCD
U8G2_KS0108_128X64_2 glcd(U8G2_R2, GLCD_D0, GLCD_D1, GLCD_D2, GLCD_D3, GLCD_D4,
GLCD_D5, GLCD_D6, GLCD_D7, GLCD_EN, GLCD_DC, GLCD_CS0, GLCD_CS1,
U8X8_PIN_NONE, U8X8_PIN_NONE);
#endif
byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress server_addr(192, 168, 100, 19);  // IP of the MySQL *server* here
char user[] = "zeiterfassung";              // MySQL user login username
char password[] = "zeit1";        // MySQL user login password
EthernetClient client;
MySQL_Connection conn((Client *) &client);

byte rfidtag[RFIDTAG_LENGHT] = { 0, 0, 0, 0, 0 };

systemClock sysClock(0, 0, 0);
void setup() {

	Serial.begin(BAUD);
	Serial.print("Initialisierung\n");
	Serial.print("GPIO init...\n");
	initGPIO();
	Serial.print("test lamps...\n");
	digitalWrite(LED_BUSY, HIGH);
	digitalWrite(LED_IN, HIGH);
	digitalWrite(LED_OUT, HIGH);
	delay(300);
	digitalWrite(LED_IN, LOW);
	digitalWrite(LED_OUT, LOW);
	Serial.print("LCD init...\n");

#ifdef LCD16X4
	lcd.begin(16, 4);
	lcd.setCursor(0,0);
	lcd.print("================");
	lcd.setCursor(0, 1);
	lcd.print("Stempeluhr");
	lcd.setCursor(0, 3);
	lcd.print("================");
#endif
#ifdef GLCD
	glcd.begin();
	glcd.firstPage();
	do {
		glcd.setFont(u8g2_font_mozart_nbp_tf);
		glcd.drawStr(0, 12, "open time clock");
		glcd.drawStr(0, 25, "github.com/chrisgoertz/");
		glcd.drawStr(0, 38, "booting...");
		glcd.drawStr(0,51, "LICENCE: GPL-3.0");
		glcd.drawStr(0,64, "Christian Goertz 2018");
	} while (glcd.nextPage());

#endif
	Serial.print("SPI init...\n");
	SPI.begin();
	Serial.print("RFID init...\n");
	rfid.init();

	Serial.print("Ethernet init...\n");
	Ethernet.begin(mac_addr);
	if (conn.connect(server_addr, 3307, user, password)) {
		Serial.println("connected to MSQL-DB");
	} else {
		Serial.println("Connecting to MYSQL-DB failed.");
	}

	digitalWrite(LED_BUSY, LOW);
	Serial.print("SETUP done\n");

}

void loop() {
	static long lastTime = 0;
	static long lastRead = 0;
	long cycletime = 0;
	byte sts = readBtns();
	setLeds(sts);
	long timeNow = millis();
	if ((timeNow - lastTime) > 1000) {
		lastTime = timeNow;
		if (sysClock.getMinutes() == 0 && sysClock.getSeconds() == 0) {
			readClockFromDB();
		}
		sysClock.secondTick();
		//printSysClock();
		if (millis() - lastRead >= 10000){
			drawSysClock(&sts);

		}
		//drawTestScreen();
		String s = sysClock.getTimeString();
		char time[9] = { 0 };
		s.toCharArray(time, 9, 0);
		sysClock.getTimeChars(time);
		//static int perc = 0;
		//drawBusyScreen(perc++, "Verarbeite...", time, "message2", "taskname",
		//		"theinfo");
		//if (perc > 100) {
//			perc = 0;
//		}
	}

	if (rfid.isCard()) {
		digitalWrite(LED_BUSY, HIGH);
		cycletime = millis();
		//rfidGetStatus();
		if (!readCard(&sts)) {
			//variable to store last card read time
			lastRead = millis();
			//strings to display
			char rfidArray[25];
			char nameArray[50];
			char buchungsModus[18];
			//format string for display selected mode
			if(sts == MODE_IN){
				sprintf(buchungsModus, "%s","Status: anmelden");
				//buchungsModus = "Status: anmelden";
			}
			if(sts == MODE_OUT){
				sprintf(buchungsModus, "%s","Status: abmelden");
//				buchungsModus = "Status: abmelden";
			}
			if(sts == MODE_INFO){
				sprintf(buchungsModus, "%s","Status: abfragen");
//								buchungsModus = "Status: abfragen"
			}
			//put string with last read rfidtag in memory
			rfidCharArray(rfidArray);
			//draw 'Busyscreen' so the user gets feedback
			//
			drawBusyScreen(33, "Datenbankverbindung", buchungsModus, rfidArray,
					"Name abfragen", "sending...");
			//put debug message to RS232
			Serial.println("readCard");
			//store data in db
			putDB(&sts);
			//get name for last rfid from database
			getNameFromDatabase(nameArray);
			//draw 'Busyscreen'
			drawBusyScreen(100, "Datenbankverbindung", buchungsModus, rfidArray,
						nameArray, "done...");
			//some debug -> RS232
			//show cycle time for last task
			Serial.print("Cycle: ");
			Serial.print(millis() - cycletime);
			Serial.println(" ms");
		}
		digitalWrite(LED_BUSY, LOW);
	}

}

void drawTestScreen() {
	glcd.setFont(u8g2_font_t0_11_mr);
	glcd.firstPage();
	do {
		glcd.drawFrame(0, 0, 128, 64);
		glcd.drawStr(2, 10, "Testbildschirm");
		glcd.drawHLine(0, 11, 128);
		glcd.drawStr(2, 20, "Name: Hans Wurst");
		glcd.drawHLine(0, 21, 128);
		glcd.drawStr(2, 30, "RFID: 01:02:AB:D3:C5");
		glcd.drawHLine(0, 31, 128);
		glcd.drawStr(2, 40, "Status: -anwesend-");
		glcd.drawHLine(0, 41, 128);
		glcd.drawStr(2, 50, "Zeitkonto: inaktiv");
		glcd.drawHLine(0, 51, 128);
		glcd.drawStr(2, 60, "Resturlaub: inaktiv");

	} while (glcd.nextPage());
}


void drawBusyScreen(uint8_t percent, const char *title, const char *info,
		const char *task, const char *message, const char *message2) {
	int barlength = (128 * percent) / 100;
	glcd.setFont(u8g2_font_mozart_nbp_tf);
	glcd.firstPage();
	do {
		glcd.drawFrame(0, 0, 128, 64);
		glcd.drawStr(2, 10, title);
		glcd.drawHLine(0, 11, 128);
		glcd.drawStr(2, 20, info);
		glcd.drawHLine(0, 21, 128);
		glcd.drawStr(2, 30, task);
		glcd.drawHLine(0, 31, 128);
		glcd.drawStr(2, 40, message);
		glcd.drawHLine(0, 41, 128);
		glcd.drawStr(2, 50, message2);
		glcd.drawHLine(0, 51, 128);
		glcd.drawBox(0, 53, barlength, 9);

	} while (glcd.nextPage());
}
void readClockFromDB() {
	sysClock.setTime(getTime());
}
void printSysClock() {
	Serial.println("System clock");
	Serial.println(sysClock.getTimeString());
}
void drawSysClock(byte *status) {
	String t = sysClock.getTimeString();
	char timeString[10];
	t.toCharArray(timeString, 9);

	glcd.firstPage();
	do {
		glcd.setFont(u8g2_font_courB14_tr);
		glcd.drawStr(20, 14, timeString);
		glcd.drawFrame(19, 0, 90, 16);
		if (*status == MODE_OUT) {
			glcd.drawXBM(48, 32, exit_height, exit_width, exit_bits);
		}
		if (*status == MODE_IN) {
			glcd.drawXBM(48, 32, perso_height, perso_width, perso_bits);
		}
	} while (glcd.nextPage());

}
/**
 * GPIO configuration
 */
void initGPIO() {
	pinMode(BTN_IN, INPUT_PULLUP);
	pinMode(BTN_OUT, INPUT_PULLUP);
	pinMode(LED_IN, OUTPUT);
	pinMode(LED_OUT, OUTPUT);
	pinMode(LED_BUSY, OUTPUT);
}

/**
 * Read the rfidtags serial
 */
byte readCard(byte *drctn) {
	static char lastRfid[RFIDTAG_LENGHT + 1] = { 0, 0, 0, 0, 1 };
	char readRfid[RFIDTAG_LENGHT + 1];
	static byte drctn_old = MODE_IN;
	bool repeatedRfid = drctn_old == *drctn ? true : false;

	if (rfid.readCardSerial()) {
		//Store serial in buffer
		for (uint8_t i = 0; i < RFIDTAG_LENGHT; i++) {
			readRfid[i] = rfid.serNum[i];

		}
		//check if at least of the five bytes is different
		//from the old value, the it's no repeat
		for (uint8_t i = 0; i < RFIDTAG_LENGHT; i++) {
			if (readRfid[i] != lastRfid[i]) {
				repeatedRfid = false;
			}
			//store serial in compare buffer (lastRFid)
			for (uint8_t i = 0; i < RFIDTAG_LENGHT; i++) {
				lastRfid[i] = readRfid[i];
				rfidtag[i] = readRfid[i];
			}
		}
	}
	drctn_old = *drctn;
	if (repeatedRfid) {
		return 1;
	}
	return 0;
}

/**
 *
 * puts the rfid-tag into db
 */
void putDB(byte *sts) {

	char cbuffer[15];
	sprintf(cbuffer, "%02X:%02X:%02X:%02X:%02X", rfidtag[4], rfidtag[3],
			rfidtag[2], rfidtag[1], rfidtag[0]);
	Serial.println(cbuffer);

	//cbuffer contains the rfidtag as string formated like:
	//00:11:22:33:44
	//serial[0]...serial[4]
	char INSERT_SQL[100];
	char drctn[4];
	if (*sts == MODE_IN) {
		sprintf(drctn, "%s", "in");
	} else {
		sprintf(drctn, "%s", "out");
	}
	sprintf(INSERT_SQL,
			"INSERT INTO zeiterfassung.stampevents (rfid, status) VALUES ('%s','%s');",
			cbuffer, drctn);
	Serial.println(INSERT_SQL);
	// Initiate the query class instance
	MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
	// Execute the query
	cur_mem->execute(INSERT_SQL);
	// Note: since there are no results, we do not need to read any data
	// Deleting the cursor also frees up memory used
	delete cur_mem;

}

/**
 * Response screen
 * will be drawn, if a readable rfid-tag is recognised
 * arg: * selected MODE [IN,OUT,INFO]
 */
void rfidResponse(byte *status) {
	//#############################
	//build String from global rfid-tag var
	char cbuffer[20];
	char* sbuffer = "";
	sprintf(cbuffer, "RFID: %02X:%02X:%02X:%02X:%02X", rfidtag[4], rfidtag[3],
			rfidtag[2], rfidtag[1], rfidtag[0]);
	byte dbstatus = rfidGetStatus();
	Serial.print("rfid-status: ");
	Serial.println(dbstatus);
	switch (dbstatus) {
	case MODE_IN:
		sbuffer = "Status: anwesend-";
		break;
	case MODE_OUT:
		sbuffer = "Status: -abwesend";
		break;
	}

	//#############################

#ifdef LCD16X4
	//if char lcd is in use
	lcd.setCursor(0, 1);
	lcd.print(cbuffer);
#endif
#ifdef GLCD
	//if glcd is in use
	glcd.firstPage();
	do {
		glcd.setFont(u8g2_font_t0_11_mr);
		glcd.drawFrame(0, 0, 128, 64);
		glcd.drawStr(2, 10, "Name: Bitchi Babe");
		glcd.drawStr(2, 20, cbuffer);
		glcd.drawStr(2, 30, sbuffer);
		glcd.drawStr(2, 40, "Zeitkonto: 12:21");
		glcd.drawStr(2, 50, "Resturlaub: 8 Tage");
	} while (glcd.nextPage());

#endif

}

/**
 * Read the status select buttons
 * buttons will pull the pin level to low
 * returns the selected mode [IN,OUT,INFO]
 */
byte readBtns() {
	static byte statDirection = MODE_IN;
	//button -> in pushed
	if (!digitalRead(BTN_IN)) {
		statDirection = MODE_IN;
		delay(50); //debounce
	}
	//button -> out pushed
	if (!digitalRead(BTN_OUT)) {
		statDirection = MODE_OUT;
		delay(50); //debounce
	}
	//buttons -> [in & out] pushed ->> MODE_INFO
	if (!digitalRead(BTN_IN) && !digitalRead(BTN_OUT)) {
		statDirection = MODE_INFO;
		delay(50); //debounce
	}
	return statDirection;
}

/**
 * sets the leds as indicators, depending on the
 * selected mode
 */
void setLeds(byte d) {
	switch (d) {
	case MODE_IN:
		digitalWrite(LED_OUT, LOW);
		digitalWrite(LED_IN, HIGH);
		break;
	case MODE_OUT:
		digitalWrite(LED_IN, LOW);
		digitalWrite(LED_OUT, HIGH);
		break;
	case MODE_INFO:
		digitalWrite(LED_IN, HIGH);
		digitalWrite(LED_OUT, HIGH);
	}
}
/**
 * fetch date from mysql db
 * return: String like "yyyy-mm-dd"
 */
String getDate() {
	row_values *row = NULL;
	char *query = "SELECT CURRENT_DATE";
	String dateString = "";
	MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
	// Execute the query
	cur_mem->execute(query);
	// Fetch the columns (required) but we don't use them.
	column_names *columns = cur_mem->get_columns();
	// Read the row (we are only expecting the one)
	do {
		row = cur_mem->get_next_row();
		if (row != NULL) {
			dateString = (row->values[0]);
		}
	} while (row != NULL);
	// Deleting the cursor also frees up memory used
	delete cur_mem;
	return dateString;
}
/**
 * fetch time from mysql db
 * return: String like "hh:mm:ss"
 */
String getTime() {
	row_values *row = NULL;
	char *query = "SELECT CURRENT_TIME";
	String dateString = "";
	MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
	// Execute the query
	cur_mem->execute(query);
	// Fetch the columns (required) but we don't use them.
	column_names *columns = cur_mem->get_columns();
	// Read the row (we are only expecting the one)
	do {
		row = cur_mem->get_next_row();
		if (row != NULL) {
			dateString = (row->values[0]);
		}
	} while (row != NULL);
	// Deleting the cursor also frees up memory used
	delete cur_mem;
	return dateString;
}
/**
 * fetch datetime from mysql db
 * return: String like "yyyy-mm-dd hh:mm:ss"
 */
String getDateTime() {
	row_values *row = NULL;
	char *query = "SELECT CURRENT_TIMESTAMP";
	String dateString = "";
	MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
	// Execute the query
	cur_mem->execute(query);
	// Fetch the columns (required) but we don't use them.
	column_names *columns = cur_mem->get_columns();
	// Read the row (we are only expecting the one)
	do {
		row = cur_mem->get_next_row();
		if (row != NULL) {
			dateString = (row->values[0]);
		}
	} while (row != NULL);
	// Deleting the cursor also frees up memory used
	delete cur_mem;
	return dateString;
}

/**
 * fetch the last status pushed status from db [in,out]
 * return: MODE_IN or MODE_OUT
 */
byte rfidGetStatus() {
	row_values *row = NULL;
	char statusString[4];
	char sql[150];
	sprintf(sql,
			"SELECT * FROM zeiterfassung.stampevents WHERE rfid = '%02X:%02X:%02X:%02X:%02X' AND date(timestmp) = cast(now() as date) order by timestmp desc limit 1;",
			rfidtag[4], rfidtag[3], rfidtag[2], rfidtag[1], rfidtag[0]);

	MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
	cur_mem->execute(sql);
	column_names *columns = cur_mem->get_columns();
	do {
		row = cur_mem->get_next_row();
		if (row != NULL) {
			sprintf(statusString, "%s", row->values[2]);

		}
	} while (row != NULL);
	delete cur_mem;
	Serial.print("statusString: ");
	Serial.println(statusString);

	if ((statusString[0] == 'i') && (statusString[1] == 'n')) {
		Serial.println("Status-s = in");
		return MODE_IN;
	}
	if ((statusString[0] == 'o') && (statusString[1] == 'u')) {
		Serial.println("Status-s = out");
		return MODE_OUT;

	}
}

void rfidCharArray(char *arr) {

	sprintf(arr, "RFID: %02X:%02X:%02X:%02X:%02X", rfidtag[4], rfidtag[3],
			rfidtag[2], rfidtag[1], rfidtag[0]);
}

void getNameFromDatabase(char *name) {
	row_values *row = NULL;

	char sql[150];
	sprintf(sql,
			"SELECT vorname, nachname FROM zeiterfassung.personen WHERE rfidString = '%02X:%02X:%02X:%02X:%02X' limit 1;",
			rfidtag[4], rfidtag[3], rfidtag[2], rfidtag[1], rfidtag[0]);
	MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
	cur_mem->execute(sql);
	column_names *columns = cur_mem->get_columns();
	do {
		row = cur_mem->get_next_row();
		if (row != NULL) {
			sprintf(name, "%s %s", row->values[0], row->values[1]);

		}
	} while (row != NULL);
	delete cur_mem;
	;
}
