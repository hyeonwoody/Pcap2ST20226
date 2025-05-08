# Pcap2ST2022-6
SMPTE ST 2022-6 패킷을 분석하고 YUV 데이터를 추출하는 CLI 기반 소스코드.
yuv파일(422, UYVY, 10bit)에서 AudioData, AudioControl & Timecode 정보를 추출하는 CLI 기반 소스코드.



## 🧑‍💻: Intro
PCAP 파일에서 SMPTE ST 2022-6 패킷을 분석하고, 
RTP 패킷 내의 HBRMT (High Bit-Rate Media Transport) 데이터를 추출하여 YUV 파일로 저장합니다. 

AudioControl은 오디오의 볼륨, 채널 설정, 음소거 설정을 담고 있으며, '%' 입니다.</br>
AudioData는 인코딩 형식에 따라 다르며 PCM 오디오 정보이며, 태그는 '#' 입니다.</br>
Timecode는 각 프레임 마다 존재하므로 프레임의 개수만큼 나오고 태그는 '$' 입니다.</br>
</br>

## ✅: Implementation 
- **got_packet_2022_6**: PCAP 파일에서 각 패킷을 처리하는 주요 함수입니다.
- **extract_hbrmt_data**: HBRMT 패킷에서 데이터를 추출하는 함수입니다.
- **is_hbrmt_present**: HBRMT 패킷의 존재 여부를 확인하는 함수입니다.
- 패킷 분석: Ethernet, IP, UDP, RTP 헤더를 순차적으로 분석합니다.
- 프레임 정보 추출: 프레임 레이트, 프레임 크기 등의 정보를 추출합니다.
- 타이밍 분석: 패킷의 도착 시간과 예상 시간의 차이를 계산하고 출력합니다.
- YUV 데이터 추출: HBRMT 페이로드에서 YUV 데이터를 추출하여 파일로 저장합니다.
  
</br>

- **UYVY10BitToUV16Bit**: C++에는 10비트 자료형이 내장되어 있지 않으므로, 비손실 패킷 파싱을 위해, 함수를 통해 변환을 수행합니다.
- **GetAudioControlPacket**: `audioControl` 구조체에 audio control 패킷 raw data를 파싱합니다.
- **GetAudioDataPacket**: `audioDataPacket_s` 구조체에 audio data 패킷 raw data를 파싱합니다.
- **GetTimecodeDataPacket**: `aincillaryTimecodePacket_s` 구조체에 timecode data 패킷 (`uint16_t* puv`)과 부가 정보를 파싱합니다.


## 🎥: Demonstration
[시연 영상](https://youtu.be/lxQEEoyy_Og)

## 📞: Contact
- 이메일: hyeonwoody@gmail.com
- 블로그: https://velog.io/@hyeonwoody
- 깃헙: https://github.com/hyeonwoody

## 🧱: Technologies Used
>C++


## 📖: Libraries Used
>lipcap

## 📚: Referance
>ST-12-1</br>
>ST-12-2</br>
>ST-12-3</br>
>https://learn.microsoft.com/ko-kr/windows/win32/medfound/about-yuv-video
>https://learn.microsoft.com/ko-kr/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
