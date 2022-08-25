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
static void Delay ( __IO uint32_t nCount );
char dispBuff[100];

int up=5;
u8 flag=0;
u8 flag1=0;
u8 flag2=0;
u8 flag3=0;
u8 flag4=0;
void move(void);
void changestate(void);
int dead(void);


struct Body
{
	int x;
	int y;
	int radius;
	int onstage;
}body;

struct Stage
{
	int x;
	int y;
	u8 length;
}stage1,stage2,stage3,stage4,stage5;

struct buff
{
	int x;
	int y;
}buff1;

//��Ϸ�ȼ�����
struct Game
{
	int Score;//����
	u8 Life;//��Ϸ�ȼ�	
}game;

void KEY1_IRQHandler(void)  //������
{
  //ȷ���Ƿ������EXTI Line�ж�
	if(EXTI_GetITStatus(KEY1_INT_EXTI_LINE) != RESET)		
	{ 
		if(body.x > 0)
		{
			LCD_SetTextColor(BLACK);       
			ILI9341_DrawCircle(body.x,body.y,body.radius,1);
			body.x-=2*body.radius;
			LCD_SetTextColor(BLUE);       
			ILI9341_DrawCircle(body.x,body.y,body.radius,1);
			changestate();
		}
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);     //���EXTI���й���λ
	}  
}

void KEY2_IRQHandler(void)  //������
{
  //�Ƿ������EXTI Line�ж�
	if(EXTI_GetITStatus(KEY2_INT_EXTI_LINE) != RESET) 
	{
		if(body.x < 240-body.radius)
		{
			LCD_SetTextColor(BLACK);       
			ILI9341_DrawCircle(body.x,body.y,body.radius,1);
			body.x+=2*body.radius;
			LCD_SetTextColor(BLUE);       
			ILI9341_DrawCircle(body.x,body.y,body.radius,1);
			changestate();
		}
		EXTI_ClearITPendingBit(KEY2_INT_EXTI_LINE);     //���EXTI���й���λ
	}  
}
void changestate()
{
	switch(body.onstage)
	{
		case -1:
		{
			if(body.y+body.radius>=stage1.y && body.y<=stage1.y && body.x>stage1.x && body.x<stage1.x+stage1.length)  //�䵽̨��
			{
				body.onstage=1;
			}else if(body.y+body.radius>=stage2.y && body.y<=stage2.y && body.x>stage2.x && body.x<stage2.x+stage2.length)  //�䵽̨��
			{
				body.onstage=2;
			}else if(body.y+body.radius>=stage3.y && body.y<=stage3.y && body.x>stage3.x && body.x<stage3.x+stage3.length)  //�䵽̨��
			{
				body.onstage=3;
			}else if(body.y+body.radius>=stage4.y && body.y<=stage4.y && body.x>stage4.x && body.x<stage4.x+stage4.length)  //�䵽̨��
			{
				body.onstage=4;
			}else if(body.y+body.radius>=stage5.y && body.y<=stage5.y && body.x>stage5.x && body.x<stage5.x+stage5.length)  //�䵽̨��
			{
				body.onstage=5;
			}
			break;
		}
		case 1:
		{
			if(body.x<stage1.x || body.x>stage1.x+stage1.length)
				body.onstage=-1;
			break;
		}
		case 2:
		{
			if(body.x<stage2.x || body.x>stage2.x+stage2.length)
				body.onstage=-1;
			break;
		}
		case 3:
		{
			if(body.x<stage3.x || body.x>stage3.x+stage3.length)
				body.onstage=-1;
			break;
		}
		case 4:
		{
			if(body.x<stage4.x || body.x>stage4.x+stage4.length)
				body.onstage=-1;
			break;
		}
		case 5:
		{
			if(body.x<stage5.x || body.x>stage5.x+stage5.length)
				body.onstage=-1;
			break;
		}
		default: 
			break;
	}
}


void move()  //�ϰ��½����ϰ�ˢ��
{

	if(body.onstage<0)  //��������
	{
		if(body.y+body.radius>=stage1.y && body.y<=stage1.y && body.x>stage1.x && body.x<stage1.x+stage1.length)  //�䵽̨��
		{
			body.onstage=1;
		}else if(body.y+body.radius>=stage2.y && body.y<=stage2.y && body.x>stage2.x && body.x<stage2.x+stage2.length)  //�䵽̨��
		{
			body.onstage=2;
		}else if(body.y+body.radius>=stage3.y && body.y<=stage3.y && body.x>stage3.x && body.x<stage3.x+stage3.length)  //�䵽̨��
		{
			body.onstage=3;
		}else if(body.y+body.radius>=stage4.y && body.y<=stage4.y && body.x>stage4.x && body.x<stage4.x+stage4.length)  //�䵽̨��
		{
			body.onstage=4;
		}else if(body.y+body.radius>=stage5.y && body.y<=stage5.y && body.x>stage5.x && body.x<stage5.x+stage5.length)  //�䵽̨��
		{
			body.onstage=5;
		}
		else
		{
			LCD_SetTextColor(BLACK);       
			ILI9341_DrawCircle(body.x,body.y,body.radius,1);
			body.y+=body.radius;
			LCD_SetTextColor(BLUE);       
			ILI9341_DrawCircle(body.x,body.y,body.radius,1);
		}
	}
	if(body.onstage>0)  //��stage��
	{	
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(body.x,body.y,body.radius,1);
		body.y-=up;
		LCD_SetTextColor(BLUE);       
		ILI9341_DrawCircle(body.x,body.y,body.radius,1);
	}
	
	LCD_SetTextColor(BLACK);       
	ILI9341_DrawRectangle(stage1.x,stage1.y,stage1.length,0,0);
	stage1.y-=up;		
	LCD_SetTextColor(RED);       
	ILI9341_DrawRectangle(stage1.x,stage1.y,stage1.length,0,0);
	
	if(stage1.y==180)
		flag1=1;
	if(flag1==1)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawRectangle(stage2.x,stage2.y,stage2.length,0,0);
		stage2.y-=up;		
		LCD_SetTextColor(RED);       
		ILI9341_DrawRectangle(stage2.x,stage2.y,stage2.length,0,0);
	}

	if(stage2.y==180)
		flag2=1;
	if(flag2)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawRectangle(stage3.x,stage3.y,stage3.length,0,0);
		stage3.y-=up;		
		LCD_SetTextColor(RED);       
		ILI9341_DrawRectangle(stage3.x,stage3.y,stage3.length,0,0);
	}
	if(stage3.y==180)
		flag3=1;
	if(flag3)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawRectangle(stage4.x,stage4.y,stage4.length,0,0);
		stage4.y-=up;		
		LCD_SetTextColor(RED);       
		ILI9341_DrawRectangle(stage4.x,stage4.y,stage4.length,0,0);
	}
	if(stage4.y==180)
		flag4=1;
	if(flag4)
	{     
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawRectangle(stage5.x,stage5.y,stage5.length,0,0);
		stage5.y-=up;		
		LCD_SetTextColor(RED);       
		ILI9341_DrawRectangle(stage5.x,stage5.y,stage5.length,0,0);
	}
	
	if(stage1.y<=40)
	{
		LCD_SetTextColor(BLACK);       
	  ILI9341_DrawRectangle(stage1.x,stage1.y,stage1.length,0,0);
		stage1.x=rand()%200;
		stage1.y=280;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}	
	if(stage2.y<=40)
	{
		LCD_SetTextColor(BLACK);       
	  ILI9341_DrawRectangle(stage2.x,stage2.y,stage2.length,0,0);
		stage2.x=rand()%200;
		stage2.y=280;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}	
	if(stage3.y<=40)
	{
		LCD_SetTextColor(BLACK);       
	  ILI9341_DrawRectangle(stage3.x,stage3.y,stage3.length,0,0);
		stage3.x=rand()%200;
		stage3.y=280;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}	
	if(stage4.y<=40)
	{
		LCD_SetTextColor(BLACK);       
	  ILI9341_DrawRectangle(stage4.x,stage4.y,stage4.length,0,0);
		stage4.x=rand()%200;
		stage4.y=280;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}
	if(stage5.y<=40)
	{
		LCD_SetTextColor(BLACK);       
	  ILI9341_DrawRectangle(stage5.x,stage5.y,stage5.length,0,0);
		stage5.x=rand()%200;
		stage5.y=280;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}
}

int dead()  
{
	if(body.y<40 || body.y>280)
		return 1;
	return 0;
}

//��Ϸ��ʼ
void play()
{
	u16 i;	
	body.onstage=1;
	body.x=120;
	body.y=200;
	body.radius=10;
	stage1.x=110;
	stage2.x=20;
	stage3.x=60;
	stage4.x=120;
	stage5.x=200;
	stage1.y=body.y+body.radius;
	stage2.y=stage3.y=stage4.y=stage5.y=280;
	stage1.length=40;
	stage2.length=50;
	stage3.length=30;
	stage4.length=40;
	stage5.length=40;
	
	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	  
	LCD_SetColors(CYAN,BLACK);
	sprintf(dispBuff,"           ��һ�ٲ�");
	ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);

	LCD_SetTextColor(BLUE);       
	ILI9341_DrawCircle(body.x,body.y,body.radius,1);
	LCD_SetTextColor(GREEN);       
	ILI9341_DrawRectangle(stage1.x,stage1.y,stage1.length,0,0);
		
	while(1)
	{	
		sprintf(dispBuff,"���� : %d     ",game.Life);
		LCD_SetColors(RED,BLACK);
		ILI9341_DispStringLine_EN_CH(LINE(18),dispBuff);
		
		LCD_SetColors(WHITE,BLACK); //���Ʊ߿�
		ILI9341_DrawRectangle(0,40,240,240,0);
		
		LCD_SetTextColor(BLUE);       
	  ILI9341_DrawCircle(body.x,body.y,body.radius,1);
		
		if(game.Life<20)
			Delay(0xFFFFF);
		else if(game.Life<40)
			Delay(0x8FFFF);
		else
			Delay(0xFFFF);
		
		move();
		
		if(dead())
		{
			macLED1_ON();
			ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	  
			sprintf(dispBuff,"           GAME OVER      ",game.Life);
			LCD_SetColors(RED,BLACK);
			ILI9341_DispStringLine_EN_CH(LINE(9),dispBuff);
		  sprintf(dispBuff,"          ���� : %d      ",game.Life);
			LCD_SetColors(BLUE,BLACK);
			ILI9341_DispStringLine_EN_CH(LINE(10),dispBuff);
			flag=1;
			macBEEP_ON ();
			Delay(0x2FFFF);
			macBEEP_OFF ();
			Delay(0x2FFFF);
			macBEEP_ON ();
			Delay(0x2FFFF);
			macBEEP_OFF ();
			Delay(0x2FFFF);
			macBEEP_ON ();
			Delay(0x2FFFF);
			macBEEP_OFF ();
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
		play();
		macLED1_OFF();
		if(flag)
			break;
		
	}
}

static void Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
}

