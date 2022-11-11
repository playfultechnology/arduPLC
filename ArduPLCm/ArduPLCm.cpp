/*
  ArduPLCm.cpp - Libreria para la placa ArduPLCm
  Copyright (c) 2012 Raimundo Alfonso
  Ray Ingenier�a Electr�nica, S.L.
  
  Esta librer�a est� basada en software libre. Tu puedes redistribuir
  y/o modificar esta librer�a bajo los terminos de licencia GNU.

  Esta biblioteca se distribuye con la esperanza de que sea �til,
  pero SIN NINGUNA GARANT�A, incluso sin la garant�a impl�cita de
  COMERCIALIZACI�N O PARA UN PROP�SITO PARTICULAR.
  Consulte los terminos de licencia GNU para m�s detalles.

  09-06-2012 - v1.00 : primera versi�n
  
  
*/


//#include "WConstants.h"
#include "ArduPLCm.h"


ArduPLCm::ArduPLCm(void){
	pinMode(DIN1,INPUT);
	pinMode(DIN2,INPUT);	
	pinMode(DIN3,INPUT);
	pinMode(DIN4,INPUT);
	pinMode(DIPSW1,INPUT);
	pinMode(DIPSW2,INPUT);	
	pinMode(DIPSW3,INPUT);	
	pinMode(DIPSW4,INPUT);	
	pinMode(DIPSW5,INPUT);	
	pinMode(DIPSW6,INPUT);	
	pinMode(RELE1,OUTPUT);
	pinMode(RELE2,OUTPUT);
	pinMode(RELE3,OUTPUT);
	pinMode(RELE4,OUTPUT);
}


void ArduPLCm::releOn(byte rele){
	digitalWrite(rele,HIGH);
}
 
void ArduPLCm::releOff(byte rele){
	digitalWrite(rele,LOW);
} 

byte ArduPLCm::leeDIPSW(void){
	byte a0,a1,a2,a3,a4,a5;
	
	// Lee dipswitch...
	a0 = !digitalRead(DIPSW1);  
	a1 = !digitalRead(DIPSW2);
	a2 = !digitalRead(DIPSW3);
	a3 = !digitalRead(DIPSW4);
	a4 = !digitalRead(DIPSW5);  
	a5 = !digitalRead(DIPSW6);  
  
	// Calcula direcci�n...
	return(a0 + a1*2 + a2*4 + a3*8 + a4*16 + a5*32);
}
 
boolean ArduPLCm::leeDIN(byte digIn){
	return(digitalRead(digIn));
} 

int ArduPLCm::leeAIN(byte anIn){
	return(analogRead(anIn));
}

float ArduPLCm::leeVoltiosAIN(byte anIn){
	return(((float)analogRead(anIn) * 5.0) / 1023.0);
}
 