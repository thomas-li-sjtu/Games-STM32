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
#define SNAKE_Max_Long 60//�ߵ���󳤶�
#define WALL_Max_Long 60
int timestamp1=0;//ˢ�¶�ҩ
int timestamp0=0;//ˢ��ǽ��
void animation(void);
void init_stage(void);
int dead(void);
void gameover(void);
int finish(void);


//�߽ṹ��
struct Snake
{
	uint16_t X[SNAKE_Max_Long];
	uint16_t Y[SNAKE_Max_Long];
	u8 Long;//�ߵĳ���
	u8 Life;//�ߵ����� 0���� 1����
	u8 Direction;//���ƶ��ķ���
}snake;

//ʳ��
struct Food
{
	u8 X;//ʳ�������
	u8 Y;//ʳ��������
	u8 Yes;//�ж��Ƿ�Ҫ����ʳ��ı��� 0��ʳ�� 1��Ҫ����ʳ��
}food0,food1,poison1,poison2;

//ǽ��
struct Wall
{
	uint16_t X;
	uint16_t Y;
	u16 length;//ǽ����
	u8 dir;//ǽ����0Ϊ����1Ϊ����
	u8 Yes;
}wall0,wall1;

//��Ϸ�ȼ�����
struct Game
{
	int Score;//����
	u8 Life;//��Ϸ�ȼ�	
}game;

void KEY1_IRQHandler(void)
{
  //ȷ���Ƿ������EXTI Line�ж�
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
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);     //���EXTI���й���λ
	}  
}

void KEY2_IRQHandler(void)
{
  //�Ƿ������EXTI Line�ж�
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

void USART1_IRQHandler(u8 GetData)//�����ж�
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		GetData=USART1->DR;
		mk=GetData;
		printf("�յ������ݣ�\n");
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




//����ʳ���ҩ��ǽ��
void init_stage()
{
	u16 i;
	if(food0.Yes==1||food1.Yes==1)//�����µ�ʳ��
	{
		//���趨����������ʾʳ��
		LCD_SetTextColor(BLACK)	;
		ILI9341_DrawRectangle(food0.X,food0.Y,10,10,1);//�ѳԵ���ʳ������
		food0.X=rand()%(200/10)*10;
		food0.Y=rand()%(200/10)*10+50;
		food0.Yes=0;
		food1.Yes=1;
	}
	if(food1.Yes==1||food0.Yes==1)//�����µ�ʳ��
	{
		LCD_SetTextColor(BLACK)	;
		ILI9341_DrawRectangle(food1.X,food1.Y,10,10,1);//�ѳԵ���ʳ������	
		food1.X=rand()%(200/10)*10;
		food1.Y=rand()%(200/10)*10+50;
		food1.Yes=0;
	}
	if(food0.Yes==0)//��ʳ���Ҫ��ʾ
	{	
		LCD_SetColors(RED,BLACK);
		ILI9341_DrawRectangle(food0.X,food0.Y,10,10,1);
	}
	if(food1.Yes==0)//��ʳ���Ҫ��ʾ
	{	
		LCD_SetColors(RED,BLACK);
		ILI9341_DrawRectangle(food1.X,food1.Y,10,10,1);
	}
	//��ҩ
	if(poison1.Yes==1||timestamp1>50)//�����µĶ�ҩ1
	{
		if(timestamp1>50)
		{
			LCD_SetTextColor(BLACK);
			ILI9341_DrawRectangle(poison1.X,poison1.Y,10,10,1);
		}
		do
		{//���趨����������ʾ��ҩ1
			poison1.X=rand()%(200/10)*10+40;
			poison1.Y=rand()%(200/10)*10+40;
			poison1.Yes=0;
		}while(poison1.X==food0.X||poison1.Y==food0.Y);
	}
	if(poison2.Yes==1||timestamp1>50)//�����µĶ�ҩ2
	{
		//���趨����������ʾ��ҩ2
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
	if(poison1.Yes==0)//�ж�ҩ��Ҫ��ʾ
	{	
		LCD_SetColors(MAGENTA,BLACK);
		ILI9341_DrawRectangle(poison1.X,poison1.Y,10,10,1);
	}
	if(poison2.Yes==0)//�ж�ҩ��Ҫ��ʾ
	{	
		LCD_SetColors(MAGENTA,BLACK);
		ILI9341_DrawRectangle(poison2.X,poison2.Y,10,10,1);
	}
	//ǽ��
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

//�ж��Ƿ�����
int dead()
{
	u16 i;
	//�ж��Ƿ�ײǽ
	if(snake.X[0]==0||snake.X[0]>240||snake.Y[0]==40||snake.Y[0]>260)
		snake.Life=1;
	if(snake.Y[0]==wall0.Y&&(snake.X[0]>wall0.X&&snake.X[0]<=wall0.X+wall0.length))
		snake.Life=1;
	if(snake.Y[0]==wall1.Y&&(snake.X[0]>wall1.X&&snake.X[0]<=wall1.X+wall1.length))
		snake.Life=1;

	//���ߵ����峬��3�ں��ж����������ײ
	for(i=3;i<snake.Long;i++)
	{
		if(snake.X[i]==snake.X[0]&&snake.Y[i]==snake.Y[0])//�������һ����ֵ����ͷ������Ⱦ���Ϊ��������ײ
		{
			snake.Life=1;
			game.Life-=1;
		}
	}
	if(snake.Life==1||game.Life==0||game.Score<0)//���������ж��Ժ������������������ѭ�������¿�ʼ
	{
		gameover();
		return 1;
	}else
		return 0;
}

//��Ϸ����
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
	
	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);        //����
	start=0;
}
int finish()
{
	if(game.Score==1000)
	{
		LCD_SetTextColor(RED);
		sprintf(dispBuff,"          ��ϲ�������ս");
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

//��Ϸ��ʼ
void play()
{
	u16 i;	
	snake.Long=2;//�����ߵĳ���
	snake.Life=0;//�߻�����
	snake.Direction=1;//�ߵ���ʼ������Ϊ��
	game.Score=0;//����Ϊ0
	game.Life=4;//�ߵ�����ֵ
	food0.Yes=1;//������ʳ��
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
		sprintf(dispBuff,"          ̰������Ϸ");
		ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
		
		sprintf(dispBuff,"���� : %d                       ",game.Score);
		LCD_SetColors(BLUE,BLACK);
		ILI9341_DispStringLine_EN_CH(LINE(18),dispBuff);
		
		LCD_SetColors(WHITE,BLACK);
		ILI9341_DrawRectangle(0,40,240,240,0);
		
		init_stage();
				
		//ȡ����Ҫ���»����ߵĽ���
		for(i=snake.Long-1;i>0;i--)
		{
			snake.X[i]=snake.X[i-1];
			snake.Y[i]=snake.Y[i-1];
		}

		switch(snake.Direction)
		{
			case 1:snake.X[0]+=10;break;//�����˶�
			case 2:snake.X[0]-=10;break;//�����˶�
			case 3:snake.Y[0]-=10;break;//�����˶�
			case 4:snake.Y[0]+=10;break;//�����˶�
		}
		for(i=0;i<snake.Long;i++)//������	
		{
			LCD_SetColors(RED,BLACK);
			ILI9341_DrawRectangle(snake.X[i],snake.Y[i],10,10,1);//��������
		}			
		
		if(game.Score < 50)
			Delay(0xFFFFF);  //�����ߵ��ƶ�ʱ�ٶ�
		else if(game.Score < 100)
			Delay(0xBFFFF);
		else if(game.Score < 120)
			Delay(0x8FFFF);
		else
			Delay(0x0FFFF);
			
		
		LCD_SetTextColor(BLACK)	;       //����LCD��ǰ��(����)��ɫ,RGB565
		ILI9341_DrawRectangle(snake.X[snake.Long-1],snake.Y[snake.Long-1],10,10,1);//��������		
		LCD_SetTextColor(RED)	;
		
		//�ж����Ƿ�Ե���ʳ��
		if(snake.X[0]==food0.X&&snake.Y[0]==food0.Y)
		{ 
			LCD_SetTextColor(BLACK)	;
			ILI9341_DrawRectangle(food0.X,food0.Y,10,10,1);//�ѳԵ���ʳ������
			LCD_SetTextColor(RED)	;
			snake.Long++;//�ߵ����峤һ��
			game.Score+=10;
			food0.Yes=1;//��Ҫ������ʾʳ��
		}else if(snake.X[0]==food1.X&&snake.Y[0]==food1.Y)
		{
			LCD_SetTextColor(BLACK)	;
			ILI9341_DrawRectangle(food1.X,food1.Y,10,10,1);//�ѳԵ���ʳ������
			LCD_SetTextColor(RED)	;
			snake.Long++;//�ߵ����峤һ��
			game.Score+=10;
			food1.Yes=1;//��Ҫ������ʾʳ��
		}else if(snake.X[0]==poison1.X&&snake.Y[0]==poison1.Y)  //�ж����Ƿ�Ե��˶�ҩ
		{ 
			LCD_SetTextColor(BLACK)	;
			ILI9341_DrawRectangle(poison1.X,poison1.Y,10,10,1);
			LCD_SetTextColor(RED)	;
			game.Score-=20;
			poison1.Yes=1;
		}else if(snake.X[0]==poison2.X&&snake.Y[0]==poison2.Y)  //�ж����Ƿ�Ե��˶�ҩ
		{ 
			LCD_SetTextColor(BLACK);
			ILI9341_DrawRectangle(poison2.X,poison2.Y,10,10,1);
			LCD_SetTextColor(RED);
			game.Score-=20;
			poison2.Yes=1;
		}		
		
		//����
		if(dead())
			break;
		//����
		if(finish())
			break;
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
			animation();
			continue;
		}
		macLED1_OFF();
		macLED2_ON();
		ILI9341_DrawRectangle(0,0,240,270,0);            //�� ILI9341 ��ʾ���ϻ�һ������
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
	sprintf(dispBuff1,"       ��K1����ʼ��Ϸ......");
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
	sprintf(dispBuff,"          ̰������Ϸ");
	ILI9341_DispStringLine_EN_CH(LINE(1),dispBuff);
	Delay(0xFFFFF);
}
