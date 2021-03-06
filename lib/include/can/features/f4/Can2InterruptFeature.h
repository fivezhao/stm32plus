/*
 * This file is a part of the open source stm32plus library.
 * Copyright (c) 2011,2012,2013,2014 Andy Brown <www.andybrown.me.uk>
 * Please see website for licensing terms.
 */

#pragma once

// ensure the MCU series is correct
#if !defined(STM32PLUS_F4)
#error This class can only be used with the STM32F4 series
#endif


/*
 * Forward declare the IRQ handler names
 */

extern "C" void CAN2_TX_IRQHandler();  /*!< USB Device High Priority or CAN2 TX Interrupts       */
extern "C" void CAN2_RX0_IRQHandler(); /*!< USB Device Low Priority or CAN2 RX0 Interrupts       */
extern "C" void CAN2_RX1_IRQHandler();        /*!< CAN2 RX1 Interrupt                                   */
extern "C" void CAN2_SCE_IRQHandler();        /*!< CAN2 SCE Interrupt                                   */


namespace stm32plus {


  /**
   * CAN feature to handle interrupts. There's only one CAN on the F1 HD therefore
   * this is a concrete class
   */

  class Can2InterruptFeature : public CanEventSource,
                               public CanFeatureBase {
    private:
      typedef void (*FPTR)();         // this trick will force the linker to include the ISR
      static FPTR _forceLinkage;

    protected:
      uint16_t _interruptMask;
      uint8_t _nvicPriority;
      uint8_t _nvicSubPriority;

    public:
      static CanEventSource *_canInstance;

    public:
      Can2InterruptFeature(Can& can);
      ~Can2InterruptFeature();

      void setNvicPriorities(uint8_t priority,uint8_t subPriority=0);

      void enableInterrupts(uint16_t interruptMask);
      void disableInterrupts(uint16_t interruptMask);

      void clearPendingInterruptsFlag(uint16_t interruptMask) const;
      void initialise() {}
  };


  /**
   * Constructor
   */

  inline Can2InterruptFeature::Can2InterruptFeature(Can& can) :
      CanFeatureBase(can) {

    _interruptMask=0;
    _nvicPriority=_nvicSubPriority=0;
    _canInstance=this;
  }


  /**
   * Destructor, if any interrupts are configured for this class then
   * disable them so that ISR calls don't disappear off into nothing
   */

  inline Can2InterruptFeature::~Can2InterruptFeature() {
    if(_interruptMask!=0)
      disableInterrupts(_interruptMask);
  }

  /**
   * Set the NVIC priority and subpriority numbers.  This function just remembers your
   * numbers. The actual setting of the priority happens in enableInterrupts()
   * @param priority
   * @param subPriority
   */

  inline void Can2InterruptFeature::setNvicPriorities(uint8_t priority,uint8_t subPriority) {
    _nvicPriority=priority;
    _nvicSubPriority=subPriority;
  }


  /**
   * Enable the interrupts specified in the mask
   * @param interruptMask The bitmask of interrupts, e.g. CAN_IT_TME / CAN_IT_FMP0 / CAN_IT_FF0
   */

  inline void Can2InterruptFeature::enableInterrupts(uint16_t interruptMask) {

    _interruptMask|=interruptMask;
    _forceLinkage=&CAN2_TX_IRQHandler;

    Nvic::configureIrq(CAN2_TX_IRQn,ENABLE,_nvicPriority,_nvicSubPriority);
    Nvic::configureIrq(CAN2_RX0_IRQn,ENABLE,_nvicPriority,_nvicSubPriority);
    Nvic::configureIrq(CAN2_RX1_IRQn,ENABLE,_nvicPriority,_nvicSubPriority);
    Nvic::configureIrq(CAN2_SCE_IRQn,ENABLE,_nvicPriority,_nvicSubPriority);

    CAN_ITConfig(_can,interruptMask,ENABLE);
  }


  /**
   * Disable the interrupts specified in the mask
   * @param interruptMask The bitmask of interrupts, e.g. I2C_IT_TXE / I2C_IT_RXNE
   */

  inline void Can2InterruptFeature::disableInterrupts(uint16_t interruptMask) {
    _interruptMask&=~interruptMask;
    CAN_ITConfig(_can,interruptMask,DISABLE);
  }


  /**
   * Clear the pending bit(s) for interrupt
   * @param interruptMask The bitmask of interrupts, e.g. I2C_IT_TXE / I2C_IT_RXNE
   */

  inline void Can2InterruptFeature::clearPendingInterruptsFlag(uint16_t interruptMask) const {
    CAN_ClearITPendingBit(_can,interruptMask);
  }
}
