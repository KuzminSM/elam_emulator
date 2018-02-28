#ifndef __circle_H
#define __circle_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

// Циклический буффер   
typedef struct CircularBuffer
{
  uint8_t *data;      // Начало буфера 
  uint16_t len;      // Длинна буффера
} CircularBuffer;

// Итератор по циклическому буфферу
typedef struct CircularIterator
{
  struct CircularBuffer *buff; // Циклический буфер
  uint16_t pos;         // Позиция указателя
} CircularIterator;

uint8_t circGetByte(struct CircularBuffer* buff, int16_t index);

// Считываение байта в цикличесом буфере + смещение итератора на следующий байт
uint8_t circParseByte(struct CircularIterator* iter, int16_t *len);
// Считываение слова в цикличесом буфере + смещение итератора на следующее слово
uint16_t circParseWord(struct CircularIterator* iter, int16_t *len);

uint16_t circParseWordLE(struct CircularIterator* iter, int16_t *len);

// Пропуск increment байт в циклическом буфере
void circParseSkip(struct CircularIterator* iter, uint16_t increment, int16_t *len);

// Контрольная сумма в циклическом буффере
uint16_t circularCrc16(struct CircularIterator* iter, int16_t len);

// Копирование из циклического буффера в обычный
void circularCopy(struct CircularIterator* src, int16_t len, uint8_t *dest);

// Адрес контроллера в стандарте ELAM (1 или 2 байта)
uint16_t circParseAddress(struct CircularIterator* iter, int16_t *len);

// Длинна из счетчика
uint16_t circLenFromCNDTR(struct CircularIterator* iter, uint16_t count);

// Поиск подстроки
int16_t circularSubStr(struct CircularIterator* iter, int16_t len, const char* str);

#ifdef __cplusplus
}
#endif
   
#endif /* __circle_H */
