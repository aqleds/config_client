#ifndef Q6_BINARY_H
#define Q6_BINARY_H


static float Q6_to_float(uint16_t q6v)
{
    return ((float)(q6v))/1024.0;
}

static uint16_t float_to_Q6(float fv)
{
    return (uint16_t)(fv*1024.0);
}

static uint16_t int16_extract(uint8_t *b_array)
{
    return ((b_array[0]<<8)|b_array[1]);
}

static void int16_store(uint8_t *b_array,uint16_t int16)
{
    b_array[0]=int16>>8;
    b_array[1]=int16&0xff;
}

static float Q6_extract(uint8_t *b_array)
{
    return Q6_to_float(int16_extract(b_array));
}

static float Q6_store(uint8_t *b_array, float fv)
{
    int16_store(b_array,float_to_Q6(fv));
}

#endif // Q6_BINARY_H
