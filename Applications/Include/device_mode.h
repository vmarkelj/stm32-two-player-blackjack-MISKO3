#ifndef INCLUDE_DEVICE_MODE_H_
#define INCLUDE_DEVICE_MODE_H_

typedef enum
{
    DEVICE_MODE_SINGLE_PLAYER = 0,
    DEVICE_MODE_MASTER,
    DEVICE_MODE_SLAVE

} device_mode_t;

/* DEKLARACIJE FUNKCIJ */
device_mode_t DeviceMode_Select(void);

#endif /* INCLUDE_DEVICE_MODE_H_ */
