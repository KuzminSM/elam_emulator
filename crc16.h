#ifndef __crc16_H
#define __crc16_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

uint16_t Crc16Byte(uint16_t crc, uint8_t ch);
uint16_t Crc16Block(uint8_t *data, uint16_t len);
uint16_t Crc16Buffer(uint8_t *buffer, uint16_t buf_size, uint16_t start, uint16_t len);
   
#ifdef __cplusplus
}
#endif
   
#endif /* __crc16_H */
