#include <pcap/pcap.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

#include "libs/bitstream/ieee/ethernet.h"
#include "libs/bitstream/ietf/ip.h"
#include "libs/bitstream/ietf/udp.h"
#include "libs/bitstream/ietf/rtp.h"
#include "libs/bitstream/smpte/2022_6_hbrmt.h"

const char *filename = "my_data.yuv";
std::ofstream output_file(filename, std::ios::binary);
int cnt;

int time_for_log(char buf[256])
{
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, 256, "%Y-%m-%d %H:%M:%S%z", &tm);
    return 0;
}

std::vector<uint8_t*> extract_hbrmt_data(uint8_t *hbrmt_pkt){
    std::vector<uint8_t*> ret;

    return ret;
}

bool is_hbrmt_present(uint8_t *hbrmt_pkt) {
    // Implement logic to check for HBMP presence based on ST-2022-6 criteria
    // This might involve checking specific header fields or patterns in the payload
    // Replace this with the actual implementation from the standard
    return false;  // Placeholder
}

void got_packet_2022_6(u_char *user, const struct pcap_pkthdr *header, const u_char *packet)
{
    pcap_t *pcap = (pcap_t*)user;
    uint8_t *ip_pkt = ethernet_payload((uint8_t*)packet);
    uint8_t *udp_pkt = ip_payload(ip_pkt);
    uint8_t *rtp_pkt = udp_payload(udp_pkt);

    if (rtp_get_type(rtp_pkt) != 98)
        printf("Error: RTP type not correct\n");
    uint8_t *hbrmt_pkt = rtp_payload(rtp_pkt);
    uint8_t header_frate = smpte_hbrmt_get_frate(hbrmt_pkt);

    struct rational {
    int num;
    int den;

    rational(int numerator, int denominator) : num(numerator), den(denominator) {}
    };

    const rational fps[] = {
        {60, 1},
        {60000, 1001},
        {50, 1},
        {48, 1},
        {48000, 1001},
        {30, 1},
        {30000, 1001},
        {25, 1},
        {24, 1},
        {24000, 1001},
    };
    //const uint64_t frame_time = UINT64_C(1000000000) * fps[header_frate].den; LEO : header_frate 값이 25일 때 fps 값이 존재하지 않아 임의로 값 입력했습니다.
    const uint64_t frame_time = UINT64_C(1000000000) * 2000;

    uint8_t header_frame = smpte_hbrmt_get_frame(hbrmt_pkt);
    uint32_t frame_size = 0;
    /* NTSC */
    if (header_frame == 0x10)
        frame_size = 858 * 525 / 2 * 5;

    /* PAL */
    else if (header_frame == 0x11)
        frame_size = 864 * 625 / 2 * 5;

    /* 720 lines */
    else if (header_frame == 0x30) {
        if (header_frate == 0x12)
            frame_size = 1980 * 750 / 2 * 5;
        else
            frame_size = 1650 * 750 / 2 * 5;
    }
    
    /* 1080 lines */
    else if (header_frame == 0x20 || header_frame == 0x21) {
        
        if (header_frate == 0x1b || header_frate == 0x1a)
            frame_size = 2750 * 1125 / 2 * 5;
        else if (header_frate == 0x18 || header_frate == 0x12)
            frame_size = 2640 * 1125 / 2 * 5;
        else
            frame_size = 2200 * 1125 / 2 * 5;
    }

    printf("FEC : %d\n", smpte_hbrmt_get_fec(hbrmt_pkt));
    printf("CF : %d\n", smpte_hbrmt_get_clock_frequency(hbrmt_pkt));
    printf("MAP : ", smpte_hbrmt_get_map(hbrmt_pkt));
    printf("FRAME : %d\n", header_frame);
    printf("FRATE : %d\n", header_frate);
    printf("MAP : %d\n", smpte_hbrmt_get_map(hbrmt_pkt));
    printf("SAMPLE : %d\n", smpte_hbrmt_get_sample(hbrmt_pkt));

    // if (smpte_hbrmt_get_frame_count(hbrmt_pkt) != 48){
    //     return;
    // }

    if (smpte_hbrmt_get_clock_frequency(hbrmt_pkt) > 0){
        printf("TimeStamp : %d", smpte_hbrmt_get_timestamp(hbrmt_pkt));
    }
    else {
        printf("No Timestamp Because CF == 0\n");
    }
    // double packet_time = frame_time
    //     / (((frame_size + HBRMT_DATA_SIZE-1) / HBRMT_DATA_SIZE) * fps[header_frate].num);
    double packet_time = frame_time
        / (((frame_size + HBRMT_DATA_SIZE-1) / HBRMT_DATA_SIZE) * 50000);

    char time_buf[256];
    if (time_for_log(time_buf))
        pcap_breakloop(pcap);

    uint64_t recv_timestamp;
    
    recv_timestamp = header->ts.tv_sec * UINT64_C(1000000000) + header->ts.tv_usec * 1000;
    //__uint128_t temp = fps[header_frate].num;
    __uint128_t temp = 50000;
    temp *= recv_timestamp;
    temp %= frame_time;
    uint64_t ptp_epoch_diff = temp;

    static int64_t box[25] = {0};
    static unsigned counter = 0;
    static int64_t sum = 0;

    if (ptp_epoch_diff > frame_time / 2) {
        sum -= box[counter];
        box[counter] = (int64_t)ptp_epoch_diff - (int64_t)frame_time;
        sum += box[counter];
        printf("%s: Marker arrived %.3f ms before epoch, %2.1f packets off, rolling average: %.3fms\n",
                time_buf,
                (frame_time - ptp_epoch_diff) / (1e6 * 50000),
                (frame_time - ptp_epoch_diff) / (packet_time * 50000),
                sum / (25e6 * 50000));
    }
    else {
        sum -= box[counter];
        box[counter] = ptp_epoch_diff;
        sum += box[counter];
        printf("%s: Marker arrived %.3f ms after epoch,  %2.1f packets off, rolling average: %.3fms\n",
                time_buf,
                ptp_epoch_diff / (1e6 * 50000),
                ptp_epoch_diff / (packet_time * 50000),
                sum / (25e6 * 50000));
    }

    counter = (counter + 1) % 25;

    uint8_t* hbrm_payload = smpte_hbmprt_payload(hbrmt_pkt);
    int dataSize = 1376;
    
    char data[dataSize];
    memcpy(data, hbrm_payload, dataSize);
        // Write the data to the file
        output_file.write(data, dataSize);
        // Check if the write operation was successful
        if (output_file.bad()) {
            std::cerr << "Error writing to file: " << filename << std::endl;
            return;
        }
    cnt++;
    
}

int main (int argc, char** argv){

    char errbuff[PCAP_ERRBUF_SIZE];
    int packet_count = -1;
    pcap_t *pcap = NULL;
    if (argc < 2){
        pcap = pcap_open_offline("one_frame_smpte_2022_6.pcap", errbuff);
    }
    else {
        pcap = pcap_open_offline(argv[1], errbuff);
    }
    if(pcap == NULL) {
        fprintf(stderr, "Couldn't open pcap : %s\n", errbuff);
        std::cout<<"Usage : ./pcap2ST20226 {filename}.pcap"<<std::endl;
        return -1;
    }

    if (!output_file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return -2;
    }
    pcap_loop(pcap, packet_count, got_packet_2022_6, (u_char*)pcap);
    printf("Counter : %d",cnt);
    output_file.close();
}