/**
  ******************************************************************************
  * @file    stm322xg_eval_camera.c
  * @author  MCD Application Team
  * @brief   This file includes the driver for Camera module mounted on
  *          STM322xG-EVAL evaluation board(MB786).
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */ 

/* File Info: ------------------------------------------------------------------
                                   User NOTES
1. How to use this driver:
--------------------------
   - This driver is used to drive the Camera.
   - The OV2640 component driver MUST be included with this driver.          

2. Driver description:
---------------------
  + Initialization steps:
     o Initialize the Camera using the BSP_CAMERA_Init() function.
     o Start the Camera capture or snapshot using CAMERA_Start() function.
     o Suspend, resume or stop the Camera capture using the following functions:
      - BSP_CAMERA_Suspend()
      - BSP_CAMERA_Resume()
      - BSP_CAMERA_Stop()

  + Options
     o Increase or decrease on the fly the brightness and/or contrast
       using the following function:
       - BSP_CAMERA_ContrastBrightnessConfig
     o Add a special effect on the fly using the following functions:
       - BSP_CAMERA_BlackWhiteConfig()
       - BSP_CAMERA_ColorEffectConfig()  
      
------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "stm322xg_eval_camera.h"   
    
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM322xG_EVAL
  * @{
  */
    
/** @defgroup STM322xG_EVAL_CAMERA STM322xG EVAL CAMERA
  * @{
  */ 

/** @defgroup STM322xG_EVAL_CAMERA_Private_Variables STM322xG EVAL CAMERA Private Variables
  * @{
  */ 
DCMI_HandleTypeDef hdcmi_eval;
CAMERA_DrvTypeDef  *camera_drv;
uint32_t current_resolution;
/**
  * @}
  */ 
  
/** @defgroup STM322xG_EVAL_CAMERA_Private_FunctionPrototypes STM322xG EVAL CAMERA Private FunctionPrototypes
  * @{
  */
static uint32_t GetSize(uint32_t resolution);
/**
  * @}
  */ 

/** @defgroup STM322xG_EVAL_CAMERA_Private_Functions STM322xG EVAL CAMERA Private Functions
  * @{
  */ 

/**
  * @brief  Initializes the Camera.
  * @param  Resolution: Camera resolution
  * @retval Camera status
  */
uint8_t BSP_CAMERA_Init(uint32_t Resolution)
{ 
  DCMI_HandleTypeDef *phdcmi;
  uint8_t ret = CAMERA_ERROR;
  
  /* Get the DCMI handle structure */
  phdcmi = &hdcmi_eval;
  
  /*** Configures the DCMI to interface with the Camera module ***/
  /* DCMI configuration */
  phdcmi->Init.CaptureRate      = DCMI_CR_ALL_FRAME;  
  phdcmi->Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
  phdcmi->Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
  phdcmi->Init.VSPolarity       = DCMI_VSPOLARITY_LOW;
  phdcmi->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  phdcmi->Init.PCKPolarity      = DCMI_PCKPOLARITY_RISING;
  phdcmi->Instance              = DCMI;
  
  /* DCMI Initialization */
  BSP_CAMERA_MspInit();
  HAL_DCMI_Init(phdcmi);    
  
  if(ov2640_drv.ReadID(CAMERA_I2C_ADDRESS) == OV2640_ID)
  {
    /* Initialize the Camera driver structure */
    camera_drv = &ov2640_drv;     
    
    /* Camera Init */   
    camera_drv->Init(CAMERA_I2C_ADDRESS, Resolution);
    
    /* Return CAMERA_OK status */
    ret = CAMERA_OK;
  }
  
  current_resolution = Resolution;
  
  return ret;
}

/**
  * @brief  Starts the Camera capture in continuous mode.
  * @param  buff: pointer to the Camera output buffer
  */
void BSP_CAMERA_ContinuousStart(uint8_t *buff)
{   
  /* Start the Camera capture */
  HAL_DCMI_Start_DMA(&hdcmi_eval, DCMI_MODE_CONTINUOUS, (uint32_t)buff, GetSize(current_resolution));  
}

/**
  * @brief  Starts the Camera capture in snapshot mode.
  * @param  buff: pointer to the Camera output buffer
  */
void BSP_CAMERA_SnapshotStart(uint8_t *buff)
{   
  /* Start the Camera capture */
  HAL_DCMI_Start_DMA(&hdcmi_eval, DCMI_MODE_SNAPSHOT, (uint32_t)buff, GetSize(current_resolution));  
}

/**
  * @brief  Suspends the Camera capture. 
  */
void BSP_CAMERA_Suspend(void) 
{
  /* Suspend the Camera Capture */
  HAL_DCMI_Suspend(&hdcmi_eval);
}

/**
  * @brief  Resumes the Camera capture. 
  */
void BSP_CAMERA_Resume(void) 
{
  /* Start the Camera Capture */
  HAL_DCMI_Resume(&hdcmi_eval);
}

/**
  * @brief  Stops the Camera capture. 
  * @retval Camera status
  */
uint8_t BSP_CAMERA_Stop(void) 
{
  DCMI_HandleTypeDef *phdcmi;
  
  uint8_t ret = CAMERA_ERROR;
  
  /* Get the DCMI handle structure */
  phdcmi = &hdcmi_eval;  
  
  if(HAL_DCMI_Stop(phdcmi) == HAL_OK)
  {
     ret = CAMERA_OK;
  }
  
  return ret;
}

/**
  * @brief  Configures the Camera contrast and brightness.
  * @param  contrast_level: Contrast level
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_CONTRAST_LEVEL4: for contrast +2
  *            @arg  CAMERA_CONTRAST_LEVEL3: for contrast +1
  *            @arg  CAMERA_CONTRAST_LEVEL2: for contrast  0
  *            @arg  CAMERA_CONTRAST_LEVEL1: for contrast -1
  *            @arg  CAMERA_CONTRAST_LEVEL0: for contrast -2
  * @param  brightness_level: Brightness level
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_BRIGHTNESS_LEVEL4: for brightness +2
  *            @arg  CAMERA_BRIGHTNESS_LEVEL3: for brightness +1
  *            @arg  CAMERA_BRIGHTNESS_LEVEL2: for brightness  0
  *            @arg  CAMERA_BRIGHTNESS_LEVEL1: for brightness -1
  *            @arg  CAMERA_BRIGHTNESS_LEVEL0: for brightness -2
  */
void BSP_CAMERA_ContrastBrightnessConfig(uint32_t contrast_level, uint32_t brightness_level)
{
  if(camera_drv->Config != NULL)
  {
    camera_drv->Config(CAMERA_I2C_ADDRESS, CAMERA_CONTRAST_BRIGHTNESS, contrast_level, brightness_level);
  }  
}

/**
  * @brief  Configures the Camera white balance.
  * @param  Mode: black_white mode
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_BLACK_WHITE_BW
  *            @arg  CAMERA_BLACK_WHITE_NEGATIVE
  *            @arg  CAMERA_BLACK_WHITE_BW_NEGATIVE
  *            @arg  CAMERA_BLACK_WHITE_NORMAL       
  */
void BSP_CAMERA_BlackWhiteConfig(uint32_t Mode)
{
  if(camera_drv->Config != NULL)
  {
    camera_drv->Config(CAMERA_I2C_ADDRESS, CAMERA_BLACK_WHITE, Mode, 0);
  }  
}

/**
  * @brief  Configures the Camera color effect.
  * @param  Effect: Color effect
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_COLOR_EFFECT_ANTIQUE               
  *            @arg  CAMERA_COLOR_EFFECT_BLUE        
  *            @arg  CAMERA_COLOR_EFFECT_GREEN    
  *            @arg  CAMERA_COLOR_EFFECT_RED        
  */
void BSP_CAMERA_ColorEffectConfig(uint32_t Effect)
{
  if(camera_drv->Config != NULL)
  {
    camera_drv->Config(CAMERA_I2C_ADDRESS, CAMERA_COLOR_EFFECT, Effect, 0);
  }  
}

/**
  * @brief  Handles DCMI interrupt request.
  */
void BSP_CAMERA_IRQHandler(void) 
{
  HAL_DCMI_IRQHandler(&hdcmi_eval);
}

/**
  * @brief  Handles DMA interrupt request.
  */
void BSP_CAMERA_DMA_IRQHandler(void) 
{
  HAL_DMA_IRQHandler(hdcmi_eval.DMA_Handle);
}

/**
  * @brief  Get the capture size.
  * @param  resolution: the current resolution.
  * @retval cpature size
  */
static uint32_t GetSize(uint32_t resolution)
{ 
  uint32_t size = 0;
  
  /* Get capture size */
  switch (resolution)
  {
  case CAMERA_R160x120:
    {
      size =  0x2580;
    }
    break;    
  case CAMERA_R320x240:
    {
      size =  0x9600;
    }
    break;
  default:
    {
      break;
    }
  }
  
  return size;
}

/**
  * @brief  Initializes the DCMI MSP.
  */
__weak void BSP_CAMERA_MspInit(void)
{  
  static DMA_HandleTypeDef hdma;
  GPIO_InitTypeDef GPIO_Init_Structure;  
  DCMI_HandleTypeDef *hdcmi = &hdcmi_eval;
  
  /*** Enable peripherals and GPIO clocks ***/
  /* Enable DCMI clock */
  __HAL_RCC_DCMI_CLK_ENABLE();

  /* Enable DMA2 clock */
  __HAL_RCC_DMA2_CLK_ENABLE(); 
  
  /* Enable GPIO clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  
  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  GPIO_Init_Structure.Pin       = GPIO_PIN_6; 
  GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;  
  HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);

  GPIO_Init_Structure.Pin       = GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |\
                                  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14; 
  GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;   
  HAL_GPIO_Init(GPIOH, &GPIO_Init_Structure);

  GPIO_Init_Structure.Pin       = GPIO_PIN_4 | GPIO_PIN_5  | GPIO_PIN_6  |\
                                  GPIO_PIN_7; 
  GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;   
  HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);  
  
  /*** Configure the DMA streams ***/
  /* Configure the DMA handler for Transmission process */
  hdma.Init.Channel             = DMA_CHANNEL_1;
  hdma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma.Init.MemInc              = DMA_MINC_ENABLE;
  hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  hdma.Init.Mode                = DMA_CIRCULAR;
  hdma.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma.Init.MemBurst            = DMA_MBURST_SINGLE;
  hdma.Init.PeriphBurst         = DMA_PBURST_SINGLE; 

  hdma.Instance = DMA2_Stream1;
  
  /* Associate the initialized DMA handle to the DCMI handle */
  __HAL_LINKDMA(hdcmi, DMA_Handle, hdma);
  
  /*** Configure the NVIC for DCMI and DMA ***/
  /* NVIC configuration for DCMI transfer complete interrupt */
  HAL_NVIC_SetPriority(DCMI_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DCMI_IRQn);  
  
  /* NVIC configuration for DMA2 transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn); 
  
  /* Configure the DMA stream */
  HAL_DMA_Init(hdcmi->DMA_Handle);   
}

/**
  * @brief  Line event callback
  * @param  hdcmi: pointer to the DCMI handle  
  */
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_LineEventCallback();
}

/**
  * @brief  Line Event callback.
  */
__weak void BSP_CAMERA_LineEventCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_LineEventCallback could be implemented in the user file
   */
}

/**
  * @brief  VSYNC event callback
  * @param  hdcmi: pointer to the DCMI handle  
  */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_VsyncEventCallback();
}

/**
  * @brief  VSYNC Event callback.
  */
__weak void BSP_CAMERA_VsyncEventCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_VsyncEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Frame event callback
  * @param  hdcmi: pointer to the DCMI handle  
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_FrameEventCallback();
}

/**
  * @brief  Frame Event callback.
  */
__weak void BSP_CAMERA_FrameEventCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_FrameEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Error callback
  * @param  hdcmi: pointer to the DCMI handle  
  */
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_ErrorCallback();
}

/**
  * @brief  Error callback.
  */
__weak void BSP_CAMERA_ErrorCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_ErrorCallback could be implemented in the user file
   */
}

/**
  * @}
  */
    
/**
  * @}
  */

/**
  * @}
  */
  
/**
  * @}
  */      

