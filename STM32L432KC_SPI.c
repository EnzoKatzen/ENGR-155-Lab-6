// STM32L432KC_SPI.c
// SPI functions to control the reading,
// writing, and initializing of an SPI
// connection

#include "STM32L432KC.h"

#define CE_PIN PB7

void initSPI(int br, int cpol, int cpha){
    pinMode(CE_PIN, GPIO_OUTPUT);   //CE
    digitalWrite(CE_PIN, 0);        //make sure to start low to block any accidents

    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR1 |= br << SPI_CR1_BR_Pos;
    SPI1->CR1 &= ~(SPI_CR1_CPOL);
    SPI1->CR1 |= cpol << SPI_CR1_CPOL_Pos;
    SPI1->CR1 &= ~(SPI_CR1_CPHA);
    SPI1->CR1 |= cpha << SPI_CR1_CPHA_Pos;
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= SPI_CR1_SSM;
    SPI1->CR1 |= SPI_CR1_SSI;

    SPI1->CR2 &= ~SPI_CR2_DS;
    SPI1->CR2 |= 0b0111 << SPI_CR2_DS_Pos; //8 bits (one byte)
    SPI1->CR2 |= SPI_CR2_FRXTH; //respond after 8 bits

    SPI1->CR1 |= SPI_CR1_SPE; //enable SPI
}


char spiSendReceive(char send){
    while (!(SPI1->SR & SPI_SR_TXE));
    *((volatile char *) (&SPI1->DR)) = send;
    while (!(SPI1->SR & SPI_SR_RXNE));
    return *((volatile char *) (&SPI1->DR));
}

void spiWrite(char address, char write){
    digitalWrite(CE_PIN, 1);
    spiSendReceive(address);
    spiSendReceive(write); 
    while (SPI1->SR & SPI_SR_BSY);
    digitalWrite(CE_PIN, 0);
}

char spiRead(char address){
    digitalWrite(CE_PIN, 1);
    spiSendReceive(address);
    char val = spiSendReceive(0x00); //empty as this is just to receive data.
    while (SPI1->SR & SPI_SR_BSY);
    digitalWrite(CE_PIN, 0);
    return val;
}