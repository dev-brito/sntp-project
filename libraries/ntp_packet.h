#ifndef __NTP_PACKET_H__
#define __NTP_PACKET_H__

#include <stdint.h>

struct ntp_packet {
    uint8_t li_vn_mode;      // Leap Indicator, Version Number e Mode
    uint8_t stratum;         // Stratum
    uint8_t poll;            // Poll Interval
    uint8_t precision;       // Precision
    uint32_t rootDelay;      // Root Delay
    uint32_t rootDispersion; // Root Dispersion
    uint32_t refId;          // Reference ID
    uint32_t refTm_s;        // Reference Timestamp (segundos)
    uint32_t refTm_f;        // Reference Timestamp (fração de segundo)
    uint32_t origTm_s;       // Originate Timestamp (segundos)
    uint32_t origTm_f;       // Originate Timestamp (fração de segundo)
    uint32_t rxTm_s;         // Receive Timestamp (segundos)
    uint32_t rxTm_f;         // Receive Timestamp (fração de segundo)
    uint32_t txTm_s;         // Transmit Timestamp (segundos)
    uint32_t txTm_f;         // Transmit Timestamp (fração de segundo)
};

#endif // __NTP_PACKET_H__
