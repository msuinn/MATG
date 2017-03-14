const int IN_A1 = 2;
const int IN_A2 = 3;
const int IN_A3 = 4;

const int IN_B1 = 5;
const int IN_B2 = 6;
const int IN_B3 = 7;

const int IN_C1 = 8;
const int IN_C2 = 9;
const int IN_C3 = 10;


// SPWM (Sine Wave)
//const int pwmSin[] = {127, 138, 149, 160, 170, 181, 191, 200, 209, 217, 224, 231, 237, 242, 246, 250, 252, 254, 254, 254, 252, 250, 246, 242, 237, 231, 224, 217, 209, 200, 191, 181, 170, 160, 149, 138, 127, 116, 105, 94, 84, 73, 64, 54, 45, 37, 30, 23, 17, 12, 8, 4, 2, 0, 0, 0, 2, 4, 8, 12, 17, 23, 30, 37, 45, 54, 64, 73, 84, 94, 105, 116 };


/// SVPWM (Space Vector Wave)
const int pwmSin[] = {128, 147, 166, 185, 203, 221, 238, 243, 248, 251, 253, 255, 255, 255, 253, 251, 248, 243, 238, 243, 248, 251, 253, 255, 255, 255, 253, 251, 248, 243, 238, 221, 203, 185, 166, 147, 128, 109, 90, 71, 53, 35, 18, 13, 8, 5, 3, 1, 1, 1, 3, 5, 8, 13, 18, 13, 8, 5, 3, 1, 1, 1, 3, 5, 8, 13, 18, 35, 53, 71, 90, 109};
//const int pwmSin[] = {128, 132, 136, 140, 143, 147, 151, 155, 159, 162, 166, 170, 174, 178, 181, 185, 189, 192, 196, 200, 203, 207, 211, 214, 218, 221, 225, 228, 232, 235, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 250, 249, 248, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 250, 249, 248, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 235, 232, 228, 225, 221, 218, 214, 211, 207, 203, 200, 196, 192, 189, 185, 181, 178, 174, 170, 166, 162, 159, 155, 151, 147, 143, 140, 136, 132, 128, 124, 120, 116, 113, 109, 105, 101, 97, 94, 90, 86, 82, 78, 75, 71, 67, 64, 60, 56, 53, 49, 45, 42, 38, 35, 31, 28, 24, 21, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 24, 28, 31, 35, 38, 42, 45, 49, 53, 56, 60, 64, 67, 71, 75, 78, 82, 86, 90, 94, 97, 101, 105, 109, 113, 116, 120, 124};

int currentStep_A1;
int currentStep_A2;
int currentStep_A3;

int currentStep_B1;
int currentStep_B2;
int currentStep_B3;

int currentStep_C1;
int currentStep_C2;
int currentStep_C3;

int sineArraySize;
int increment = 0;

int count;

void setup() {
  pinMode(IN_A1, OUTPUT); 
  pinMode(IN_A2, OUTPUT); 
  pinMode(IN_A3, OUTPUT); 

  pinMode(IN_B1, OUTPUT); 
  pinMode(IN_B2, OUTPUT); 
  pinMode(IN_B3, OUTPUT); 

  pinMode(IN_C1, OUTPUT); 
  pinMode(IN_C2, OUTPUT); 
  pinMode(IN_C3, OUTPUT); 

  sineArraySize = sizeof(pwmSin)/sizeof(int); // Find lookup table size
  int phaseShift = sineArraySize / 3;         // Find phase shift and initial A, B C phase values
  
  currentStep_A1 = 0;
  currentStep_A2 = currentStep_A1 + phaseShift;
  currentStep_A3 = currentStep_A2 + phaseShift;

  currentStep_B1 = 0;
  currentStep_B2 = currentStep_B1 + phaseShift;
  currentStep_B3 = currentStep_B2 + phaseShift;

  currentStep_C1 = 0;
  currentStep_C2 = currentStep_C1 + phaseShift;
  currentStep_C3 = currentStep_C2 + phaseShift;

  sineArraySize--; // Convert from array Size to last PWM array number

  count = 0;

}

void loop() {
  

    if (count < 1800) { 
        rotateMotor_C(87, 1);
        count++;
    }
    
    else {
        rotateMotor_C(0, 1); 
    }
    
    delay(1);

}

void rotateMotor_A(int spd, int drctn) {
    if (spd != 0) spd = 101-spd;
    
    if (count%spd == 0) {
    analogWrite(IN_A1, pwmSin[currentStep_A1]);
    analogWrite(IN_A2, pwmSin[currentStep_A2]);
    analogWrite(IN_A3, pwmSin[currentStep_A3]);  
  
    if (drctn==true) increment = 1;
    else increment = -1;     
  
    currentStep_A1 = currentStep_A1 + increment;
    currentStep_A2 = currentStep_A2 + increment;
    currentStep_A3 = currentStep_A3 + increment;
  
    //Check for lookup table overflow and return to opposite end if necessary
    if(currentStep_A1 > sineArraySize)  currentStep_A1 = 0;
    if(currentStep_A1 < 0)  currentStep_A1 = sineArraySize;
   
    if(currentStep_A2 > sineArraySize)  currentStep_A2 = 0;
    if(currentStep_A2 < 0)  currentStep_A2 = sineArraySize;
   
    if(currentStep_A3 > sineArraySize)  currentStep_A3 = 0;
    if(currentStep_A3 < 0) currentStep_A3 = sineArraySize; 
    
   }
}

void rotateMotor_B(int spd, int drctn) {
    if (spd != 0) spd = 101-spd;
    
    if (count%spd == 0) {
    analogWrite(IN_B1, pwmSin[currentStep_B1]);
    analogWrite(IN_B2, pwmSin[currentStep_B2]);
    analogWrite(IN_B3, pwmSin[currentStep_B3]);  
  
    if (drctn==true) increment = 1;
    else increment = -1;     
  
    currentStep_B1 = currentStep_B1 + increment;
    currentStep_B2 = currentStep_B2 + increment;
    currentStep_B3 = currentStep_B3 + increment;
  
    //Check for lookup table overflow and return to opposite end if necessary
    if(currentStep_B1 > sineArraySize)  currentStep_B1 = 0;
    if(currentStep_B1 < 0)  currentStep_B1 = sineArraySize;
   
    if(currentStep_B2 > sineArraySize)  currentStep_B2 = 0;
    if(currentStep_B2 < 0)  currentStep_B2 = sineArraySize;
   
    if(currentStep_B3 > sineArraySize)  currentStep_B3 = 0;
    if(currentStep_B3 < 0) currentStep_B3 = sineArraySize; 
    
   }
}

void rotateMotor_C(int spd, int drctn) {
    if (spd != 0) spd = 101-spd;
    
    if (count%spd == 0) {
    analogWrite(IN_C1, pwmSin[currentStep_C1]);
    analogWrite(IN_C2, pwmSin[currentStep_C2]);
    analogWrite(IN_C3, pwmSin[currentStep_C3]);  
  
    if (drctn==true) increment = 1;
    else increment = -1;     
  
    currentStep_C1 = currentStep_C1 + increment;
    currentStep_C2 = currentStep_C2 + increment;
    currentStep_C3 = currentStep_C3 + increment;
  
    //Check for lookup table overflow and return to opposite end if necessary
    if(currentStep_C1 > sineArraySize)  currentStep_C1 = 0;
    if(currentStep_C1 < 0)  currentStep_C1 = sineArraySize;
   
    if(currentStep_C2 > sineArraySize)  currentStep_C2 = 0;
    if(currentStep_C2 < 0)  currentStep_C2 = sineArraySize;
   
    if(currentStep_C3 > sineArraySize)  currentStep_C3 = 0;
    if(currentStep_C3 < 0) currentStep_C3 = sineArraySize; 
    
   }
}
