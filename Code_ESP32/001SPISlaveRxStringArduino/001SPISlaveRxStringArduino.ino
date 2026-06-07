#include <SPI.h>

volatile uint8_t rxBuffer[256];
volatile uint8_t rxIndex = 0;
volatile uint8_t dataLength = 0;
volatile bool lengthReceived = false;
volatile bool messageReady = false;

// SPI Interrupt Service Routine
ISR(SPI_STC_vect)
{
    uint8_t receivedByte = SPDR;

    if (!lengthReceived)
    {
        dataLength = receivedByte;
        rxIndex = 0;
        lengthReceived = true;
    }
    else
    {
        if (rxIndex < dataLength)
        {
            rxBuffer[rxIndex++] = receivedByte;
        }

        if (rxIndex >= dataLength)
        {
            rxBuffer[dataLength] = '\0';
            messageReady = true;
            lengthReceived = false;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    // SPI Slave pins
    pinMode(MISO, OUTPUT);
    pinMode(MOSI, INPUT);
    pinMode(SCK, INPUT);
    pinMode(SS, INPUT);

    // Enable SPI Slave
    SPCR |= _BV(SPE);

    // Enable SPI Interrupt
    SPCR |= _BV(SPIE);

    Serial.println("SPI Slave Ready");
}

void loop()
{
    if (messageReady)
    {
        noInterrupts();

        char msg[256];
        strcpy(msg, (char*)rxBuffer);

        messageReady = false;

        interrupts();

        Serial.print("Received: ");
        Serial.println(msg);

        Serial.print("Length: ");
        Serial.println(strlen(msg));
    }
}