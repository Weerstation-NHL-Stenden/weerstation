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
#include "BME280_STM32.h"
#include "sgp30.h"
#include "string.h"
#include "stdio.h"
#include "SI1145.h"
#include "sgp30.h"
#include <stdarg.h>
#include "Weather_Meter_Kit.h"
#include <stdlib.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SI1145_ADDRESS (0X60 << 1) //The I2C address of the SI1145 which is used as the UV index sensor, it is shifted by 1 to the left because this is said in the ST documentation.
#define SI1145_MESSURE_UV_ADDRESS 0x2C //The address for the command to measure the UV index.
#define RainSensor_Pin GPIO_PIN_0 //The pin to which the rain sensor is connected.
#define RainSensor_GPIO_Port GPIOA
#define WindSpeed_Pin GPIO_PIN_1
#define I2C_TIMEOUT 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
void MeasureLightIntensity(void);
void MeasureUltraVioletLight(void);
void writeValue(uint8_t reg, uint8_t val);
//void writeParam(uint8_t val1, uint8_t val2);
void SI1145Init(void);
void callibrateUVSensor(void);
void sgp30_test(void);
void MeasureWindDirection(void);
float getClosestWindDirection(float voltage);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void CalculateAmountOfRain(void);
void getAdcValues(void);
uint8_t read8(uint8_t reg);
bool write_then_read(uint8_t *readBuffer, uint16_t readLen, const uint8_t *writeBuffer, uint16_t writeLen);
uint8_t writeParam(uint8_t p, uint8_t v);
uint16_t read16(uint8_t reg);
//uint8_t iic_init(void);
//uint8_t iic_deinit(void);
//uint8_t iic_write_cmd(uint8_t addr, uint8_t *buf, uint16_t len);
//uint8_t iic_read_cmd(uint8_t addr, uint8_t *buf, uint16_t len);
//void delay_ms(uint32_t ms);
//void debug_print(const char *const fmt, ...);
;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float Temperature, Pressure, Humidity, VoltageLDR, ResistanceLDR, Lux, LightIntensity, UVIndex, test, VoltageWind, resistanceWind, WindDirection, mmRain, windSpeedKmh;
uint16_t co2, tvoc, AdcValueLDR, AdcValueWind;
char DataBuffer[100];
uint8_t sgp30Buffer[2], buf[6];
sgp30_handle_t sgp30;
int rainCounter = 0, windCounter = 0, lastCheckedSecond = 0, lastCheckedDate = 0;
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;
uint16_t adcResults[2];
uint8_t convComplete = 0;

int windDirectionADCs[8] = {WMK_ADC_ANGLE_N, WMK_ADC_ANGLE_NE, WMK_ADC_ANGLE_E, WMK_ADC_ANGLE_SE, WMK_ADC_ANGLE_S, WMK_ADC_ANGLE_SW, WMK_ADC_ANGLE_W, WMK_ADC_ANGLE_NW};
static float STM32VoltageUsual = 3.3, LDRResistance = 10000.0, MaxAdcValue = 4095.0, Wm2PerLuxForSunRays = 0.0079;

void HAL_ADC_ConvCpltCallBack(ADC_HandleTypeDef* hadc)
{
	convComplete = 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_ADC_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  BME280_Config(OSRS_1, OSRS_1, OSRS_1, MODE_NORMAL, T_SB_0p5, IIR_16);
  SI1145Init();
  __enable_irq();
  //DRIVER_SGP30_LINK_INIT(&sgp30, sgp30_handle_t);
  //DRIVER_SGP30_LINK_IIC_INIT(&sgp30, iic_init);
  //DRIVER_SGP30_LINK_IIC_DEINIT(&sgp30, iic_deinit);
  //DRIVER_SGP30_LINK_IIC_WRITE_COMMAND(&sgp30, iic_write_cmd);
  //DRIVER_SGP30_LINK_IIC_READ_COMMAND(&sgp30, iic_read_cmd);
  //DRIVER_SGP30_LINK_DELAY_MS(&sgp30, delay_ms);
  //DRIVER_SGP30_LINK_DEBUG_PRINT(&sgp30, debug_print);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  BME280_Measure();

	  getAdcValues();

	  MeasureLightIntensity();

	  MeasureWindDirection();

	  //MeasureUltraVioletLight();

	  //CalculateAmountOfRain();

	  //sgp30_init(&sgp30);

	  //sgp30_iaq_init(&sgp30);

	  //test = sgp30_read(&sgp30, &co2, &tvoc);

	  //sgp30_deinit(&sgp30);

	  sprintf(DataBuffer, "a%.1fa|b%.1fb|c%.0fc|d%fd|e%fe|f1f|g1g|h.2%fh|i1i|", Temperature, Humidity, Pressure, LightIntensity, WindDirection,/* windSpeedKmh, mmRain,*/ UVIndex/*, co2*/);

	  HAL_UART_Transmit(&huart1, (uint8_t*)DataBuffer, strlen(DataBuffer), 100);
	  //HAL_UART_Transmit(&huart2, (uint8_t*)rxData, strlen(rxData), 100);
	  memset(DataBuffer, 0, sizeof(DataBuffer));
	  HAL_Delay(1000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */
  //__HAL_LINKDMA(&hadc, DMA_Handle, hdma_adc);
  /* USER CODE END ADC_Init 2 */

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
  hi2c1.Init.Timing = 0x2000090E;
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
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 15;
  sTime.Minutes = 53;
  sTime.Seconds = 20;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 14;
  sDate.Year = 23;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void writeValue(uint8_t reg, uint8_t val)
{
	    uint8_t buffer[2] = {reg, val};
	    HAL_I2C_Mem_Write(&hi2c1, SI1145_ADDRESS, reg, I2C_MEMADD_SIZE_16BIT, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

/*void writeParam(uint8_t val1, uint8_t val2)
{
	writeValue(SI1145_REG_PARAMWR, val2);
	writeValue(SI1145_REG_COMMAND, val1 | SI1145_PARAM_SET);
}*/

bool write_then_read(uint8_t *readBuffer, uint16_t readLen, const uint8_t *writeBuffer, uint16_t writeLen)
{
    if (HAL_I2C_Mem_Write(&hi2c1, SI1145_ADDRESS << 1, writeBuffer[0], I2C_MEMADD_SIZE_8BIT, (uint8_t *)&writeBuffer[1], writeLen - 1, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    if (HAL_I2C_Mem_Read(&hi2c1, SI1145_ADDRESS << 1, readBuffer[0], I2C_MEMADD_SIZE_8BIT, readBuffer, readLen, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    return true;
}
uint8_t read8(uint8_t reg)
{
    uint8_t buffer[1];
    write_then_read(buffer, 1, &reg, 1);
    return buffer[0];
}

uint16_t read16(uint8_t reg)
{
  uint8_t buffer[2] = {reg, 0};
  write_then_read(buffer, 1, buffer, 2);
  return ((uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8));
}

uint8_t writeParam(uint8_t p, uint8_t v)
{
  writeValue(SI1145_REG_PARAMWR, v);
  writeValue(SI1145_REG_COMMAND, p | SI1145_PARAM_SET);
  return read8(SI1145_REG_PARAMRD);
}

void SI1145Init(void)
{
	// enable UVindex measurement coefficients!
	writeValue(SI1145_REG_UCOEFF0, 0x29);
	writeValue(SI1145_REG_UCOEFF1, 0x89);
	writeValue(SI1145_REG_UCOEFF2, 0x02);
	writeValue(SI1145_REG_UCOEFF3, 0x00);

	  // enable UV sensor
	writeParam(SI1145_PARAM_CHLIST,
	             SI1145_PARAM_CHLIST_ENUV | SI1145_PARAM_CHLIST_ENALSIR |
	                 SI1145_PARAM_CHLIST_ENALSVIS | SI1145_PARAM_CHLIST_ENPS1);
	  // enable interrupt on every sample
	writeValue(SI1145_REG_INTCFG, SI1145_REG_INTCFG_INTOE);
	writeValue(SI1145_REG_IRQEN, SI1145_REG_IRQEN_ALSEVERYSAMPLE);

	  /****************************** Prox Sense 1 */

	  // program LED current
	writeValue(SI1145_REG_PSLED12, 0x03); // 20mA for LED 1 only
	writeParam(SI1145_PARAM_PS1ADCMUX, SI1145_PARAM_ADCMUX_LARGEIR);
	  // prox sensor #1 uses LED #1
	writeParam(SI1145_PARAM_PSLED12SEL, SI1145_PARAM_PSLED12SEL_PS1LED1);
	  // fastest clocks, clock div 1
	writeParam(SI1145_PARAM_PSADCGAIN, 0);
	  // take 511 clocks to measure
	writeParam(SI1145_PARAM_PSADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
	  // in prox mode, high range
	writeParam(SI1145_PARAM_PSADCMISC,
	             SI1145_PARAM_PSADCMISC_RANGE | SI1145_PARAM_PSADCMISC_PSMODE);

	writeParam(SI1145_PARAM_ALSIRADCMUX, SI1145_PARAM_ADCMUX_SMALLIR);
	  // fastest clocks, clock div 1
	writeParam(SI1145_PARAM_ALSIRADCGAIN, 0);
	  // take 511 clocks to measure
	writeParam(SI1145_PARAM_ALSIRADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
	  // in high range mode
	writeParam(SI1145_PARAM_ALSIRADCMISC, SI1145_PARAM_ALSIRADCMISC_RANGE);

	  // fastest clocks, clock div 1
	writeParam(SI1145_PARAM_ALSVISADCGAIN, 0);
	  // take 511 clocks to measure
	writeParam(SI1145_PARAM_ALSVISADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
	  // in high range mode (not normal signal)
	writeParam(SI1145_PARAM_ALSVISADCMISC, SI1145_PARAM_ALSVISADCMISC_VISRANGE);

	  /************************/

	  // measurement rate for auto
	writeValue(SI1145_REG_MEASRATE0, 0xFF); // 255 * 31.25uS = 8ms

	  // auto run
	writeValue(SI1145_REG_COMMAND, SI1145_PSALS_AUTO);

	/*
	//writeValue(SI1145_REG_UCOEFF0, 0x29);
	//writeValue(SI1145_REG_UCOEFF1, 0x89);
	//writeValue(SI1145_REG_UCOEFF2, 0x02);
	//writeValue(SI1145_REG_UCOEFF3, 0x00);

	HAL_Delay(100);

	writeValue(SI1145_REG_HWKEY, 0x17);

	HAL_Delay(100);

	// enable UVindex measurement coefficients!
	writeValue(SI1145_REG_UCOEFF0, 0x7B);
	writeValue(SI1145_REG_UCOEFF1, 0x6B);
	writeValue(SI1145_REG_UCOEFF2, 0x01);
	writeValue(SI1145_REG_UCOEFF3, 0x00);

	// Initialize UV sensor
	writeParam(SI1145_PARAM_CHLIST, SI1145_PARAM_CHLIST_ENUV | SI1145_PARAM_CHLIST_ENALSIR | SI1145_PARAM_CHLIST_ENALSVIS | SI1145_PARAM_CHLIST_ENPS1);

	// Enable interrupts on every sample event
	writeValue(SI1145_REG_INTCFG, SI1145_REG_INTCFG_INTOE);
	writeValue(SI1145_REG_IRQEN, SI1145_REG_IRQEN_ALSEVERYSAMPLE);

	// Apply LED current for proximity LED #1
	writeValue(SI1145_REG_PSLED12, 0x03 );
	writeParam(SI1145_PARAM_PS1ADCMUX, SI1145_PARAM_ADCMUX_LARGEIR);

	// Use proximity LED #1 for sensor
	writeParam(SI1145_PARAM_PSLED12SEL, SI1145_PARAM_PSLED12SEL_PS1LED1);

	// Set ADC clock divider to fastest (value 0 according to docs)
	writeParam(SI1145_PARAM_PSADCGAIN, 0);

	// Set recovery peroid the ADC takes before making a PS measurement to 511
	writeParam(SI1145_PARAM_PSADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);

	// Proximity sensor mode set to high range
	writeParam(SI1145_PARAM_PSADCMISC, SI1145_PARAM_PSADCMISC_RANGE | SI1145_PARAM_PSADCMISC_PSMODE);

	// Use small IR photodiode (0x00)
	writeParam(SI1145_PARAM_ALSIRADCMUX, SI1145_PARAM_ADCMUX_SMALLIR);

	// Set ADC clock divider to fastest
	writeParam(SI1145_PARAM_ALSIRADCGAIN, 0);

	// Set recovery peroid the ADC takes before making a IR measurment to 511
	writeParam(SI1145_PARAM_ALSIRADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);

	// IR sensor mode set to high range
	writeParam(SI1145_PARAM_ALSIRADCMISC, SI1145_PARAM_ALSIRADCMISC_RANGE);

	// Set ADC clock divider to fastest
	writeParam(SI1145_PARAM_ALSVISADCGAIN, 0 );

	// Set recovery peroid the ADC takes before making a visible light measurment to 511
	writeParam(SI1145_PARAM_ALSVISADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);

	// IR sensor mode set to high range
	writeParam(SI1145_PARAM_ALSVISADCMISC, SI1145_PARAM_ALSVISADCMISC_VISRANGE);

	// Rate which represents time interval at which sensor wakes up to perform meassurment
	writeValue(SI1145_REG_MEASRATE0, 0xFF);

	// Autonomous ALS and PS measurment mode in a loop
	writeValue(SI1145_REG_COMMAND, SI1145_PSALS_AUTO);

	HAL_Delay(100);
	*/
}

void getAdcValues(void)
{
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)adcResults, 2);
	AdcValueLDR = adcResults[0];
	AdcValueWind = adcResults[1];
}

void MeasureLightIntensity(void)
{
	VoltageLDR = (STM32VoltageUsual * AdcValueLDR) / MaxAdcValue;  //The voltage going trough the LDR is calculated by multiplying the STMs voltage with the obtained AdcValue which is than divided by the maximum possible AdcValue
	ResistanceLDR = (LDRResistance * VoltageLDR) / (STM32VoltageUsual - VoltageLDR);
	Lux = (2500/VoltageLDR-500) / ResistanceLDR;
	LightIntensity = Wm2PerLuxForSunRays * Lux; //The LightIntensity is calculated by multiplying the amount of lux with the W/m2 per lux for sunrays.
}

void MeasureUltraVioletLight(void)
{
    uint8_t readbuffer[2];

    HAL_I2C_Mem_Read(&hi2c1, SI1145_ADDRESS, SI1145_MESSURE_UV_ADDRESS, I2C_MEMADD_SIZE_16BIT, readbuffer, 2, 100);

    UVIndex = ((float)((uint16_t)readbuffer[0] | ((uint16_t)readbuffer[1] << 8))) / 100.0f;

    memset(readbuffer, 0, sizeof(readbuffer));
}

void MeasureWindDirection(void)
{
	WindDirection = getClosestWindDirection(AdcValueWind);
}

float getClosestWindDirection(float resistance)
{
	float closestNumber = windDirectionADCs[0]; //sets the closest number as the first windDirectionVoltage
	float closestDifference = fabs(resistance - windDirectionADCs[0]); //Calculates the difference between the actual voltage and the first windDirectionVoltage.

	for (int i = 1; i < 8; i++) //Loops trough all the windDricetionVoltages
	{
		float currentDifference = fabs(resistance - windDirectionADCs[i]); //Calculates the difference between the current windDirectionADC and the actual ADC.
		if (currentDifference < closestDifference) //Checks if the current difference is smaller than the closest difference.
		{
			closestNumber = windDirectionADCs[i]; //Sets the current windDirectionADC as the closest one
			closestDifference = currentDifference; //Sets the current difference as the closest.
		}
	}

	const int tolerance = 1;  // Adjust this tolerance as needed

		//compares the closest voltage to all the voltages to get the wind direction.
	    if (fabs(closestNumber - WMK_ADC_ANGLE_N) < tolerance)
	    {
	        return 360.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_NE) < tolerance)
	    {
	        return 45.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_E) < tolerance)
	    {
	        return 90.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_SE) < tolerance)
	    {
	        return 135.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_S) < tolerance)
	    {
	        return 180.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_SW) < tolerance)
	    {
	        return 225.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_W) < tolerance)
	    {
	        return 270.0;
	    }
	    else if (fabs(closestNumber - WMK_ADC_ANGLE_NW) < tolerance)
	    {
	        return 315.0;
	    }
	    else
	    {
	        return 404.0;
	    }
}

void CalculateAmountOfRain(void)
{
	int currentDate;
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	currentDate = sDate.Date;
	if (lastCheckedDate != currentDate)
	{
		rainCounter = 0;
		lastCheckedDate = currentDate;
	}
	mmRain = 0.2794 * rainCounter;
}

void CalculateWindSpeed(void)
{
	int currentSecond;
	int secondDifference;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	currentSecond = sTime.Seconds;
	if (lastCheckedSecond != currentSecond)
	{
		if (currentSecond == 0)
		{
			secondDifference = 60 - lastCheckedSecond;
			windSpeedKmh = (windCounter * 2.4) / secondDifference;
			windCounter = 0;
		}
		else
		{
			secondDifference = currentSecond - lastCheckedSecond;
			windSpeedKmh = (windCounter * 2.4) / secondDifference;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == RainSensor_Pin)
	{
		rainCounter++;
	}
	if (GPIO_Pin == WindSpeed_Pin)
	{
		windCounter++;
	}
}

uint8_t iic_init(void)
{
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		return 1;  // Return error status
	}
	else
	{
		return 0; // Return success status
	}
}

uint8_t iic_deinit(void)
{
    // Implementation for iic_deinit
	 HAL_StatusTypeDef status;

	    // Deinitialize the I2C peripheral
	    // Replace 'hi2c' with your I2C handle if it has a different name
	    status = HAL_I2C_DeInit(&hi2c1);

	    if (status == HAL_OK)
	    {
	        return 0;  // Return success status
	    }
	    else
	    {
	        return 1;  // Return error status
	    }
}

uint8_t iic_write_cmd(uint8_t addr, uint8_t *buf, uint16_t len)
{
    // Implementation for iic_write_cmd
	HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Write(&hi2c1, addr, 0, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY);

    return (status == HAL_OK) ? 0 : 1;
}

uint8_t iic_read_cmd(uint8_t addr, uint8_t *buf, uint16_t len) {
    // Implementation for iic_read_cmd
	HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(&hi2c1, addr, 0, 1, buf, len, HAL_MAX_DELAY);

    return (status == HAL_OK) ? 0 : 1;
}

void sgp30_test(void)
{
	HAL_I2C_Mem_Read(&hi2c1, 0x58, 0x2050, I2C_MEMADD_SIZE_8BIT, buf, 6, 100);

	memset(buf, 0, sizeof(buf)); // Clears the readbuffer
}

void delay_ms(uint32_t ms) {
    // Implementation for delay_ms
    HAL_Delay(ms);
}

void debug_print(const char *const fmt, ...) {
    // Implementation for debug_print
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
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
