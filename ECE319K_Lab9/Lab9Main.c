// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Fatima Younus and Dayana Romo
// Last Modified: 12/31/2023

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
uint32_t language=0;// eng =0 span=1
uint32_t gamescore=0;
uint32_t DEATHFLAG=0;
uint32_t RESTARTFLAG=0;
uint32_t PERSONINDEX=0;
uint32_t M=1;
static uint32_t GULPFLAG=0;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

typedef enum {rotten,good} status_t;
typedef enum {caught,notcaught} alive_t;
struct food{
    uint32_t x, oldx, y, oldy; // coordinates
    uint32_t vy;
    const uint16_t *image; // ptr->image
    status_t edibleness; // dead/alive
    alive_t caughtornotcaught;
};
typedef struct food food_t;

food_t FOODS[6]={
  {0,16,0,0,1, cakeslice,good,notcaught},
{80,16,0,0,0, rottenpizza,rotten,notcaught},
{40,16,0,0,1, burger,good,notcaught},
{60,16,0,0,0, rottencake,rotten,notcaught},
{20,16,0,0,1, pizza,good, notcaught},
{100,16,0,0,0, rottenburger, rotten,notcaught}
};

typedef enum {alive,dead} stat_t;
struct person{
    uint32_t capacity; //for next state
    uint32_t x, oldx, y, oldy;
    const uint16_t *image; // ptr->image
    stat_t life; // dead/alive
};
typedef struct person person_t;

person_t Person[3]={
    {2,0,0,160,0, maincharacteralive,alive},//
    {2,0,0,0,0, IMFULL,alive},
    {0,0,0,160,0, DEADMAN,dead}

};


void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

static int ChokingCharFlag=0;
static int NeedToDrawChar=0;
static int NeedToDrawFood=0;
static int DeleteFoodSprite=0;
static int SpriteToDelete=0;
static int FoodAtBottom=0;
static int SpriteAtBottom=0;
int globalfood=0;
int PAUSEFLAG=0;
int PREVPAUSEFLAG=0;


void englishinstructions(void) {
    ST7735_FillScreen(0x0000);   // set screen to black
     ST7735_SetCursor(1, 1);
    ST7735_OutString("Hi, my name is BOB!");
    ST7735_SetCursor(1, 2);
    ST7735_OutString("IM SO HUNGRY!");
    ST7735_SetCursor(1, 3);
    ST7735_OutString("Please feed me");
    ST7735_SetCursor(1, 4);
    ST7735_OutString("help me get food in");
    ST7735_SetCursor(1, 5);
    ST7735_OutString("my mouth and make");
    ST7735_SetCursor(1, 6);
    ST7735_OutString("sure I dont choke ");
    ST7735_SetCursor(1, 7);
    ST7735_OutString("by pressing B3. I" );
    ST7735_SetCursor(1, 8);
    ST7735_OutString("will only swallow" );
    ST7735_SetCursor(1, 9);
    ST7735_OutString("when mouth is full" );
    ST7735_SetCursor(1, 10);
     ST7735_OutString("Hold B4 to" );
    ST7735_SetCursor(1, 11);
    ST7735_OutString("teleport" );
       ST7735_SetCursor(1, 12);
     ST7735_OutString("Press B3 to" );
     ST7735_SetCursor(1, 13);
     ST7735_OutString("continue" );

}
void spanishinstructions(void){
    ST7735_FillScreen(0x0000);   // set screen to black
       ST7735_SetCursor(1, 1);
       ST7735_OutString("Hola, me llamo BOB!");
       ST7735_SetCursor(1, 2);
        ST7735_OutString("TENGO MUCHA HAMBRE!");
       ST7735_SetCursor(1, 3);
        ST7735_OutString("Porfavor dame de");
        ST7735_SetCursor(1, 4);
       ST7735_OutString(" comer. Ayudame a");
       ST7735_SetCursor(1, 5);
       ST7735_OutString("meter comida en mi");
       ST7735_SetCursor(1, 6);
       ST7735_OutString("boca y asegurate de");
       ST7735_SetCursor(1, 7);
        ST7735_OutString("no ahogarme" );
        ST7735_SetCursor(1, 8);
      ST7735_OutString("presionando B3.");
      ST7735_SetCursor(1, 9);
      ST7735_OutString("Solo tragare cuando" );
      ST7735_SetCursor(1,10);
       ST7735_OutString("tenga la boca llena");
       ST7735_SetCursor(1, 10);
           ST7735_OutString("Manten presionado" );
          ST7735_SetCursor(1, 11);
          ST7735_OutString("B4para teleportarme" );
       ST7735_SetCursor(1,12);
      ST7735_OutString("presiona B3 para");
      ST7735_SetCursor(1,13);
      ST7735_OutString("continuar");}

static uint32_t collisioncounter=0;


void die(void){
    uint32_t data;
    gamescore = gamescore + collisioncounter;
     if (language==0){
         ST7735_FillScreen(0x0000);   // set screen to black
               ST7735_SetCursor(1, 1);
               ST7735_OutString("I died! :(");
               ST7735_SetCursor(1, 2);
                ST7735_OutString("WHY DID ");
               ST7735_SetCursor(1, 3);
                ST7735_OutString("YOU KILL ME!");
                ST7735_SetCursor(1, 4);
               ST7735_OutString(" at least I died");
               ST7735_SetCursor(1, 5);
               ST7735_OutString("doing what");
                 ST7735_SetCursor(1, 6);
                ST7735_OutString("I loved");
                ST7735_SetCursor(1, 7);
                ST7735_OutString("score:");
                ST7735_SetCursor(60,7);
                ST7735_OutUDec(gamescore);
                ST7735_SetCursor(1, 8);
                   ST7735_OutString(" goodbye!");
                     ST7735_SetCursor(1, 9);

         //ST7735_DrawBitmap(0, 159, DEADSCREEN, 128,160);
         Clock_Delay1ms(3000);
         RESTARTFLAG=1;
         }

     else if (language==1){
         ST7735_FillScreen(0x0000);   // set screen to black
                      ST7735_SetCursor(1, 1);
                      ST7735_OutString("Me Mori! :(");
                      ST7735_SetCursor(1, 2);
                       ST7735_OutString("PORQUE ME ");
                      ST7735_SetCursor(1, 3);
                       ST7735_OutString("MATATES!");
                       ST7735_SetCursor(1, 4);
                      ST7735_OutString("Por lo menos");
                      ST7735_SetCursor(1, 5);
                      ST7735_OutString("me mori hacido");
                        ST7735_SetCursor(1, 6);
                       ST7735_OutString("lo que amo");
                       ST7735_SetCursor(1,7);
                       ST7735_OutString("Puntos:");
                       ST7735_SetCursor(60,7);
                          ST7735_OutUDec(gamescore);
                       ST7735_SetCursor(1, 8);
                          ST7735_OutString(" adios!");
                            ST7735_SetCursor(1, 9);
                //ST7735_DrawBitmap(0, 159, DEADSCREEN, 128,160);
                Clock_Delay1ms(3000);
     RESTARTFLAG=1;}}



uint8_t FOODFLAGS[6] ={1,0,0,0,0,0};
uint32_t foodindex=0;

void MoveFood(void){
    if ((FOODFLAGS[0] !=0) && (FOODS[0].caughtornotcaught != caught)){
        FOODS[0].y=(FOODS[0].y)+2;}

        if (FOODS[0].y>=50){
            foodindex=1;
            FOODFLAGS[foodindex]=1;
        }
    if ((FOODFLAGS[1] !=0) && (FOODS[1].caughtornotcaught != caught)){
        FOODS[1].y=(FOODS[1].y)+2;}
        if (FOODS[1].y>=50){
            foodindex=2;
          FOODFLAGS[foodindex]=1;}
    if ((FOODFLAGS[2] !=0) && (FOODS[2].caughtornotcaught != caught)){
        FOODS[2].y=(FOODS[2].y)+2;}
        if (FOODS[2].y>=50){
            foodindex=3;
            FOODFLAGS[foodindex]=1;}
     if ((FOODFLAGS[3] !=0) && (FOODS[3].caughtornotcaught != caught)){
            FOODS[3].y=(FOODS[3].y)+2;}
            if (FOODS[3].y>=50){
                foodindex=4;
                FOODFLAGS[foodindex]=1;}
      if ((FOODFLAGS[4] !=0)&& (FOODS[4].caughtornotcaught != caught)){
              FOODS[4].y=(FOODS[4].y)+2;}
              if (FOODS[4].y>=50){
                  foodindex=5;
               FOODFLAGS[foodindex]=1;}
       if ((FOODFLAGS[5] !=0) && (FOODS[5].caughtornotcaught != caught)){
               FOODS[5].y=(FOODS[5].y)+2;}
                 if (FOODS[5].y>=50){
                     foodindex=0;
                 FOODFLAGS[foodindex]=1;}



    NeedToDrawFood=1;
}

uint32_t olddata=0;
int jitterflag=0;
// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg, data2;
  if((TIMG12->CPU_INT.IIDX) == 1) // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    //ADCin();
    // 1) sample slide pot  (going to do this inside the character x thing
    data2=Switch_In();
        if (data2 & 0x08){//PB17
            PAUSEFLAG=1;

            return;}

    uint32_t data = ADCin();
//    if(   ((data+10)<=(olddata))   ||   ((olddata) <= (data-10))   ){
//        jitterflag=1;
//    }
//    olddata=data;
//
//    uint32_t characterX;
//    if(jitterflag!=0){
//        characterX=characterX;// 3) move sprites
//        Person[0].x=characterX;
//        NeedToDrawChar=1;
//        if (characterX>=100){
//            characterX=100;
//        }
//    }
//    else{
//        characterX=(120*(data)/4095);// 3) move sprites
//        Person[0].x=characterX;
//        NeedToDrawChar=1;
//        if (characterX>=100){
//            characterX=100;
//        }
//    }
//    jitterflag=0;
//    //MoveFood();
    uint32_t characterX=(120*(data)/4095);// 3) move sprites
      if (characterX>=100){
          characterX=100;
      }
      //if (characterX>=10){
       //     characterX=10;
        //}
      Person[0].x=characterX;
      NeedToDrawChar=1;
      MoveFood();

    uint32_t data3 = Switch_In(); // one of your buttons
    if (ChokingCharFlag==1 && ((data3 == 16))){
        GULPFLAG=1;
        gamescore+=collisioncounter;
        collisioncounter=0;
        ChokingCharFlag=0;
    }

    for(int i=0; i<6;i++){
        if(FOODS[i].y >= 120) {
            if(((Person[0].x + 33)>= (FOODS[i].x)) && (((FOODS[i].x >= Person[0].x)))){

            DeleteFoodSprite=1;
            SpriteToDelete=i;

            if((ChokingCharFlag==1) && (Person[0].capacity) <collisioncounter){
                DEATHFLAG=1;
            }


            if(Person[0].capacity == collisioncounter){//check if character is choking
                ChokingCharFlag=1;
            }
        }

    }
    // 2) read input switches
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";

const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";

const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";

const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish},
  {Goodbye_English,Goodbye_Spanish},
  {Language_English,Language_Spanish}
};




static int TOUCHED0FLAG=0;
static int TOUCHED1FLAG=0;
static int TOUCHED2FLAG=0;
static int TOUCHED3FLAG=0;
static int TOUCHED4FLAG=0;
static int TOUCHED5FLAG=0;


// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
   while(1){
     RESTARTFLAG=0;

    uint32_t data=0;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  Switch_Init(); // initialize switches
    LED_Init();    // initialize LED
    DEATHFLAG=0;

  ST7735_FillScreen(ST7735_BLACK);


 ST7735_DrawBitmap(0, 159, TITTLESCREEN, 128,160);
 while ((data & 0x41) == 0)
 { data=Switch_In();}
 if ((data & 0x01) != 0){
 englishinstructions();}

else {if ((data & 0x40) !=0) {
 spanishinstructions();
         language=1;}
}
 while ((data & 0x10) == 0)
 { data=Switch_In();}
 ST7735_FillScreen(ST7735_BLACK);

  ADCinit();     //PB18 = ADC1 channel 5, slidepot

  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();



  while(RESTARTFLAG==0){
      while ((PAUSEFLAG != PREVPAUSEFLAG)){
          if (((data & 0x08) == 0))
           { data=Switch_In();
           }
          else{PREVPAUSEFLAG=PAUSEFLAG;
              PAUSEFLAG=0;}
      }

          if (DEATHFLAG==1){
              Sound_CHOKE();
             ( Person[2].x)=(Person[0].x);
              ST7735_DrawBitmap(Person[2].x,160, DEADMAN, 33,44);
          Clock_Delay1ms(3000);
              die();
              NeedToDrawFood=0;
              DeleteFoodSprite=0;
              NeedToDrawChar=0;
          }
         if(NeedToDrawFood==1){
             if (FOODS[0].caughtornotcaught != caught){
                 ST7735_DrawBitmap(FOODS[0].x,FOODS[0].y, cakeslice, 16,16);
                 if (FOODS[0].y>= 150){
                     ST7735_DrawBitmap(FOODS[0].x,FOODS[0].y, blankcharacter, 33,44);
                     FOODS[0].y= 0;
                      FOODS[0].x = (Random32()&0x5F);
                      FOODFLAGS[0]=1;

                 }
             }
             if (FOODS[1].caughtornotcaught != caught){
                 ST7735_DrawBitmap(FOODS[1].x,FOODS[1].y, pizza, 16,16);
                 if (FOODS[1].y>= 150){
                     ST7735_DrawBitmap(FOODS[1].x,FOODS[1].y, blankcharacter, 33,44);
                                 FOODS[1].y= 0;
                                  FOODS[1].x = (Random32()&0x5F);
                                  FOODFLAGS[1]=1;}
             }
             if (FOODS[2].caughtornotcaught != caught){
                 ST7735_DrawBitmap(FOODS[2].x,FOODS[2].y, burger, 16,16);
                 if (FOODS[2].y>= 150){
                     ST7735_DrawBitmap(FOODS[2].x,FOODS[2].y, blankcharacter, 33,44);
                                 FOODS[2].y= 0;
                                  FOODS[2].x = (Random32()&0x5F);
                                  FOODFLAGS[2]=1;}
             }
             if (FOODS[3].caughtornotcaught != caught){
                 ST7735_DrawBitmap(FOODS[3].x,FOODS[3].y, rottencake, 16,16);
                 if (FOODS[3].y>= 150){
                     ST7735_DrawBitmap(FOODS[3].x,FOODS[3].y, blankcharacter, 33,44);
                                 FOODS[3].y= 0;
                                  FOODS[3].x = (Random32()&0x5F);
                                  FOODFLAGS[3]=1;}
             }
             if (FOODS[4].caughtornotcaught != caught){
                 ST7735_DrawBitmap(FOODS[4].x,FOODS[4].y, rottenpizza, 16,16);
                 if (FOODS[4].y>= 150){
                     ST7735_DrawBitmap(FOODS[4].x,FOODS[4].y, blankcharacter, 33,44);
                FOODS[4].y= 0;
                 FOODS[4].x = (Random32()&0x5F);
                 FOODFLAGS[4]=1;}
             }
             if (FOODS[5].caughtornotcaught != caught){
                 ST7735_DrawBitmap(FOODS[5].x,FOODS[5].y, rottenburger, 16,16);
                 if (FOODS[5].y>= 150){
                     ST7735_DrawBitmap(FOODS[5].x,FOODS[5].y, blankcharacter, 33,44);
                        FOODS[5].y= 0;
                     FOODS[5].x = (Random32()&0x5F);
                     FOODFLAGS[5]=1;}
             }
             //NeedToDrawFood=0;
         }

     if(DeleteFoodSprite==1){
         if (SpriteToDelete==0){
             ST7735_DrawBitmap((FOODS[0].x),FOODS[0].y, blankcharacter, 33,44);
             if(TOUCHED0FLAG==0){
                 Sound_NOM();
                collisioncounter= collisioncounter+1;
                FOODS[0].y= 0;
                 FOODS[0].x = (Random32()&0x5F) + 2;
               FOODS[0].caughtornotcaught = notcaught;
               TOUCHED0FLAG=1;
               FOODFLAGS[0]=1;
              if  (FOODS[0].edibleness == rotten){ DEATHFLAG=1;}
            }
             else if (TOUCHED0FLAG==1){
             FOODFLAGS[0]=0;
             TOUCHED0FLAG=0;}

         }
         if (SpriteToDelete==1){
             ST7735_DrawBitmap((FOODS[1].x),FOODS[1].y, blankcharacter, 33,44);
             if(TOUCHED1FLAG==0){
                 Sound_NOM();
                collisioncounter= collisioncounter+1;
                FOODS[1].y= 0;
                        FOODS[1].x = (Random32()&0x5F) + 2;
                        FOODS[1].caughtornotcaught = notcaught;
                        FOODFLAGS[1]=1;
                         TOUCHED1FLAG=1;
              if  (FOODS[1].edibleness == rotten){ DEATHFLAG=1;}
            }
             else if (TOUCHED1FLAG == 1){
             FOODFLAGS[1]=0;
             TOUCHED1FLAG=0;}

         }
         if (SpriteToDelete==2){
              ST7735_DrawBitmap((FOODS[2].x),FOODS[2].y, blankcharacter, 33,44);
              if(TOUCHED2FLAG==0){
                  Sound_NOM();
                 collisioncounter= collisioncounter+1;
                 FOODS[2].y= 0;
                 FOODS[2].x = (Random32()&0x5F) + 2;
                 FOODS[2].caughtornotcaught = notcaught;
                 FOODFLAGS[2]=1;
                 TOUCHED2FLAG=1;
                 if  (FOODS[2].edibleness == rotten){ DEATHFLAG=1;}
             }}
              else if(TOUCHED2FLAG == 1){
              FOODFLAGS[2]=0;
              TOUCHED2FLAG=0;}
         if (SpriteToDelete==3){
              ST7735_DrawBitmap((FOODS[3].x),FOODS[3].y, blankcharacter, 33,44);
              if(TOUCHED3FLAG==0){
                  Sound_NOM();
                 collisioncounter= collisioncounter+1;
                 FOODS[3].y= 0;
                  FOODS[3].x = (Random32()&0x5F) + 2;
                  FOODS[3].caughtornotcaught = notcaught;
                  FOODFLAGS[3]=1;
                  TOUCHED3FLAG=1;
                  if  (FOODS[3].edibleness == rotten){ DEATHFLAG=1;}
             }
              else if (TOUCHED3FLAG==1){
              FOODFLAGS[3]=0;
              TOUCHED3FLAG=0;}
          }
         if (SpriteToDelete==4){
               ST7735_DrawBitmap((FOODS[4].x),FOODS[4].y, blankcharacter, 33,44);
               if(TOUCHED4FLAG==0){
                   Sound_NOM();
                  collisioncounter= collisioncounter+1;
                  FOODS[4].y= 0;
                  FOODS[4].x = (Random32()&0x5F) + 2;
                  FOODS[4].caughtornotcaught = notcaught;
                  FOODFLAGS[4]=1;
                  TOUCHED4FLAG=1;
                  if  (FOODS[4].edibleness == rotten){ DEATHFLAG=1;}
              }
               else if (TOUCHED4FLAG==1){
               FOODFLAGS[4]=0;
               TOUCHED4FLAG=0;}

           }
         if (SpriteToDelete==5){
               ST7735_DrawBitmap((FOODS[5].x),FOODS[5].y, blankcharacter, 33,44);
               if(TOUCHED5FLAG==0){
                   Sound_NOM();
                   collisioncounter= collisioncounter+1;
                   FOODS[5].y= 0;
                   FOODS[5].x = (Random32()&0x5F) + 2;
                   FOODS[5].caughtornotcaught = notcaught;
                   FOODFLAGS[5]=1;
                   TOUCHED5FLAG=1;
                   if  (FOODS[5].edibleness == rotten){ DEATHFLAG=1;}
               }
               else if (TOUCHED5FLAG==1){
                              FOODFLAGS[5]=0;
                              TOUCHED5FLAG=0;}

               FOODFLAGS[0]=1;}

//static int resetflags=0;
//               int i=1;
//               while(FOODFLAGS[i]==0){
//                   i++;
//                   if(i==5){
//                       resetflags=1;
//                       break;

               }

         DeleteFoodSprite=0;


     if(NeedToDrawChar ==1){
         if (GULPFLAG==1){
             Sound_GULP();
             GULPFLAG=0;
            Person[0].capacity = (Random32()&0x0F);
         }
         if(ChokingCharFlag==1){

             ST7735_DrawBitmap((Person[0].oldx),160, blankcharacter, 33,44);
             ST7735_DrawBitmap((Person[0].x),160, IMFULL, 33,44);
             Person[1].oldx=Person[1].x;
             NeedToDrawChar=0;
         }
         if(ChokingCharFlag==0){
              ST7735_DrawBitmap((Person[0].oldx),160, blankcharacter, 33,44);
              ST7735_DrawBitmap((Person[0].x),160, maincharacteralive, 33,44);
              Person[0].oldx=Person[0].x;
              NeedToDrawChar=0;
          }
     }
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}
}
