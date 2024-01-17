# YUVParser
yuv파일(422, UYVY, 10bit)에서 AudioData, AudioControl & Timecode 정보를 추출하는 CLI 기반 소스코드.

</br>

## 🧑‍💻: Intro
yuv파일(422, UYVY, 10bit)에서 AudioData, AudioControl & Timecode 정보를 추출하는 CLI 기반 소스코드.
AudioControl은 오디오의 볼륨, 채널 설정, 음소거 설정을 담고 있으며, '%' 입니다.
AudioData는 인코딩 형식에 따라 다르며 PCM 오디오 정보이며, 태그는 '#' 입니다.
Timecode는 각 프레임 마다 존재하므로 프레임의 개수만큼 나오고 태그는 '$' 입니다.
>
</br>

## ✅: Implementation 
- **UYVY10BitToUV16Bit**: C++에는 10비트 자료형이 내장되어 있지 않으므로, 비손실 패킷 파싱을 위해, 함수를 통해 변환을 수행합니다.
- **GetAudioControlPacket**: `audioControl` 구조체에 audio control 패킷 raw data를 파싱합니다.
- **GetAudioDataPacket**: `audioDataPacket_s` 구조체에 audio data 패킷 raw data를 파싱합니다.
- **GetTimecodeDataPacket**: `aincillaryTimecodePacket_s` 구조체에 timecode data 패킷 (`uint16_t* puv`)과 부가 정보를 파싱합니다.
</br>

## 🎥: Demonstration
[http://3.37.53.164:51730/BubblePop](https://youtu.be/lxQEEoyy_Og)

</br>

## 📞: Contact
- 이메일: hyeonwoody@gmail.com
- 블로그: https://velog.io/@hyeonwoody
- 깃헙: https://github.com/hyeonwoody

</br>

## 🧱: Technologies Used
>C++

## 📖: Libraries Used

</br>

## 📚: Referance
>ST-12-1</br>
>ST-12-2</br>
>ST-12-3</br>
>https://learn.microsoft.com/ko-kr/windows/win32/medfound/about-yuv-video
>https://learn.microsoft.com/ko-kr/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
