/*
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>Lab 6 Temperature Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>Lab 6 Temperature Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
char* tempResStr = "<p>Choose an Temperature Resolution:</p><form action=\"sr\"><select name=\"res\">\
    <option value=\"8b\">8 Bits</option>\
    <option value=\"9b\">9 Bits</option>\
    <option value=\"10b\">10 Bits</option>\
    <option value=\"11b\">11 Bits</option>\
    <option value=\"12b\">12 Bits</option>\
  </select><input type=\"submit\" value=\"Submit Resolution\"></form>";

char* webpageEnd   = "</body></html>";


#define LED_PIN PA6

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

#define INIT_REGISTER 0x80
void updateTempRes(char request[])
{
  //delays for the time it takes to measure temperature
  if (inString(request, "8b") == 1){
    spiWrite(INIT_REGISTER, 0b11100000); //8 bits of resolution
    //delay_millis(TIM15, 75);
  } else if (inString(request, "9b") == 1){
    spiWrite(INIT_REGISTER, 0b11100010); //9 bits of resolution
    //delay_millis(TIM15, 150);
  } else if (inString(request, "10b") == 1){
    spiWrite(INIT_REGISTER, 0b11100100); //10 bits of resolution
    //delay_millis(TIM15, 300);
  } else if (inString(request, "11b") == 1){
    spiWrite(INIT_REGISTER, 0b11100110); //11 bits of resolution
    //delay_millis(TIM15, 600);
  } else if (inString(request, "12b") == 1){
    spiWrite(INIT_REGISTER, 0b11101000); //12 bits of resolution
    //delay_millis(TIM15, 1200);
  }
}

void readTemp(char str[]){
  int MSB = spiRead(0x02);
  int LSB = spiRead(0x01);
  int16_t rawNum = (MSB << 8); //must be a 16 bit int so sign is in most significant bit
  rawNum |= LSB;
  float temp = rawNum / 256.0; //256 = 2^8
  sprintf(str, "The temperature is %f C", temp);
}

/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////
void setup(void){
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(LED_PIN, GPIO_OUTPUT);  
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);

  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //Enable clock to SPI
  pinMode(PB3, GPIO_ALT);   //SCK
  GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3);
  GPIOB->AFR[0] |=  (5 << GPIO_AFRL_AFSEL3_Pos);
  pinMode(PB4, GPIO_ALT);   //MISO
  GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL4);
  GPIOB->AFR[0] |=  (5 << GPIO_AFRL_AFSEL4_Pos);
  pinMode(PB5, GPIO_ALT);   //MOSI
  GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL5);
  GPIOB->AFR[0] |=  (5 << GPIO_AFRL_AFSEL5_Pos);




  initSPI(4, 1, 1); //Baud rate is 2.5 MHz, Polarity is 1, and Phase is 1
  spiWrite(INIT_REGISTER, 0b11100000); //first three bits must be on, one shot off, then set to 8 bits of resolution, and SD off.

}

int main(void) {
  setup();
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);
  int led_status = 0;

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }
    
    if(inString(request, "led") == 1) {
      led_status = updateLEDStatus(request);
    }else if(inString(request, "r") == 1){
      updateTempRes(request);
    }

    char tempStatusStr[50];
    readTemp(tempStatusStr);

    // Update string with current LED state


    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    // finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED

    sendString(USART, "<h2>LED Status</h2>");


    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, "<h2>Temperature Status</h2>");

    sendString(USART, "<p>");
    sendString(USART, tempStatusStr);
    sendString(USART, "</p>");

    sendString(USART, tempResStr); // buttons for controlling temperature resolution


  
    sendString(USART, webpageEnd);
  }
}
