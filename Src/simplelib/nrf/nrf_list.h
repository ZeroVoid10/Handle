#ifndef __NRF_LIST_H
#define __NRF_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_comm.h"

typedef struct nrf_static_list {
    nrf_pck packages;
    uint32_t next
} nrf_list;

#ifdef __cplusplus
}
#endif

#endif /* __NRF_LIST_H */