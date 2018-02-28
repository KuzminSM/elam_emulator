#ifndef __modbus_H
#define __modbus_H

#include <stdint.h>
#include "circle.h"

#ifdef __cplusplus
 extern "C" {
#endif

/// Modbus команда: Чтение состояния выходов
#define MODBUS_FNC_READ_COIL_STATUS 0x01

/// Modbus команда: Чтение состояния дискретных входов (ТС)
#define MODBUS_FNC_READ_INPUT_STATUS 0x02 // ИРЗ не поддерживает

/// Modbus команда: Чтение значений хранимых регистров
#define MODBUS_FNC_READ_HOLDING_REGISTERS 0x03

/// Modbus команда: Чтение входных регистров (ТИТ)
#define MODBUS_FNC_READ_INPUT_REGISTERS 0x04

/// Modbus команда: Запись одного выхода
#define MODBUS_FNC_FORCE_SINGLE_COIL 0x05

/// Modbus команда: Запись значения в одиночный регистр
#define MODBUS_FNC_PRESET_SINGLE_REGISTER 0x06

/// Modbus команда: Чтение кода ошибки
#define MODBUS_FNC_READ_EXCEPTION_STATUS 0x07 // ИРЗ не поддерживает

/// Код ошибки
#define MODBUS_ERROR_FLAG 0x80

/// Modbus команда: Запись нескольких регистров
#define MODBUS_FNC_PRESET_MULTIPLE_REGS 0x10
   
#define ELAM_MAX_COMMANDS 20   

#define MODBUS_ILLEGAL_CMD 0x01
#define MODBUS_ILLEGAL_DATA 0x02

// Modbus команда
typedef struct ModbusCmd
{
  uint16_t address;   // Адрес контроллера
  uint8_t cmd;        // Код команды
  struct CircularIterator data;      // Данные команды
  int16_t data_len;  // Длинна данных 

} ModbusCmdDef;

// ELAM пакет из нескольких команд
typedef struct
{
  ModbusCmdDef cmds[ELAM_MAX_COMMANDS]; // массив с командами
  uint16_t size; // количество команд

} ModbusLine;

// Проверка пакета с запросом (корректные длинны команд и CRC)
// Результат разбора в res
uint16_t ParseRTURequest(struct CircularIterator* iter, int16_t len, ModbusLine *res);
// Обработка команды с запросом request, результаты в buffer, размером size
// Возвращает рамер ответа в байтах.
int16_t ProcessRTURequest(ModbusLine *request, uint8_t *buffer, int16_t size);

/*
uint16_t ParseRTUResponse(struct CircularIterator* iter, int16_t len, ModbusLine *res);
int16_t ProcessRTUResponse(ModbusLine *request, ModbusLine *response, uint8_t *buffer, int16_t size);
*/

#ifdef __cplusplus
}
#endif
   
#endif /* __modbus_H */
