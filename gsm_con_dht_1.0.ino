#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
#include "DHT.h"

SMSGSM sms;
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int numdata;
boolean started=false;
char smsbuffer[160];
char sms_position;
char phone_number[21]; 
char destino[20];
char sms_text[160];
int humedad,i=0,z=1,x=0,on=0,okred=0,okterm=0,ok=2,okcorriente=0,okbt=0,segs,estado=0,espera=0,corriente=0;
int red=0,valor=0,termica=0,ups,vmax=200,imax=6500;
float lectura,corr,rms,accumrms;
long interval = 60000,intervalred=0,intervalreset=3000,previousMillis = 0; 
unsigned long currentMillis = millis();

void checkSMS(void);
void estadosms (void);
void mido (void);

void setup(){ 
 
 DDRD=B11000100;
 DDRB=B00000001;
 DDRC=0;
 PORTD=B00000100;
 PORTB=1;
 dht.begin();
 if (gsm.begin(9600)) 
    {
        started=true;
    } 
 while(i<20){
   sms.DeleteSMS(i);
   i++;
 }
 i=0;
}     
void loop(){       
      
while((ok==2)||(ok==3)){
  
      currentMillis = millis();
  
      mido();   
      checkSMS();
      
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    
    PORTD=B10000000;
    PORTB=0;    
    z=0;
    if(ok==3){
    ok=1;
  }
    else{
    ok=0;
}  
  }

  }
   
while(ok==0){   
      mido();   
      checkSMS();
    } 
while(ok==1){
     mido();
     checkSMS();

 if((red<120)&&(okred==0)){
  PORTD=B00000100;
  PORTB=1;
  sms.SendSMS(destino, "Falla en Red Electrica.  ID: 1510");
  okred=1;
  z=1;
  on=1;
 }       

 if((red>120)&&(red<vmax)&&(okbt==0)){
  sms.SendSMS(destino, "Baja Tension en Red Electrica.  ID: 1510");
  okbt=1;
 }
 
 if((red>vmax)&&(termica<150)&&(okterm==0)){
  sms.SendSMS(destino, "Falla en Llave Termomagnetica.  ID: 1510");
  okterm=1;
 }
 
  if((corriente>imax)&&(okcorriente==0)){
  mido();
  if(corriente<imax){
    okcorriente=0;
  }
  else{
  for(i=0;i<161;i++){
  smsbuffer[i]=0;
}
  strcpy(smsbuffer,"Exceso de Corriente.\r\n");
  
if(corriente/1000==0){
  smsbuffer[22]=(corriente/100)+48;
  smsbuffer[23]=((corriente%100)/10)+48;
  smsbuffer[24]=((corriente%100)%10)+48;
}
else{
smsbuffer[22]=(corriente/1000)+48;
smsbuffer[23]=((corriente%1000)/100)+48;
smsbuffer[24]=(((corriente%1000)%100)/10)+48;
smsbuffer[25]=(((corriente%1000)%100)%10)+48;
}
  strcat(smsbuffer,"mA\r\n");
  strcat(smsbuffer,"ID: 1510");
  sms.SendSMS(destino, smsbuffer);
  okcorriente=1;
  }
 }
   
  if((corriente<imax)&&(okcorriente==1)){
  okcorriente=0;
 }
 
   mido();
   checkSMS();

if((okred==1)&&(red>vmax)){
  okred=0;
  okbt=0;
  on=0;
  espera=0;
    if(z==0){       
      sms.SendSMS(destino, "Servicio electrico reestablecido.  ID: 1510");
    }
   else{   
  if(espera==0){
    sms.SendSMS(destino, "Servicio electrico reestablecido. 1 minuto reconexion.  ID: 1510"); 
    espera=1;
    okterm=0;
    previousMillis=millis();  
  }
  
  while(espera==1){
    currentMillis = millis();
      
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
      
   if(z==1){   
     PORTD=B10000000;
     PORTB=0;
     z=0;    
     sms.SendSMS(destino, "Modulo Encendido.  ID: 1510");      
     espera=2;  
    }

   }
   else{
     mido();   
     checkSMS();  
   }
}
}
}
   mido();
   checkSMS();

if((termica>200)&&(okterm==1)){
  sms.SendSMS(destino, "Llave termomagnetica corregida.  ID: 1510");
  okterm=0;
  okred=0;
  okbt=0;
  okcorriente=0;
}                  
}
}   

     
void checkSMS(void){
  sms_position=sms.IsSMSPresent(SMS_UNREAD);
        if (sms_position) 
        {          
            sms.GetSMS(sms_position, phone_number, sms_text, 100);  
            if((ok==0)||(ok==2)){
        if(strcmp(sms_text,"Destino")==0)
     {
     strcpy(destino,phone_number);
     sms.DeleteSMS(sms_position);     
     sms.SendSMS(destino, "Numero de Destino Guardado.  Equipo Nro: 1510");

      for(i=0;i<21;i++)
       {
           phone_number[i]=0;

       }
       
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       } 
     if(ok==2){
     ok=3;  
     }
     else{
       ok=1;
     }
     mido();
     estadosms();
     return;
    }  
            
    else{
         for(i=0;i<21;i++)
       {
           phone_number[i]=0;

       }

        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }         
    }
            }
            if(ok==1){
  if(strcmp(phone_number,destino)==0){ 
     
     if(strcmp(sms_text,"Destino")==0)
     {
        
        for(i=0;i<21;i++)
       {
           destino[i]=0;
       }   

      strcpy(destino,phone_number);
     sms.SendSMS(destino, "Numero de Destino Guardado.  Equipo Nro: 1510");
     mido();
     estadosms();
     sms.DeleteSMS(sms_position);

        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }     
     }
     
    if(strcmp(sms_text,"Apagar")==0){
       if(z==0){
        PORTD=B00000100;
        PORTB=1;
        sms.SendSMS(destino, "Modulo Apagado.  ID: 1510");     
        z=1;
        sms.DeleteSMS(sms_position);
       }
       else{
        sms.SendSMS(destino, "El modulo se encuentra apagado.  ID: 1510");
        sms.DeleteSMS(sms_position);
       }
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }  
    }
     
  if(strcmp(sms_text,"Resetear")==0){ 
        
        while(z==1){
        sms.SendSMS(destino, "El modulo se encuentra apagado.  ID: 1510");     
        sms.DeleteSMS(sms_position);

        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }  
       return;        
        }
       
       PORTD=B00000100;
       PORTB=1;
       
       if(z==0){ 

      
      currentMillis = millis();
  
      mido();   
      checkSMS();
      
  while(currentMillis - previousMillis > 900) {
    previousMillis = currentMillis;
        PORTD=B10000000;
        PORTB=0;  
        sms.DeleteSMS(sms_position);
        sms.SendSMS(destino, "Modulo Reseteado.  ID: 1510"); 
        
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }
       return; 
}  
  
}     
      
      } 

     if(strcmp(sms_text,"Estado")==0){ 
        sms.DeleteSMS(sms_position);
        mido();
        estadosms();
        
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       } 
           
      }
      
     if(strcmp(sms_text,"Encender")==0){
       while(z==0){
        sms.SendSMS(destino, "El modulo se encuentra encendido.  ID: 1510");
        sms.DeleteSMS(sms_position);
       
       for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }
       return;
       }

       while((ok==2)||(ok==3)){
          sms.DeleteSMS(sms_position);          
          sms.SendSMS(destino, "Modulo iniciando. Aguarde por favor. ID: 1510");         
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }     
       return;
   }
       while((red<190)){
          sms.DeleteSMS(sms_position);          
          sms.SendSMS(destino, "Hay una falla en red electrica. Responda Si para encender el modulo ID: 1510");
          on=1;
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       } 
       return;
        }
       
       while(espera==1){   
          sms.DeleteSMS(sms_position);
          sms.SendSMS(destino, "Reconexion automatica en 1 minuto. Responda Si para encender el modulo  ID: 1510");
          on=1;
          espera=0;
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }   
       return;
        } 
        
        while((on==0)||(z==1)){
        PORTD=B10000000;
        PORTB=0;        
        sms.DeleteSMS(sms_position);
        sms.SendSMS(destino, "Modulo Encendido.  ID: 1510");
        z=0;
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       } 
       return;
        }
        
     }

      if((on==1)&&(strcmp(sms_text,"Si")==0)){
        while(z==1){
        PORTD=B10000000;
        PORTB=0;         
        sms.DeleteSMS(sms_position);
        sms.SendSMS(destino, "Modulo Encendido.  ID: 1510");
        on=0;
        z=0;
        okbt=0;
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }  
       return;       
       }
      while(z==0){
       
        sms.DeleteSMS(sms_position);
       for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }
       return;
      } 
      }

      if(strncmp(sms_text,"Vbt",3)==0){
          for(i=0;i<3;i++)
       {
           sms_text[i]=0;
       }  
          for(i=0;i<3;i++)
       {
           sms_text[i]=sms_text[i+4];
       }  
        sms_text[3]='\0';        
        vmax=atoi(sms_text);
        okbt=0;
        sms.DeleteSMS(sms_position);
        sms.SendSMS(destino, "Nivel de baja tension seteado.  ID: 1510");
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }  
      }

      if(strncmp(sms_text,"Imax",4)==0){
          for(i=0;i<4;i++)
       {
           sms_text[i]=0;
       }  
          for(i=0;i<4;i++)
       {
           sms_text[i]=sms_text[i+5];
       }  
        sms_text[4]='\0';        
        imax=atoi(sms_text);
        okcorriente=0;
        okbt=0;
        sms.DeleteSMS(sms_position);
        sms.SendSMS(destino, "Valor limite de corriente seteado.  ID: 1510");
        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }  
      }

  }
      
      else{

           sms.DeleteSMS(sms_position);

        for(i=0;i<100;i++)
       {
           sms_text[i]=0;
       }  
      }
        }
        }
        
        return;
}
             
void estadosms (void){
for(i=0;i<161;i++){
  smsbuffer[i]=0;
}
strcpy(smsbuffer,"RED=");
smsbuffer[4]=(red/100)+48;
smsbuffer[5]=((red%100)/10)+48;
smsbuffer[6]=((red%100)%10)+48;
strcat(smsbuffer,"V\r\n");
strcat(smsbuffer,"Corriente=");
strcat(smsbuffer,"Humedad=");
smsbuffer[28]=humedad;
strcat(smsbuffer,"%\r\n");
if(corriente/1000==0){
  smsbuffer[32]=(corriente/100)+48;
  smsbuffer[33]=((corriente%100)/10)+48;
  smsbuffer[34]=((corriente%100)%10)+48;
}
else{
smsbuffer[35]=(corriente/1000)+48;
smsbuffer[36]=((corriente%1000)/100)+48;
smsbuffer[37]=(((corriente%1000)%100)/10)+48;
smsbuffer[38]=(((corriente%1000)%100)%10)+48;
}
strcat(smsbuffer,"mA\r\n");
if(z==0){
strcat(smsbuffer,"Modulo Activado\r\n");  
}
if(z==1){
strcat(smsbuffer,"Modulo Desactivado\r\n"); 
}
strcat(smsbuffer,"ID:1510");
sms.SendSMS(destino, smsbuffer);
return;
}

void mido (void){
 i=0;
 lectura=0;
 corr=0;
 accumrms=0;
 rms=0;
 red=0;
 termica=0;
humedad=dht.readHumidity();
red=((analogRead(4)*5)/1023)*74;
termica=((analogRead(5)*5)/1023)*74;
 long c_timer=micros();
 while(i<30000)
{
  //ups=(analogRead(5))/2;
  lectura=((analogRead(0)-507)*50.0)/1023.0;
  corr=corr+(lectura*lectura);
  i++;
  while((micros()-c_timer)<200);
  c_timer=micros();
}

     rms=(sqrt(corr/55000.00))*1000.00;

     if(red==0){
      rms=0;
     }
     if(z==1){
      rms=0;
     }
     if(okred==1){
      rms=0;
     }
     if(okterm==1){
      rms=0;
     }
     if(rms<=0055){  
     rms=0;
     }
     corriente=rms;
     

 return;
}
