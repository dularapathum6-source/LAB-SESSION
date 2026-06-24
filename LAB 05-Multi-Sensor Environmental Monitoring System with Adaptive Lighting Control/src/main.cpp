#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define F_CPU 16000000UL

#define LIGHT_LED PB0
#define GAS_LED   PB1


void UART_Init(unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << TXEN0);  

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_Transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_SendString(char *str)
{
    while (*str)
    {
        UART_Transmit(*str++);
    }
}


void ADC_Init()
{
    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}


uint16_t ADC_Read(uint8_t channel)
{
    ADMUX &= 0xF0;
    ADMUX |= (channel & 0x0F);

    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));

    return ADC;
}


int main(void)
{
    char buffer[50];

    uint16_t ldrValue;
    uint16_t potValue;
    uint16_t gasValue;

    uint16_t gasThreshold = 200;

    DDRB |= (1 << LIGHT_LED);
    DDRB |= (1 << GAS_LED);

    ADC_Init();

    
    UART_Init(103);

    while (1)
    {
        ldrValue = ADC_Read(0);
        potValue = ADC_Read(1);
        gasValue = ADC_Read(2);

        
        if (ldrValue < potValue)
            PORTB |= (1 << LIGHT_LED);
        else
            PORTB &= ~(1 << LIGHT_LED);

        
        if (gasValue > gasThreshold)
            PORTB |= (1 << GAS_LED);
        else
            PORTB &= ~(1 << GAS_LED);

        
        sprintf(buffer, "LDR=%u  POT=%u  GAS=%u\r\n",
                ldrValue, potValue, gasValue);

        UART_SendString(buffer);

        _delay_ms(500);
    }
}