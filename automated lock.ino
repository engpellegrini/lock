#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

//PINS
#define LOCK 12
#define LINE_1 11
#define LINE_2 10
#define LINE_3 9
#define LINE_4 8
#define COLUMN_1 7
#define COLUMN_2 6
#define COLUMN_3 5
#define COLUMN_4 4
#define BUZZER 3
#define SERVO 2
//OP_MODES
#define DEFAULT_MODE 1
#define CURRENT_PASSWORD 2
#define NEW_PASSWORD 3
#define CONFIRM_NEW_PASSWORD 4

String entry;
String new_password;
int cursor_position;
bool locked; // 1 = aberto / 0 = fechado
int op_mode;

LiquidCrystal screen(A0,A1,A2,A3,A4,A5);
Servo servo;

void SETUP_SERVO()
{
  servo.attach(SERVO);
  servo.write(0);
  locked = true;
}

void SETUP_KEYBOARD()
{
  pinMode(LOCK, INPUT_PULLUP);
  pinMode(COLUMN_1,INPUT_PULLUP);
  pinMode(COLUMN_2,INPUT_PULLUP);
  pinMode(COLUMN_3,INPUT_PULLUP);
  pinMode(COLUMN_4,INPUT_PULLUP);
  pinMode(LINE_1,OUTPUT);
  pinMode(LINE_2,OUTPUT);
  pinMode(LINE_3,OUTPUT);
  pinMode(LINE_4,OUTPUT);

  digitalWrite(LOCK,HIGH);
  digitalWrite(COLUMN_1,HIGH);
  digitalWrite(COLUMN_2,HIGH); 
  digitalWrite(COLUMN_3,HIGH);
  digitalWrite(COLUMN_4,HIGH);
}

void SEND_COMMAND(char key_pressed)
{
  switch(key_pressed)
  {
    case '1': case '2': case '3': case'4': case '5': case'6': case '7': case '8': case'9': case '0':
      entry = entry + key_pressed;
      PLAY_DEFAULT_SOUND();
      screen.setCursor(cursor_position,1);
      screen.print('*');
      cursor_position ++ ;
      break;
    case '*':
      PLAY_DEFAULT_SOUND();
      CLEAR_ENTRY();
      break;
    case '#':
      if (op_mode == DEFAULT_MODE)
      {
        if (COMPLETE_LENGTH(entry) == READ_PASSWORD() and locked == true)
        {
          PLAY_OPEN_SOUND();
          servo.write(90);
          locked = false;
        }
        else
        {
          PLAY_ERROR_SOUND();
        }
        CLEAR_ENTRY();
        break;
      }
      
      if (op_mode == CURRENT_PASSWORD)
      {
        if(COMPLETE_LENGTH(entry) == READ_PASSWORD())
        {
          PLAY_DEFAULT_SOUND();
          CHANGE_OP_MODE(NEW_PASSWORD);
          break;       
        }
        else
        {
          screen.clear();
          screen.setCursor(0,0);
          screen.print("Senha incorreta");
          PLAY_ERROR_SOUND();
          delay(3000);
          CHANGE_OP_MODE(CURRENT_PASSWORD);
          break;
        }
      }
      
      if (op_mode == NEW_PASSWORD)
      {
        if (entry.length() < 4)
        {
          screen.clear();
          screen.setCursor(0,0);
          screen.print("A senha digitada");
          screen.setCursor(0,1);
          screen.print("e muito curta");
          PLAY_ERROR_SOUND();
          delay(3000);
          CHANGE_OP_MODE(NEW_PASSWORD);
          break;
        }
        else
        {
          if (entry.length() > 12)
          {
            screen.clear();
            screen.setCursor(0,0);
            screen.print("A senha digitada");
            screen.setCursor(0,1);
            screen.print("e muito longa");
            PLAY_ERROR_SOUND();
            delay(3000);
            CHANGE_OP_MODE(NEW_PASSWORD);
            break;
          }
          else
          {
          PLAY_DEFAULT_SOUND();
          new_password = entry;
          CHANGE_OP_MODE(CONFIRM_NEW_PASSWORD); 
          break;       
          }
        }    
      }
      
      if (op_mode == CONFIRM_NEW_PASSWORD)
      {
        if (entry == new_password)
        {
            SAVE_PASSWORD(COMPLETE_LENGTH(entry));
            PLAY_DEFAULT_SOUND();
            screen.clear();
            screen.setCursor(0,0);
            screen.print("Senha alterada");
            screen.setCursor(0,1);
            screen.print("com sucesso!");
            delay(3000);
            CHANGE_OP_MODE(DEFAULT_MODE);
            break;
        }
        else
        {
          screen.clear();
          screen.setCursor(0,0);
          screen.print("Senhas digitadas");
          screen.setCursor(0,1);
          screen.print("nao conferem");
          PLAY_ERROR_SOUND();
          delay(3000);
          CHANGE_OP_MODE(DEFAULT_MODE);
          break;
        }
        
      }
      break; 
    case 'L': case 'D':
      if (locked == false)
      {
        PLAY_LOCK_SOUND();
        servo.write(0);
        locked = true;
        CHANGE_OP_MODE(DEFAULT_MODE);
      }
    break;
    case 'A':
      PLAY_DEFAULT_SOUND();
      CHANGE_OP_MODE(CURRENT_PASSWORD);
      break;
    case 'C':
      PLAY_DEFAULT_SOUND();
      CHANGE_OP_MODE(DEFAULT_MODE);
      break;
    }
  
}

void CHANGE_OP_MODE(int OPERACAO)
{
  op_mode = OPERACAO;
  entry = "";
  cursor_position = 0;
  
  screen.clear();
  screen.setCursor(0,0);
  
  if (op_mode == CURRENT_PASSWORD)
  {
    screen.print("Senha atual:");
  }
  if (op_mode == NEW_PASSWORD)
  {
    screen.print("Nova senha:");
  }
  if (op_mode == CONFIRM_NEW_PASSWORD)
  {
    screen.print("Confirmar senha:");
  }
  if (op_mode == DEFAULT_MODE)
  {
    screen.print("Digite a senha:");
  }
  
  screen.setCursor(0,1);
  
}

void setup() {
  // put your setup code here, to run once:
  SETUP_KEYBOARD();
  SETUP_SERVO();
  
  screen.begin(16,2);

  Serial.begin(9600);
  
  CHANGE_OP_MODE(DEFAULT_MODE);
}

void loop() 
{
  for (int line = 1; line < 5 ; line ++)
  { 

    digitalWrite(LINE_1,HIGH);
    digitalWrite(LINE_2,HIGH); 
    digitalWrite(LINE_3,HIGH);
    digitalWrite(LINE_4,HIGH);
    digitalWrite(line + 7,LOW); //soma-se 7 pois a porta referente a linha 1 é a porta 8, linha 2, 9 e assim por diante.

    if (digitalRead(COLUMN_1) == LOW)
    {
     SEND_COMMAND(KEYPRESSED(line,1));
      while (digitalRead(COLUMN_1) == LOW) {};
    }
    if (digitalRead(COLUMN_2) == LOW)
    {
      SEND_COMMAND(KEYPRESSED(line,2));
      while (digitalRead(COLUMN_2) == LOW) {};
    } 
    if (digitalRead(COLUMN_3) == LOW)
    {
      SEND_COMMAND(KEYPRESSED(line,3));
      while (digitalRead(COLUMN_3) == LOW) {};
    } 
    if (digitalRead(COLUMN_4) == LOW)
    {
      SEND_COMMAND(KEYPRESSED(line,4));
      while (digitalRead(COLUMN_4) == LOW) {};
    }
    if (digitalRead(LOCK) == LOW)
    {
      SEND_COMMAND('L');
      while (digitalRead(LOCK) == LOW) {};
    }
    delay(25); 
} 


}

char KEYPRESSED(int line, int column)
{
  switch (line)
  {
    case 4:
      switch(column)
      {
        case 1:
          return '1';        
          break;
        case 2:
          return '2';          
          break;
        case 3:
          return '3'; 
          break;
        case 4:
          return 'A';
          break;
      }
      break;
    case 3:
      switch(column)
      {
        case 1:
          return '4'; 
        case 2:
          return '5';
        case 3:
          return '6';  
          break;
        case 4:
          return 'B';
          break;
      }
      break;
    case 2:
      switch(column)
      {
        case 1:
          return '7';  
          break;
        case 2:
          return '8';  
          break;
        case 3:
          return '9';  
          break;
        case 4:
          return 'C';
          break;
      }
      break;
    case 1:
      switch(column)
      {
        case 1:
          return '*';
          break;
        case 2:
          return '0'; 
          break;
        case 3:
        return '#';
          break;
        case 4:
          return 'D';
          break;
      }
      break;
  }
}

void PLAY_OPEN_SOUND()
{
  tone(BUZZER,2637,100);
  delay(100);
  tone(BUZZER,2637,100);
  delay(100);
  tone(BUZZER,0,100);
  delay(100);
  tone(BUZZER,2637,100);
  delay(100);
  
  tone(BUZZER,0,100);
  delay(100);
  tone(BUZZER,2093,100);
  delay(100);
  tone(BUZZER,2637,100);
  delay(100);
  tone(BUZZER,0,100);
  delay(100);
  
  tone(BUZZER,3136,100);
  delay(100);
}

void PLAY_ERROR_SOUND()
{
   tone(BUZZER,400,200);
   delay(300);
   tone(BUZZER,400,200);
}

void PLAY_LOCK_SOUND()
{
  tone(BUZZER,200,500);
}

void PLAY_DEFAULT_SOUND()
{
  tone(BUZZER,4000,100);
}

void CLEAR_ENTRY()
{
  entry = "";
  cursor_position = 0;
  screen.setCursor(0,1);
  screen.print("                ");
}

String COMPLETE_LENGTH(String str)
{
  for (int i=1; i<=16; i++)
  {
    if (str.length() == 16)
    {
      break;
    }
    str = str + '0';
  }
  return str;
}

String READ_PASSWORD()
{
  String password;
  for (int address=0;address<16;address++)
  {
    char value = EEPROM.read(address);
    password = password + value;
  }
  Serial.println(password);
  return password;
}

void SAVE_PASSWORD(String password)
{
  for (int i = 0 ; i < password.length(); i++)
  {
    EEPROM.write(i,password[i]);
    Serial.println(EEPROM.read(i));
  }
}

