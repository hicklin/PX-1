//Pin connections for UNO
//
//scl -> A5
//sda -> A4
//
//Pin connections for mega
//
//scl -> C21
//sda -> C20
//
#include <Wire.h>

#define ITG3200_ADDRESS (0xD0 >> 1)
#define ITG3200_REGISTER_XMSB (0x1D)
#define ITG3200_REGISTER_DLPF_FS (0x16)
#define ITG3200_FULLSCALE (0x03 << 3)
#define ITG3200_42HZ (0x03)

float gyro_data[3];
float gyro[3];
float deg[3];
long oldT;
long newT;
float Dtime;


char c;

float gyro_offset[3];

int i = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  for(int i = 0; i < 3; ++i) {
    gyro_data[i] = 0;
    deg[i] = 0;
  }
  
  init_itg3200();
  
  // Reset gyros to zero
  GyOffset();

  Serial.print("Gyros offsets x: ");
  Serial.print(gyro_offset[0]);
  Serial.print("\ty: ");
  Serial.print(gyro_offset[1]);
  Serial.print("\tz: ");
  Serial.println(gyro_offset[2]);
}

void loop() {
   read_itg3200();
   newT = micros();
   Dtime = newT - oldT;
   oldT = newT;
   Serial.print("GYRO: ");
   for (int i=0; i<3; i++) {
     gyro[i] = (BitToDperS(gyro_data[i] - gyro_offset[i])) * Dtime/1000000;
     deg[i] = deg[i] + gyro[i];
     if(deg[i] >= 360) {
       deg[i] = deg[i] - 360;
     }
     if(deg[i] <= -360) {
       deg[i] = deg[i] + 360;
     }
     Serial.print("\t");
     Serial.print(deg[i]);
   }
   Serial.print("\n");
//   delay(100);
}

void i2c_write(int address, byte reg, byte data) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void i2c_read(int address, byte reg, int count, byte* data) {
 int i = 0;

 Wire.beginTransmission(address);
 Wire.write(reg);
 Wire.endTransmission();
 Wire.beginTransmission(address);
 Wire.requestFrom(address,count);
 while(Wire.available()){
   c = Wire.read();
   data[i] = c;
   i++;
 }
 Wire.endTransmission();
} 

void init_itg3200() {
  byte data = 0;

  i2c_write(ITG3200_ADDRESS, ITG3200_REGISTER_DLPF_FS, ITG3200_FULLSCALE | ITG3200_42HZ);

  i2c_read(ITG3200_ADDRESS, ITG3200_REGISTER_DLPF_FS, 1, &data);

  Serial.println((unsigned int)data);
}

void read_itg3200() {
  byte bytes[6];
  memset(bytes,0,6);

  i2c_read(ITG3200_ADDRESS, ITG3200_REGISTER_XMSB, 6, bytes);
  for (int i=0;i<3;++i) {
  gyro_data[i] = (int)bytes[2*i + 1] + (((int)bytes[2*i]) << 8);
  }
}

void GyOffset() {
  while(i<20) {
    read_itg3200();
    for (int i=0; i<3; i++) {
      gyro_offset[i] = gyro_offset[i] + gyro_data[i];
    }
    i++;
  }
  for (int i=0; i<3; i++) {
    gyro_offset[i] = gyro_offset[i]/20;
  }
  oldT = micros();
}

float BitToDperS(float x) {
  return x/14.375;
}
