#include "stm32f10x.h"
 
#include <stdio.h>
#include "./led/bsp_led.h" 
#include "./Beep/bsp_beep.h"
#include "./key/bsp_exti.h"
#include "./i2c/bsp_i2c_gpio.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./usart/bsp_usart.h"
#include "./SysTick/bsp_SysTick.h"
#include "./key/bsp_key.h"  
#define movedown 5
#define moveup 10
#define width 20  //ǽ��
u8 start = 0; //��Ϸ��ʼ��ʶ
extern mk;
char dispBuff[100];
struct Bird
{
	u8 x; //x����
	int y; //y����
	u8 radius; //�뾶
	u8 up; //��������
}bird;
struct Game
{
	int score;
	u8 flag;
}game;
struct Wall
{
	int x;
	int top;
  int down;
}wall;

void animation(void);
void play(void);
int gameover(void);
void LCD_Test(void);
void init_wall(void);
static void Delay ( __IO uint32_t nCount ) //��ʱ
{
	 for ( ; nCount != 0; nCount -- );
}

void KEY1_IRQHandler(void)  //�����·�
{
  //ȷ���Ƿ������EXTI Line�ж�
	if(EXTI_GetITStatus(KEY1_INT_EXTI_LINE) != RESET) 
	{
		// LED1 ȡ��		
		if(start==0)
		{
			start=1;
			EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);  
			return;
		}
		else
		{
			LCD_SetTextColor(BLACK)	;      
			ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
			
			bird.y+=movedown;
			LCD_SetTextColor(RED);
			ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
		}
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);   
	}  
}
void KEY2_IRQHandler(void)  //�����Ϸ�
{
  //ȷ���Ƿ������EXTI Line�ж�
	if(EXTI_GetITStatus(KEY2_INT_EXTI_LINE) != RESET) 
	{
		bird.up=1;
		LCD_SetTextColor(BLACK)	;   
		ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
		
		bird.y-=moveup;
		LCD_SetTextColor(RED);
		ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
    //����жϱ�־λ
		EXTI_ClearITPendingBit(KEY2_INT_EXTI_LINE);     
	}  
}
void init_wall()
{
	wall.x=240;
	wall.top=160-width;
	wall.down=wall.top+2*width;
}

int gameover()
{
	if(bird.y<40||bird.y>280)
		return 1;
	else if(bird.x==wall.x&&bird.y-bird.radius>wall.top&&bird.y+bird.radius<wall.down)
		return 0;
	else if(bird.x!=wall.x)
		return 0;
	
	return 1;
}
void play()
{
	bird.up=0;  //bird
	bird.radius=5;
	bird.x=80;
	bird.y=160;
	game.score=0; //��ʼ��game
	game.flag=1;
	
	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	
	
	LCD_SetTextColor(RED);  //bird��ʼ��
	ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
	
	init_wall();
	while(1)
	{
		LCD_SetColors(CYAN,BLACK);
		sprintf(dispBuff,"����: %d        Flappy Bird",game.score);
		ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
		
		LCD_SetColors(WHITE,BLACK);  //��Ϸ��
		ILI9341_DrawRectangle(0,40,240,240,0);  
				
		LCD_SetTextColor(BLACK)	;       //bird�ƶ�
		ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
		bird.y+=movedown;
		LCD_SetTextColor(RED);
		ILI9341_DrawCircle(bird.x,bird.y,bird.radius,1);
		
		wall.x-=20;
		LCD_SetTextColor(GREEN);
		ILI9341_DrawRectangle(wall.x,40,0,wall.top-40,0);
		LCD_SetTextColor(GREEN);
		ILI9341_DrawRectangle(wall.x,wall.down,0,280-wall.down,0);

		if(wall.x<=40)  //�µ�wall
		{
			LCD_SetTextColor(BLACK); 
			ILI9341_DrawRectangle(wall.x,0,0,400,0);
			wall.x=240;
			wall.top=rand()%160+40;
			wall.down=wall.top+2*width;
			game.score+=10;
		}
	
  	Delay(0x2FFFFF);
		LCD_SetTextColor(BLACK);       //wall�ƶ�
		ILI9341_DrawRectangle(wall.x,0,0,400,0);
		bird.up=0;
		
		if(game.score>150)
		{
			LCD_SetColors(RED,BLACK);
			sprintf(dispBuff,"     ��ϲͨ�أ�����");
			ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
			break;
		}
			
		
		if(gameover())  //��Ϸ����
		{
			ILI9341_DispStringLine_EN(LINE(1),"          game over!");
			macBEEP_ON ();
			Delay(0x0FFFFF);
			macBEEP_OFF();
			Delay(0x0FFFFF);
			macBEEP_ON();
			Delay(0x0FFFFF);
			macBEEP_OFF();
			Delay(0x0FFFFF);
			macBEEP_ON();
			Delay(0x0FFFFF);
			macBEEP_OFF();
			
			ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);        //����
			start=0;
			break;
		}
	}
}


int main ( void )
{
	ILI9341_GramScan (6);    //ѡ����Ļɨ�跽ʽ

  ILI9341_Init();           //ILI9341��ʼ������
  LED_Init();               //LED��ʼ��
	Beep_Init();              //BEEP��ʼ��
	USART_Config();	           //USART GPIO ����,������������
  Key_GPIO_Config();         //���ð����õ���I/O��
  SysTick_Init();            //����ϵͳ�δ�ʱ�� SysTick
  LCD_SetFont(&Font8x16);    //����Ӣ����������
	
	LCD_SetColors(YELLOW,BLACK);  //����LCD��ǰ��(����)��������ɫ,RGB565
  EXTI_Key_Config();         //����IOΪEXTI�жϿڣ��������ж����ȼ�
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);    //��ILI9341��ʾ����ĳһ������ĳ����ɫ��������
	while ( 1 )
	{ 
		macLED1_ON();
		if(start==0)
		{
			animation();  //��������
			continue;
		}
		macLED1_OFF();
		macLED2_ON();
		ILI9341_DrawRectangle(0,0,240,270,0);            //�� ILI9341 ��ʾ���ϻ�һ������
		play();
		macLED2_OFF();
	}
}

void animation()
{
char dispBuff1[200];
	
	LCD_Test();
	LCD_SetColors(YELLOW,BLACK);
	ILI9341_DrawRectangle(0,40,240,240,0);
	Delay(0xF0000);
	LCD_SetColors(WHITE,BLACK);
	ILI9341_DrawRectangle(10,50,220,220,0);
	Delay(0xF0000);
	LCD_SetColors(BLUE,BLACK);
	ILI9341_DrawRectangle(20,60,200,200,0);
	Delay(0xF0000);
	LCD_SetColors(RED,BLACK);
	ILI9341_DrawRectangle(30,70,180,180,0);
	Delay(0xF0000);
	LCD_SetColors(MAGENTA,BLACK);
	ILI9341_DrawRectangle(40,80,160,160,0);
	Delay(0xF0000);
	LCD_SetColors(GREEN,BLACK);
	ILI9341_DrawRectangle(50,90,140,140,0);
	Delay(0xF0000);
	LCD_SetColors(CYAN,BLACK);
	ILI9341_DrawRectangle(60,100,120,120,0);
	Delay(0xF0000);
	LCD_SetColors(RED,BLACK);
	ILI9341_DrawRectangle(70,110,100,100,0);
	Delay(0xF0000);
	LCD_SetColors(GREY,BLACK);
	ILI9341_DrawRectangle(80,120,80,80,0);
	Delay(0xF0000);
	LCD_SetColors(GBLUE,BLACK);
	ILI9341_DrawRectangle(90,130,60,60,0);
	Delay(0xF0000);
	
	LCD_SetTextColor(GREEN);
	sprintf(dispBuff1,"       ��K1����ʼ��Ϸ......");
	ILI9341_DispStringLine_EN_CH(LINE(18),dispBuff1);
	Delay(0xFFFFF);
}

void LCD_Test(void)
{
	char dispBuff[200];
	
	LCD_SetTextColor(GREEN);
	sprintf(dispBuff,"       Flappy Bird��Ϸ");
	ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
	Delay(0xFFFFF);
}

