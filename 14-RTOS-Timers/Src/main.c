//main.c

/*
 * twoTimers timer 1 is autoreload which toggles the led
 * timer2 is oneshot tha transmits to uart can can be started again
*/



//----------------------------------------------------------------------------------
//includes
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include "led.h"

//Global Vars
//----------------------------------------------------------------------------------
UART_HandleTypeDef huart2;
QueueHandle_t QueueHandler;

TimerHandle_t TautoReload ;
TimerHandle_t ToneShot;

#define SIMPLE_TASK_STACK_SIZE 1024

uint8_t tmpstr = 0;
//--------------------------------------------------------------------------------

//prototypes
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

void AutoReloadCallBack(TimerHandle_t xTimer);
void OneShotCallBack(TimerHandle_t xTimer);

void Simple_Transmitter(void* params);




//----------------------------------------------------------------------------------
int main(void)
{

  //inits
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  MX_USART2_UART_Init();
  //user inits
  led_init();
  led_state('L');


  //queue

  QueueHandler = xQueueCreate(100 , sizeof(uint8_t) );

  //Timers

  TautoReload = xTimerCreate("Timer 1 A" , 1000 , pdTRUE , NULL , AutoReloadCallBack);

  ToneShot = xTimerCreate("Timer 2 O" , 5000 , pdFALSE , NULL , OneShotCallBack);

  xTimerStart(TautoReload , 0);
  xTimerStart(ToneShot , 0);

  //Tasks

  xTaskCreate(Simple_Transmitter , "Simple task to process and trasmit " , SIMPLE_TASK_STACK_SIZE ,NULL , 1 , NULL);

  //RTOS start
  vTaskStartScheduler();

  while (1)
  {




  }//end while(1)

}//end main

//----------------------------------------------------------------------------------


void Simple_Transmitter(void* params){
	static uint8_t receiveCounter = 0;

	uint8_t receivedByte;

	BaseType_t qStatus;
	while(1){
		qStatus = xQueueReceive(QueueHandler , &receivedByte , portMAX_DELAY);
		if(qStatus != pdPASS){
			printf("Failed to receive\r\n");
		}else{
			// if the received bytes have 'true' in them it
			if(receiveCounter == 0){
			    if(receivedByte == 'g'){
			        receiveCounter = 1;
			    }else{
			        receiveCounter = 0;
			    }
			}
			else if(receiveCounter == 1){
			    if(receivedByte == 'o'){
			        receiveCounter = 0;
			        xTimerStart(ToneShot , 0);
			    }else{
			        receiveCounter = 0;
			    }
			}

		}

	}//end while

}//end task



void AutoReloadCallBack(TimerHandle_t xTimer){

	led_toggle();
}

void OneShotCallBack(TimerHandle_t xTimer){

	HAL_UART_Transmit(&huart2, (uint8_t *)"5000 Ticks Passed", 17, HAL_MAX_DELAY);
}





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	BaseType_t HP_Task_Woken = pdFALSE;

	uint8_t receivedByte = tmpstr;


	xQueueSendFromISR(QueueHandler , &receivedByte , &HP_Task_Woken );
	HAL_UART_Receive_IT(&huart2, &tmpstr , 1);


	portYIELD_FROM_ISR(HP_Task_Woken);
}


//----------------------------------------------------------------------------------
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{


  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
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

  HAL_NVIC_SetPriority(USART2_IRQn , 5 , 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_UART_Receive_IT(&huart2, &tmpstr , 1);


}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();


}


/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }

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
