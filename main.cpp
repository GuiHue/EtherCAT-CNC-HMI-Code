#include "mbed.h"
#include "stdint.h"
#include "EasyCAT.h"   
#include "QEI.h"


// Blinking rates in milliseconds
#define BLINKING_ETHERCAT_ERROR_RATE     200ms

void Application (void); 

// Define analog in pins
AnalogIn Ana1(PA_0);               // analog input 1
AnalogIn Ana2(PA_1);               // analog input 2
AnalogIn Ana3(PA_4);               // analog input 3
AnalogIn Ana4(PB_0);               // analog input 4



// Define shift register pins
DigitalOut shcpPin(PB_2);  // 74HC595 pin 11 (SHCP) shift via HC14D RCLK A5 brown     R0
DigitalOut stcpPin(PB_15);  // 74HC595 pin 12 (STCP) latch via HC14D SRCLK //PL clocks oposite to STCP A4 blk PC1 R2
DigitalOut dataPin(PB_1);   // 74HC595 pin 14 (Data DS) A2 red PA_4 R1
DigitalIn  readPin(PB_14);     // Serial Out of 74HC597 A1 white //R3
DigitalOut oePin(PB_13);

// Define encoder pins
QEI enc1(PB_7, PC_13, NC, 624, QEI::X2_ENCODING);
QEI enc2(PC_2, PC_3, NC, 624, QEI::X2_ENCODING);


EasyCAT EASYCAT;                    // EasyCAT instantiation


//---- global functions ---------------------------------------------------------------------------
void parallelLoad() {
    stcpPin = 0;  // Start the parallel load
    stcpPin = 1;  // Finish the parallel load
}

// Function to read one byte from the shift registers (called once per shift register present)
uint8_t shiftIn() {
    uint8_t value = 0;
    // Shift in 8 bits from the PISO shift register
    stcpPin = 0; //means the inverted to this PL is high
    for (int i = 7; i >=0; --i) {
        // Pulse the shift register clock to shift in the next bit
        // Read the serial output (QH) and update the value
        value |= (readPin << i);
        shcpPin = 1;
        
        shcpPin = 0;
    }
    return value;
}

// Function to write one byte to the outputs (8 bit, e.g. 1 shift register)
void shiftOut(uint8_t value) {
    for (int i = 7; i >= 0; --i) {
        // Shift out the data bit
        dataPin = (value >> i) & 1;
        // Pulse the clock pin
        shcpPin = 1;
        shcpPin = 0;
    }
}




int main()
{
    // Initialise the digital pin LED1 as an output
    #ifdef LED1
        DigitalOut led(PC_1);
    #else
        bool led;
    #endif
    if (EASYCAT.Init() == true)                           // initialization
    {                                                     // succesfully completed
        printf ("EASYCAT initialized\n");
    }   else                                                  // initialization failed   
    {                                                     // the EasyCAT board was not recognized
        printf ("initialization failed\n");                 //     
        while (1)                                           // stay in loop for ever
        {                                                      // with the led blinking
        led = !led;
        ThisThread::sleep_for(BLINKING_ETHERCAT_ERROR_RATE);
        printf ("EASYCAT Initialization failed\n");
        }                                                   // 
    } 
    oePin = 0;
    led=1;
    while (true) {
        wait_us(5);                                //
        EASYCAT.MainTask();                               // execute the EasyCAT task                                                                  
        Application();                                    // execute the user application
       }
}

void Application (void)                                        

{ 
    // --- analog inputs management ---
    EASYCAT.BufferIn.Cust.analog1 = Ana1.read(); ; 
    EASYCAT.BufferIn.Cust.analog2 = Ana2.read(); ; 
    EASYCAT.BufferIn.Cust.analog3 = Ana3.read(); ; 
    EASYCAT.BufferIn.Cust.analog4 = Ana4.read(); ; 
    
    // --- encoder input management
    int32_t encoder1 = static_cast<int32_t>(enc1.getPulses());
    EASYCAT.BufferIn.Cust.enc1 = encoder1;
    int32_t encoder2 = static_cast<int32_t>(enc2.getPulses());
    EASYCAT.BufferIn.Cust.enc2 = encoder2;
  
    // Read inputs
    parallelLoad();
    // Read the entire byte from the PISO shift register 
    EASYCAT.BufferIn.Cust.in1_8   = shiftIn();
    EASYCAT.BufferIn.Cust.in9_16  = shiftIn();
    EASYCAT.BufferIn.Cust.in17_24 = shiftIn();
    EASYCAT.BufferIn.Cust.in25_32 = shiftIn();
    EASYCAT.BufferIn.Cust.in33_40 = shiftIn();
    EASYCAT.BufferIn.Cust.in41_48 = shiftIn();
    EASYCAT.BufferIn.Cust.in49_56 = shiftIn();
    EASYCAT.BufferIn.Cust.in57_64 = shiftIn();
       
    // Write Output
    shiftOut(EASYCAT.BufferOut.Cust.out57_64); // this ends up in the first register. 
    shiftOut(EASYCAT.BufferOut.Cust.out49_56); // this ends up in the second
    shiftOut(EASYCAT.BufferOut.Cust.out41_48);
    shiftOut(EASYCAT.BufferOut.Cust.out33_40);
    shiftOut(EASYCAT.BufferOut.Cust.out25_32);
    shiftOut(EASYCAT.BufferOut.Cust.out17_24);
    shiftOut(EASYCAT.BufferOut.Cust.out9_16);
    shiftOut(EASYCAT.BufferOut.Cust.out1_8);

    parallelLoad(); 

}   



