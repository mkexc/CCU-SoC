#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <string.h>

LiquidCrystal_I2C lcd(0x3F,20,4);

char buf [100];
volatile byte pos;
volatile boolean process_it;
char *token;
const char delim[2] = "-";
int i;
int CCU_Params[3];

void applyParams(void);

void setup (void)
{
  Serial.begin (9600);   // debugging
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  // turn on interrupts
  SPCR |= _BV(SPIE);
  
  pos = 0;
  process_it = false;

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
byte c = SPDR;
  
  // add to buffer if room
  if (pos < sizeof buf)
    {
    buf [pos++] = c;
    
    // example: newline means time to process buffer
    if (c == '\n')
      process_it = true;
      
    }  // end of room available
}

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  if (process_it){
    buf [pos] = 0;  
    Serial.println (buf);
    pos = 0;
    process_it = false;

    token = strtok (buf,delim);
    i=0;
    while(token != NULL && i < 3){
      switch (i)
      {
      case 0:
        Serial.print("Token 1: ");
        Serial.println(token);
        if(strcmp(token,"1") == 0)
          CCU_Params[i++] = 1;
        else
          CCU_Params[i++] = 0;
        break;
      case 1:
        Serial.print("Token 2: ");
        Serial.println(token);
        if(strcmp(token,"H") == 0)
          CCU_Params[i++] = 1;
        else
          CCU_Params[i++] = 0;
        break;
      case 2:
        Serial.print("Token 3: ");
        Serial.println(token);
        CCU_Params[i] = atoi(token);
        if (CCU_Params[i] > 5)
          CCU_Params[i] = 5;
        else if (CCU_Params[i] < 0)
          CCU_Params[i] = 0;
        i++;
        break;
      default:
        Serial.print("Parsing error!");
        break;
      }
      token = strtok(NULL, delim);
    }
    applyParams();
  }  // end of flag set    
    
}  // end of loop

void applyParams(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CCU STATUS: ");
  lcd.setCursor(12,0);
  if(CCU_Params[0]){
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  lcd.setCursor(0,1);
  lcd.print("MODE: ");
  lcd.setCursor(6,1);
  if(CCU_Params[1]){
    lcd.print("HEAT ");
  } else {
    lcd.print("COLD ");
  }
  lcd.setCursor(11,1);
  lcd.print("FAN: ");
  lcd.setCursor(15,1);
  lcd.print(CCU_Params[2]);
}