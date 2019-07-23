/*
arduino-theb
General purpose arduino bot with defense capabilities (in development) 
@author thebrentc@gmail.com
*/
 
// Pin 13 has an LED connected on most Arduino boards.
int led = 13;
int button = 2;
int pir = 3;
int led2 = 12;
int speaker = 9;

// set states and timings
// consts
int OFF = 0;
int ON = 1; 
int ALARM = 2;
// global state
int state = OFF;
int statechange = -1;
// component states 
int led_state = LOW;
//int led_change = 500; 
//long led_timer = 0;
int led2_state = LOW;
int led2_change = 400;
long led2_timer = 0;
int speaker_state = OFF;
int speaker_change = 500;
long speaker_timer = 0;
long alarm_change = 30000;
long alarm_timer = 0;

// sound variables and samples
// ref http://ardx.org/src/circ/CIRC06-code.txt
char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
int sound = tones[0];
int sound_duration = 300;

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speaker, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speaker, LOW);
    delayMicroseconds(tone);
  }
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);  
  Serial.println("Hello world");
  
  // initialize the components
  //pinMode(button, INPUT); // not needed?
  pinMode(pir, INPUT);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(speaker, OUTPUT);

  // initialise timings
  set_timings();
}

void set_timings() {
  //led_timer = 0;
  led2_timer = millis();
  speaker_timer = millis();
}

// the loop routine runs over and over again forever:
void loop() {

  int buttonState = digitalRead(button);  
  if (buttonState == LOW) { // LOW || HIGH <- the way the button is connected
      Serial.write("Button\n ");      
      state++; 
      if (state > ALARM) { state = OFF; }
      statechange = state;
      Serial.write("Change state "); 
      Serial.write(state==OFF?"OFF":state==ON?"ON":state==ALARM?"ALARM":"Unknown"); 
      Serial.write("\n ");
      // reset timings
      set_timings();
      delay(300); // delay for clean button press      
  }

  if (state >= ON) {
    int motion = digitalRead(pir);  
    if (state == ON && motion == HIGH && buttonState == HIGH /* not pressed*/) { 
        Serial.println("Motion ");      
         state = ALARM;
         statechange = ALARM;               
    } else if (state == ALARM && motion == LOW && buttonState == HIGH /* not pressed*/ && millis() - alarm_timer >= alarm_change) {
         Serial.println("Motion ended + alarm expired");
         state = ON;
         statechange = ON;   
    } 
  }
  
  // handle state change directly
  if (statechange > -1) {
    if (statechange == OFF) { 
      digitalWrite(led, LOW); 
      led2_state = LOW;
      digitalWrite(led2, led2_state);
      speaker_state == OFF;      
      digitalWrite(speaker, LOW);   
    } else if (statechange == ON) {      
      digitalWrite(led, HIGH); 
      alarm_timer = millis(); // delay arming alarm to avoid false positives
      led2_state = LOW;
      digitalWrite(led2, led2_state);
      speaker_state == OFF;        
    } else if (statechange == ALARM) {
      Serial.println("Alarm!");
      alarm_timer = millis();
      led2_state = HIGH;
      digitalWrite(led2, led2_state);
      speaker_state == ON;
      playTone(sound, sound_duration);
    }
    statechange = -1;        
  }

  // continue with timings
  if (state >= ON) {
    
   digitalWrite(led, HIGH);

   if (state == ALARM) {

     if (millis() - led2_timer >= led2_change) {
        //Serial.print(millis()); Serial.print(" ? "); Serial.print(led2_timer); Serial.print("\n ");
        led2_state = (led2_state == LOW)? HIGH : LOW;
        //Serial.print("led2 "); 
        //Serial.print(led2_state==LOW?"LOW":led2_state==HIGH?"HIGH":"Unknown"); 
        //Serial.print("\n ");     
        digitalWrite(led2, led2_state);
        led2_timer = millis();
      }
    
      if (millis() - speaker_timer >= speaker_change) {
        speaker_state = (speaker_state == OFF)? ON : OFF;
        //Serial.write("speaker  "); 
        //Serial.write(speaker_state==OFF?"OFF":speaker_state==ON?"ON":"Unknown"); 
        //Serial.write("\n ");
        if (speaker_state == ON) {
          playTone(sound, sound_duration); // TODO fix blocking
        }      
        speaker_timer = millis();
      }
   }
    
  } 
}
