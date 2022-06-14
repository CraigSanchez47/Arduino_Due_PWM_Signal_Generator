// PWM signal for Capstone Project
// By: Craig Sanchez
// 6/10/2022

// Enable asynchorous, complementary output, dual-slope PWM at 100kHz on 2 channels (0 to 1), at 50% duty cycle default
// 4 PWM outputs on 2 complementary channels on pins D34, D35, D36 and D37 for channels 0 to 1 respectively

// User input frequency in KHz
// Control Mode Select: 
// • Phase Shift α (degrees) 5 to 175
// • Duty Cycle S1&S2 (degrees) 0 to 95
// • Dead Time insertion for Channel 0 and Channel 1

#include <stdio.h>
#include <string.h>

volatile float f = 0.0000;
volatile float value = 0.0000;
volatile float duty = 0.0000;
volatile float duty2 = 0.0000;
volatile float inverse_duty = 0.0000;
volatile float non_inverse = 0.0000;
float m_time = 0.00000000;
float m_period = 0.00000000;

void setup() {
  // PWM Set-up on pin PC2 (Arduino Pin 34) Select Instances = PWM; Signal PWML0; Channel 0
  // PWM Set-up on pin PC3 (Arduino Pin 35) Select Instances = PWM; Signal PWMH0; Channel 0
  // PWM Set-up on pin PC4 (Arduino Pin 36) Select Instances = PWM; Signal PWML1; Channel 1
  // PWM Set-up on pin PC5 (Arduino Pin 37) Select Instances = PWM; Signal PWMH1; Channel 1

  PMC -> PMC_PCER1 |= PMC_PCER1_PID36; // Enable PWM Registers (36 Peripheral IDs)

  PWM->PWM_DIS = PWM_DIS_CHID0;       // Disable Channel 0 register for updates
  PWM->PWM_DIS = PWM_DIS_CHID1;       // Disable Channel 1 register for updates

  REG_PIOC_ABSR |= PIO_ABSR_P2;       // Set PWM pin 34 perhipheral type B (I/O PWM pin function)
  REG_PIOC_ABSR |= PIO_ABSR_P3;       // Set PWM pin 35 perhipheral type B (I/O PWM pin function)
  REG_PIOC_ABSR |= PIO_ABSR_P4;       // Set PWM pin 36 perhipheral type B (I/O PWM pin function)
  REG_PIOC_ABSR |= PIO_ABSR_P5;       // Set PWM pin 37 perhipheral type B (I/O PWM pin function)

  REG_PIOC_PDR |= PIO_PDR_P2;         // Set PWM pin 34 to an output signal
  REG_PIOC_PDR |= PIO_PDR_P3;         // Set PWM pin 35 to an output signal
  REG_PIOC_PDR |= PIO_PDR_P4;         // Set PWM pin 36 to an output signal
  REG_PIOC_PDR |= PIO_PDR_P5;         // Set PWM pin 37 to an output signal

  REG_PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(1) | PWM_CLK_DIVB(1); // Set the PWM clock rate to 84MHz (84MHz/1). Adjust DIVA for the resolution. Generate a single CLK.

  REG_PWM_CMR0 = PWM_CMR_CALG | PWM_CMR_CPRE_CLKA | PWM_CMR_DTE; // The period is left aligned, clock source as CLKA on channel 0, dual slope, Enable channel 0 dead-time insertion
  ////REG_PWM_CMR0 = PWM_CMR_CPRE_CLKA; // enable single slope PWM and set the clock source as CLKA, aligment shift

  REG_PWM_CMR1 = PWM_CMR_CALG | PWM_CMR_CPRE_CLKB |PWM_CMR_DTE; // The period is left aligned, clock source as CLKB on channel 1, dual slope, Enable channel 1 dead-time insertion
  ////REG_PWM_CMR1 = PWM_CMR_CPRE_CLKB; // enable single slope PWM ; better resolution? aligment shift: adjust frequency and duty cycle to (840 and 420) respectively

  REG_PWM_DT0 = PWM_DT_DTH(0) | PWM_DT_DTL(0); // Set Channel 0 dead-time for 0ns delay on output PWML0/PWMH0 
  REG_PWM_DT1 = PWM_DT_DTH(0) | PWM_DT_DTL(0); // Set channel 1 dead-time for 0ns delay on output PWMH1/PWML1

  REG_PWM_CPRD0 = 420; // Channel 0: Set the PWM frequency DIVA/(2 * Desired Frequency) = CPRD; example (84MHz/(2*100kHz)) = 420
  REG_PWM_CDTY0 = 210; // Channel 0: Set the PWM duty Cyle to CPRD/number = %; Example (420/2) = 210 for 50%
  
  REG_PWM_CPRD1 = 420; // Channel 1: Set PWM frequency DIVB/(2*Desired Frequency)
  REG_PWM_CDTY1 = 210; // Channel 1: Set PWM duty cycle; CPRDx*(%duty cyle) = CDTYx; Example 315 for 75% duty cycle,

  REG_PWM_ENA = PWM_ENA_CHID0;  // Enable PWM channel 0
  REG_PWM_ENA = PWM_ENA_CHID1;  // Enable PWM Channel 1

  Serial.begin(115200); // Programming Port Enabled on COM port (recommended)
  //SerialUSB.begin(115200); // actives the Native USB serial port
  Serial.println("(f) for Frequency in kHz");                 // Change frequnecy in kHz
  Serial.println("(d) for Duty Cycle in degrees (5-95)");     // duty cycle change of PWM pairs
  Serial.println("(p) for Phase Shift in degrees (0-175)");   // phase shift PWM pairs
  Serial.println("(a) for Dead time insertion on Channel 0"); // Add deadtime to PWMH0
  Serial.println("(b) for Dead Time insertion on Channel 1"); // Add deadtime to PWMH1
  
  value = 420.0000;  // default frequency value
  f = 100000.000000; //default frequency 100KHz
}
void loop() {
  if(Serial.available()>0){
    byte command = Serial.read();    // Serail read the incoming byte command
    if (command == 'f'){
      f = Serial.parseFloat();
      Serial.print("Frequency set to (kHz): ");
      Serial.println(f);
      f = f *1000.000000000;          // Converts frequency to KHz
      value = (84.0000e6)/(2.0000*f); // Calculates the frequency
      REG_PWM_CPRDUPD0 = value;       // update period to channel 0
      REG_PWM_CPRDUPD1 = value;       // update period to channel 1
      REG_PWM_ENA = PWM_ENA_CHID0;    // enable channel 0
      REG_PWM_ENA = PWM_ENA_CHID1;    // enable channel 1
    }
    else if(command == 'd'){
      float d = Serial.parseFloat();
      Serial.print("Duty cycle set to (degrees): ");
      Serial.println(d);
      inverse_duty = 100.0000-d;      // sets inverse duty cycle for Channel 1
      non_inverse = d/100.0000;       // Converts duty cycle from degress to value
      inverse_duty = inverse_duty/100.0000; // Converst inverse duty from degrees to value
      duty = non_inverse*value;       // multiplies duty cycle by the frequency value
      duty2 = inverse_duty*value;     // multiplies inverse duty cycle by the frequency value
      REG_PWM_CDTYUPD0 = duty;        // Update Duty Cycle register channel 0
      REG_PWM_CDTYUPD1 = duty2;       // Update Duty Cycle register channel 1
      REG_PWM_ENA = PWM_ENA_CHID0;    // enable channel 0
      REG_PWM_ENA = PWM_ENA_CHID1;    // enable channel 1
    }
    else if (command == 'p'){
      PWM->PWM_DIS = PWM_DIS_CHID0;       // Disable Channel 0 for register changes 
      PWM->PWM_DIS = PWM_DIS_CHID1;       // Disable Channel 1 for register changes
      float p = Serial.parseFloat();
      Serial.print("Phase shift set to (degrees): ");
      Serial.println(p);
      m_period = (1/f);                    // period in seconds
      m_time = (p*m_period)/360.00000;     // divide period by 360 degrees
      m_time = m_time*1000000.00000;       // convert to us
      REG_PWM_ENA = PWM_ENA_CHID0;         // Enable PWM channel 0
      delayMicroseconds(m_time);           // delay time in micro-seconds
      REG_PWM_ENA = PWM_ENA_CHID1;         // Enable PWM Channel 1
    }
    else if (command == 'a') {
      int t1 = Serial.parseInt(); // Parse the int value
      Serial.print("Dead Time Insertion set for Channel 0: ");
      Serial.print(t1);
      REG_PWM_DTUPD0 = PWM_DT_DTH(t1) | PWM_DT_DTL(0); // Set Channel 0 dead-time on output PWML0/PWMH0
    }
    else if (command == 'b') {
      int t2 = Serial.parseInt(); // Parse the int value 
      Serial.print("Dead Time Insertion set for Channel 1 : ");
      Serial.print(t2);
      REG_PWM_DTUPD1 = PWM_DT_DTH(t2) | PWM_DT_DTL(0); // Set channel 1 dead-time on output PWMH1/PWML1      
    }
 }

}
