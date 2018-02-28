#include "modbus.h"

#include <stddef.h>
#include <stdio.h>
#include "crc16.h"
#include "mb_regs.h"

#define make_uint16(hi, lo) ((hi) << 8 | (lo))

void buffWriteByte(uint8_t arg, uint8_t **buffer, int16_t *len)
{
  if (len && ((*len)-- > 0)) // Есть куда писать
  { 
    *(*buffer)++ = arg;
  }
}

void buffWriteWord(uint16_t arg, uint8_t **buffer, int16_t *len)
{
  if (len && ((*len)-- > 0)) // Есть куда писать
  {
    *(*buffer)++ = arg >> 8;
  }
  if (len && ((*len)-- > 0)) // Есть куда писать
  {
    *(*buffer)++ = arg & 0xff;
  }
}

void buffWriteAddress(uint16_t address, uint8_t **buffer, int16_t *len)
{
  if (address > 247)
  {
    address -= 248;
    buffWriteByte(address >> 8 | 0xf8, buffer, len);
    buffWriteByte(address & 0xff, buffer, len);
  }
  else
  {
    buffWriteByte(address & 0xff, buffer, len);
  }
}

void buffWriteCrc16(uint16_t crc, uint8_t **buffer, int16_t *len)
{
  buffWriteByte(crc & 0xff, buffer, len); // lo CRC
  buffWriteByte(crc >> 8, buffer, len);   // Hi CRC
}

uint16_t ParseRTURequest(struct CircularIterator* iter, int16_t len, ModbusLine *req)
{
  uint16_t i = 0;
  req->size = 0;
  
  while (len > 0)
  {
    struct CircularIterator cmd_start = *iter;
    int16_t start_cmd_len = len;
    
    req->cmds[i].address = circParseAddress(iter, &len); // адрес контроллера
    req->cmds[i].cmd = circParseByte(iter, &len); // Код команды
    req->cmds[i].data = *iter; // Данные команды
    switch (req->cmds[i].cmd)
    {
    case MODBUS_FNC_READ_COIL_STATUS:
    case MODBUS_FNC_READ_INPUT_STATUS:
    case MODBUS_FNC_READ_HOLDING_REGISTERS:
    case MODBUS_FNC_READ_INPUT_REGISTERS:
    case MODBUS_FNC_FORCE_SINGLE_COIL:
    case MODBUS_FNC_PRESET_SINGLE_REGISTER: 
      circParseSkip(iter, 4, &len);  // У всех этих запросов длинна данных команды 4 байта
      req->cmds[i].data_len = 4;
      break;
    case MODBUS_FNC_PRESET_MULTIPLE_REGS:
      circParseSkip(iter, 4, &len);  // Адрес Регистра(2 байта) + Количество(2 байта)
      uint8_t data_len = circParseByte(iter, &len); // Длинна данных
      circParseSkip(iter, data_len, &len); // Данные
      req->cmds[i].data_len = data_len + 5 ;
      break;
    default:
      return 3; // Неизвестная команда
    };
    
    circParseSkip(iter, 2, &len); // CRC16
    
    if (len < 0)
      return 2; // Незавершенная команда
    
    if (circularCrc16(&cmd_start, start_cmd_len - len) != 0)
    {
      if( len > 0 ) circParseSkip(iter, len, NULL);
      return 1; // Не совпадает котрольная сумма
    }
    
    req->size = ++i;
  }
  
  return 0;
}

void ProcessErrorCode(ModbusCmdDef *req, uint8_t **buffer, int16_t *len, uint8_t err)
{
    // Формируем ответ
    uint8_t *cmd_start = *buffer;
    int16_t size = *len;
    // Запись адреса контроллера
    buffWriteAddress(req->address, buffer, len);

    // Запись кода команды
    buffWriteByte(req->cmd | MODBUS_ERROR_FLAG, buffer, len);

    // Запись кода ошибки
    buffWriteByte(err, buffer, len);

    // CRC16
    uint16_t crc = Crc16Block( cmd_start, size - *len );
    buffWriteCrc16(crc, buffer, len);
}

void ProcessReadRegs(ModbusCmdDef *req, uint8_t **buffer, int16_t *len)
{
  struct CircularIterator iter = req->data;
  int16_t req_len = req->data_len;
  // Читаем адрес первого регистра
  uint16_t address = circParseWord(&iter, &req_len);
  // Читаем количество регистров
  uint16_t reg_count = circParseWord(&iter, &req_len);  

  // Проверка адресов регистров на корректность
  int max_regs = (req->cmd == MODBUS_FNC_READ_HOLDING_REGISTERS) ?
                  HoldingRegistersCount(req->address) : InputRegistersCount(req->address);
  if(address + reg_count > max_regs)
  {
    ProcessErrorCode(req, buffer, len, MODBUS_ILLEGAL_DATA);
    return;
  }

  // Формируем ответ
  uint8_t *cmd_start = *buffer;
  int16_t size = *len;
  // Запись адреса контроллера  
  buffWriteAddress(req->address, buffer, len);

  // Запись кода команды
  buffWriteByte(req->cmd, buffer, len);
  
  // Длинна данных
  uint16_t byte_len = 2*reg_count;  
  if( req->address > 247 ) // Если ELAM до длинна в 2-х байтах
  {
    buffWriteWord(byte_len, buffer, len);
  }
  else
  {
    buffWriteByte(byte_len & 0xff, buffer, len);
  }
  
  // Данные регистров
  for( uint16_t i = 0; i < reg_count; ++i )
  { 
    uint16_t reg = (req->cmd == MODBUS_FNC_READ_HOLDING_REGISTERS) ?
                    GetHoldingReg(req->address, address + i) : GetInputReg(req->address, address + i);
    buffWriteWord(reg, buffer, len);
  }
  // CRC16
  uint16_t crc = Crc16Block( cmd_start, size - *len );
  buffWriteCrc16(crc, buffer, len);
}  

void ProcessWriteRegs(ModbusCmdDef *req, uint8_t **buffer, int16_t *len)
{
  struct CircularIterator iter = req->data;
  int16_t req_len = req->data_len;
  // Читаем адрес первого регистра
  uint16_t address = circParseWord(&iter, &req_len);
  // Читаем количество регистров
  uint16_t reg_count = circParseWord(&iter, &req_len);
  // Читаем длинну данных
  uint8_t data_size = circParseByte(&iter, &req_len);
  req_len = data_size;
  // Читаем и записываем новые значения регистров
  for (uint16_t i = 0; i < reg_count; ++i)
  { 
    SetHoldingReg(req->address, address + i, circParseWord(&iter, &req_len) );
  }

  // Проверка адресов регистров на корректность
  if(address + reg_count > HoldingRegistersCount(req->address))
  {
    ProcessErrorCode(req, buffer, len, MODBUS_ILLEGAL_DATA);
    return;
  }

  // Формируем ответ
  uint8_t *cmd_start = *buffer;
  int16_t size = *len;
  // Запись адреса контроллера  
  buffWriteAddress(req->address, buffer, len);
  //Код команды
  buffWriteByte(req->cmd, buffer, len);
  // Адрес первого регистра
  buffWriteWord(address, buffer, len);
  // Количество регистров
  buffWriteWord(reg_count, buffer, len);
  // CRC16
  uint16_t crc = Crc16Block(cmd_start, size - *len);
  buffWriteCrc16(crc, buffer, len);
}

void ProcessWriteOne(ModbusCmdDef *req, uint8_t **buffer, int16_t *len)
{
  struct CircularIterator iter = req->data;
  int16_t req_len = req->data_len;
  // Читаем адрес регистра
  uint16_t address = circParseWord(&iter, &req_len);
  // Читаем новое значение регистра
  uint16_t reg_data = circParseWord(&iter, &req_len);
  SetHoldingReg(req->address, address, reg_data);

  // Проверка адреса регистра на корректность
  if(address >= HoldingRegistersCount(req->address))
  {
    ProcessErrorCode(req, buffer, len, MODBUS_ILLEGAL_DATA);
    return;
  }

  // Формируем ответ
  uint8_t *cmd_start = *buffer;
  int16_t size = *len;
  // Запись адреса контроллера  
  buffWriteAddress(req->address, buffer, len);
  //Код команды
  buffWriteByte(req->cmd, buffer, len);
  // Адрес первого регистра
  buffWriteWord(address, buffer, len);
  // Данные
  buffWriteWord(reg_data, buffer, len);
  // CRC16
  uint16_t crc = Crc16Block(cmd_start, size - *len);
  buffWriteCrc16(crc, buffer, len);
}

void ProcessForceCoil(ModbusCmdDef *req, uint8_t **buffer, int16_t *len)
{
  struct CircularIterator iter = req->data;
  int16_t req_len = req->data_len;
  // Читаем адрес ТУ
  uint16_t address = circParseWord(&iter, &req_len);
  // Читаем значение ТУ
  uint16_t coil_data = circParseWord(&iter, &req_len);
  ForceCoil(req->address, address, coil_data);

  // Проверка адресов регистров на корректность
  if(address >= CoilCount(req->address))
  {
    ProcessErrorCode(req, buffer, len, MODBUS_ILLEGAL_DATA);
    return;
  }

  // Формируем ответ
  uint8_t *cmd_start = *buffer; // Запоминаем начало для CRC
  int16_t size = *len; 
  // Запись адреса контроллера  
  buffWriteAddress(req->address, buffer, len);
  //Код команды
  buffWriteByte(req->cmd, buffer, len);
  // Адрес первого регистра
  buffWriteWord(address, buffer, len);
  // Данные
  buffWriteWord(coil_data, buffer, len);
  // CRC16
  uint16_t crc = Crc16Block(cmd_start, size - *len);
  buffWriteCrc16(crc, buffer, len);
}

void ProcessCoilInputStatus(ModbusCmdDef *req, uint8_t **buffer, int16_t *len)
{
  struct CircularIterator iter = req->data;
  int16_t req_len = req->data_len;
  // Читаем адрес первого ТС или ТУ
  uint16_t address = circParseWord(&iter, &req_len);
  // Читаем количество ТС или ТУ
  uint16_t input_count = circParseWord(&iter, &req_len);

  // Проверка адресов на корректность
  int max_regs = (req->cmd == MODBUS_FNC_READ_COIL_STATUS) ?
                  CoilCount(req->address) : InputStatusCount(req->address);
  if(address + input_count > max_regs)
  {
    ProcessErrorCode(req, buffer, len, MODBUS_ILLEGAL_DATA);
    return;
  }

  // Формируем ответ
  uint8_t *cmd_start = *buffer;
  int16_t size = *len;
  // Запись адреса контроллера  
  buffWriteAddress(req->address, buffer, len);
  // Запись кода команды
  buffWriteByte(req->cmd, buffer, len);

  // Длинна данных
  uint8_t byte_len = input_count >> 3;
  if (input_count & 0x07) ++byte_len;
  buffWriteByte(byte_len, buffer, len);

  uint8_t bit_mask = 1;
  uint8_t write_byte = 0;
  // Данные выходов или телесигналов
  while(input_count--)
  {
    if( (req->cmd == MODBUS_FNC_READ_COIL_STATUS) ? GetCoilStatus(req->address, address) : GetInputStatus(req->address, address) )
      write_byte |= bit_mask;
    ++address;

    if( (bit_mask == 0x80) || (input_count == 0) )
    {
      buffWriteByte(write_byte, buffer, len);
      bit_mask = 1;
      write_byte = 0;
    } 
    else
    {
      bit_mask <<= 1;
    }
  }

  // CRC16
  uint16_t crc = Crc16Block(cmd_start, size - *len);
  buffWriteCrc16(crc, buffer, len);
}

int16_t ProcessRTURequest(ModbusLine *req, uint8_t *buffer, int16_t len)
{
  int16_t size = len;

  for (uint16_t i = 0; i < req->size; ++i)
  {
    switch (req->cmds[i].cmd)
    {
    case MODBUS_FNC_READ_COIL_STATUS:      
    case MODBUS_FNC_READ_INPUT_STATUS:
      ProcessCoilInputStatus(&req->cmds[i], &buffer, &len);
      break;
    case MODBUS_FNC_READ_HOLDING_REGISTERS:
    case MODBUS_FNC_READ_INPUT_REGISTERS:
      ProcessReadRegs(&req->cmds[i], &buffer, &len);
      break;
    case MODBUS_FNC_FORCE_SINGLE_COIL:
      ProcessForceCoil(&req->cmds[i], &buffer, &len);
      break;
    case MODBUS_FNC_PRESET_SINGLE_REGISTER:
      ProcessWriteOne(&req->cmds[i], &buffer, &len);
      break;
    case MODBUS_FNC_PRESET_MULTIPLE_REGS:
      ProcessWriteRegs(&req->cmds[i], &buffer, &len);
      break;
    default:
      ProcessErrorCode(&req->cmds[i], &buffer, &len, MODBUS_ILLEGAL_CMD);
    }
  }
  
  return size - len;
}  

/*
uint16_t ParseRTUResponse(struct CircularIterator* iter, int16_t len, ModbusLine *resp)
{
  uint16_t i = 0;
  resp->size = 0;

  while (len > 0)
  {
    struct CircularIterator cmd_start = *iter;
    uint16_t start_cmd_len = len;

    resp->cmds[i].address = circParseAddress(iter, &len); // адрес контроллера
    resp->cmds[i].cmd = circParseByte(iter, &len); // Код команды
    resp->cmds[i].data = *iter; // Данные команды
    uint16_t data_len = 0;

    switch (resp->cmds[i].cmd)
    {
    case MODBUS_FNC_READ_COIL_STATUS:
    case MODBUS_FNC_READ_INPUT_STATUS:
      data_len = circParseByte(iter, &len); // Длинна данных
      circParseSkip(iter, data_len, &len); // Данные
      resp->cmds[i].data_len = data_len + 1;
      break;
    case MODBUS_FNC_READ_HOLDING_REGISTERS:
    case MODBUS_FNC_READ_INPUT_REGISTERS:
      data_len = (resp->cmds[i].address > 247) ?  // Длинна данных для ELAM 2 байта, для обычного Modbus 1 байт 
        circParseWord(iter, &len) : circParseByte(iter, &len);
      circParseSkip(iter, data_len, &len); // Данные
      resp->cmds[i].data_len = (resp->cmds[i].address > 247) ? data_len + 2 : data_len + 1;
      break;
    case MODBUS_FNC_FORCE_SINGLE_COIL:
    case MODBUS_FNC_PRESET_SINGLE_REGISTER:
      circParseSkip(iter, 4, &len);  // У этих ответов длинна данных 4 байта (адрес + данные) 
      resp->cmds[i].data_len = 4;
      break;
    case MODBUS_FNC_PRESET_MULTIPLE_REGS:
      circParseSkip(iter, 3, &len);  // У этих ответов длинна данных 4 байта (адрес + длинна) 
      resp->cmds[i].data_len = 3;
      break;
      break;
    };

    circParseSkip(iter, 2, &len); // CRC16

    if (len < 0)
      return 2; // Незавершенная команда

    if (circularCrc16(&cmd_start, start_cmd_len - len) != 0)
      return 1; // Не совпадает котрольная сумма

    resp->size = ++i;
  }

  return 0;
}

//TODO: Реализовать
int16_t ProcessRTUResponse(ModbusLine *req, ModbusLine *resp, uint8_t *buffer, int16_t len)
{
  int16_t size = len;

  for (uint16_t i = 0; i < resp->size; ++i)
  {
    switch (resp->cmds[i].cmd)
    {
    case MODBUS_FNC_READ_COIL_STATUS:
    case MODBUS_FNC_READ_INPUT_STATUS:
    case MODBUS_FNC_READ_HOLDING_REGISTERS:
    case MODBUS_FNC_READ_INPUT_REGISTERS:
    case MODBUS_FNC_FORCE_SINGLE_COIL:
    case MODBUS_FNC_PRESET_SINGLE_REGISTER:
    case MODBUS_FNC_PRESET_MULTIPLE_REGS:
      break;
    }
  }

  return size - len;
}
*/
