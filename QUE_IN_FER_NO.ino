/*##########################################################################
################### INCLUDES AND DEFINES ###################################*/
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <uRTCLib.h> // RTC lib

  #define TIMEOUT_COUNT 1000 // aumentei pra 3000 pq tava dando ruim

  // SOM +++++++++++++++++++++++++++++++++++++++++++++
    #include <SoftwareSerial.h>
    #include "RedMP3.h" //MP3 maroto
    #define volume_Pot A0
    #define ARDUINO_RX 7 //mp3
    #define ARDUINO_TX 8 //mp3
    
  // END SOM++++++++++++++++++++++++++++++++++++++++++

/*################### END INCLUDES AND DEFINES ###############################
############################################################################*/

/*############################################################################
################### VARIABLES OR FIXED #######################################*/
  // RTC ======================================================================

  uRTCLib rtc(0x68); //a4 e a5 pinos do rtc

  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


  // LCD =======================================================================

  LiquidCrystal_I2C lcd(0x27, 16, 4);

  // ENCODER ===================================================================

  int switchPin = 13;  // button pin KEY
  int click = HIGH; // button value

  int pinA = 12;    // Rotary encoder Pin S1
  int pinB = 11;    // Rotary encoder Pin S2
  int pinAstateCurrent = LOW;   // Current state of Pin S1
  int pinAStateLast = pinAstateCurrent;   // Last read value of Pin S2

  // SOM ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MP3 mp3(ARDUINO_RX, ARDUINO_TX); //MP3
    int8_t musica1 = 0x01; //primeira música do cartão SD
    int8_t musica2 = 0x02; //segunda música do cartão SD
    int8_t musica3 = 0x03; //terceira música do cartão SD
    int8_t folderName = 0x01; //pasta do cartão
    int8_t lastVolume; //volume atual
  // END SOM+++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // AUX ========================================================================

  int st=0; //setting state 0 - main screen state

  bool refresh_screen=true; // impede q fique printando a tela 900x até ir pra proxima (NÃO FICA PISCANDO)

  int encoder = 0;

  int line=0;

  int timer = 0; // pra contar o num de ciclos até voltar pra tela inicial

/*################### END VARIABLES OR FIXED ####################################
#################################################################################*/

/*############################################################################
############################### SETUP #######################################*/

  void setup() {
    
    Serial.begin(9600);   // Initialise the serial monitor

    // ENCODER
    pinMode(switchPin, INPUT_PULLUP);   // Enable the switchPin as input with a PULLUP resistor
    pinMode(pinA, INPUT);   // Set PinA as input
    pinMode(pinB, INPUT);   // Set PinB as input

    // LCD
    lcd.init();       // Initialize the LCD
    lcd.setBacklight(HIGH); // Turn on the backlight

    // SOM ++++++++++++++++++++++++++++++++++++
      mp3.setVolume(lastVolume); //selecionar o volume
    //END SOM +++++++++++++++++++++++++++++++++

    //RTC
    URTCLIB_WIRE.begin();
    
    // Comment out below line once you set the date & time.
  // Following line sets the RTC with an explicit date & time
  // for example to set January 13 2022 at 12:56 you would call:
  //rtc.set(0, 26, 15, 7, 27, 5, 23);
  // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  // set day of week (1=Sunday, 7=Saturday)
  }

  

/*########################### END SETUP #####################################
##############################################################################*/

/*###################################################################################
############################### AUX FUNCTIONS #######################################*/
  // ENCODER ==============================================================
    // Read and interpret rotary encoder inputs because it wans't working on the other thing
    int read_encoder() {
      int movement = 0; //
      int pinAstateCurrent = digitalRead(pinA);   // Read the current state of pinA

      // If the encoder knob was turned
      if ((pinAStateLast == LOW) && (pinAstateCurrent == HIGH)) {   // Check the direction of the turn
        if (digitalRead(pinB) == HIGH) {
          movement = 1; // direção do relogiooo
        } else {
          movement = -1; //oposto do relogiooo
        }
      }

      pinAStateLast = pinAstateCurrent; //guarda o ultimo estado do pinA pra comparar depois
      return movement; //retorna o movimento binito 
    }

    //nova função de navegação do encoder
    int Cursor_nav() {
      static int aux = 0;   //onde o encoder tá 
      static int aux_last = -1; // will store the last position of aux, I should write everything in one language

      click = digitalRead(switchPin); // Read the digital value of the switch (LOW/HIGH)
      // If the switch is pressed (LOW), return current line
      if (click == LOW) {
        return aux;
      }

      // Read encoder
      int encoder_move = read_encoder(); //determines if the rotary encoder has moved

      if (encoder_move == 1) {
        aux++;
        if (aux > 3) aux = 0;
      } 
      else if (encoder_move == -1) {
        aux--;
        if (aux < 0) aux = 3; //volta pra primeira linha se atingir o 3
      }

      // Only print cursor if it moved
      if (aux != aux_last) {
        lcd.setCursor(0, 0);  // Clear all possible cursor positions
        lcd.print("  ");
        lcd.setCursor(0, 1);
        lcd.print("  ");
        lcd.setCursor(0, 2);
        lcd.print("  ");
        lcd.setCursor(0, 3);
        lcd.print("  ");

        lcd.setCursor(0, aux);
        lcd.print("->");

        // Save the current cursor position as the last position
        aux_last = aux;
      }

      return aux;
    }
  // END ENCODER

  // SOM 
    int encoderChangeVolume(int volume){ //ajustar volume
      int encoder_move = read_encoder(); //determines if the rotary encoder has moved
        if (encoder_move == 1) {
          volume++;
          } 
        else if (encoder_move == -1) {
          volume--;
        }

        volume = constrain(volume, 0, 30);

      // Only print cursor if it moved
      if (volume != lastVolume) {
        mp3.setVolume(volume);
        lastVolume = volume;
      }
      return volume;
     }
  // END SOM
/*############################## END AUX FUNCTIONS #######################################
###########################################################################################*/

/*#################################################################################
############################## LOOP ###########################################*/
  void loop() {

    // ENCODER
    static int click_last = HIGH;
    click = digitalRead(switchPin);
  

    // RTC
    rtc.refresh();
    

    // SWITCH SCREENS
    switch(st){
      case 0: // main screen 
        if (refresh_screen) {
          Home_screen();
          refresh_screen=false;   // Only refresh the screen once per state change
        }
        if (click==LOW){    // If button was pressed, transition to the settings screen
          st=1;   
          refresh_screen=true;
          delay(200);
        }
        timer = 0;      
        break;

      case 1: // settings screen
        if (refresh_screen) {
          Settings_screen();        
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==0){    // If button was pressed while 'Treat' was selected, transition to the treat screen
          st=18;
          refresh_screen=true;
          delay(200);
        } 
        if(click==LOW && line==1){    // schedule
          st=2;
          refresh_screen=true;
          delay(200);
        } 
        if (click == LOW && line == 2) { //calibration
          st = 11;
          refresh_screen = true;
          delay(200);
        }
        if (click == LOW && line == 3) { // Sound
          st = 15;
          refresh_screen = true;
          delay(200);
        }
              
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        // delay(200);
        }
        
        break;

      case 2: // schedule screen 
        if (refresh_screen) {
          Schedule_screen();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==3){
          st=1;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==1){
          st=3;
          refresh_screen=true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;      
        
        case 3: // schedule options
        if (refresh_screen) {
          Schedule_options();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==1){
          st=4;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==2){
          st=5;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==3){
          st=2;
          refresh_screen=true;
          delay(200);
        }
        if (click == LOW && line == 0) { //Remove
          st = 9;
          refresh_screen = true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;
        case 4: // time set
        if (refresh_screen) {
          Schedule_time_set();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==0){
          st=6;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==1){
          st=7;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==2){
          st=8;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==3){
          st=3;
          refresh_screen=true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;

        case 5: // new time set
        if (refresh_screen) {
          Schedule_newtime_set();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==0){
          st=6;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==1){
          st=7;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==2){
          st=8;
          refresh_screen=true;
          delay(200);
        }
        if(click==LOW && line==3){
          st=3;
          refresh_screen=true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;
        
        case 6: // time set
        if (refresh_screen) {
          Time_set();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==3){
          st=4;
          refresh_screen=true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;

        case 7: // portion set
        if (refresh_screen) {
          Portion_set();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==3){
          st=4;
          refresh_screen=true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;

        case 8: // slow mode
        if (refresh_screen) {
          Slow_Mode();
          refresh_screen=false;
          timer = 0;
        }
        line = Cursor_nav();
        
        if(click==LOW && line==3){
          st=4;
          refresh_screen=true;
          delay(200);
        }
        else{
          if(timer>=TIMEOUT_COUNT){
            st = 0;
            refresh_screen=true;
          }
        }
        break;
        case 9:  // schedule remove
        if (refresh_screen) {
          Schedule_remove();
          refresh_screen = false;
          timer = 0;
        }
        line = Cursor_nav();

        if (click == LOW && line == 3) {
          st = 3;
          refresh_screen = true;
          delay(200);
        }
        if (click == LOW && line == 1 || click == LOW && line == 2) {
          st = 10;
          refresh_screen = true;
          delay(200);
        } else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 10:  // Not found
        if (refresh_screen) {
          Not_found();
          refresh_screen = false;
          timer = 0;
        }

        if (click == LOW) {
          st = 9;
          refresh_screen = true;
          delay(200);
        } else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 11:  // CALIBRATE
        if (refresh_screen) {
          Calibrate();
          refresh_screen = false;
          timer = 0;
        }
        line = Cursor_nav();

        if (click == LOW && line == 3) {
          st = 1;
          refresh_screen = true;
          delay(200);
        }
        if (click == LOW && line == 2) {
          st = 12;
          refresh_screen = true;
          delay(200);
        } else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 12:  // Calibrate_Droppingfood
        if (refresh_screen) {
          Calibrate_DF();
          refresh_screen = false;
          timer = 0;
        }

        if (click == LOW) {
          st = 13;
          refresh_screen = true;
          delay(200);
        } else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

    case 13:  // Calibrate_TakeFoodOut
        if (refresh_screen) {
          Calibrate_TFO();
          refresh_screen = false;
          timer = 0;
        }
      
        if (click == LOW) {
          st = 14;
          refresh_screen = true;
          delay(200);
        } else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 14:  // CalibrateD
        if (refresh_screen) {
          CalibrateD();
          refresh_screen = false;
          timer = 0;
        }
      
        if (click == LOW) {
          st = 0;
          refresh_screen = true;
          delay(200);
        } else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

        case 15:  // Sound
        if (refresh_screen) {
          Sound();
          refresh_screen = false;
          timer = 0;
        }
        line = Cursor_nav();

        if (click == LOW && line == 1) {
          st = 16;
          refresh_screen = true;
          delay(200);
        }
        if (click == LOW && line == 2) {
          st = 17;
          refresh_screen = true;
          delay(200);
        }
        if (click == LOW && line == 3) {
          st = 1;
          refresh_screen = true;
          delay(200);
        } 
        else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 16:  // Sound_choose
        if (refresh_screen) {
          Sound_choose();
          refresh_screen = false;
          timer = 0;
        }
        line = Cursor_nav();
        if (click == LOW && line == 1) {
          mp3.playWithVolume(musica1, lastVolume);
          refresh_screen = true;
          delay(200);
        } 
        if (click == LOW && line == 2) {
          mp3.playWithVolume(musica2, lastVolume);
          refresh_screen = true;
          delay(200);
        } 
        if (click == LOW && line == 3) {
          st = 15;
          refresh_screen = true;
          mp3.stopPlay();
          delay(200);
        } 
        else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 17:  // Sound_volume
        if (refresh_screen) {
          Sound_volume();
          refresh_screen = false;
          timer = 0;
          delay(20);
        }      
        
        
        if (click == LOW) {
          st = 15;
          refresh_screen = true;
          delay(200);
        } 
        else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;

      case 18:  // Treat
        if (refresh_screen) {
          Treat();
          refresh_screen = false;
          timer = 0;
        }

        if (click == LOW) {
          st = 0;
          refresh_screen = true;
          delay(200);
        } 

        else {
          if (timer >= TIMEOUT_COUNT) {
            st = 0;
            refresh_screen = true;
          }
        }
        break;
    }

    delay(15);
    timer++;
  }
/*######################### END LOOP ###############################################
##################################################################################*/


/*################################################################################
############################ TELAS #################################################*/

  void Home_screen() {
    
    
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(rtc.day()); lcd.print("/"); lcd.print(rtc.month());lcd.print("/");lcd.print(rtc.year());
    // lcd.setCursor(17, 0);
    // lcd.print(rtc.dayOfWeek()-1);
    lcd.setCursor(7, 1);
    lcd.print(rtc.hour());lcd.print(":");lcd.print(rtc.minute());
    lcd.setCursor(2, 3);
    lcd.print("Next meal: 16:30");
    
  }

  void Settings_screen() {
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Treat");
    lcd.setCursor(2,1);
    lcd.print("Schedule");
    lcd.setCursor(2, 2);
    lcd.print("Calibrate");
    lcd.setCursor(2, 3);
    lcd.print("Sound");
  }

  void Schedule_screen() {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Schedule: ");
  
    lcd.setCursor(2, 1);
    lcd.print("08:00");
  
    lcd.setCursor(2, 2);
    lcd.print("12:00");

    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Schedule_options() {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Remove");
  
    lcd.setCursor(2, 1);
    lcd.print("Edit");
  
    lcd.setCursor(2, 2);
    lcd.print("New");

    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Schedule_time_set() {
    lcd.clear();
    lcd.setCursor(12, 0);
    lcd.print("New");
  
    lcd.setCursor(2, 0);
    lcd.print("Time:");

    lcd.setCursor(12, 0);
    lcd.print("8:00");
  
    lcd.setCursor(2,1);
    lcd.print("Portion");

    lcd.setCursor(12, 1);
    lcd.print("150 g");

    lcd.setCursor(2,2);
    lcd.print("Slow");

    lcd.setCursor(12, 2);
    lcd.print("On");

    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Schedule_newtime_set() {
    lcd.clear();
    lcd.setCursor(12, 0);
    lcd.print("Edit");
  
    lcd.setCursor(2, 0);
    lcd.print("Time:");

    lcd.setCursor(12, 0);
    lcd.print("8:00");
  
    lcd.setCursor(2,1);
    lcd.print("Portion");

    lcd.setCursor(12, 1);
    lcd.print("150 g");

    lcd.setCursor(2,2);
    lcd.print("Slow");

    lcd.setCursor(12, 2);
    lcd.print("On");

    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Time_set() {
    lcd.clear();
    
    lcd.setCursor(7, 0);
    lcd.print("Time");
  
    lcd.setCursor(2,2);
    lcd.print("8:00");

    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Portion_set() {
    lcd.clear();
    
    lcd.setCursor(5, 0);
    lcd.print("Portion");
  
    lcd.setCursor(2,2);
    lcd.print("150 g");

    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Slow_Mode() {
    lcd.clear();
    
    lcd.setCursor(5, 0);
    lcd.print("Slow Mode");
  
    lcd.setCursor(2,1);
    lcd.print("On");

    lcd.setCursor(2,2);
    lcd.print("Off");

    lcd.setCursor(2,3);
    lcd.print("Back");
  }

  void Schedule_remove(void) {
    lcd.clear();
    lcd.setCursor(4, 0);  // HEADER ---------------------------------
    lcd.print("Remove ");
    lcd.print("08:00");  //aqui vai a variavel
    lcd.print(" ?");
    lcd.setCursor(2, 1);  //yes
    lcd.print("Yes");
    lcd.setCursor(2, 2);  // No
    lcd.print("No");
    lcd.setCursor(2, 3);  // RETURN ---------------------------------
    lcd.print("Back");
  }

  void Not_found(void) {

    lcd.clear();
    lcd.setCursor(8, 1);  // HEADER ---------------------------------
    lcd.print("o_o");
    lcd.setCursor(5, 2);  //yes
    lcd.print("Not Found");
  }

  void Calibrate(void) {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Callibrate:");
    lcd.setCursor(2, 2);
    lcd.print("Start");
    lcd.setCursor(2, 3);
    lcd.print("Back");
  }

  void Calibrate_DF(void) {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Dropping food...");
    lcd.setCursor(7, 3);
    lcd.print("wait");
  }

  void Calibrate_TFO(void) {
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("Take the food out!");
  }

  void CalibrateD(void) {
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("CALIBRATED!");
    lcd.setCursor(7, 2);
    lcd.print(";)");
  }

  void Sound(void) {
    lcd.clear();
    lcd.setCursor(7, 0);  // HEADER ---------------------------------
    lcd.print("Sound ");
    lcd.setCursor(2, 1);  
    lcd.print("Choose sound");
    lcd.setCursor(2, 2);  
    lcd.print("Volume");
    lcd.setCursor(2, 3);  // RETURN ---------------------------------
    lcd.print("Back");
  }

  void Sound_choose(void) {
    lcd.clear();
    lcd.setCursor(5, 0);  // HEADER ---------------------------------
    lcd.print("Choose Sound ");
    lcd.setCursor(2, 1);  
    lcd.print("Sound 1");
    lcd.setCursor(2, 2);  
    lcd.print("Sound 2");
    lcd.setCursor(2, 3);  // RETURN ---------------------------------
    lcd.print("Back");
  }

  void Sound_volume() {
    lcd.clear();
    lcd.setCursor(5, 0);  // HEADER ---------------------------------
    lcd.print("Volume");
    int v = encoderChangeVolume(lastVolume);
    lcd.setCursor(7, 2);  // HEADER ---------------------------------
    lcd.print(v);
    lcd.setCursor(2, 3);  // RETURN ---------------------------------
    lcd.print("Back");
  }

  void Treat(){
    lcd.clear();
    lcd.setCursor(6, 0); 
    lcd.print("D   D");
    lcd.setCursor(5, 1); 
    lcd.print("  o.o ");
    lcd.setCursor(6, 2); 
    lcd.print("  w ");
  }

/*######################### END TELAS ###############################################
##################################################################################*/

