/**
  ******************************************************************************
  * @file    GPIO/IOToggle/air32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "air32f10x_it.h" 


 
void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 air32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_air32f10x_xx.s).                                            */
/******************************************************************************/

void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    /* Clear Interrupt pending bit */
    RTC_ClearITPendingBit(RTC_FLAG_SEC);
  }
}

uint16_t tmpCC4[2] = {0, 0};
extern uint32_t IncrementVar_OperationComplete(void);
extern uint32_t GetVar_OperationComplete(void);
extern void SetVar_PeriodValue(uint32_t Value);

void TIM5_IRQHandler(void)
{
	uint32_t tmp = 0; 

	if (TIM_GetITStatus(TIM5, TIM_IT_CC4) == SET)
	{
		tmpCC4[IncrementVar_OperationComplete()] = (uint16_t)(TIM5->CCR4);

		TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

	if (GetVar_OperationComplete() >= 2)
	{
		/* Compute the period length */
		tmp = (uint16_t)(tmpCC4[1] - tmpCC4[0] + 1);
		SetVar_PeriodValue(tmp);

		/* Disable the interrupt */
		TIM_ITConfig(TIM5, TIM_IT_CC4, DISABLE);
		TIM_Cmd(TIM5, DISABLE);
	}
	}
}
