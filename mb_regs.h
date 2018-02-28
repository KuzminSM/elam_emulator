#ifndef __mb_regs_H
#define __mb_regs_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

int InputRegistersCount(uint16_t  plc_addr);
int HoldingRegistersCount(uint16_t  plc_addr);
int InputStatusCount(uint16_t  plc_addr);
int CoilCount(uint16_t  plc_addr);

uint16_t GetInputReg(uint16_t plc_addr, uint16_t reg_addr);
void SetInputReg(uint16_t plc_addr, uint16_t reg_addr, uint16_t value);

uint16_t GetHoldingReg(uint16_t plc_addr, uint16_t reg_addr);
void SetHoldingReg(uint16_t plc_addr, uint16_t reg_addr, uint16_t value);
   
bool GetInputStatus(uint16_t plc_addr, uint16_t reg_addr);
void SetInputStatus(uint16_t plc_addr, uint16_t reg_addr, bool value);

void  ForceCoil(uint16_t plc_addr, uint16_t reg_addr, bool value);

void SetCoilStatus(uint16_t plc_addr, uint16_t reg_addr, bool value);
bool GetCoilStatus(uint16_t plc_addr, uint16_t reg_addr);

#ifdef __cplusplus
}
#endif
   
#endif /* __mb_regs_H */
