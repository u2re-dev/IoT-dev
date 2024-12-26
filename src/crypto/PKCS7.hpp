#pragma once

//
#include <std/std.hpp>

//
typedef enum {
    BLOCK_SIZE_128_BIT      = 128 / 8,  /* 16 bytes block */
    BLOCK_SIZE_256_BIT      = 256 / 8,  /* 32 bytes block */
    BLOCK_SIZE_CUSTOM_VALUE = 0         /* you can set your own constant to use */
} paddingBlockSize;                     /* can be used as third argument to the function addPadding() */

typedef struct {
    void*    dataWithPadding;        /* result of adding padding to the data */
    uint64_t dataLengthWithPadding;  /* length of the result */
    uint8_t  valueOfByteForPadding;  /* used for padding byte value */
} PKCS7_Padding;                            

PKCS7_Padding* addPadding(const void* const data, const uint64_t dataLength, const uint8_t BLOCK_SIZE);

typedef struct {
    void const* dataWithoutPadding;         /* result of remove padding from data */
    uint64_t    dataLengthWithoutPadding;   /* length of the result */
    uint8_t     valueOfRemovedByteFromData; /* value of byte that was used for padding */
} PKCS7_unPadding;                              

PKCS7_unPadding* removePadding(const void* const data, const uint64_t dataLength);

//
void freePaddingResult(PKCS7_Padding* puddingResult);
void freeUnPaddingResult(PKCS7_unPadding* unPuddingResult);

//
PKCS7_Padding* addPadding(const void* const data, const uint64_t dataLength, const uint8_t BLOCK_SIZE, void* REAL_MEM)
{
    if (0 == BLOCK_SIZE)
    {
        puts("ERROR: block size value must be 0 < BLOCK_SIZE < 256");
        exit(-1);
    }
    
    PKCS7_Padding* paddingResult = (PKCS7_Padding*) malloc(sizeof(PKCS7_Padding));
    if (NULL == paddingResult)
    {
        perror("problem with PKCS7_Padding* paddingResult");    /* if memory allocation failed */
        exit(-1);
    }

    uint8_t paddingBytesAmount           = BLOCK_SIZE - (dataLength % BLOCK_SIZE);  /* number of bytes to be appended */
    paddingResult->valueOfByteForPadding = paddingBytesAmount;                      /* according to the PKCS7 */
    paddingResult->dataLengthWithPadding = dataLength + paddingBytesAmount;         /* size of the final result */
    
    uint8_t* dataWithPadding = (uint8_t*)REAL_MEM;//(uint8_t*) malloc(paddingResult->dataLengthWithPadding);
    if (NULL == paddingResult)
    {
        perror("problem with uint8_t* dataWithPadding");  /* if memory allocation failed */
        exit(-1);
    }
    
    memcpy(dataWithPadding, data, dataLength);  /* copying the original data for further adding padding */
    for (uint8_t i = 0; i < paddingBytesAmount; i++)
    {
        dataWithPadding[dataLength + i] = paddingResult->valueOfByteForPadding;   /* adding padding bytes */
    }
    paddingResult->dataWithPadding = dataWithPadding;

    return paddingResult;
}

//
PKCS7_Padding* addPadding(const void* const data, const uint64_t dataLength, const uint8_t BLOCK_SIZE)
{
    if (0 == BLOCK_SIZE)
    {
        puts("ERROR: block size value must be 0 < BLOCK_SIZE < 256");
        exit(-1);
    }
    
    PKCS7_Padding* paddingResult = (PKCS7_Padding*) malloc(sizeof(PKCS7_Padding));
    if (NULL == paddingResult)
    {
        perror("problem with PKCS7_Padding* paddingResult");    /* if memory allocation failed */
        exit(-1);
    }

    uint8_t paddingBytesAmount           = BLOCK_SIZE - (dataLength % BLOCK_SIZE);  /* number of bytes to be appended */
    paddingResult->valueOfByteForPadding = paddingBytesAmount;                      /* according to the PKCS7 */
    paddingResult->dataLengthWithPadding = dataLength + paddingBytesAmount;         /* size of the final result */
    
    uint8_t* dataWithPadding = (uint8_t*) malloc(paddingResult->dataLengthWithPadding);
    if (NULL == paddingResult)
    {
        perror("problem with uint8_t* dataWithPadding");  /* if memory allocation failed */
        exit(-1);
    }
    
    memcpy(dataWithPadding, data, dataLength);  /* copying the original data for further adding padding */
    for (uint8_t i = 0; i < paddingBytesAmount; i++)
    {
        dataWithPadding[dataLength + i] = paddingResult->valueOfByteForPadding;   /* adding padding bytes */
    }
    paddingResult->dataWithPadding = dataWithPadding;

    return paddingResult;
}

//
PKCS7_unPadding* removePadding(const void* const data, const uint64_t dataLength)
{
    PKCS7_unPadding* unpaddingResult = (PKCS7_unPadding*) malloc(sizeof(PKCS7_unPadding));
    if (NULL == unpaddingResult)
    {
        perror("problem with PKCS7_Padding* unpaddingResult");  /* if memory allocation failed */
        exit(-1);
    }

    uint8_t paddingBytesAmount                  = *((uint8_t *)data + dataLength - 1);  /* last byte contains length of data to be deleted */
    unpaddingResult->valueOfRemovedByteFromData = paddingBytesAmount;                   /* according to the PKCS7 */
    unpaddingResult->dataLengthWithoutPadding   = dataLength - paddingBytesAmount;      /* size of the final result */
    unpaddingResult->dataWithoutPadding = data;
    bzero((uint8_t*)data + (dataLength - paddingBytesAmount), paddingBytesAmount);

    //
    return unpaddingResult;
}

void freePaddingResult(PKCS7_Padding* puddingResult)
{
    free(puddingResult->dataWithPadding);
    free(puddingResult);
}

void freeUnPaddingResult(PKCS7_unPadding* unPuddingResult)
{
    free(unPuddingResult);
}
