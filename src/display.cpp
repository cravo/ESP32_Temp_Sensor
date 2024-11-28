// For this project I'm using this display: https://www.amazon.co.uk/dp/B0B76Z83Y4?ref=ppx_yo2ov_dt_b_fed_asin_title
// A Freenove LCD1602
//
// Wired GND to GND, Power to 5V, SDA to G21 and SCL to G22 (21 and 22 being the defaults for I2C connection)

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c LCD i/o class header

hd44780_I2Cexp lcd_display; // use device at this address


// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

void display_setup()
{
    int status;

	// initialize LCD with number of columns and rows: 
	// hd44780 returns a status from begin() that can be used
	// to determine if initalization failed.
	// the actual status codes are defined in <hd44780.h>
	// See the values RV_XXXX
	//
	// looking at the return status from begin() is optional
	// it is being done here to provide feedback should there be an issue
	//
	status = lcd_display.begin(LCD_COLS, LCD_ROWS);
	if(status) // non zero status means it was unsuccesful
	{
        Serial.print("Display setup error: ");
        Serial.print(status);

		// begin() failed so blink error code using the onboard LED if possible
		hd44780::fatalError(status); // does not return
	}

	// Print a message to the LCD
	lcd_display.print("Hello, World!");
}

void display_show(const char * text)
{
    lcd_display.print(text);
}

void display_clear()
{
    lcd_display.clear();
}

// Set the display cursor to the start of the given line (0-based)
void display_line(int line)
{
    lcd_display.setCursor(0,line);
}
