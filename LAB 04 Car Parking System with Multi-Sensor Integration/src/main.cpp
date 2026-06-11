#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define F_CPU 16000000UL

#define TRIG PD2
#define ECHO PD3
#define BUTTON PD4

#define GREEN PB0
#define YELLOW PB1
#define RED PB2

#define BAUD 9600
#define UBRR_VALUE 103

void USART_Init(void)
{
    UBRR0H = (UBRR_VALUE >> 8);
    UBRR0L = UBRR_VALUE;

    UCSR0B = (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void USART_SendString(char *str)
{
    while (*str)
    {
        USART_Transmit(*str++);
    }
}

long measureDistance()
{
    long count = 0;

    PORTD &= ~(1 << TRIG);
    _delay_us(2);

    PORTD |= (1 << TRIG);
    _delay_us(10);
    PORTD &= ~(1 << TRIG);

    while (!(PIND & (1 << ECHO)));

    while (PIND & (1 << ECHO))
    {
        count++;
        _delay_us(1);
    }

    long distance = count / 58;

    return distance;
}

int main(void)
{
    DDRD |= (1 << TRIG);

    DDRD &= ~((1 << ECHO) | (1 << BUTTON));

    PORTD |= (1 << BUTTON);

    DDRB |= (1 << GREEN) | (1 << YELLOW) | (1 << RED);

    USART_Init();

    uint8_t initial_capacity = 10;
    uint8_t vehicles = 0;

    uint8_t detected = 0;

    char buffer[50];

    while (1)
    {
        long distance = measureDistance();

        if (distance < 10 && detected == 0)
        {
            if (vehicles < initial_capacity)
            {
                vehicles++;

                sprintf(buffer,
                        "Vehicle Count = %d\r\n",
                        vehicles);

                USART_SendString(buffer);
            }

            detected = 1;
        }

        if (distance > 15)
        {
            detected = 0;
        }

        if (!(PIND & (1 << BUTTON)))
        {
            vehicles = 0;

            USART_SendString("System Reset\r\n");

            _delay_ms(300);
        }

        uint8_t available =
            initial_capacity - vehicles;

        PORTB &= ~((1 << GREEN) |
                   (1 << YELLOW) |
                   (1 << RED));

        if (available > 5)
        {
            PORTB |= (1 << GREEN);
        }
        else if (available > 0)
        {
            PORTB |= (1 << YELLOW);
        }
        else
        {
            PORTB |= (1 << RED);
        }

        _delay_ms(100);
    }
}