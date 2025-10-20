/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "vl53l5cx_api.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;
I2C_HandleTypeDef hi2c5;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim15;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
typedef struct
{
	VL53L5CX_Configuration 	Sensor;
	VL53L5CX_ResultsData 	Results_Sensor;
	uint8_t SensorisAlive;
	uint8_t status;

} VL53L5CX_Sensor;

VL53L5CX_Sensor Sensor1;
VL53L5CX_Sensor Sensor2;
VL53L5CX_Sensor Sensor3;
VL53L5CX_Sensor Sensor4;
VL53L5CX_Sensor Sensor5;

uint8_t GND1_Pin_read = 0;
uint8_t VIN1_Pin_read = 0;

uint16_t Position = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_I2C4_Init(void);
static void MX_I2C5_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM15_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


uint8_t get_data_by_polling(VL53L5CX_Sensor *Sensor){
	uint8_t p_data_ready, status = 0;
	HAL_StatusTypeDef ReadOK = HAL_OK;
	status = vl53l5cx_check_data_ready(&Sensor->Sensor, &p_data_ready);
	if(p_data_ready){
		status = vl53l5cx_get_ranging_data(&Sensor->Sensor, &Sensor->Results_Sensor);

		do{
			ReadOK = HAL_UART_Transmit(&huart2, (uint8_t*)&Position, 2, 1000);
		}while(ReadOK != HAL_OK);

		do{
			ReadOK = HAL_UART_Transmit(&huart2, (uint8_t*)(&Sensor->Sensor.platform.SensorNumber), 2, 1000);
		}while(ReadOK != HAL_OK);

		do{
			ReadOK = (HAL_UART_Transmit(&huart2, (uint8_t*)(&Sensor->Results_Sensor.distance_mm), 128, 1000) == HAL_BUSY);
		}while(ReadOK != HAL_OK);

		if(ReadOK == HAL_OK){
			HAL_Delay(10);
			return 1;
		}
		else{
			HAL_Delay(10);
			return 0;
		}
	}
	HAL_Delay(10);
	return 0;
}

void ReturnHome(void){

	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);

	for(int i = 0; i<50; i++){
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);
	}

	do{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);
		GND1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11);
		VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);
	} while(!((VIN1_Pin_read == 1) && GND1_Pin_read == 1));

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_Delay(100);

	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);

}

void MoveNextPos(){

	uint8_t OldVal = 1;
	uint8_t NewVal = 0;
	HAL_Delay(100);

	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);

	do{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);

		VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);
		uint16_t p = 0;

		while(1){
			if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == 1){
				p++;
				if(p > 20000){
					break;
				}
			}
			else{
				VIN1_Pin_read = 0;
				break;
			}
		}
	} while((VIN1_Pin_read == 0));

	HAL_Delay(100);

	VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);

	do{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);

		VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);
		NewVal = VIN1_Pin_read;

		uint16_t p = 0;
		while(1){
			if(NewVal == HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14)){
				p++;
				if(p > 20000){
					break;
				}
			}
			else{
				NewVal = OldVal;
				break;
			}
		}
	} while(!(OldVal > NewVal)); //Premikam dokler nisem nazaj na nic


	HAL_Delay(100);
	VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);

	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
	HAL_Delay(15);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
	HAL_Delay(15);
	OldVal = NewVal;

	do{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
		HAL_Delay(15);

		VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);
		NewVal = VIN1_Pin_read;

		uint16_t p = 0;
		while(1){
			if(NewVal == HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14)){
				p++;
				if(p > 20000){
					break;
				}
			}
			else{
				NewVal = OldVal;
				break;
			}
		}
	} while(!(OldVal < NewVal)); //Premikam dokler nisem nazaj na 1

	VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);

	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
	HAL_Delay(15);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
	HAL_Delay(15);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	Position = Position + 1;

	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);

	HAL_Delay(100);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	for(int i = 0; i<50000; i++){
		asm("nop");
	}

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_I2C4_Init();
  MX_I2C5_Init();
  MX_TIM3_Init();
  MX_TIM15_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
    NVIC_EnableIRQ(SysTick_IRQn);
    HAL_Delay(100);

  	Sensor1.Sensor.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
  	Sensor1.Sensor.platform.I2C = &hi2c1;
  	Sensor1.Sensor.platform.SensorNumber = '1';

  	Sensor2.Sensor.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
  	Sensor2.Sensor.platform.I2C = &hi2c2;
  	Sensor2.Sensor.platform.SensorNumber = '2';

  	Sensor3.Sensor.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
  	Sensor3.Sensor.platform.I2C = &hi2c3;
  	Sensor3.Sensor.platform.SensorNumber = '3';

  	Sensor4.Sensor.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
  	Sensor4.Sensor.platform.I2C = &hi2c4;
  	Sensor4.Sensor.platform.SensorNumber = '4';

  	Sensor5.Sensor.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
  	Sensor5.Sensor.platform.I2C = &hi2c5;
  	Sensor5.Sensor.platform.SensorNumber = '5';


  	Sensor1.status = vl53l5cx_is_alive(&Sensor1.Sensor, &Sensor1.SensorisAlive);
  	if(Sensor1.SensorisAlive){
  		Sensor1.status = vl53l5cx_init(&Sensor1.Sensor);
  		Sensor1.status = vl53l5cx_set_ranging_frequency_hz(&Sensor1.Sensor, 10);	// Set 2Hz ranging frequency
  		Sensor1.status = vl53l5cx_set_resolution(&Sensor1.Sensor, VL53L5CX_RESOLUTION_8X8);
  		Sensor1.status = vl53l5cx_set_ranging_mode(&Sensor1.Sensor, VL53L5CX_RANGING_MODE_CONTINUOUS);  // Set mode continuous
  	}

  	Sensor2.status = vl53l5cx_is_alive(&Sensor2.Sensor, &Sensor2.SensorisAlive);
  	if(Sensor2.SensorisAlive){
  		Sensor2.status = vl53l5cx_init(&Sensor2.Sensor);
  		Sensor2.status = vl53l5cx_set_ranging_frequency_hz(&Sensor2.Sensor, 10);	// Set 2Hz ranging frequency
  		Sensor2.status = vl53l5cx_set_resolution(&Sensor2.Sensor, VL53L5CX_RESOLUTION_8X8);
  		Sensor2.status = vl53l5cx_set_ranging_mode(&Sensor2.Sensor, VL53L5CX_RANGING_MODE_CONTINUOUS);  // Set mode continuous
  	}
  	Sensor3.status = vl53l5cx_is_alive(&Sensor3.Sensor, &Sensor3.SensorisAlive);
  	if(Sensor3.SensorisAlive){
  		Sensor3.status = vl53l5cx_init(&Sensor3.Sensor);
  		Sensor3.status = vl53l5cx_set_ranging_frequency_hz(&Sensor3.Sensor, 10);	// Set 2Hz ranging frequency
  		Sensor3.status = vl53l5cx_set_resolution(&Sensor3.Sensor, VL53L5CX_RESOLUTION_8X8);
  		Sensor3.status = vl53l5cx_set_ranging_mode(&Sensor3.Sensor, VL53L5CX_RANGING_MODE_CONTINUOUS);  // Set mode continuous
  	}
  	Sensor4.status = vl53l5cx_is_alive(&Sensor4.Sensor, &Sensor4.SensorisAlive);
  	if(Sensor4.SensorisAlive){
  		Sensor4.status = vl53l5cx_init(&Sensor4.Sensor);
  		Sensor4.status = vl53l5cx_set_ranging_frequency_hz(&Sensor4.Sensor, 10);	// Set 2Hz ranging frequency
  		Sensor4.status = vl53l5cx_set_resolution(&Sensor4.Sensor, VL53L5CX_RESOLUTION_8X8);
  		Sensor4.status = vl53l5cx_set_ranging_mode(&Sensor4.Sensor, VL53L5CX_RANGING_MODE_CONTINUOUS);  // Set mode continuous
  	}
  	Sensor5.status = vl53l5cx_is_alive(&Sensor5.Sensor, &Sensor5.SensorisAlive);
  	if(Sensor5.SensorisAlive){
  		Sensor5.status = vl53l5cx_init(&Sensor5.Sensor);
  		Sensor5.status = vl53l5cx_set_ranging_frequency_hz(&Sensor5.Sensor, 10);	// Set 2Hz ranging frequency
  		Sensor5.status = vl53l5cx_set_resolution(&Sensor5.Sensor, VL53L5CX_RESOLUTION_8X8);
  		Sensor5.status = vl53l5cx_set_ranging_mode(&Sensor5.Sensor, VL53L5CX_RANGING_MODE_CONTINUOUS);  // Set mode continuous
  	}

  	vl53l5cx_start_ranging(&Sensor1.Sensor);
  	vl53l5cx_start_ranging(&Sensor2.Sensor);
  	vl53l5cx_start_ranging(&Sensor3.Sensor);
  	vl53l5cx_start_ranging(&Sensor4.Sensor);
  	vl53l5cx_start_ranging(&Sensor5.Sensor);

  	uint8_t StartRead = 0;
  	Position = 0;
//  	ReturnHome();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    	HAL_UART_Receive(&huart2, &StartRead, 1, 500);


//		GND1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11);
//		VIN1_Pin_read = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);


    	if(StartRead == '7'){
//    		MoveNextPos();
    		StartRead = 0;
    		for(int i = 0; i < 1; i++){

				while(!get_data_by_polling(&Sensor1)){
					asm("nop");
				}
				while(!get_data_by_polling(&Sensor5)){
					asm("nop");
				}
				while(!get_data_by_polling(&Sensor3)){
					asm("nop");
				}
				while(!get_data_by_polling(&Sensor4)){
					asm("nop");
				}
				while(!get_data_by_polling(&Sensor2)){
					asm("nop");
				}

//				MoveNextPos();
//				if(Position >= 8){
//					Position = 0;
//					HAL_Delay(1000);
//				}

    		}

    	}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_I2C2
                              |RCC_PERIPHCLK_I2C3|RCC_PERIPHCLK_I2C5
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInitStruct.PLL3.PLL3M = 4;
  PeriphClkInitStruct.PLL3.PLL3N = 15;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 6;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C1235CLKSOURCE_PLL3;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00100618;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00100618;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00100618;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C3);
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00100618;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C4);
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief I2C5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C5_Init(void)
{

  /* USER CODE BEGIN I2C5_Init 0 */

  /* USER CODE END I2C5_Init 0 */

  /* USER CODE BEGIN I2C5_Init 1 */

  /* USER CODE END I2C5_Init 1 */
  hi2c5.Instance = I2C5;
  hi2c5.Init.Timing = 0x00100618;
  hi2c5.Init.OwnAddress1 = 0;
  hi2c5.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c5.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c5.Init.OwnAddress2 = 0;
  hi2c5.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c5.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c5.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c5, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c5, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C5);
  /* USER CODE BEGIN I2C5_Init 2 */

  /* USER CODE END I2C5_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 0;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 65535;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim15, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 2000000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LPn2_Pin|LPn4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LPn3_GPIO_Port, LPn3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, M0_Pin|M1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, STEP_Pin|DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LPn5_GPIO_Port, LPn5_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LPn1_GPIO_Port, LPn1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : VCC1_Pin GND1_Pin INT1_Pin */
  GPIO_InitStruct.Pin = VCC1_Pin|GND1_Pin|INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : LPn2_Pin LPn4_Pin */
  GPIO_InitStruct.Pin = LPn2_Pin|LPn4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : INT2_Pin INT4_Pin */
  GPIO_InitStruct.Pin = INT2_Pin|INT4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LPn3_Pin M0_Pin M1_Pin */
  GPIO_InitStruct.Pin = LPn3_Pin|M0_Pin|M1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : INT3_Pin */
  GPIO_InitStruct.Pin = INT3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STEP_Pin */
  GPIO_InitStruct.Pin = STEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(STEP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DIR_Pin */
  GPIO_InitStruct.Pin = DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DIR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LPn5_Pin */
  GPIO_InitStruct.Pin = LPn5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LPn5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : INT5_Pin */
  GPIO_InitStruct.Pin = INT5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LPn1_Pin */
  GPIO_InitStruct.Pin = LPn1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LPn1_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
