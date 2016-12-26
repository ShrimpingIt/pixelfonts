// Font: advocut
// 419 bytes
byte fontHeight = 10;
byte fontBytes[] = {190,24,0,128,1,4,136,34,1,3,76,76,54,25,25,194,224,96,8,27,146,176,1,138,1,124,8,34,8,31,20,32,64,1,4,56,64,0,24,4,16,0,2,12,14,6,240,33,136,32,124,16,224,15,49,162,72,194,8,17,130,72,194,134,7,16,240,3,129,23,146,72,34,6,31,146,72,66,134,0,242,40,96,0,27,146,72,194,6,19,146,72,194,7,40,160,65,128,2,17,130,160,128,130,32,68,160,0,1,1,162,72,192,0,31,130,232,162,10,15,252,136,32,2,63,254,72,34,9,27,124,8,34,8,17,254,8,34,8,31,254,72,34,137,32,254,72,32,129,0,124,8,34,10,25,254,64,0,129,63,130,248,35,8,16,130,248,33,128,63,40,16,33,136,63,128,0,2,136,63,4,32,64,128,63,254,16,128,128,63,124,8,34,8,31,254,72,32,1,3,124,8,34,10,31,128,248,35,129,4,236,48,33,137,36,100,8,32,128,63,2,8,224,7,32,128,248,225,7,32,64,248,224,7,32,64,0,226,135,59,16,64,224,142,1,8,192,131,128,1,226,72,34,137,35,254,8,98,0,14,192,8,226,143,0,1,4,32,0,32,128,0,2,8,32,1,8,0,4,42,168,192,227,15,34,136,192,1,7,34,136,64,1,7,34,136,248,3,7,42,168,192,128,0,63,10,192,129,40,162,240,249,131,0,2,240,232,3,160,126,254,128,0,5,34,126,0,130,15,2,240,32,0,15,62,8,32,0,15,28,136,32,2,7,254,136,32,2,7,28,136,32,130,63,2,240,32,128,1,36,168,160,130,4,2,126,32,130,7,32,128,224,129,7,32,64,224,128,7,32,64,0,130,7,54,32,128,128,13,30,128,2,138,31,50,168,160,130,9,1,162,72,192,0,1,162,72,192,0,1,162,72,192,0,1,162,72,192,0};
int fontOffsets[] = {0,10,40,80,120,150,190,200,220,240,270,300,310,330,340,370,410,430,470,510,550,590,630,670,710,750,760,770,810,830,870,910,960,1000,1040,1080,1120,1160,1200,1240,1280,1310,1350,1390,1430,1480,1520,1560,1600,1650,1690,1730,1780,1820,1860,1910,1950,2000,2040,2060,2090,2110,2150,2200,2220,2260,2300,2340,2380,2420,2450,2490,2530,2540,2560,2600,2620,2670,2710,2750,2790,2830,2870,2910,2940,2980,3020,3070,3110,3150,3190,3230,3270,3310,3350};

int numSides = 4; //the number of sides in the tilt switch
int tiltPins[] = {8,7,5,6}; //listed in clockwise direction
int numLights = 10;
int povPins[] = {9,10,11,12,13,14,15,16,17,18};

int columnDelay = 2;
int spaceCols = 3;

String message = "@ShrimpingIt!";

void drawMessage(){
  
  for(int letterPos = 0; letterPos < message.length(); letterPos++){

    //use ascii charcode to index font information starting with 33 = first printable char
    int fontPos = int(message.charAt(letterPos)) - 33;
    
    //draw space or letter
    if(fontPos == -1){
      //it's a space
      drawSpace();
    }
    else{
      //it's a printable character. draw it
      drawCharacter(fontPos);
    }
    
    //draw a space between each character of message
    drawBlankCol();
  }  
}

void drawBlankCol(){
    for(int row = 0; row < fontHeight; row++){
      digitalWrite(povPins[row],LOW);
    }
    delay(columnDelay);
}

void drawSpace(){
  for(int col = 0; col < spaceCols; col++){
    drawBlankCol();
  }
}

void drawCharacter(int fontPos){
  int startBit = fontOffsets[fontPos]; //first bit of character
  int endBit = fontOffsets[fontPos + 1]; //start of next char (or dummy char at end)
  
  for(int colStart = startBit; colStart < endBit; colStart += fontHeight){
    for(int row = 0; row < fontHeight; row++){
      //calculate the bit position
      int bitPos = colStart + row;
      int byteOffset = bitPos / 8;
      int bitOffset = bitPos % 8;
      //read the individual bit
      if(((fontBytes[byteOffset]) & (1 << bitOffset)) == 0){ 
        digitalWrite(povPins[row], LOW);
      }
      else{
        digitalWrite(povPins[row], HIGH);
      }
    }
    delay(columnDelay); //wait between columns
  }
}


/** Configure input and output pins corresponding with each tilt direction to begin. */
void setup(){
  
  for(int side = 0; side < numSides; side++){    
    //configure tilt pins as input and set high resistance
    pinMode(tiltPins[side],INPUT);
    digitalWrite(tiltPins[side],LOW);
  }
  
  for(int light = 0; light < numLights; light++){
    //configure lights as output and turn off
    pinMode(povPins[light], OUTPUT);
    digitalWrite(povPins[light],LOW);
  }
  
}

/** visit each side in turn to check if it's connected
* and display each direction using different LEDs
*/
void loop(){
  int tilt = getTilt();
  drawMessage();
}

int getTilt(){
 for(int side = 0; side < numSides; side++){
   if(checkTilted(side)){
     return side;
   }
 } 
 return -1;
}

/** Configures each pin in turn as a button (with pull-up resistor) and a neighbour as ground.
* Then checks to see if the 'button' is closed (e.g. the coin is connecting the pins).
*/
boolean checkTilted(int focusPos){
  
  int focusPin = tiltPins[focusPos];
  
  //configure lower neighbour as high resistance (Z)
  int lowerPin = tiltPins[wrap(focusPos-1,numSides)]; //the previous pin (neighbour in anti-clockwise direction) 
  pinMode(lowerPin,INPUT);
  digitalWrite(lowerPin,LOW);

  //configure upper neighbour as output to ground (sink)
  int upperPin = tiltPins[wrap(focusPos+1,numSides)]; //the next pin (neighbour in clockwise direction)
  pinMode(upperPin, OUTPUT);
  digitalWrite(upperPin,LOW);

  //configure focus pin as input with pull-up resistor
  pinMode(focusPin, INPUT);
  digitalWrite(focusPin, HIGH);
  
  return digitalRead(focusPin) == LOW;
  
}

/** Keeps a value within bounds when overflowing or underflowing, 
* so that incrementing or decrementing a number stays in bounds. 
* (workaround for the fact that % in C isn't a mathematical modulus and behaves weird with negative numbers) */
int wrap(int value, int upperBound){
  return value < 0 ? ((value % upperBound) + upperBound) % upperBound : value % upperBound;
}
