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
static void LCD_Test(void);	
static void Delay ( __IO uint32_t nCount );
uint32_t time = 0;
u8 start = 0;
char str[20];
uint16_t  key_flag=0;
extern mk;
char dispBuff[100];
#define SNAKE_Max_Long 60//蛇的最大长度
#define WALL_Max_Long 60
int timestamp1=0;//刷新毒药
int timestamp0=0;//刷新墙壁
void animation(void);
void init_stage(void);
int dead(void);
void gameover(void);
int finish(void);


//蛇结构体
struct Snake
{
	uint16_t X[SNAKE_Max_Long];
	uint16_t Y[SNAKE_Max_Long];
	u8 Long;//蛇的长度
	u8 Life;//蛇的生命 0活着 1死亡
	u8 Direction;//蛇移动的方向
}snake;

//食物
struct Food
{
	u8 X;//食物横坐标
	u8 Y;//食物纵坐标
	u8 Yes;//判断是否要出现食物的变量 0有食物 1需要出现食物
}food0,food1,poison1,poison2;

//墙壁
struct Wall
{
	uint16_t X;
	uint16_t Y;
	u16 length;//墙长度
	u8 dir;//墙方向，0为横向，1为纵向
	u8 Yes;
}wall0,wall1;

//游戏等级分数
struct Game
{
	int Score;//分数
	u8 Life;//游戏等级	
}game;

void KEY1_IRQHandler(void)
{
  //确保是否产生了EXTI Line中断
	if(EXTI_GetITStatus(KEY1_INT_EXTI_LINE) != RESET)		
	{ 
		if(start==0)
		{
			start=1;
			return;
		}
		key_flag=3;
		printf("key 1\n");
		if(key_flag==3&&(snake.Direction == 3||snake.Direction == 4))
			snake.Direction=2;
		else{
		if(key_flag==3&&snake.Direction!=4)
			snake.Direction=3;
				}
		 printf("Direction=%d\n",snake.Direction);
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);     //清除EXTI的行挂起位
	}  
}

void KEY2_IRQHandler(void)
{
  //是否产生了EXTI Line中断
	if(EXTI_GetITStatus(KEY2_INT_EXTI_LINE) != RESET) 
	{
		key_flag=4;
		printf("key 2\n");
		if(key_flag==4&&(snake.Direction == 3||snake.Direction == 4))
		  snake.Direction=1;
		else{
		if(key_flag==4&&snake.Direction!=3)
		  snake.Direction=4;
				}
		 printf("Direction=%d\n",snake.Direction);
		EXTI_ClearITPendingBit(KEY2_INT_EXTI_LINE);     
	}  
	
	if(EXTI_GetITStatus(KEY3_INT_EXTI_LINE) != RESET) 
	{
		key_flag=1;
		printf("key 3\n");
		if(key_flag==2&&snake.Direction!=2)
		  snake.Direction=1;
		 printf("Direction=%d\n",snake.Direction);
		EXTI_ClearITPendingBit(KEY3_INT_EXTI_LINE);     
	}  
		if(EXTI_GetITStatus(KEY4_INT_EXTI_LINE) != RESET) 
	{
		key_flag=2;
		printf("key 4\n");
		if(key_flag==2&&snake.Direction!=1)
		  snake.Direction=2;
		 printf("Direction=%d\n",snake.Direction);
		EXTI_ClearITPendingBit(KEY4_INT_EXTI_LINE);     
	} 
}

void USART1_IRQHandler(u8 GetData)//串口中断
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		GetData=USART1->DR;
		mk=GetData;
		printf("收到的数据：\n");
		printf("%d\n",GetData);
		printf("0x%x\n",mk);
		if(mk==0x1)
		{	
			snake.Direction=1;
		}
		if(GetData==0x2)
		{
			snake.Direction=2;
		}
	}	
}




//生成食物，毒药，墙壁
void init_stage()
{
	u16 i;
	if(food0.Yes==1||food1.Yes==1)//出现新的食物
	{
		//在设定的区域内显示食物
		LCD_SetTextColor(BLACK)	;
		ILI9341_DrawRectangle(food0.X,food0.Y,10,10,1);//把吃到的食物消除
		food0.X=rand()%(200/10)*10;
		food0.Y=rand()%(200/10)*10+50;
		food0.Yes=0;
		food1.Yes=1;
	}
	if(food1.Yes==1||food0.Yes==1)//出现新的食物
	{
		LCD_SetTextColor(BLACK)	;
		ILI9341_DrawRectangle(food1.X,food1.Y,10,10,1);//把吃到的食物消除	
		food1.X=rand()%(200/10)*10;
		food1.Y=rand()%(200/10)*10+50;
		food1.Yes=0;
	}
	if(food0.Yes==0)//有食物就要显示
	{	
		LCD_SetColors(RED,BLACK);
		ILI9341_DrawRectangle(food0.X,food0.Y,10,10,1);
	}
	if(food1.Yes==0)//有食物就要显示
	{	
		LCD_SetColors(RED,BLACK);
		ILI9341_DrawRectangle(food1.X,food1.Y,10,10,1);
	}
	//毒药
	if(poison1.Yes==1||timestamp1>50)//出现新的毒药1
	{
		if(timestamp1>50)
		{
			LCD_SetTextColor(BLACK);
			ILI9341_DrawRectangle(poison1.X,poison1.Y,10,10,1);
		}
		do
		{//在设定的区域内显示毒药1
			poison1.X=rand()%(200/10)*10+40;
			poison1.Y=rand()%(200/10)*10+40;
			poison1.Yes=0;
		}while(poison1.X==food0.X||poison1.Y==food0.Y);
	}
	if(poison2.Yes==1||timestamp1>50)//出现新的毒药2
	{
		//在设定的区域内显示毒药2
		if(timestamp1>50)
		{
			LCD_SetTextColor(BLACK);
			ILI9341_DrawRectangle(poison2.X,poison2.Y,10,10,1);
		}
		do
		{
			poison2.X=rand()%(200/10)*10+40;
			poison2.Y=rand()%(200/10)*10+40;
			poison2.Yes=0;
			timestamp1=0;
		}while(poison2.X==food0.X||poison2.Y==food0.Y||poison2.X==poison1.X||poison2.Y==poison1.Y);
	}
	if(poison1.Yes==0)//有毒药就要显示
	{	
		LCD_SetColors(MAGENTA,BLACK);
		ILI9341_DrawRectangle(poison1.X,poison1.Y,10,10,1);
	}
	if(poison2.Yes==0)//有毒药就要显示
	{	
		LCD_SetColors(MAGENTA,BLACK);
		ILI9341_DrawRectangle(poison2.X,poison2.Y,10,10,1);
	}
	//墙壁
	if(wall0.Yes==1||timestamp0>80)
	{
		if(timestamp0>80)
		{
			LCD_SetTextColor(BLACK);
			for(i=0;i<wall0.length;i++)
			{
				ILI9341_DrawRectangle(wall0.X+i,wall0.Y,10,10,1);
			}
		}
		wall0.length=rand()%40+20;
		wall0.X=rand()%(150/10)*10+25;
		wall0.Y=rand()%(150/10)*10+50;
		wall0.Yes=0;
	}
	if(wall1.Yes==1||timestamp0>80)
	{
		if(timestamp0>80)
		{
			LCD_SetTextColor(BLACK);
			for(i=0;i<wall1.length;i++)
			{
				ILI9341_DrawRectangle(wall1.X+i,wall1.Y,10,10,1);
			}
		}
		wall1.length=rand()%40+20;
		wall1.X=rand()%(150/10)*10+25;
		wall1.Y=rand()%(150/10)*10+50;
		wall1.Yes=0;
		timestamp0=0;
	}	
	if(wall0.Yes==0)
	{	
		LCD_SetColors(GREEN,BLACK);
		for(i=0;i<wall0.length;i++)
		{
			ILI9341_DrawRectangle(wall0.X+i,wall0.Y,10,10,1);
		}
	}
	if(wall1.Yes==0)
	{	
		LCD_SetColors(GREEN,BLACK);
		for(i=0;i<wall1.length;i++)
		{
			ILI9341_DrawRectangle(wall1.X+i,wall1.Y,10,10,1);
		}
	}
	
	timestamp1+=1;
	timestamp0+=1;
}

//判断是否死亡
int dead()
{
	u16 i;
	//判断是否撞墙
	if(snake.X[0]==0||snake.X[0]>240||snake.Y[0]==40||snake.Y[0]>260)
		snake.Life=1;
	if(snake.Y[0]==wall0.Y&&(snake.X[0]>wall0.X&&snake.X[0]<=wall0.X+wall0.length))
		snake.Life=1;
	if(snake.Y[0]==wall1.Y&&(snake.X[0]>wall1.X&&snake.X[0]<=wall1.X+wall1.length))
		snake.Life=1;

	//当蛇的身体超过3节后判断蛇自身的碰撞
	for(i=3;i<snake.Long;i++)
	{
		if(snake.X[i]==snake.X[0]&&snake.Y[i]==snake.Y[0])//自身的任一坐标值与蛇头坐标相等就认为是自身碰撞
		{
			snake.Life=1;
			game.Life-=1;
		}
	}
	if(snake.Life==1||game.Life==0||game.Score<0)//以上两种判断以后如果设死掉了跳出内循环，重新开始
	{
		gameover();
		return 1;
	}else
		return 0;
}

//游戏结束
void gameover()
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
	
	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);        //清屏
	start=0;
}
int finish()
{
	if(game.Score==1000)
	{
		LCD_SetTextColor(RED);
		sprintf(dispBuff,"          恭喜你完成挑战");
		ILI9341_DispStringLine_EN_CH(LINE(10),dispBuff);

		Delay(0xFFFFFF);
		Delay(0xFFFFFF);
		game.Score =0;
		ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	
		start=0;
		return 1;
	}else
		return 0;
}

//游戏开始
void play()
{
	u16 i;	
	snake.Long=2;//定义蛇的长度
	snake.Life=0;//蛇还活着
	snake.Direction=1;//蛇的起始方向定义为右
	game.Score=0;//分数为0
	game.Life=4;//蛇的生命值
	food0.Yes=1;//出现新食物
	food1.Yes=1;
	poison1.Yes=1;
	poison2.Yes=1;
	wall0.Yes=1;
	wall0.dir=0;
	wall1.Yes=1;
	wall1.dir=1;
	snake.X[0]=0;snake.Y[0]=50;
	snake.X[1]=10;snake.Y[1]=50;

	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	  
	
	while(1)
	{
		LCD_SetColors(CYAN,BLACK);
		sprintf(dispBuff,"          贪吃蛇游戏");
		ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
		
		sprintf(dispBuff,"分数 : %d                       ",game.Score);
		LCD_SetColors(BLUE,BLACK);
		ILI9341_DispStringLine_EN_CH(LINE(18),dispBuff);
		
		LCD_SetColors(WHITE,BLACK);
		ILI9341_DrawRectangle(0,40,240,240,0);
		
		init_stage();
				
		//取得需要重新画的蛇的节数
		for(i=snake.Long-1;i>0;i--)
		{
			snake.X[i]=snake.X[i-1];
			snake.Y[i]=snake.Y[i-1];
		}

		switch(snake.Direction)
		{
			case 1:snake.X[0]+=10;break;//向右运动
			case 2:snake.X[0]-=10;break;//向左运动
			case 3:snake.Y[0]-=10;break;//向上运动
			case 4:snake.Y[0]+=10;break;//向下运动
		}
		for(i=0;i<snake.Long;i++)//画出蛇	
		{
			LCD_SetColors(RED,BLACK);
			ILI9341_DrawRectangle(snake.X[i],snake.Y[i],10,10,1);//画蛇身体
		}			
		
		if(game.Score < 50)
			Delay(0xFFFFF);  //控制蛇的移动时速度
		else if(game.Score < 100)
			Delay(0xBFFFF);
		else if(game.Score < 120)
			Delay(0x8FFFF);
		else
			Delay(0x0FFFF);
			
		
		LCD_SetTextColor(BLACK)	;       //设置LCD的前景(字体)颜色,RGB565
		ILI9341_DrawRectangle(snake.X[snake.Long-1],snake.Y[snake.Long-1],10,10,1);//消除蛇身		
		LCD_SetTextColor(RED)	;
		
		//判断蛇是否吃到了食物
		if(snake.X[0]==food0.X&&snake.Y[0]==food0.Y)
		{ 
			LCD_SetTextColor(BLACK)	;
			ILI9341_DrawRectangle(food0.X,food0.Y,10,10,1);//把吃到的食物消除
			LCD_SetTextColor(RED)	;
			snake.Long++;//蛇的身体长一节
			game.Score+=10;
			food0.Yes=1;//需要重新显示食物
		}else if(snake.X[0]==food1.X&&snake.Y[0]==food1.Y)
		{
			LCD_SetTextColor(BLACK)	;
			ILI9341_DrawRectangle(food1.X,food1.Y,10,10,1);//把吃到的食物消除
			LCD_SetTextColor(RED)	;
			snake.Long++;//蛇的身体长一节
			game.Score+=10;
			food1.Yes=1;//需要重新显示食物
		}else if(snake.X[0]==poison1.X&&snake.Y[0]==poison1.Y)  //判断蛇是否吃到了毒药
		{ 
			LCD_SetTextColor(BLACK)	;
			ILI9341_DrawRectangle(poison1.X,poison1.Y,10,10,1);
			LCD_SetTextColor(RED)	;
			game.Score-=20;
			poison1.Yes=1;
		}else if(snake.X[0]==poison2.X&&snake.Y[0]==poison2.Y)  //判断蛇是否吃到了毒药
		{ 
			LCD_SetTextColor(BLACK);
			ILI9341_DrawRectangle(poison2.X,poison2.Y,10,10,1);
			LCD_SetTextColor(RED);
			game.Score-=20;
			poison2.Yes=1;
		}		
		
		//死亡
		if(dead())
			break;
		//结束
		if(finish())
			break;
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
		macLED1_ON();
		if(start==0)
		{
			animation();
			continue;
		}
		macLED1_OFF();
		macLED2_ON();
		ILI9341_DrawRectangle(0,0,240,270,0);            //在 ILI9341 显示器上画一个矩形
		printf("test\n");
		play();
		macLED2_OFF();
	}
}

void animation(void)
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
	sprintf(dispBuff1,"       按K1键开始游戏......");
	ILI9341_DispStringLine_EN_CH(LINE(18),dispBuff1);
	Delay(0xFFFFF);
}

static void Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
}

void LCD_Test(void)
{
	char dispBuff[200];
	
	LCD_SetTextColor(GREEN);
	sprintf(dispBuff,"          贪吃蛇游戏");
	ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
	Delay(0xFFFFF);
}
