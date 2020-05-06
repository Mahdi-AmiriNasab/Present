
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
	* @author 				: Mahdi AmiriNasab
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "hal_st7920.h"
#include "onewire.h"
#include "ds18b20.h"

#ifdef 		DEBOUNCER
#define 	Wait			5
#define 	Wait_Long	35
#endif
//#define		toDispaly(lighT,displaY)               displaY=lighT/28.2
//#define		tolight(lighT,displaY)	               lighT=displaY*28.2353

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

uint8_t current_page  = Main_LightDimming, last_page = Main_About , page = Page_Main;
uint16_t red_light = 0 ,green_light  = 0 ,blue_light = 0 ,white_light = 0;
uint16_t needed_red_light = 0 ,needed_green_light  = 0 ,needed_blue_light = 0 ,needed_white_light = 0;
uint8_t display_red_light = 0 ,display_green_light  = 0 ,display_blue_light = 0 ,display_white_light = 0;
uint8_t temp_value = 0;
uint16_t local_DMX_address = 1 , x = 0,DMX_type = 513;
uint8_t DMX_packet[520];
uint8_t DMX_red_permission = DMX_Allowed  
, DMX_green_permission = DMX_Allowed
,DMX_blue_permission = DMX_Allowed
, DMX_white_permission = DMX_Allowed;
uint8_t DS_ROM[1][8]; 
float tempp;
uint8_t temp1[20];
TM_OneWire_t OW;
uint8_t try_out = 0 ,temp_check = 1;
uint16_t temperature_value = 0;
uint8_t ERR_status = FE_OFF;

extern  uint8_t last_red_packet ,last_green_packet 
		,last_blue_packet ,last_white_packet ;

uint8_t save_light (uint16_t light_to_save , uint8_t colour);
uint8_t save_address (uint16_t addr_to_save);
uint8_t load_light (uint16_t *light_to_load, uint8_t colour);
uint8_t load_address (uint16_t *addr_to_load);
uint8_t toDispaly(uint8_t colour, uint16_t Light);
uint16_t toLight(uint8_t colour,uint8_t Display);

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM8_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  HAL_Init();
	SystemClock_Config();
  MX_GPIO_Init();
	
	MX_DMA_Init();
  MX_I2C1_Init();
 // MX_IWDG_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM8_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
	
  
	GLCD_Init(); 							// 	GLCD initialization 
  write_factory_logo();			//	Show Manufacturer logo
	
	
	 while (!TM_OneWire_First(&OW) && try_out < 20) // One wire initialization
	 {
     HAL_Delay(5); 	
		 try_out++;
	 } 
	 TM_OneWire_GetFullROM(&OW, DS_ROM[0]);					// Find any device on one wire bus     				
	 if(TM_DS18B20_Is(DS_ROM[0]))
	 { 
     TM_DS18B20_SetResolution(&OW, DS_ROM[0], TM_DS18B20_Resolution_12bits);
      
     TM_DS18B20_SetAlarmHighTemperature(&OW, DS_ROM[0], 60);
     TM_DS18B20_SetAlarmLowTemperature(&OW, DS_ROM[0], 10);
      
     TM_DS18B20_StartAll(&OW);				
   }
	 
	HAL_Delay(200); 																							// Let the sensor gets ready
	
	load_address(&local_DMX_address);															// Loed the last saved address from external EEPROM
	if(local_DMX_address > 512 || local_DMX_address == 0)					// Check the address validation 
	{																															// Invalid saved address. set it on default value
		local_DMX_address = 1;																
		save_address(local_DMX_address);
	}
	
	load_light(&needed_red_light , Red_Home);													//	Loed the last saved light values from external EEPROM
		
	if(needed_red_light > Pulse_Width || needed_red_light == 0)				// Check the red light value validation 
	{
		needed_red_light = 1;
		save_light(needed_red_light ,Red_Home);
	}
	load_light(&needed_green_light , Green_Home);
	if(needed_green_light > Pulse_Width || needed_green_light == 0)		// Check the green light value validation 
	{
		needed_green_light = 1;
		save_light(needed_green_light ,Green_Home);
	}
	load_light(&needed_blue_light , Blue_Home);
	if(needed_blue_light > Pulse_Width || needed_blue_light == 0)			// Check the blue light value validation 
	{
		needed_blue_light = 1;
		save_light(needed_blue_light ,Blue_Home);
	}
	load_light(&needed_white_light , White_Home);		
	if(needed_white_light > Pulse_Width || needed_white_light == 0)		// Check the white light value validation 
	{
		needed_white_light = 1;
		save_light(needed_white_light ,White_Home);
	}
	
	if(needed_red_light != 1 | needed_green_light != 1 | needed_blue_light != 1)
		needed_white_light = 1;
	
	
	HAL_TIM_Base_Start_IT(Timer_Delay_Red);
	HAL_TIM_Base_Start_IT(Timer_Delay_Green);
	HAL_TIM_Base_Start_IT(Timer_Delay_Blue);
	HAL_TIM_Base_Start_IT(Timer_Delay_White);
	HAL_TIM_Base_Start_IT(Timer_Time_Base);
	
	HAL_TIM_PWM_Start(Timer_Light_Red ,Channel_light_Red);
	HAL_TIM_PWM_Start(Timer_Light_Green,Channel_light_Green);
	HAL_TIM_PWM_Start(Timer_Light_Blue ,Channel_light_Blue);
	HAL_TIM_PWM_Start(Timer_Light_White ,Channel_light_White);
	HAL_UART_Receive_DMA(&huart2 ,DMX_packet ,DMX_type);			

	
	
	last_red_packet = DMX_packet[Red_Address];											// Hold the last received DMX packet related to red color
	last_green_packet = DMX_packet[Green_Address];									// Hold the last received DMX packet related to green color
	last_blue_packet = DMX_packet[Blue_Address];										// Hold the last received DMX packet related to blue color
	last_white_packet = DMX_packet[White_Address];									// Hold the last received DMX packet related to white color
	
	current_page = Main_LightDimming;																// Set Main_LightDimming page as default page
	page = Page_Main;																								// Set Page_Main page as category page
	
	//save_light(100,Red_Home);

	
  while (1)
  {
		/*
		save_address(50);
		HAL_Delay(50);
		
		*/
		if(ERR_status == FE_ON)																												// Check UART frame error flag
		{
			__HAL_UART_DISABLE_IT(&huart2 ,UART_IT_RXNE);																// Clear UART it flag
			ERR_status = FE_OFF;																												// Check UART frame error flag																		
			if(DMX_packet[Blue_Address] != last_blue_packet && DMX_packet[0] == 0) 			// Buffer received packet
			{
				if(needed_white_light <= 1)																								// Check if the received data is valid																						
				{
					needed_blue_light = DMX_packet[Blue_Address] * Blue_MUL;
					last_blue_packet = DMX_packet[Blue_Address];
					page = Page_DMXDim;
					current_page = Blue_DMXDim;
				}
				else
					needed_blue_light = 1;
				
			}
			if(DMX_packet[Green_Address] != last_green_packet && DMX_packet[0] == 0)	// Buffer received packet
			{
				if(needed_white_light <= 1)																							// Check if the received data is valid
				{
					needed_green_light = DMX_packet[Green_Address] * Green_MUL;
					last_green_packet = DMX_packet[Green_Address];
					page = Page_DMXDim;
					current_page = Green_DMXDim;
				}
				else
					needed_green_light = 1;
				
			}
			if(DMX_packet[White_Address] != last_white_packet && DMX_packet[0] == 0)	// Buffer received packet
			{
				if(needed_red_light <= 1 & needed_green_light <= 1 & needed_blue_light <= 1) 	// Check if the received data is valid
				{
					needed_white_light = DMX_packet[White_Address] * White_MUL;
					last_white_packet = DMX_packet[White_Address];
					page = Page_DMXDim;
					current_page = White_DMXDim;
				}
				else
					needed_white_light = 1;
				
				
			}
			if(DMX_packet[Red_Address] != last_red_packet && DMX_packet[0] == 0)			// Buffer received packet
			{
				if(needed_white_light <= 1)																							// Check if the received data is valid
				{
					needed_red_light = DMX_packet[Red_Address] * Red_MUL;
					last_red_packet = DMX_packet[Red_Address];
					page = Page_DMXDim;
					current_page = Red_DMXDim;
				}
				else
					needed_red_light = 1;
				
			}
		}
	
		if(temp_check)																							// Check if the temperature sensor is available
		{
		
			try_out++;
			 if (TM_DS18B20_Is(DS_ROM[0]))
				 if (!TM_DS18B20_AllDone(&OW))
					 HAL_Delay(1); 
				 if (TM_DS18B20_Read(&OW, DS_ROM[0], &tempp))
					 TM_DS18B20_StartAll(&OW);
			
			if(try_out >= 50)																				// Try out
			{
				temp_check = 0;
				try_out = 0;
			}
			if(tempp != 0)																					// Check if received temperature data is valid
			{
				temperature_value = tempp * 10;
				tempp = 0;
				temp_check = 0;
				try_out = 0;
				//page = Page_Main;
				//current_page = Show_Temp;
			}
		}
		if(page == Page_Main) 																	 // Check if current illustrating group is Page_Main
		{
			switch (current_page)
			{
				case Main_LightDimming:															 // Set light values in this page
				{
					if(current_page != last_page)
					{
						write_main_LightDimming();
						last_page = Main_LightDimming;
					}
					if(DOWN)
					{
						current_page = Main_Settings;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Main_About;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_Light;
						current_page = Light_ManualDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Main_Settings:																   // Set DMX address in this page
				{
					if(current_page != last_page)
					{
						write_main_Settings();
						last_page = Main_Settings;
					}
					if(DOWN)
					{
						current_page = Main_Temperature;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Main_LightDimming;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_setting;
						current_page = Settings_DMXAddress;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Main_Temperature:															 // Check the bulk temperature in this page
				{
					if(current_page != last_page)
					{
						Write_main_Temperature();
						last_page = Main_Temperature;
					}
					if(DOWN)
					{
						current_page = Main_About;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Main_Settings;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Show_Temp;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Main_About:																		 // A brief introduction about the manufacturer
				{
					if(current_page != last_page)
					{
						write_main_About();
						last_page = Main_About;
					}
					if(DOWN)
					{
						current_page = Main_LightDimming;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Main_Temperature;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Show_About;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Show_About:																		 // Contact information
				{
					if(current_page != last_page)
					{
						write_About();
						last_page = Show_About;
					}
					if(UP)
					{
						current_page = Show_About;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(DOWN)
					{
						current_page = Show_About;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Main_About;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Show_Temp:																			 // Show the bulk temperature percisely
				{
					if(current_page != last_page)
					{
						write_LED_Temperature(temp_value);
						last_page = Show_Temp;
					}
					write_temp(temperature_value);
					if(UP)
					{
						current_page = Show_Temp;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(DOWN)
					{
						current_page = Show_Temp;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Main_Temperature;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
			}
			
		}
		
		else if(page == Page_Light)															 // Check if current illustrating group is Page_Light
		{
			switch (current_page)
			{
				case Light_ManualDim:																 // Dim the light manually
				{
					if(current_page != last_page)
					{
						write_Light_Manual_Dim();
						last_page = Light_ManualDim;
					}
					if(DOWN)
					{
						current_page = Light_DMXDim;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Light_BCK;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_MauanlDim;
						current_page = ManualDim_Red;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Light_DMXDim: 																	 // Show the received DMX value
				{
					if(current_page != last_page)
					{
						write_Light_DMX_Dim();
						last_page = Light_DMXDim;
					}
					if(DOWN)
					{
						current_page = Light_BCK;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Light_ManualDim;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_DMXDim;
						current_page = DMXDim_Red;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Light_BCK:																			 // Get back to the previous page
				{
					if(current_page != last_page)
					{
						write_Light_Back_to_main();
						last_page = Light_BCK;
					}
					if(DOWN)
					{
						current_page = Light_ManualDim;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Light_DMXDim;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_Main;
						current_page = Main_LightDimming;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
			}
		}
		else if(page == Page_MauanlDim)													 // Check if current illustrating group is Page_MauanlDim
		{
			switch (current_page)
			{
				case ManualDim_Red:																	 //	Dim the red light manually
				{
					if(current_page != last_page)
					{
						write_ManualDim_Red();
						last_page = ManualDim_Red;
					}
					if(DOWN)
					{
						current_page = ManualDim_Green;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = ManualDim_BCK;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Red_ManualDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case ManualDim_Green:																 //	Dim the green light manually
				{
					if(current_page != last_page)
					{
						write_ManualDim_Green();
						last_page = ManualDim_Green;
					}
					if(DOWN)
					{
						current_page = ManualDim_Blue;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = ManualDim_Red;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Green_ManualDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case ManualDim_Blue:																 //	Dim the blue light manually
				{
					if(current_page != last_page)
					{
						write_ManualDim_Blue();
						last_page = ManualDim_Blue;
					}
					if(DOWN)
					{
						current_page = ManualDim_White;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = ManualDim_Green;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Blue_ManualDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case ManualDim_White:																 //	Dim the white light manually
				{
					if(current_page != last_page)
					{
						write_ManualDim_White();
						last_page = ManualDim_White;
					}
					if(DOWN)
					{
						current_page = ManualDim_BCK;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = ManualDim_Blue;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = White_ManualDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case ManualDim_BCK:																	 //	Dim the red light manually
				{
					if(current_page != last_page)
					{
						write_ManualDim_BCK();
						last_page = ManualDim_BCK;
					}
					if(DOWN)
					{
						current_page = ManualDim_Red;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = ManualDim_White;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_Main;
						current_page = Main_LightDimming;
						while(ENTER);
						HAL_Delay(Wait);
					}
					break;
				}
				
				case Red_ManualDim:																	 // Enter the desire value
				{
					if(current_page != last_page)
					{
						display_red_light = toDispaly(Red_Home , needed_red_light);
						
						write_Red_ManualDim();
						write_big_font(display_red_light);
						last_page = Red_ManualDim;
					}
					if((DOWN && display_red_light > 0) && (needed_white_light <= 1))
					{
						write_big_font(--display_red_light);
						needed_red_light = toLight(Red_Home ,display_red_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Chann*el_light_Red ,needed_red_light);
						while(DOWN && display_red_light > 0)
						{
							write_big_font(--display_red_light);
							needed_red_light = toLight(Red_Home ,display_red_light);
							HAL_Delay(Wait);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if((UP && display_red_light < 255) && (needed_white_light <= 1))
					{
						write_big_font(++display_red_light);
						needed_red_light = toLight(Red_Home ,display_red_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(UP && display_red_light < 255)
						{
							write_big_font(++display_red_light);
							needed_red_light = toLight(Red_Home ,display_red_light);
							HAL_Delay(Wait);
						//	__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = ManualDim_Red;
					/*	if(*/save_light(needed_red_light ,Red_Home); /*!= HAL_OK)
						{  write_factory_logo(); while(1);}*/
							
						while(ENTER);
						HAL_Delay(Wait);
					}
					break;
				}
				case Green_ManualDim:														     // Enter the desire value
				{
					if(current_page != last_page)
					{
						display_green_light = toDispaly(Green_Home , needed_green_light);
						
						write_Green_ManualDim();
						write_big_font(display_green_light);
						last_page = Green_ManualDim;
					}
					if((DOWN && display_green_light > 0 )&& needed_white_light <= 1)
					{
						write_big_font(--display_green_light);
						needed_green_light = toLight(Green_Home ,display_green_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(DOWN && display_green_light > 0)
						{
							write_big_font(--display_green_light);
							needed_green_light = toLight(Green_Home ,display_green_light);
							HAL_Delay(Wait);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if((UP && display_green_light < 255) && needed_white_light <= 1)
					{
						write_big_font(++display_green_light);
						needed_green_light = toLight(Green_Home ,display_green_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(UP && display_green_light < 255)
						{
							write_big_font(++display_green_light);
							needed_green_light = toLight(Green_Home ,display_green_light);
							HAL_Delay(Wait);
						//	__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = ManualDim_Green;
						save_light(needed_green_light ,Green_Home);
						while(ENTER);
						HAL_Delay(Wait);
					}
					break;
				}
				case Blue_ManualDim:																 // Enter the desire value
				{
					if(current_page != last_page)
					{
						display_blue_light = toDispaly(Blue_Home , needed_blue_light);
						
						write_Blue_ManualDim();
						write_big_font(display_blue_light);
						last_page = Blue_ManualDim;
					}
					if((DOWN && display_blue_light > 0) && (needed_white_light <= 1))
					{
						write_big_font(--display_blue_light);
						needed_blue_light = toLight(Blue_Home ,display_blue_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(DOWN && display_blue_light > 0)
						{
							write_big_font(--display_blue_light);
							needed_blue_light = toLight(Blue_Home ,display_blue_light);
							HAL_Delay(Wait);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if((UP && display_blue_light < 255) && needed_white_light <= 1)
					{
						write_big_font(++display_blue_light);
						needed_blue_light = toLight(Blue_Home ,display_blue_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(UP && display_blue_light < 255)
						{
							write_big_font(++display_blue_light);
							needed_blue_light = toLight(Blue_Home ,display_blue_light);
							HAL_Delay(Wait);
						//	__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = ManualDim_Blue;
						save_light(needed_blue_light ,Blue_Home);
						while(ENTER);
						HAL_Delay(Wait);
					}
					break;
				}
				case White_ManualDim:														  	 // Enter the desire value
				{
					if(current_page != last_page)
					{
						display_white_light = toDispaly(White_Home , needed_white_light);
						
						write_White_ManualDim();
						write_big_font(display_white_light);
						last_page = White_ManualDim;
					}
					if((DOWN && display_white_light > 0) && (needed_red_light <= 1 & needed_green_light <= 1 & needed_blue_light <= 1 ))
					{
						write_big_font(--display_white_light);
						needed_white_light = toLight(White_Home ,display_white_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(DOWN && display_white_light > 0)
						{
							write_big_font(--display_white_light);
							needed_white_light = toLight(White_Home ,display_white_light);
							HAL_Delay(Wait);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if(UP && display_white_light < 255 && (needed_red_light <= 1 & needed_green_light <= 1 & needed_blue_light <= 1 ))
					{
						write_big_font(++display_white_light);
						needed_white_light = toLight(White_Home ,display_white_light);
						HAL_Delay(Wait_Long);
						//__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						while(UP && display_white_light < 255)
						{
							write_big_font(++display_white_light);
							needed_white_light = toLight(White_Home ,display_white_light);
							HAL_Delay(Wait);
						//	__HAL_TIM_SET_COMPARE(Timer_Light_Red ,Channel_light_Red ,needed_red_light);
						}
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = ManualDim_White;
						save_light(needed_white_light ,White_Home);
						while(ENTER);
						HAL_Delay(Wait);
					}
					break;
				}
				
			}
		}
		else if(page == Page_DMXDim)														 // Check if current illustrating group is Page_DMXDim
		{
			switch (current_page)
			{
				case DMXDim_Red:																		 // Enter to show the received DMX value
				{
					if(current_page != last_page)
					{
						write_ManualDim_Red();
						last_page = DMXDim_Red;
					}
					if(DOWN)
					{
						current_page = DMXDim_Green;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = DMXDim_BCK;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Red_DMXDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case DMXDim_Green:																	 // Enter to show the received DMX value													
				{
					if(current_page != last_page)
					{
						write_ManualDim_Green();
						last_page = DMXDim_Green;
					}
					if(DOWN)
					{
						current_page = DMXDim_Blue;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = DMXDim_Red;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Green_DMXDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case DMXDim_Blue: 																	 // Enter to show the received DMX value
				{
					if(current_page != last_page)
					{
						write_ManualDim_Blue();
						last_page = DMXDim_Blue;
					}
					if(DOWN)
					{
						current_page = DMXDim_White;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = DMXDim_Green;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = Blue_DMXDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case DMXDim_White:																	 // Enter to show the received DMX value
				{
					if(current_page != last_page)
					{
						write_ManualDim_White();
						last_page = DMXDim_White;
					}
					if(DOWN)
					{
						current_page = DMXDim_BCK;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = DMXDim_Blue;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = White_DMXDim;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case DMXDim_BCK:																		 // Get back to the previous page
				{
					if(current_page != last_page)
					{
						write_ManualDim_BCK();
						last_page = DMXDim_BCK;
					}
					if(DOWN)
					{
						current_page = DMXDim_Red;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = DMXDim_White;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_Main;
						current_page = Main_LightDimming;
						while(ENTER);
						HAL_Delay(Wait);
					}
					break;
				}
				case Red_DMXDim:																		 // Show the received DMX value
				{
					if(current_page != last_page)
					{
						write_DMXDim_Red();
						last_page = Red_DMXDim;
					}
					write_big_font(DMX_packet[Red_Address]);
					if(DOWN)
					{
						current_page = Red_DMXDim;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Red_DMXDim;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = DMXDim_Red;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Green_DMXDim: 																	 // Show the received DMX value
				{
					if(current_page != last_page)
					{
						write_DMXDim_Green();
						last_page = Green_DMXDim;
					}
					write_big_font(DMX_packet[Green_Address]);
					if(DOWN)
					{
						current_page = Green_DMXDim;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Green_DMXDim;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = DMXDim_Green;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Blue_DMXDim:																		 // Show the received DMX value
				{
					if(current_page != last_page)
					{
						write_DMXDim_Blue();
						last_page = Blue_DMXDim;
					}
					write_big_font(DMX_packet[Blue_Address]);
					if(DOWN)
					{
						current_page = Blue_DMXDim;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Blue_DMXDim;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = DMXDim_Blue;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case White_DMXDim: 																	 // Show the received DMX value
				{
					if(current_page != last_page)
					{
						write_DMXDim_White();
						last_page = White_DMXDim;
					}
					write_big_font(DMX_packet[White_Address]);
					if(DOWN)
					{
						current_page = White_DMXDim;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = White_DMXDim;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = DMXDim_White;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
			}
		}
		else if(page == Page_setting)														 // Check if current illustrating group is Page_setting
		{ 
			switch (current_page)
			{
				case Settings_DMXAddress: 													 // Enter to change the DMX address
				{
					if(current_page != last_page)
					{
						write_Settings_DMXADD(local_DMX_address);
						last_page = Settings_DMXAddress;
					}
					if(DOWN)
					{
						current_page = Settings_BCK;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Settings_BCK;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						current_page = DMXAddress_Red;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case DMXAddress_Red:																 // Set the red light address. the following lights will assign incrementally
				{
					if(current_page != last_page)
					{
						x = local_DMX_address;
						write_DMX_address(x);
						last_page = DMXAddress_Red;
					}
					if(DOWN)
					{
						if(--x < 1)
							x = 512;
						write_big_font(x);
						HAL_Delay(Wait_Long);
						while(DOWN)
						{
							if(--x < 1)
								x = 512;
							write_big_font(x);
							HAL_Delay(Wait);
						}
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						if(++x > 512)
							x = 1;
						write_big_font(x);
						HAL_Delay(Wait_Long);
						while(UP)
						{
							if(++x > 512)
								x = 1;
							write_big_font(x);
							HAL_Delay(Wait);
						}
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						local_DMX_address = x;
						last_red_packet = DMX_packet[Red_Address];
						last_green_packet = DMX_packet[Green_Address];
						last_blue_packet = DMX_packet[Blue_Address];
						last_white_packet = DMX_packet[White_Address];
						current_page = Settings_DMXAddress;
						save_address(local_DMX_address);
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}
				case Settings_BCK:																   // Get back to the previous page
				{
					if(current_page != last_page)
					{
						write_Settings_Back_to_main();
						last_page = Settings_BCK;
					}
					if(DOWN)
					{
						current_page = Settings_DMXAddress;
						while(DOWN);
						HAL_Delay(Wait);
					}
					else if(UP)
					{
						current_page = Settings_DMXAddress;
						while(UP);
						HAL_Delay(Wait);
					}
					else if(ENTER)
					{
						page = Page_Main;
						current_page = Main_Settings;
						while(ENTER);
						HAL_Delay(Wait);
					}
					
					break;
				}	
		}
	}
 
 
	}

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/100);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 1000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = Pulse_Width - 1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 400;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2550;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 400;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2550;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM5 init function */
static void MX_TIM5_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 7200;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 10000;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM6 init function */
static void MX_TIM6_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 400;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2550;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 400;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 2550;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM8 init function */
static void MX_TIM8_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 3600 - 1;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim8);

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 250000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_2;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 1, 0);
 //	HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, WP_Pin|RST_Pin|CS2_Pin|E_Pin 
                          |RW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CS1_Pin|D7_Pin|D6_Pin|D5_Pin 
                          |D4_Pin|D3_Pin|D2_Pin|D1_Pin 
                          |D0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RS_Pin|TD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : WP_Pin RST_Pin CS2_Pin E_Pin 
                           RW_Pin */
  GPIO_InitStruct.Pin = WP_Pin|RST_Pin|CS2_Pin|E_Pin 
                          |RW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : CS1_Pin D7_Pin D6_Pin D5_Pin 
                           D4_Pin D3_Pin D2_Pin D1_Pin 
                           D0_Pin */
  GPIO_InitStruct.Pin = CS1_Pin|D7_Pin|D6_Pin|D5_Pin 
                          |D4_Pin|D3_Pin|D2_Pin|D1_Pin 
                          |D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : RS_Pin TD_Pin */
  GPIO_InitStruct.Pin = RS_Pin|TD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : P1_Pin P2_Pin P3_Pin */
  GPIO_InitStruct.Pin = P1_Pin|P2_Pin|P3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */
uint8_t save_light (uint16_t light_to_save , uint8_t colour)
{
	uint8_t buffer[3] ,status;
	buffer[0] = light_to_save >> 8;
	buffer[1] = light_to_save;
	
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_RESET);
	HAL_Delay(delay_mem);
	status = HAL_I2C_Mem_Write(&hi2c1 ,Memory_Device_Address ,Light_Start_Memory_Home + colour ,I2C_MEMADD_SIZE_8BIT ,buffer ,2 ,10);
	HAL_Delay(delay_mem);
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_SET);
	
	return status;
}

uint8_t save_address (uint16_t addr_to_save)
{
	uint8_t buffer[3] ,status;
	buffer[0] = addr_to_save >> 8;
	buffer[1] = addr_to_save;
	
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_RESET);
	HAL_Delay(delay_mem);
	status = HAL_I2C_Mem_Write(&hi2c1 ,Memory_Device_Address ,Address_Start_Memory_Home ,I2C_MEMADD_SIZE_8BIT ,buffer ,2 ,10);	
	HAL_Delay(delay_mem);
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_SET);
	return status;
}

uint8_t load_light (uint16_t *light_to_load, uint8_t colour)
{
	uint8_t buffer[3] , status;
	uint16_t load = 0;
	
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_RESET);
	HAL_Delay(delay_mem);
	status = HAL_I2C_Mem_Read(&hi2c1 ,Memory_Device_Address ,Light_Start_Memory_Home + colour ,I2C_MEMADD_SIZE_8BIT ,buffer ,2 ,1000);	HAL_Delay(delay_mem);
	HAL_Delay(delay_mem);
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_SET);
	
	load = buffer[0] << 8;
	load |= buffer[1];
	
	*light_to_load = load;
	return status;
	
}


uint8_t load_address (uint16_t *addr_to_load)
{
	uint8_t buffer[3] , status;
	uint16_t load = 0;
	
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_RESET);
	HAL_Delay(delay_mem);
	status = HAL_I2C_Mem_Read(&hi2c1 ,Memory_Device_Address ,Address_Start_Memory_Home ,I2C_MEMADD_SIZE_8BIT ,buffer ,2 ,1000);	HAL_Delay(delay_mem);
	HAL_Delay(delay_mem);
	HAL_GPIO_WritePin(WP_GPIO_Port ,WP_Pin ,GPIO_PIN_SET);
	
	load = buffer[0] << 8;
	load |= buffer[1];
	
	*addr_to_load = load;
	return status;
	

}




uint8_t toDispaly(uint8_t colour, uint16_t Light)
{
	if(colour == Red_Home)
	{
		return Light / Red_DIV;
	}
	else if(colour == Green_Home)
	{
		return Light / Green_DIV;
	}
	else if(colour == Blue_Home)
	{
		return Light / Blue_DIV;
	}
	else if(colour == White_Home)
	{
		return Light / White_DIV;
	}
	
}
uint16_t toLight(uint8_t colour, uint8_t Display)
{
	if(colour == Red_Home)
	{
		return Display * Red_MUL;
	}
	else if(colour == Green_Home)
	{
		return Display * Green_MUL;
	}
	else if(colour == Blue_Home)
	{
		return Display * Blue_MUL;
	}
	else if(colour == White_Home)
	{
		return Display * White_MUL;
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
