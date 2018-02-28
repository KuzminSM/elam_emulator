#include "circle.h"

#include <string.h>
#include "crc16.h"

#define make_uint16(hi, lo) ((hi) << 8 | (lo))

uint8_t circGetByte(struct CircularBuffer* buff, int16_t idx)
{
  while (idx >= buff->len)
    idx -= buff->len;
  return buff->data[idx];
}  

uint8_t circParseByte(struct CircularIterator* iter, int16_t *len)
{
  if ( len && ((*len)-- <= 0 ) ) return 0;
  
  uint8_t result = iter->buff->data[iter->pos];

  if (++iter->pos == iter->buff->len) 
    iter->pos = 0;

  return result;
}

uint16_t circParseWord(struct CircularIterator* iter, int16_t *len)
{
  uint8_t hi = circParseByte(iter, len);
  uint8_t lo = circParseByte(iter, len);

  return make_uint16(hi, lo);
}

uint16_t circParseWordLE(struct CircularIterator* iter, int16_t *len)
{
  uint8_t lo = circParseByte(iter, len);
  uint8_t hi = circParseByte(iter, len);

  return make_uint16(hi, lo);
}

void circParseSkip(struct CircularIterator* iter, uint16_t increment, int16_t *len)
{
  if (len) *len -= increment;
  iter->pos += increment;

  while (iter->pos >= iter->buff->len)
    iter->pos -= iter->buff->len;
}

void circularCopy(struct CircularIterator* src, int16_t len, uint8_t *dest)
{
  while (len > 0)
    *dest++ = circParseByte(src, &len);
}

uint16_t circParseAddress(struct CircularIterator* iter, int16_t *len)
{
  uint16_t address = circParseByte(iter, len); // адрес контроллера
  if (address > 247) // ELAM адрес (2 байта)
  {
    address = make_uint16(address - 248, circParseByte(iter, len)) + 248;
  }
  return address;
}

uint16_t circularCrc16(struct CircularIterator* iter, int16_t len)
{
  uint16_t crc_word = 0xFFFF;
  
  while (len > 0) 
    crc_word = Crc16Byte(crc_word, circParseByte(iter,&len));
  
  return crc_word;
}

uint16_t circLenFromCNDTR(struct CircularIterator* iter, uint16_t CNDTR)
{
  uint16_t new_count = iter->buff->len - CNDTR;
  
  if ( new_count > iter->pos )
  {
    return new_count - iter->pos;
  }
  else if ( new_count < iter->pos )
  {
    return iter->buff->len - iter->pos + new_count;            
  }
  return 0;
}  

int16_t circularSubStr(struct CircularIterator* iter, int16_t len, const char* str)
{
  uint16_t m = strlen(str);
  for(uint16_t i = 0; i <= (len-m); ++i)
  {
    uint16_t j = 0;
    while( (j<m) && (circGetByte(iter->buff, iter->pos + i + j) == str[j] ) )
      ++j;
    if( j == m )
      return i;
  }
  return -1;          
}
