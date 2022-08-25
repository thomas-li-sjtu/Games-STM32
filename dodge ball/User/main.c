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
u8 flag=0;
u8 flag1=0;
u8 flag2=0;
u8 flag3=0;
u8 flag4=0;
u8 fall=5;
char dispBuff[100];

void hit(void);
int dead(void);


struct Plane
{
	int bodyx;
	int length;
	int headx;
	int heady;
	int life;
}plane;


struct Enemy
{
	int x;
	int y;
	u8 radius;
	int life;
}enemy1,enemy2,enemy3,enemy4,enemy5,enemy6;

//游戏等级分数
struct Game
{
	int Score;//分数
	u8 Life;//游戏等级	
}game;

void KEY1_IRQHandler(void)  //飞机左移
{
  //确保是否产生了EXTI Line中断
	if(EXTI_GetITStatus(KEY1_INT_EXTI_LINE) != RESET)		
	{ 
		if(plane.bodyx > 0)
		{
			LCD_SetTextColor(BLACK);       
			ILI9341_DrawRectangle(plane.bodyx,270,20,5,1);  //飞机机身长20宽5
			ILI9341_DrawCircle(plane.headx,plane.heady,5,1);
			plane.bodyx-=20;
			plane.heady=270;
			plane.headx-=20;
			LCD_SetTextColor(BLUE);       
			ILI9341_DrawRectangle(plane.bodyx,270,20,5,1);  //飞机向左移动10
			ILI9341_DrawCircle(plane.headx,plane.heady,5,1);
		}
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);     //清除EXTI的行挂起位
	}  
}

void KEY2_IRQHandler(void)  //飞机右移
{
  //是否产生了EXTI Line中断
	if(EXTI_GetITStatus(KEY2_INT_EXTI_LINE) != RESET) 
	{
		if(plane.bodyx < 220)
		{
			LCD_SetTextColor(BLACK);       
			ILI9341_DrawRectangle(plane.bodyx,270,20,5,1);  //飞机机身长20宽5
			ILI9341_DrawCircle(plane.headx,plane.heady,5,1);
			plane.bodyx+=20;
			plane.heady=270;
			plane.headx+=20;
			LCD_SetTextColor(BLUE);       
			ILI9341_DrawRectangle(plane.bodyx,270,20,5,1);  //飞机向右移动10
			ILI9341_DrawCircle(plane.headx,plane.heady,5,1);
		}
		EXTI_ClearITPendingBit(KEY2_INT_EXTI_LINE);     //清除EXTI的行挂起位
	}  
}

void hit()  //障碍下降与障碍刷新
{
	LCD_SetTextColor(BLACK);       
	ILI9341_DrawCircle(enemy1.x,enemy1.y,enemy1.radius,1);
	enemy1.y+=fall;		
	LCD_SetTextColor(RED);       
	ILI9341_DrawCircle(enemy1.x,enemy1.y,enemy1.radius,1);
	
	if(enemy1.y==100)
		flag1=1;
	if(flag1==1)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy2.x,enemy2.y,enemy2.radius,1);
		enemy2.y+=fall;		
		LCD_SetTextColor(GREEN);       
		ILI9341_DrawCircle(enemy2.x,enemy2.y,enemy2.radius,1);
	}

	if(enemy2.y==100)
		flag2=1;
	if(flag2)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy3.x,enemy3.y,enemy3.radius,1);
		enemy3.y+=fall;
		LCD_SetTextColor(RED);       
		ILI9341_DrawCircle(enemy3.x,enemy3.y,enemy3.radius,1);
	}
	if(enemy3.y==100)
		flag3=1;
	if(flag3)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy4.x,enemy4.y,enemy4.radius,1);
		enemy4.y+=fall;		
		LCD_SetTextColor(MAGENTA);
		ILI9341_DrawCircle(enemy4.x,enemy4.y,enemy4.radius,1);
	}

	
	if(enemy1.y>270)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy1.x,enemy1.y,enemy1.radius,1);
		enemy1.x=rand()%200+20;
		enemy1.y=60;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}	
	if(enemy2.y>270)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy2.x,enemy2.y,enemy2.radius,1);
		enemy2.x=rand()%200+20;
		enemy2.y=60;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}	
	if(enemy3.y>270)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy3.x,enemy3.y,enemy3.radius,1);
		enemy3.x=rand()%200+20;
		enemy3.y=60;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}	
	if(enemy4.y>270)
	{
		LCD_SetTextColor(BLACK);       
		ILI9341_DrawCircle(enemy4.x,enemy4.y,enemy4.radius,1);
		enemy4.x=rand()%200+20;
		enemy4.y=60;
		game.Life++;
		macLED2_ON();
		Delay(0xFFF);
		macLED2_OFF();
	}
}

int dead()  
{
	if(enemy1.x+enemy1.radius>plane.bodyx && enemy1.x<plane.bodyx 
		|| enemy1.x-enemy1.radius<plane.bodyx+plane.length && enemy1.x>plane.bodyx+plane.length
	  || enemy1.x>plane.bodyx && enemy1.x<plane.bodyx+plane.length)
	{
		if(enemy1.y+enemy1.radius>plane.heady+2)
			return 1;
	}
	
	if(enemy2.x+enemy2.radius>plane.bodyx && enemy2.x<plane.bodyx 
		|| enemy2.x-enemy2.radius<plane.bodyx+plane.length && enemy2.x>plane.bodyx+plane.length
	  || enemy2.x>plane.bodyx && enemy2.x<plane.bodyx+plane.length)
	{
		if(enemy2.y+enemy2.radius>plane.heady+2)
			return 1;
	}
	
	if(enemy3.x+enemy3.radius>plane.bodyx && enemy3.x<plane.bodyx 
		|| enemy3.x-enemy3.radius<plane.bodyx+plane.length && enemy3.x>plane.bodyx+plane.length
	  || enemy3.x>plane.bodyx && enemy3.x<plane.bodyx+plane.length)
	{
		if(enemy3.y+enemy3.radius>plane.heady+2)
			return 1;
	}
	
	if(enemy4.x+enemy4.radius>plane.bodyx && enemy4.x<plane.bodyx 
		|| enemy4.x-enemy4.radius<plane.bodyx+plane.length && enemy4.x>plane.bodyx+plane.length
	  || enemy4.x>plane.bodyx && enemy4.x<plane.bodyx+plane.length)
	{
		if(enemy4.y+enemy4.radius>plane.heady+2)
			return 1;
	}
	return 0;
}

//游戏开始
void play()
{
	u16 i;	
	plane.bodyx=110;
	plane.headx=120;
	plane.length=20;
	plane.heady=270;
	plane.life=1;
	
	game.Life=1;
	game.Score=0;
	
	enemy1.x=20;
	enemy2.x=80;
	enemy3.x=160;
	enemy4.x=200;
	
	enemy1.y=enemy2.y=enemy3.y=enemy4.y=60;
	enemy1.radius=enemy2.radius=enemy3.radius=enemy4.radius=12;
	enemy1.life=1;
	enemy2.life=1;
	enemy3.life=1;
	enemy4.life=1;
	
	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	  
	LCD_SetColors(CYAN,BLACK);
	sprintf(dispBuff,"            躲避球");
	ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);

	LCD_SetTextColor(BLUE);       
	ILI9341_DrawRectangle(plane.bodyx,270,20,5,1);  
	ILI9341_DrawCircle(plane.headx,plane.heady,5,1);
	
	while(1)
	{	
		sprintf(dispBuff,"分数 : %d                       ",game.Life);
		LCD_SetColors(RED,BLACK);
		ILI9341_DispStringLine_EN_CH(LINE(18),dispBuff);
		
		LCD_SetColors(WHITE,BLACK); //绘制边框
		ILI9341_DrawRectangle(0,40,240,240,0);
		
		LCD_SetTextColor(BLUE);       
		ILI9341_DrawRectangle(plane.bodyx,270,20,5,1);
		ILI9341_DrawCircle(plane.headx,plane.heady,5,1);
		
		if(game.Life<20)
			Delay(0xFFFF);
		else if(game.Life<30)
			Delay(0x2FFF);
		else
		{
			fall=10;
			Delay(0x2FFF);
		}
		
		hit();
		if(dead())
		{
			macLED1_ON();
			ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	  
			sprintf(dispBuff,"      GAME OVER      ",game.Life);
			LCD_SetColors(RED,BLACK);
			ILI9341_DispStringLine_EN_CH(LINE(9),dispBuff);
		  sprintf(dispBuff,"     分数 : %d      ",game.Life);
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
	ILI9341_GramScan (6);    //选择屏幕扫描方式

  ILI9341_Init();           //ILI9341初始化函数
  LED_Init();               //LED初始化
	Beep_Init();              //BEEP初始化
	USART_Config();	           //USART GPIO 配置,工作参数配置
  Key_GPIO_Config();         //配置按键用到的I/O口
  SysTick_Init();            //启动系统滴答定时器 SysTick
  LCD_SetFont(&Font8x16);    //设置英文字体类型
	
	LCD_SetColors(YELLOW,BLACK);  //设置LCD的前景(字体)及背景颜色,RGB565
  EXTI_Key_Config();         //配置IO为EXTI中断口，并设置中断优先级
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);    //对ILI9341显示器的某一窗口以某种颜色进行清屏
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

