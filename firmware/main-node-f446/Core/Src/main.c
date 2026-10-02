/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "log/log.h"
#include "drivers/mpu6050/mpu6050.h"
#include "drivers/w25q64/w25q64.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* ~11 bytes/ms at 115200 baud -> 100 ms covers ~1 KB per printf call */
#define UART_TX_TIMEOUT_MS    100U
#define BOARD_NAME            "NUCLEO-F446RE"
#define FIRMWARE_VERSION      "0.1.0"
#define W25Q64_TEST_LEN   256U
#define HEARTBEAT_PERIOD_MS 1000U
#define ACQ_PERIOD_MS 100U
#define ACQ_LOG_EVERY 10U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* Definitions for heartbeat */
osThreadId_t heartbeatHandle;
const osThreadAttr_t heartbeat_attributes = {
  .name = "heartbeat",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for acquisition */
osThreadId_t acquisitionHandle;
const osThreadAttr_t acquisition_attributes = {
  .name = "acquisition",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
void heartbeat_task(void *argument);
void acquisition_task(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    (void) file;
    HAL_StatusTypeDef status;
    status = HAL_UART_Transmit(&huart2, (const uint8_t *) ptr, len, UART_TX_TIMEOUT_MS);
    if(status != HAL_OK){
        return -1;
    }
    return len;
}

static int flash_pattern_test(const w25q64_t *dev, uint32_t addr,
                              const uint8_t *pattern, const char *name)
{
    uint8_t chk_buf[W25Q64_TEST_LEN] = {0};
    w25q64_status_t st;
    st = w25q64_sector_erase(dev, addr);
    if(st != W25Q64_OK){
        char msg[64];
        snprintf(msg, sizeof(msg), "W25Q64 pattern %s erase failed: %d", name, (int)st);
        log_write(LOG_LEVEL_ERROR, msg);
        return 0;
    }
    st = w25q64_page_program(dev, addr, pattern, W25Q64_TEST_LEN);
    if(st != W25Q64_OK){
        char msg[64];
        snprintf(msg, sizeof(msg), "W25Q64 pattern %s program failed: %d", name, (int)st);
        log_write(LOG_LEVEL_ERROR, msg);
        return 0;
    }
    st = w25q64_read_data(dev, addr, chk_buf, W25Q64_TEST_LEN);
    if(st != W25Q64_OK){
        char msg[64];
        snprintf(msg, sizeof(msg), "W25Q64 pattern %s read data failed: %d", name, (int)st);
        log_write(LOG_LEVEL_ERROR, msg);
        return 0;
    }
    if(memcmp(pattern, chk_buf, W25Q64_TEST_LEN) != 0){
        char msg[64];
        snprintf(msg, sizeof(msg), "W25Q64 pattern %s: FAIL", name);
        log_write(LOG_LEVEL_ERROR, msg);
        return 0;
    }
    char msg[64];
    snprintf(msg, sizeof(msg), "W25Q64 pattern %s: PASS", name);
    log_write(LOG_LEVEL_INFO, msg);
    return 1;
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  printf("=================================\r\n");
  printf("Embedded Monitoring System\r\n");
  printf("Board    : %s\r\n", BOARD_NAME);
  printf("Firmware : %s\r\n", FIRMWARE_VERSION);
  printf("=================================\r\n");
  log_write(LOG_LEVEL_INFO, "Boot OK");

  mpu6050_status_t mpu_st = mpu6050_init(&hi2c1);
  if(mpu_st != MPU6050_OK){
      char msg[32];
      snprintf(msg, sizeof(msg), "MPU6050 init failed: %d", (int)mpu_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      log_write(LOG_LEVEL_INFO, "MPU6050 init OK");
  }

  const w25q64_t flash = {
      .hspi    = &hspi1,
      .cs_port = FLASH_CS_GPIO_Port,
      .cs_pin  = FLASH_CS_Pin
  };
  uint8_t flash_id[3];
  w25q64_status_t flash_st = w25q64_read_jedec_id(&flash, flash_id);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 JEDEC read failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 JEDEC ID: %02X %02X %02X", flash_id[0], flash_id[1], flash_id[2]);
      log_write(LOG_LEVEL_INFO, msg);
  }
  uint8_t sr1;
  flash_st = w25q64_read_status(&flash, &sr1);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 SR1 read failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 SR1 (boot): %02X", sr1);
      log_write(LOG_LEVEL_INFO, msg);
  }
  flash_st = w25q64_wait_busy(&flash, 10);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 wait busy failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      log_write(LOG_LEVEL_INFO, "W25Q64 ready (not busy)");
  }
  flash_st = w25q64_write_enable(&flash);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 WREN failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      log_write(LOG_LEVEL_INFO, "W25Q64 WREN sent");
  }
  flash_st = w25q64_read_status(&flash, &sr1);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 SR1 read failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 SR1 (after WREN): %02X", sr1);
      log_write(LOG_LEVEL_INFO, msg);
  }
  uint32_t start_tick = HAL_GetTick();
  flash_st = w25q64_sector_erase(&flash, 0x000000);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 sector erase failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      uint32_t erase_ms = HAL_GetTick() - start_tick;
      snprintf(msg, sizeof(msg), "W25Q64 erase OK: %lu ms", erase_ms);
      log_write(LOG_LEVEL_INFO, msg);
  }
  flash_st = w25q64_read_status(&flash, &sr1);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 SR1 read failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 SR1 (after erase): %02X", sr1);
      log_write(LOG_LEVEL_INFO, msg);
  }
  uint8_t wr_buf[4] = {0xDE, 0xAD, 0xBE, 0xEF};  // nội dung muốn GHI vào flash
  uint8_t rd_buf[4] = {0};                       // chỗ trống để NHẬN data đọc ra
  flash_st = w25q64_read_data(&flash, 0x000000, rd_buf, 4);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 read data failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 read (erased): %02X %02X %02X %02X",
               rd_buf[0], rd_buf[1], rd_buf[2], rd_buf[3]);
      log_write(LOG_LEVEL_INFO, msg);
  }
  flash_st = w25q64_page_program(&flash, 0x000000, wr_buf, 4);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 page program failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      log_write(LOG_LEVEL_INFO, "W25Q64 page program ok");
  }
  flash_st = w25q64_read_data(&flash, 0x000000, rd_buf, 4);
  if(flash_st != W25Q64_OK){
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 read data failed: %d", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 read back: %02X %02X %02X %02X",
               rd_buf[0], rd_buf[1], rd_buf[2], rd_buf[3]);
      log_write(LOG_LEVEL_INFO, msg);
  }
  if(memcmp(wr_buf, rd_buf, sizeof(wr_buf)) == 0){
      log_write(LOG_LEVEL_INFO, "W25Q64 verify: PASS");
  }
  else{
      log_write(LOG_LEVEL_ERROR, "W25Q64 verify: FAIL");
  }
  flash_st = w25q64_page_program(&flash, 0x0000FE, wr_buf, 4);
  if(flash_st == W25Q64_ERR_PARAM){
      log_write(LOG_LEVEL_INFO, "W25Q64 cross-page reject: PASS");
  }
  else{
      char msg[50];
      snprintf(msg, sizeof(msg), "W25Q64 cross-page reject: FAIL (%d)", (int)flash_st);
      log_write(LOG_LEVEL_ERROR, msg);
  }
  uint8_t pat_buf[W25Q64_TEST_LEN];
  int pass_cnt = 0;
  memset(pat_buf, 0x00, sizeof(pat_buf));
  pass_cnt += flash_pattern_test(&flash, 0x000000, pat_buf, "0x00");
  memset(pat_buf, 0x55, sizeof(pat_buf));
  pass_cnt += flash_pattern_test(&flash, 0x000000, pat_buf, "0x55");
  memset(pat_buf, 0xAA, sizeof(pat_buf));
  pass_cnt += flash_pattern_test(&flash, 0x000000, pat_buf, "0xAA");
  for(uint16_t i = 0; i < sizeof(pat_buf); i++){
      pat_buf[i] = (uint8_t)i;
  }
  pass_cnt += flash_pattern_test(&flash, 0x000000, pat_buf, "incremental");
  char msg[64];
  snprintf(msg, sizeof(msg), "W25Q64 pattern summary: %d/4 PASS", pass_cnt);
  log_write(LOG_LEVEL_INFO, msg);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of heartbeat */
  heartbeatHandle = osThreadNew(heartbeat_task, NULL, &heartbeat_attributes);

  /* creation of acquisition */
  acquisitionHandle = osThreadNew(acquisition_task, NULL, &acquisition_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : FLASH_CS_Pin */
  GPIO_InitStruct.Pin = FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FLASH_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_heartbeat_task */
/**
  * @brief  Function implementing the heartbeat thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_heartbeat_task */
void heartbeat_task(void *argument)
{
  /* USER CODE BEGIN 5 */
  uint32_t beat = 0;
  uint32_t next_wake = osKernelGetTickCount();
  char msg[32];
  /* Infinite loop */
  for(;;)
  {
    snprintf(msg, sizeof(msg), "heartbeat %lu", beat);
    log_write(LOG_LEVEL_INFO, msg);
    next_wake += HEARTBEAT_PERIOD_MS;
    osDelayUntil(next_wake);
    beat++;
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_acquisition_task */
/**
* @brief Function implementing the acquisition thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_acquisition_task */
void acquisition_task(void *argument)
{
  /* USER CODE BEGIN acquisition_task */
  uint32_t sample_cnt = 0;
  uint32_t next_wake = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
      mpu6050_raw_t raw;
      mpu6050_status_t mpu_st = mpu6050_read_raw(&hi2c1, &raw);
      if(mpu_st != MPU6050_OK){
          char msg[32];
          snprintf(msg, sizeof(msg), "MPU6050 read failed %d", (int)mpu_st);
          log_write(LOG_LEVEL_ERROR, msg);
      }
      else{
          if(sample_cnt % ACQ_LOG_EVERY == 0){
              char msg[100]; /* 6 x ("AX=" + "-32768") + 5 spaces + '\0' = 60, 100 for headroom */
              snprintf(msg, sizeof(msg), "s=%lu AX=%d AY=%d AZ=%d GX=%d GY=%d GZ=%d",
                      sample_cnt,
                      raw.accel_x,
                      raw.accel_y,
                      raw.accel_z,
                      raw.gyro_x,
                      raw.gyro_y,
                      raw.gyro_z);
              log_write(LOG_LEVEL_INFO, msg);
          }
          sample_cnt++;
      }
      next_wake += ACQ_PERIOD_MS;
      osDelayUntil(next_wake);
  }
  /* USER CODE END acquisition_task */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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
