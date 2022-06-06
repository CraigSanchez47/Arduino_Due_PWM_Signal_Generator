// PWM signal for Capstone Project
// Enable asynchorous, complementary output, dual-slope PWM at 100kHz on 2 channels (0 to 1), at 50% duty cycle default
// 4 PWM outputs on 2 complementary channels on pins D34, D35, D36 and D37 for channels 0 to 1 respectively

// User input frequency in KHz
// Control Mode Select: 
// • Phase Shift α (degrees) 5 to 175
// • Duty Cycle S1&S2 (degrees) 0 to 95
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

  PMC -> PMC_PCER1 |= PMC_PCER1_PID36; // Enable PWM 

  PWM->PWM_DIS = PWM_DIS_CHID0;       // Disable Channel 0 for register changes 
  PWM->PWM_DIS = PWM_DIS_CHID1;       // Disable Channel 1 for register changes

  REG_PIOC_ABSR |= PIO_ABSR_P2;       // Set PWM pin 34 perhipheral type B
  REG_PIOC_ABSR |= PIO_ABSR_P3;       // Set PWM pin 35 perhipheral type B
  REG_PIOC_ABSR |= PIO_ABSR_P4;       // Set PWM pin 36 perhipheral type B
  REG_PIOC_ABSR |= PIO_ABSR_P5;       // Set PWM pin 37 perhipheral type B

  REG_PIOC_PDR |= PIO_PDR_P2;         // Set PWM pin 34 to an output 
  REG_PIOC_PDR |= PIO_PDR_P3;         // Set PWM pin 35 to an output
  REG_PIOC_PDR |= PIO_PDR_P4;         // Set PWM pin 36 to an output
  REG_PIOC_PDR |= PIO_PDR_P5;         // Set PWM pin 37 to an output

  REG_PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(1) | PWM_CLK_DIVB(1); // Set the PWM clock rate to 84MHz (84MHz/1). Adjust DIVA for the resolution you are looking for

  REG_PWM_CMR0 = PWM_CMR_CALG | PWM_CMR_CPRE_CLKA | PWM_CMR_DTE; // The period is left aligned, clock source as CLKA on channel 0, dual slope, Enable channel 0 dead-time insertion
  //REG_PWM_CMR0 = PWM_CMR_CPRE_CLKA; // enable single slope PWM and set the clock source as CLKA, can cause phase shift hence adjust accordingly
  //REG_PWM_DT0 = PWM_DT_DTH(50) | PWM_DT_DTL(50); // Set Channel 0 dead-time for 560ns delay on output PWML0/PWMH0

  REG_PWM_CMR1 = PWM_CMR_CALG | PWM_CMR_CPRE_CLKB |PWM_CMR_DTE; // The period is left aligned, clock source as CLKB on channel 1, dual slope, Enable channel 1 dead-time insertion
  //REG_PWM_CMR1 = PWM_CMR_CPRE_CLKB; // enable single slope PWM ; better resolution? can cause phase shift due to alignment (840 and 420)
  //REG_PWM_DT1 = PWM_DT_DTH(50) | PWM_DT_DTL(50); // Set channel 1 dead-time for 560ns delay on output PWMH1/PWML1

  REG_PWM_CPRD0 = 420; // Channel 0: Set the PWM frequency DIVA/(2 * Desired Frequency) = CPRD; example (84MHz/(2*100kHz)) = 420
  REG_PWM_CDTY0 = 210; // Channel 0: Set the PWM duty Cyle to CPRD/number = %; Example (420/2) = 210 for 50%
  
  REG_PWM_CPRD1 = 420; // Channel 1: Set PWM frequency DIVB/(2*Desired Frequency)
  REG_PWM_CDTY1 = 210; // Channel 1: Set PWM duty cycle; CPRDx*(%duty cyle) = CDTYx; Example 315 for 75% duty cycle,

  REG_PWM_ENA = PWM_ENA_CHID0;         // Enable PWM channel 0
  //delayMicroseconds(2.5);              // 2.5us delay-> 90 degrees
  REG_PWM_ENA = PWM_ENA_CHID1;         // Enable PWM Channel 1

  Serial.begin(115200);
  //SerialUSB.begin(115200);
  Serial.println("(f) for Frequency in kHz"); // in kHz
  Serial.println("(d) for duty cycle in degrees (5-95)"); // duty cycle change
  Serial.println("(p) for phase shift in degrees (0-175)"); // phase shift
  value = 420.0000;
  f = 100000.000000; //default frequency 100KHz
}
void loop() {
  
  if(Serial.available()>0){
    byte command = Serial.read();
    if (command == 'f'){
      f = Serial.parseFloat();
      Serial.print("Frequency set to (kHz): ");
      Serial.println(f);
      f = f *1000.000000000; 
      value = (84.0000e6)/(2.0000*f); // Calculates the frequency
    //Serial.print("Set value on register: ")
    //Serial.println(value);
      REG_PWM_CPRDUPD0 = value; // update period to channel 0
      REG_PWM_CPRDUPD1 = value; // update period to channel 1
      REG_PWM_ENA = PWM_ENA_CHID0;
      REG_PWM_ENA = PWM_ENA_CHID1;
    }
    else if(command == 'd'){
      float d = Serial.parseFloat();
      Serial.print("Duty cycle set to (degrees): ");
      Serial.println(d);
      inverse_duty = 100.0000-d;
      non_inverse = d/100.0000;
      inverse_duty = inverse_duty/100.0000;
      duty = non_inverse*value;
      duty2 = inverse_duty*value;
      REG_PWM_CDTYUPD0 = duty;
      REG_PWM_CDTYUPD1 = duty2;
      REG_PWM_ENA = PWM_ENA_CHID0;
      REG_PWM_ENA = PWM_ENA_CHID1;
    }
    else if (command == 'p'){
      PWM->PWM_DIS = PWM_DIS_CHID0;       // Disable Channel 0 for register changes 
      PWM->PWM_DIS = PWM_DIS_CHID1;       // Disable Channel 1 for register changes
      float p = Serial.parseFloat();
      Serial.print("Phase shift set to (degrees): ");
      Serial.println(p);
      m_period = (1/f); // s
      //Serial.println(m_period);
      m_time = (p*m_period)/360.00000; 
      //Serial.println(m_time);
      m_time = m_time*1000000.00000;   // convert to us
      //Serial.println(m_time);
      REG_PWM_ENA = PWM_ENA_CHID0;         // Enable PWM channel 0
      delayMicroseconds(m_time);           // pauses for us
      REG_PWM_ENA = PWM_ENA_CHID1;         // Enable PWM Channel 1
    }
 }

}
