#include "mb_regs.h"
#include "bitsmodel.h"
#include "registermodel.h"

int InputRegistersCount(uint16_t  plc_addr)
{
    return RegisterModel::input()->count(plc_addr);
}

int HoldingRegistersCount(uint16_t  plc_addr)
{
    return RegisterModel::holding()->count(plc_addr);;
}

int InputStatusCount(uint16_t  plc_addr)
{
    return BitsModel::input()->count(plc_addr);
}

int CoilCount(uint16_t  plc_addr)
{
    return BitsModel::coil()->count(plc_addr);
}

uint16_t GetInputReg(uint16_t plc_addr, uint16_t reg_addr)
{
    return RegisterModel::input()->regData(plc_addr, reg_addr);
}

void SetInputReg(uint16_t plc_addr, uint16_t reg_addr, uint16_t value)
{
    RegisterModel::input()->setRegData(plc_addr, reg_addr, value);
}

uint16_t GetHoldingReg(uint16_t plc_addr, uint16_t reg_addr)
{
    return RegisterModel::holding()->regData(plc_addr, reg_addr);
}

void SetHoldingReg(uint16_t plc_addr, uint16_t reg_addr, uint16_t value)
{
    RegisterModel::holding()->setRegData(plc_addr, reg_addr, value);
}

bool GetInputStatus(uint16_t plc_addr, uint16_t reg_addr)
{
    return BitsModel::input()->bitData(plc_addr, reg_addr);
}

void SetInputStatus(uint16_t plc_addr, uint16_t reg_addr, bool value)
{
    BitsModel::input()->setBitData(plc_addr, reg_addr, value);
}

bool GetCoilStatus(uint16_t plc_addr, uint16_t reg_addr)
{
    return BitsModel::coil()->bitData(plc_addr, reg_addr);
}

void ForceCoil(uint16_t plc_addr, uint16_t reg_addr, bool value)
{
    BitsModel::coil()->setBitData(plc_addr, reg_addr, value);
}

void SetCoilStatus(uint16_t plc_addr, uint16_t reg_addr, bool value)
{
    BitsModel::coil()->setBitData(plc_addr, reg_addr, value);
}
