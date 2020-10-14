/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch1;

/* USER CODE BEGIN PV */
#define LED_CFG_USE_RGBW                0       /*!< Set to 1 to use RGBW leds Set to 0 to use WS2812B leds */

#define LED_CFG_LEDS_CNT                16      /*!< Number of leds in a strip row */

#if LED_CFG_USE_RGBW
#define LED_CFG_BYTES_PER_LED           4
#else /* LED_CFG_USE_RGBW */
#define LED_CFG_BYTES_PER_LED           3
#endif /* !LED_CFG_USE_RGBW */

#define LED_CFG_RAW_BYTES_PER_LED       (LED_CFG_BYTES_PER_LED * 8)

static uint8_t leds_colors[LED_CFG_BYTES_PER_LED * LED_CFG_LEDS_CNT];		//brief           Array of 4x (or 3x) number of leds (R, G, B[, W] colors)

/**
 * \brief           Temporary array for dual LED with extracted PWM duty cycles
 *
 * We need LED_CFG_RAW_BYTES_PER_LED bytes for PWM setup to send all bits.
 * Before we can send data for first led, we have to send reset pulse, which must be 50us long.
 * PWM frequency is 800kHz, to achieve 50us, we need to send 40 pulses with 0 duty cycle = make array size MAX(2 * LED_CFG_RAW_BYTES_PER_LED, 40)
 */
static uint8_t 				tmp_led_data[2 * LED_CFG_RAW_BYTES_PER_LED];
static uint8_t          	is_reset_pulse;     /*!< Status if we are sending reset pulse or led data */
static volatile uint8_t 	is_updating;        /*!< Is updating in progress? */
static volatile uint32_t    current_led;       	/*!< Current LED number we are sending */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN PFP */
void 		TransfertCplt(DMA_HandleTypeDef *hdm);
void 		TransfertHalfCplt(DMA_HandleTypeDef *hdma);
void        led_init(void);
uint8_t     led_update(uint8_t block);
void 		led_update_sequence(uint8_t tc);

#if LED_CFG_USE_RGBW
uint8_t     led_set_color(size_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
uint8_t     led_set_color_all(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
uint8_t     led_set_color_rgbw(size_t index, uint32_t rgbw);
uint8_t     led_set_color_all_rgbw(uint32_t rgbw);
#else /* LED_CFG_USE_RGBW */
uint8_t     led_set_color(size_t index, uint8_t r, uint8_t g, uint8_t b);
uint8_t     led_set_color_all(uint8_t r, uint8_t g, uint8_t b);
uint8_t     led_set_color_rgb(size_t index, uint32_t rgb);
uint8_t     led_set_color_all_rgb(uint32_t rgb);
#endif /* !LED_CFG_USE_RGBW */

uint8_t     led_is_update_finished(void);
uint8_t     led_start_reset_pulse(uint8_t num);

/* Anim */
void animRing(uint8_t repeat);
void colorWipe(uint8_t r, uint8_t g, uint8_t b, uint8_t mode);
void rainbow(void);
void rainbowCycle(uint8_t repeat);
void theaterChase(uint8_t r, uint8_t g, uint8_t b, uint8_t repeat);
void animStop(uint8_t repeat);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM2_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  // INFINITE LOOP         																	             //
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* Infinite loop */
  while (1)
  {

	  animRing(3);
	  colorWipe(0x1F, 0x00, 0x00,0);
	  colorWipe(0x00, 0x1F, 0x00,1);
	  colorWipe(0x00, 0x00, 0x1F,1);
	  colorWipe(0x1F, 0x1F, 0x1F,1);
	  colorWipe(0x00, 0x00, 0x00,1);
//	  rainbow();
	  rainbowCycle(2);
	  theaterChase(0x1F, 0x00, 0x0b,3);
	  animStop(20);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}




/*#################################################################################################*/
/*#################################################################################################*/
/*## Functions  SYS                                                                              ##*/
/*#################################################################################################*/
/*#################################################################################################*/



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
  {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 89;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//::::: ANIMATION :::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
struct pColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
void Wheel(struct pColor *color, uint8_t WheelPos) {
	if(WheelPos < 85) {
		color->r= WheelPos * 3;
		color->g= 255 - WheelPos * 3;
		color->b=0;
		return;
	} else if(WheelPos < 170) {
		WheelPos -= 85;
		color->r= 255 - WheelPos;
		color->g= 0;
		color->b= WheelPos * 3;
		return;
	} else {
		WheelPos -= 170;
		color->r= 0;
		color->g= WheelPos * 3;
		color->b= 255 - WheelPos * 3;
		return;
	}
}

//Theatre-style crawling lights.
void theaterChase(uint8_t r, uint8_t g, uint8_t b, uint8_t repeat) {
	led_set_color_all(0, 0, 0);
	while (repeat-- > 0){
		for (int j=0; j<10; j++) {  //do 10 cycles of chasing
			for (int q=0; q < 2; q++) {
				for (int i=0; i < LED_CFG_LEDS_CNT; i=i+2) {
					led_set_color(i+q, r, g, b); //turn every third pixel on
				}
				led_update(1);
				HAL_Delay(150);

				for (int i=0; i < LED_CFG_LEDS_CNT; i=i+2) {
					led_set_color(i+q, 0, 0, 0); //turn every third pixel off
				}
			}
		}
	}
}


// Rainbow cycle
void rainbow(void) {
  uint16_t i, j;
  struct pColor ledColor;

  for(j=0; j<256; j++) {
    for(i=0; i<LED_CFG_LEDS_CNT; i++) {
      Wheel(&ledColor, (i+j) & 255);
      led_set_color(i, ledColor.r, ledColor.g, ledColor.b);
    }
    led_update(1);
	HAL_Delay(8);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t repeat) {
	uint16_t i, j;
	struct pColor ledColor;

	while (repeat-- > 0){
		for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
			for(i=0; i<LED_CFG_LEDS_CNT; i++) {
				Wheel(&ledColor, ((i * 256 / LED_CFG_LEDS_CNT) + j) & 255);
				led_set_color(i, ledColor.r, ledColor.g, ledColor.b);
			}
			led_update(1);
			HAL_Delay(2);
		}
	}
}

// Wipe cycle
void colorWipe(uint8_t r, uint8_t g, uint8_t b, uint8_t mode){
	if (mode==0) {
		led_set_color_all(0, 0, 0);
	}
	for(uint16_t i=0; i<LED_CFG_LEDS_CNT;i++){
		led_set_color(i, r, g, b);
		led_update(1);
		HAL_Delay(80);
	}
}


// STOP red / white color
void animStop(uint8_t repeat){
	while (repeat-- > 0){
		for (uint8_t i = 0; i < LED_CFG_LEDS_CNT; i+=2)	led_set_color(i,0xFF, 0x00, 0x00), led_set_color(i+1,0x00, 0x00, 0x00);
		led_update(1);
		HAL_Delay(180);
		for (uint8_t i = 0; i < LED_CFG_LEDS_CNT; i+=2)	led_set_color(i,0x00, 0x00, 0x00), led_set_color(i+1,0x80, 0x80, 0x80);
		led_update(1);
		HAL_Delay(180);
	}
}
// Ring color
void animRing(uint8_t repeat){
	while (repeat-- > 0){
		for (uint8_t i = 0; i < LED_CFG_LEDS_CNT; i++) {
			led_set_color((i + 0) % LED_CFG_LEDS_CNT, 0x1F, 0x00, 0x00);
			led_set_color((i + 1) % LED_CFG_LEDS_CNT, 0x1F, 0x00, 0x00);
			led_set_color((i + 2) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 3) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 4) % LED_CFG_LEDS_CNT, 0x00, 0x00, 0x1F);
			led_set_color((i + 5) % LED_CFG_LEDS_CNT, 0x00, 0x00, 0x1F);
			led_set_color((i + 6) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 7) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 8) % LED_CFG_LEDS_CNT, 0x1F, 0x00, 0x00);
			led_set_color((i + 9) % LED_CFG_LEDS_CNT, 0x1F, 0x00, 0x00);
			led_set_color((i + 10) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 11) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 12) % LED_CFG_LEDS_CNT, 0x00, 0x00, 0x1F);
			led_set_color((i + 13) % LED_CFG_LEDS_CNT, 0x00, 0x00, 0x1F);
			led_set_color((i + 14) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_set_color((i + 15) % LED_CFG_LEDS_CNT, 0x00, 0x1F, 0x00);
			led_update(1);
			led_set_color_all(0, 0, 0);
			HAL_Delay(80);
		}
	}
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


/**
  * @brief Transfert completed callback TIM_DMADelayPulseCplt TIM_DMADelayPulseHalfCplt
  * @retval None
  */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
//void TransfertCplt(DMA_HandleTypeDef *hdma){
	led_update_sequence(1);
}
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim){
//void TransfertHalfCplt(DMA_HandleTypeDef *hdma){
	led_update_sequence(0);
}

/**
 * \brief           Prepares data from memory for PWM output for timer
 * \note            Memory is in format R,G,B, while PWM must be configured in G,R,B[,W]
 * \param[in]       ledx: LED index to set the color
 * \param[out]      ptr: Output array with at least LED_CFG_RAW_BYTES_PER_LED-words of memory
 */
static uint8_t led_fill_led_pwm_data(size_t ledx, uint8_t* ptr) {
    size_t i;

    if (ledx < LED_CFG_LEDS_CNT) {
        for (i = 0; i < 8; i++) {
            ptr[i] =        (leds_colors[LED_CFG_BYTES_PER_LED * ledx + 1] & (1 << (7 - i))) ? (2 * TIM2->ARR / 3) : (TIM2->ARR / 3);
            ptr[8 + i] =    (leds_colors[LED_CFG_BYTES_PER_LED * ledx + 0] & (1 << (7 - i))) ? (2 * TIM2->ARR / 3) : (TIM2->ARR / 3);
            ptr[16 + i] =   (leds_colors[LED_CFG_BYTES_PER_LED * ledx + 2] & (1 << (7 - i))) ? (2 * TIM2->ARR / 3) : (TIM2->ARR / 3);
#if LED_CFG_USE_RGBW
            ptr[24 + i] =   (leds_colors[LED_CFG_BYTES_PER_LED * ledx + 3] & (1 << (7 - i))) ? (2 * TIM2->ARR / 3) : (TIM2->ARR / 3);
#endif /* LED_CFG_USE_RGBW */
        }
        return 1;
    }
    return 0;
}

/**
 * \brief           Update sequence function, called on each DMA transfer complete or half-transfer complete events
 * \param[in]       tc: Transfer complete flag. Set to `1` on TC event, or `0` on HT event
 *
 * \note            TC = Transfer-Complete event, called at the end of block
 * \note            HT = Half-Transfer-Complete event, called in the middle of elements transfered by DMA
 *                  If block is 48 elements (our case),
 *                      HT is called when first LED_CFG_RAW_BYTES_PER_LED elements are transfered,
 *                      TC is called when second LED_CFG_RAW_BYTES_PER_LED elements are transfered.
 *
 * \note            This function must be called from DMA interrupt
 */
void led_update_sequence(uint8_t tc) {
    tc = !!tc;                                  /* Convert to 1 or 0 value only */

    /* Check for reset pulse at the end of PWM stream */
    if (is_reset_pulse == 2) {                  /* Check for reset pulse at the end */
    	HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);

        is_updating = 0;                        /* We are not updating anymore */
        return;
    }

    /* Check for reset pulse on beginning of PWM stream */
    if (is_reset_pulse == 1) {                  /* Check if we finished with reset pulse */
        /*
         * When reset pulse is active, we have to wait full DMA response,
         * before we can start modifying array which is shared with DMA and PWM
         */
        if (!tc) {                              /* We must wait for transfer complete */
            return;                             /* Return and wait to finish */
        }

        /* Disable timer output and disable DMA stream */
        HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);

        is_reset_pulse = 0;                     /* Not in reset pulse anymore */
        current_led = 0;                        /* Reset current led */
    } else {
        /*
         * When we are not in reset mode,
         * go to next led and process data for it
         */
        current_led++;                          /* Go to next LED */
    }

    /*
     * This part is used to prepare data for "next" led,
     * for which update will start once current transfer stops in circular mode
     */
    if (current_led < LED_CFG_LEDS_CNT) {
        /*
         * If we are preparing data for first time (current_led == 0)
         * or if there was no TC event (it was HT):
         *
         *  - Prepare first part of array, because either there is no transfer
         *      or second part (from HT to TC) is now in process for PWM transfer
         *
         * In other case (TC = 1)
         */
        if (current_led == 0 || !tc) {
            led_fill_led_pwm_data(current_led, &tmp_led_data[0]);
        } else {
            led_fill_led_pwm_data(current_led, &tmp_led_data[LED_CFG_RAW_BYTES_PER_LED]);
        }

        /*
         * If we are preparing first led (current_led = 0), then:
         *
         *  - We setup first part of array for first led,
         *  - We have to prepare second part for second led to have one led prepared in advance
         *  - Set DMA to circular mode and start the transfer + PWM output
         */
        if (current_led == 0) {
            current_led++;                      /* Go to next LED */
            led_fill_led_pwm_data(current_led, &tmp_led_data[LED_CFG_RAW_BYTES_PER_LED]);   /* Prepare second LED too */

            /* Set DMA to circular mode and set length to 48 elements for 2 leds */
            hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
            if (HAL_DMA_Init(&hdma_tim2_ch1) != HAL_OK)
            {
            	Error_Handler();
            }
            HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*) tmp_led_data,2 * LED_CFG_RAW_BYTES_PER_LED);
        }

    /*
     * When we reached all leds, we have to wait to transmit data for all leds before we can disable DMA and PWM:
     *
     *  - If TC event is enabled and we have EVEN number of LEDS (2, 4, 6, ...)
     *  - If HT event is enabled and we have ODD number of LEDS (1, 3, 5, ...)
     */
    } else if ((!tc && (LED_CFG_LEDS_CNT & 0x01)) || (tc && !(LED_CFG_LEDS_CNT & 0x01))) {
    	HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);

        /* It is time to send final reset pulse, 50us at least */
        led_start_reset_pulse(2);                /* Start reset pulse at the end */
    }
}


/**
 * \brief           Set R,G,B color for specific LED
 * \param[in]       index: LED index in array, starting from `0`
 * \param[in]       r,g,b: Red, Green, Blue values
 * \return          `1` on success, `0` otherwise
 */
uint8_t
led_set_color(size_t index, uint8_t r, uint8_t g, uint8_t b
#if LED_CFG_USE_RGBW
, uint8_t w
#endif /* LED_CFG_USE_RGBW */
) {
    if (index < LED_CFG_LEDS_CNT) {
        leds_colors[index * LED_CFG_BYTES_PER_LED + 0] = r;
        leds_colors[index * LED_CFG_BYTES_PER_LED + 1] = g;
        leds_colors[index * LED_CFG_BYTES_PER_LED + 2] = b;
#if LED_CFG_USE_RGBW
        leds_colors[index * LED_CFG_BYTES_PER_LED + 3] = w;
#endif /* LED_CFG_USE_RGBW */
        return 1;
    }
    return 0;
}




/**
 * \brief           Set R,G,B color for specific LED
 * \param[in]       index: LED index in array, starting from `0`
 * \param[in]       r,g,b: Red, Green, Blue values
 * \return          `1` on success, `0` otherwise
 */
uint8_t led_set_color_all(uint8_t r, uint8_t g, uint8_t b
#if LED_CFG_USE_RGBW
, uint8_t w
#endif /* LED_CFG_USE_RGBW */
) {
    size_t index;
    for (index = 0; index < LED_CFG_LEDS_CNT; index++) {
        leds_colors[index * LED_CFG_BYTES_PER_LED + 0] = r;
        leds_colors[index * LED_CFG_BYTES_PER_LED + 1] = g;
        leds_colors[index * LED_CFG_BYTES_PER_LED + 2] = b;
#if LED_CFG_USE_RGBW
        leds_colors[index * LED_CFG_BYTES_PER_LED + 3] = w;
#endif /* LED_CFG_USE_RGBW */
    }
    return 1;
}


/**
 * \brief           Start reset pulse sequence
 * \param[in]       num: Number indicating pulse is for beginning (1) or end (2) of PWM data stream
 */
uint8_t led_start_reset_pulse(uint8_t num) {
    is_reset_pulse = num;                       /* Set reset pulse flag */

    memset(tmp_led_data, 0, sizeof(tmp_led_data));   /* Set all bytes to 0 to achieve 50us pulse */
    if (num == 1) {
        tmp_led_data[0] = TIM2->ARR / 2;
    }

    /* Set DMA to normal mode, set memory to beginning of data and length to 40 elements */
    /* 800kHz PWM x 40 samples = ~50us pulse low */
    hdma_tim2_ch1.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&hdma_tim2_ch1) != HAL_OK)
    {
    	Error_Handler();
    }
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*) tmp_led_data,40);


    return 1;
}

/**
 * \brief           Check if update procedure is finished
 * \return          `1` if not updating, `0` if updating process is in progress
 */
uint8_t
led_is_update_finished(void) {
    return !is_updating;                        /* Return updating flag status */
}

/**
 * \brief           Start LEDs update procedure
 * \param[in]       block: Set to `1` to block for update process until finished
 * \return          `1` if update started, `0` otherwise
 */
uint8_t led_update(uint8_t block) {
    if (is_updating) {                          /* Check if update in progress already */
        return 0;
    }
    is_updating = 1;                            /* We are now updating */

    led_start_reset_pulse(1);                   /* Start reset pulse */
    if (block) {
        while (!led_is_update_finished());      /* Wait to finish */
    }
    return 1;
}
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%% NEOPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%





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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
//
