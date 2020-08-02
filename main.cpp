 
/*  ====================================================

bibliotheque
C:\Users\fblai\.platformio\lib\M5Stack\examples\Modules\W5500\Ethernet2.zip\Ethernet2\src\utility

  *******************************************
  *  projet SWID_M5 ( Switch ID pour M5Stack)        
  *******************************************

          version 0.3

  date debut :  1 aout 2020 
  MODIF      :  2 aout    2020

  programmation: Francois Blais Montreal QC

  capture de trame pour la decouverte de commutateur (lldp)

  materiels
  ---------
  M5stack core  4meg prog   1.2 meg  ram 
  shield ethernet w5500
  ecran affichage avec un OLED  integer au bibli m5stack (esp32)
   
 bibliotheques
 -------------
 chemin de bibli shield w5500 lecture en mode raw   C:\Users\fblai\OneDrive\Documents\Arduino\libraries\Ethernet\src\utility

 C:\Users\fblai\OneDrive\Documents\Arduino\libraries
 ===============  problem  ================================ 
conflit i2c  et spi   ou bibliotheque  ( adafruit 1306 ??? )   jai pris ssd1306ascii

   module a jouter  un batterie et autre lan rj45

  ==================================================== */

 /* ====================================================
                 bibli
 ==================================================== */

#include <Arduino.h>
#include <M5Stack.h>       
#include <Ethernet2.h>
#include <utility/w5500.h>



  /* ====================================================
                 affichage 
 ==================================================== */



String LCD_data[7];

void presentation(String L1,String L2,String L3,String L4,String L5,String L6,String L7,String L8){
     M5.Lcd.clear();
     M5.Lcd.setCursor(70, 20, 4);
     M5.Lcd.println(L1);       
     M5.Lcd.setCursor(70, 60, 4);
     M5.Lcd.println(L2);      
     M5.Lcd.setCursor(70, 80, 4);
     M5.Lcd.println(L3);     
     M5.Lcd.setCursor(70, 100, 4);
     M5.Lcd.println(L4); 
     M5.Lcd.setCursor(70, 120, 4);
     M5.Lcd.println(L5);      
     M5.Lcd.setCursor(70, 140, 4);
     M5.Lcd.println(L6);      
     M5.Lcd.setCursor(70, 160, 4);
     M5.Lcd.println(L7);     
     M5.Lcd.setCursor(70, 180, 4);
     M5.Lcd.println(L8); 
 } 

 
  /* ====================================================
                   ethernet
 ==================================================== */

SOCKET s; // the socket that will be openend in RAW mode
 
int rbuflen ;

 byte  trame[335];
 
byte mac[] = {  0xFB, 0xAA, 0xBB, 0xFB, 0xDE, 0xFB };
int NBR = 0;
int position[12];

  /* ====================================================
                 declaration de mes fonctions
 ==================================================== */

bool si_lldp();
void lldp();
String decoupe( unsigned int local  );  
int trouve( const byte rech[] );
String decoupevlan( unsigned int local  );
int* trouvetlv();
String decompte;
void matrame();
String x2i(String monhexa);

// ========================================================================================================
// =============== fonctionne au debut du script seulement one shot ============================================
// ========================================================================================================
 
void setup()
{
  Serial.begin(115200);

    M5.begin(); 
    M5.Power.begin();
    
    M5.Lcd.setTextFont(4);
    M5.Lcd.setCursor(50, 100, 4);

 // ============ initialise lecran oled ===================

   
   presentation( "SWID-M5 v0.3 " ,    " "  ,"Capteur Ethernet", "" ,"Trame LLDP " ," "," ","F.Blais MTL 2020" );
  delay(5000); 
  
 // ============ initialise ethernet en mode raw brodcast ===================  
 

  w5500.init();
  w5500.writeSnMR(s, SnMR::MACRAW);
  w5500.execCmdSn(s, Sock_OPEN);
}
// ===================================================================================================== 
// ==================== en boucle continue ============================================================== 
// ===================================================================================================== 
void loop()
{
 NBR++;   
 
// ===================== info sur la switch  =======================
  

Serial.println(decompte);


   rbuflen = w5500.getRXReceivedSize(s);



    if(rbuflen>0) {
    if(rbuflen > sizeof(trame))
    rbuflen = sizeof(trame);
    w5500.recv_data_processing(s, trame, rbuflen);
    w5500.execCmdSn(s, Sock_RECV);
 }



 decompte = "  b:" + String(NBR) + "   g:" +  String(rbuflen) ;

 

if ( rbuflen > 0 ) {
    if (si_lldp()) {
       lldp();
       presentation( "" , LCD_data[0],LCD_data[1] ,LCD_data[2] ,LCD_data[3] , LCD_data[4] ,LCD_data[5], LCD_data[6]);
      delay(30000);
    }
 }

}
//  ===============================================================================================
//  ==================================  mes fonctions    ====================================
//  ===============================================================================================

void lldp(){
 
   int local=0;
   String partie;
   trouvetlv();     

 //   description 
  partie = decoupe(   position[5] ) ;
  LCD_data[1] =  partie.substring(1,14);
  LCD_data[2] =  partie.substring(15,partie.length() );
  
  
// ==============

 // equivalent Unit/port
LCD_data[4] = decoupe(position[4] ) ;
LCD_data[4].trim();
 

// ===============
   // equivalent 00-80-c2-01 signature de la ligne vlan   pos:187   0x0bfc = 3068
  byte vlan[]={0x00,0x80,0xc2,0x01}; 
  local = trouve(vlan);

  if (  local < 330 )
    LCD_data[6] = "Vlan:"  + decoupevlan(local) ;
  else
   LCD_data[6] = "   pas Vlan"  ;

 
}
// ============================================= 
bool si_lldp() {
   
 if(   trame[2]  == 1 &&  trame[3]  == 128 &&  trame[4]  == 194 &&  trame[7]  == 14 )       {
      matrame();
       return true;       
      } 
    else   
       return false;    
}
//===========================================================
void matrame()
{
  for(int x = 0 ; x< 334 ; x++)
 {  
   Serial.print(trame[x]);   Serial.print(" ");
 } 
 Serial.println("");
 for(int x = 0 ; x< 334 ; x++)
 {  
   Serial.print(trame[x],HEX);    
 } 
 Serial.println("");
}
//  ========================================================
//
//  EXTRAIT UN MORCEAU EN HEXA ET LE RENVOI EN STRING

 String decoupe( unsigned int local  ) {
 unsigned int i;
 int grandeur = trame[local+1] ;                                                           
  String texte=" "; 
  for ( i = ( local + 2 )   ; i < (local + grandeur + 2)  ; i++  ) 
        texte +=   (char)trame[i] ;
  return texte;  
}
//  ========================================================
//
//  EXTRAIT UN MORCEAU EN HEXA ET LE RENVOI EN STRING

 String decoupevlan( unsigned int local  ) {
 unsigned int i;
 int grandeur = trame[local-1] ;
 grandeur = grandeur - 4 ;
 
  String texte ; 
  for ( i = (local+4)   ; i< ( (local+4) + ( grandeur) )   ; i++  ) 
    texte += String(trame[i],HEX);
  texte = x2i(texte);
  texte.trim();
  return texte;  
}
//  ========================================================
//
//  RECHERCHE UN MORCEAU SPECIFIQUE DANS LA TRAME
//
// =======================================================
 int trouve(const byte rech[] ) {
  unsigned int i = 0;
  for ( i = 0 ; i < 330  ; i++ ) {
    if (trame[i] == rech[0])
      if( (trame[i+1] == rech[1]) and (trame[i+2] == rech[2]) and (trame[i+3] == rech[3]) )
      return i;
       }  
       return 0;       
  }  
  // =================================================================
  // trouve les deplacements 
  // 
 int* trouvetlv(){
       int nbr;
      int pos = 16;       position[0]=0;   position[1]=16;    
   
  for ( nbr = 2 ; nbr < 9  ; nbr++ ) {
  
    position[nbr] =  ( (pos + 2) + trame[pos+1]   );
    pos  = position[nbr];
 
   }
    return position ;
 }
// ======================================================================
   String print_ip(const byte a[], unsigned int offset, unsigned int length) {
  String ip;
  for (unsigned int i = offset; i < offset + length; i++) {
    //    if(i>offset) Serial.print('.');
    //   Serial.print(a[i], DEC);
    if (i > offset) ip = ip + '.';
    ip = ip + String (a[i]);
  }
   
  return ip;
}
// ======================================================================
String print_mac(const byte a[], unsigned int offset, unsigned int length) {
  String Mac;
  
  LCD_data[1] = "";
  for (unsigned int i = offset; i < offset + length; ++i) {

    if (i > offset) {
      //  LCD_data[1] = LCD_data[1] + Mac + ':';
      Mac = Mac + ':';
    }
    if (a[i] < 0x10) {
      Mac = Mac + '0';
      //    LCD_data[1] = LCD_data[1] + Mac + '0';
    }
    Mac = Mac + String (a[i], HEX);
  }
  LCD_data[1] = LCD_data[1]  + Mac;
  return Mac;
}
//  ====================================================
//
//  converti hexa vers dec en type string
//
//  ====================================================
String x2i(String monhexa)
{
  int x = 0; int nbr;
  for(nbr=0 ; nbr < monhexa.length() ; nbr++) {
    
    if (monhexa[nbr] >= '0' && monhexa[nbr] <= '9') {
      x *= 16;
      x += monhexa[nbr] - '0';
    }
    else if (monhexa[nbr] >= 'A' && monhexa[nbr] <= 'F') {
      x *= 16;
      x += (monhexa[nbr] - 'A') + 10;
    }
    else if (monhexa[nbr] >= 'a' && monhexa[nbr] <= 'f') {
      x *= 16;
      x += (monhexa[nbr] - 'a') + 10;
    }
    else break;
  }
  return String(x,DEC);
}
 // ================= fin script =====================================================================================================  
 // ================= fin script =====================================================================================================  
 // ================= fin script =====================================================================================================  
