#define FSYNC 2
#define SDATA 11
#define SCLK 13

long freq; 
void AD9837Write(char cmd, char dat);
void set_freq(long frequency);
unsigned char flag_type = 0;

void setup()
{
  pinMode(FSYNC, OUTPUT); 
  pinMode(SDATA, OUTPUT); 
  pinMode(SCLK, OUTPUT);
  Serial.begin(9600); 
  digitalWrite(FSYNC, HIGH);
  digitalWrite(SDATA, HIGH);
  digitalWrite(SCLK, HIGH);
  String help  = "<----Command---->\r\n";
        help += "Set sine wave -- sine freq\r\n";
        help += "Ex. set sine wave @1Mhz--> sin 1000000\r\n";
        help += "Set set triangular wave -- tri freq\r\n";
        help += "Ex. set triangular wave @100Khz--> tri 100000\r\n";
        help += "Set set square --  sqr freq\r\n";
        help += "Ex. set square wave @2Khz--> sqr 2000\r\n"; 
  Serial.println(help);
  set_freq(1000);
}

void loop()
{
  if(Serial.available())
  {
      String req = Serial.readStringUntil('\n');
      if(req.indexOf(F("sin")) != -1)
      {
          flag_type=0;
          set_freq(freq_str_to_long(req));
      }
      if(req.indexOf(F("sqr")) != -1)
      {
          flag_type=1;
          set_freq(freq_str_to_long(req)*2);
      }
      if(req.indexOf(F("tri")) != -1)
      {
          flag_type=2;
          set_freq(freq_str_to_long(req));
      } 
  }
  
 }

long freq_str_to_long(String dat)
{
    char index1 = dat.indexOf(F(" "));
    return(dat.substring(index1+1).toInt());
}
void set_freq(long frequency)
{
  //
  int MSB;
  int LSB;
  int phase = 0;
  
  long FREQ0_REG0;
  float AD9837Val = 0.00000000;
  
  AD9837Val = (((float)(frequency))/25000000);
  FREQ0_REG0 = AD9837Val*0x10000000;
  
  
  Serial.println("");
  Serial.print("FREQ0_REG0 is ");
  Serial.print(FREQ0_REG0);
  Serial.println("");
  
  MSB = (int)((FREQ0_REG0 & 0xFFFC000)>>14); //14 bits
  LSB = (int)(FREQ0_REG0 & 0x3FFF);
  
  //Set control bits DB15 ande DB14 to 0 and 1, FREQ0 register write(D15, D14 = 01)
  LSB |= 0x4000; //0b 0100 0000 0000 0000
  MSB |= 0x4000; //0b 0100 0000 0000 0000
 
  phase &= 0xC000; //0b1100 0000 0000 0000 =>When writing to a phase register, Bit D15 and Bit D14 are set to 11. 
                      
  AD9837Write(0x2100);//0b 0010 0001 0000 0000 => Set D13(set frequency registers. B28 bit)and D8 (Reset=1)
 
  delay(100);

  AD9837Write(LSB);  //Write Freq lower 14 bits 

  AD9837Write(MSB);  //Write Freq upper 14 bits

  AD9837Write(phase);  //mid-low
  delay(10);
  switch(flag_type)
  {
    case 0:
      AD9837Write(0x2000); //sin wave 
      Serial.print("Sine ");
      Serial.print(frequency);
    break;
    case 1:
      AD9837Write(0x2020); //square
      Serial.print("Square ");
      frequency=frequency/2;
      Serial.print(frequency);
    break;
    case 2:
      AD9837Write(0x2002); //triangle
      Serial.print("Triangle ");
      Serial.print(frequency);
    break;  
  }
  
  Serial.println(" Hz");
}


void AD9837Write(int dat)
{
  int x;
  digitalWrite(FSYNC, LOW);  //Set FSYNC low
  for (x = 0; x < 16; x++)
  {
    if (dat & 0x8000)
    { 
    digitalWrite(SDATA, HIGH);   //Set SDATA according to MSB in cmd
    }
    else
   { 
    digitalWrite(SDATA, LOW);
   }
    digitalWrite(SCLK, LOW);  //CLK transistion to latch the bit into the AD9835
    digitalWrite(SCLK, HIGH);   
    dat <<= 1;  //Shift to next bit in cmd
  }
  digitalWrite(FSYNC, HIGH);  //Set FSYNC high
}


