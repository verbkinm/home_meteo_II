#ifndef TECH_H
#define TECH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct Tech
{
    uint8_t flags;
    uint8_t type;

    uint64_t dataSize;

    uint8_t *data;
};

uint64_t techCheckSum(const struct Tech *body);
uint64_t techSize(const struct Tech *body);
uint64_t techToData(const struct Tech *body, char *outData, uint64_t outDataSize);
void clearTech(struct Tech *tech);

#ifdef __cplusplus
}
#endif

#endif // TECH_H
