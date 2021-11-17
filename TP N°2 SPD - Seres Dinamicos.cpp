#include <LiquidCrystal.h>
#include <Keypad.h>
#define BOTON_RESET 1
#define BOTON_TESTPASSWORD 2
#define BOTON_CONFIGURACION 3
#define NO_BOTON -1
#define VUELTAS 10
int demoraTemporal = 1000;
int espera = 30;
int flagIntentos = 2;
int FLAGRESET = 0;
int FLAGTEST = 0;
int FLAGCONFIG = 0;
int FLAGNEWPASS = 0;
int boton = A4;
int ledVerde = 10;
int ledRojo = 12;
int ledAmarillo = 11;
int colMove = 0;
int rowMove = 0;
int i = 0; 
int contador = 0;
int MAX_TECLAS = 7;
const byte rows = 4; //four rows
const byte cols = 4;//four columns
int demoraLed = 1000;
int demora = 500;
int estado = 1; 
char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[rows] = {7, 6, 5, 4}; //connect to the row pinouts of the keypad
byte colPins[cols] = {3, 2, 8, A5}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );
char key;
unsigned long int millisAntes = 0;
unsigned long int millisAntesLed = 0;


//-----------------------------------------------
LiquidCrystal lcd(9,13,A0,A1,A2,A3);
//-----------------------------------------------

String mensajeDefault= "PASSWORD:";
String mensajeFail= "PASSWORD FAIL";
String mensajeOk= "PASSWORD OK";
String mensajeNewPass= "SET NEW PASS:";
String mensajeOldPass= "ENTER OLD PASS:";
String password = "2C2021";
String passIngresado;
String passwordAux;

// C++ code
//
void setup() {  
  	pinMode(boton,INPUT);
  	pinMode(ledVerde,OUTPUT);
  	pinMode(ledRojo,OUTPUT);
  	pinMode(ledAmarillo,OUTPUT);
  	Serial.begin(9600);
  	lcd.begin(16,2);
}

void loop() {
    
    int opcion = leerBoton();
    // Se crea un switch con el valor ya mapeado del divisor de tension para saber que boton se pulso
    switch (opcion) { 
        case BOTON_RESET:
        FLAGRESET = 1;
        break;
        case BOTON_CONFIGURACION:
        ResetLockKey();
        FLAGCONFIG = 1;
        break;
        case BOTON_TESTPASSWORD:
        FLAGTEST = 1;
        break;
      case NO_BOTON:
      break;
    }  

    if(flagIntentos != 0 ) {
        key = keypad.getKey();  
        lcd.setCursor(0,0);

        if(FLAGCONFIG && FLAGNEWPASS == 0) {
            lcd.print(mensajeOldPass);   
        } else if(FLAGCONFIG == 0) {
            lcd.print(mensajeDefault); 
        }        
    }    
  
    if(key != NO_KEY && i < MAX_TECLAS) {        
        passIngresado += key;          
        lcd.setCursor(colMove,1);
        lcd.print(key);
        colMove++;
        i++;  
    }

    if(FLAGRESET) {
        ResetLockKey();
        resetFlags();
    }

    if(FLAGCONFIG) {
        
        if(password == passIngresado || passwordAux == password) {
            
            if(FLAGNEWPASS == 0) {
                passwordAux = password;
                ResetLockKey();
                lcd.setCursor(0,0);
                lcd.clear();
                lcd.print(mensajeNewPass);
            }

            FLAGNEWPASS = 1;

            if(i == MAX_TECLAS) { // Se fija si se ingresaron 7 caracteres y se guarda la contraseña nueva
                passwordAux = "";                    
                password = passIngresado;
                resetFlags();
            }

        } else if(password != passIngresado && i == MAX_TECLAS) {

            resetFlags();            
        }
    }

    if(contador != VUELTAS && FLAGCONFIG != 1) {       
        
        if(passIngresado.length() == MAX_TECLAS || FLAGTEST) {            
            digitalWrite(ledAmarillo,LOW);

            if(password == passIngresado && flagIntentos != 0) { // led verde, contraseña valida
                flagIntentos = 3; // los intento son dos, pero al pasar por led amarillo siempre decrementa 1
                
                if(millis()- millisAntes >= demora && estado == 1) {                
                    LedYMostrarMensaje(ledVerde,mensajeOk,1);
                    millisAntes = millis();
                    contador ++;
                    estado = 0;
                
                } else if(millis() - millisAntes >= demora && estado == 0) {                
                    LedYMostrarMensaje(ledVerde,mensajeOk,0);
                    millisAntes = millis();
                    contador ++;
                    estado = 1;
                }    
            
            } else if(password != passIngresado && flagIntentos != 0) {  // led rojo, contraseña invalida

                if(millis() - millisAntes >= demora && estado == 1) {
    
                    LedYMostrarMensaje(ledRojo,mensajeFail,1);
                    millisAntes = millis();
                    contador ++;
                    estado = 0;
                
                } else if(millis() - millisAntes >= demora && estado == 0) {
                
                    LedYMostrarMensaje(ledRojo,mensajeFail,0);
                    millisAntes = millis();
                    contador ++;
                    estado = 1;              	
                }          
            }  
        }
        
        if(flagIntentos == 0) {

            if(millis() - millisAntes >= demoraTemporal && espera != -1) {
                esperaIntentos();                
     			millisAntes = millis();
            }				
        }

    } else if(contador == VUELTAS && FLAGCONFIG != 1) {

        if((passIngresado.length() == MAX_TECLAS || FLAGTEST == 1)){
            Serial.println(flagIntentos);            
            ResetLockKey();

            if(flagIntentos != 0) {
                LedYMostrarMensaje(ledAmarillo,mensajeDefault,1);
                FLAGTEST = 0;
            }

        flagIntentos--;
        }
    }

    delay(15);
}   
    
/*
//Prende o apaga un led y muestra un mensaje en el display
//int pin: pin del led a cambiar estado
//String mensaje: mensaje a mostrar en display
//int estado: 1 si se debe prender, 0 si se debe apagar el led
*/
void LedYMostrarMensaje(int pin, String mensaje, int estado) {  
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print(mensaje);
	digitalWrite(pin,estado);  
}

//Reinicia la cerradura
void ResetLockKey() {   
    lcd.setCursor(0,0);
    lcd.clear();
    digitalWrite(ledRojo,LOW);  
    digitalWrite(ledVerde,LOW);  
    passIngresado = "";
    i = 0;
    contador = 0;
    colMove = 0;
}

//Lee el valor de la entrada analógica equivalente a la tensión de los botones y devuelve 1,2 o 3 dependiendo del boton
int leerBoton() {
    int retorno = NO_BOTON;
    int fBoton = analogRead(boton);

    if(fBoton >= 505 && fBoton <= 520) {
        retorno = BOTON_RESET;
    }  
        
    if(fBoton >= 675 && fBoton <= 690) {
        retorno = BOTON_TESTPASSWORD;
    }  

    if(fBoton >= 755 && fBoton <= 775) {
        retorno = BOTON_CONFIGURACION;
    }  
    return retorno;
}

//Reinicia las banderas
void resetFlags() {
    FLAGNEWPASS = 0;
    FLAGCONFIG = 0;
    FLAGTEST = 0;
    FLAGRESET = 0;
}

//Al cabo de 3 intentos incorrectos de contraseña, muestra un mensaje de REINTENTE EN N° SEGUNDOS y reinicia la cerradura
void esperaIntentos() {
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print(" REINTENTE EN ");
    lcd.print(espera);
    lcd.setCursor(1,1);
    lcd.print(" SEGUNDOS ");
    
    if(espera %2 == 0) {
        digitalWrite(ledRojo,LOW);  
        digitalWrite(ledVerde,HIGH);  
        lcd.print("o_o");    
    } else {
        digitalWrite(ledRojo,HIGH);  
        digitalWrite(ledVerde,LOW);  
        lcd.print("p_q");
    }
    
    if(espera == 0) {
        contador = VUELTAS;
        flagIntentos = 2;
        espera = 30;
        lcd.clear();
        ResetLockKey();  
    } else {
        espera--;
    }
}