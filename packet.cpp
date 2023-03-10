
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define GRAPH 1
#if GRAPH
#endif

#include "packet.h"

#define YandUV_d 0
#define AudioDataPacket_d 0
#define EMPTY 0


#define AUDIOSAMPLE1 0 //PCM

#define AUDIOSAMPLE2 0 //aud  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define AUDIOSAMPLE22 1 // THIS

#define FRAME11252200 0
#define FRAME7501980 0
#define FRAME7501650 0
#define IFRAME11252200 0
#define FRAMEMANUAL 1
		

#define AUDIOSAMPLE3 0 // single channel
#define ADUIOSAMPLE33 0// single channel 32

#define AUDIOSAMPLE4 0 // channel by channel

#define AUDIODATA 0
#define AUDIOCONTROL 0
#define TIMECODEDATA 1

#define FINAL1 0
#define FINAL2 0
#define FINAL3 0 //24 bits => split by 8
#define FINAL33 0 
#define FINAL4 1

#define FRAMES 1
unsigned int frameIndex = 0;

		



audioControl GetAudioControl(uint16_t *p, size_t n)
{
	if (n != 0x0B)
	{
		return audioControl() = {0};
	}

	audioControl control = {0};

	int16_t w0 = *p++;
	int16_t w1 = *p++;
	int16_t w2 = *p++;

	control.AF        = w0 & 0x1FF;
	control.RATE.asx  = (w1 >> 0) & 0x01;
	control.RATE.code = (w1 >> 1) & 0x07;
	control.ACT[0]    = (w2 >> 0) & 0x01;
	control.ACT[1]    = (w2 >> 1) & 0x01;
	control.ACT[2]    = (w2 >> 2) & 0x01;
	control.ACT[3]    = (w2 >> 3) & 0x01;

	for (int i = 0; i < AUDIO_GROUP_NUM/2; i++)
	{
		int16_t d0 = 0x1FF & *p++;
		int16_t d1 = 0x1FF & *p++;
		int16_t d2 = 0x1FF & *p++;

		control.DEL[i].valid = d0 & 0x01;
		control.DEL[i].delay |= ((d0 >> 1) & 0xFF);
		control.DEL[i].delay |= (d1 << 8);
		control.DEL[i].delay |= ((d2 & 0xFF) << 17);
		control.DEL[i].delay |= (((d2 >> 8) & 0x01) << 31); // sign bit
	}

	control.RSRV[0] = 0x1FF & *p++;
	control.RSRV[1] = 0x1FF & *p++;

	return control;
}

audioGroup_s GetAudioGroup (uint16_t *p, size_t n){
	if (n != 0x18)
		return audioGroup_s() = {0};

	audioGroup_s group = {0};
	for (int i=0; i< AUDIO_GROUP_NUM; i++){
		int16_t w0 = 0xFF & *p++; // rest of 2 bits == parity check
		int16_t w1 = 0xFF & *p++;
		int16_t w2 = 0xFF & *p++;
		int16_t w3 = 0xFF & *p++;
		group.CH[i].Z = (w0 >> 3) & 0x01;

		group.CH[i].V = (w3 >> 4) & 0x01;
		group.CH[i].U = (w3 >> 5) & 0x01;
		group.CH[i].C = (w3 >> 6) & 0x01;
		group.CH[i].P = (w3 >> 7) & 0x01;
		

		int parity = Parity8 (w0 & 0xF0); //exclude preamble
		parity ^= Parity8(w1);
		parity ^= Parity8(w2);
		parity ^= Parity8(w3);

		group.CH[i].aud |= ((w0 & 0xF0) >> 4);
		group.CH[i].aud |= (w1 << 4);
		group.CH[i].aud |= (w2 << 12);
		group.CH[i].aud |= ((w3 & 0x0F) << 20);

		if (parity != 0){
			std::cerr << std::dec << "AES CH" << i+1 << ": parity bit error";
			std::cerr << std::endl;
		}
	}
	return group;
}

void AudioSampleFile1(){

	int32_t* audioOut;

	int size = audioIn_fifo.size();

	audioOut = (int32_t *) malloc (sizeof (int32_t)*size);

	for (int i =0; i<size; i++){
		int32_t tmp = audioIn_fifo.front();
		audioIn_fifo.pop();
		memcpy ( audioOut + i, &tmp, 3);
	}

	FILE *fp = fopen ("sampleFinal1.wav", "wb");
	//75000


	//fwrite(&ADP.PCM.CH[i].aud, 1, 3, fp2);
	//fclose(fp2);
	fwrite (audioOut, size, 3, fp);
	fclose (fp);

	free(audioOut);
}

void AudioSampleFile2(){
	int32_t* audioOut;

	int size = audioIn_fifo.size();

	audioOut = (int32_t *) malloc (sizeof(int32_t)*size);


	int i = 0;
	while (!audioIn_fifo.empty()){
		int32_t pop = audioIn_fifo.front();
		audioIn_fifo.pop();

		int32_t tmp0 = (pop >> 16) & 0xFF;
		int32_t tmp1 = (pop >> 8) & 0xFF;
		int32_t tmp2 = (pop) & 0xFF;

		int32_t real = tmp0 << 16;
		real |= tmp1 << 8;
		real |= tmp2;

		memcpy (&audioOut[i], &real, 3);

		i++;
	}	

	FILE *fp = fopen ("sampleFinal2.wav", "w");

	fwrite (audioOut, size, 3, fp);
	fclose(fp);

	free(audioOut);
}

void AudioSampleFile33 (){
	int8_t* audioOut;
	int32_t* audioOut11;

	int8_t tmp;
	int32_t pop32;
	
	int size = audioIn_fifo.size();

	audioOut = (int8_t *) malloc (sizeof(int8_t ) * 3 * size);
	audioOut11 = (int32_t *) malloc (sizeof(int32_t) * size);
	int i = 0;
	int j = 0;
	while (i < size){
		
		// pop = audioIn_fifo8.front();
		// audioIn_fifo8.pop();
		pop32 = audioIn_fifo.front();
		audioIn_fifo.pop();

		mempcpy (&audioOut11[i], &pop32, 1);

		tmp = (pop32 >> 16) & 0xFF;		
		memcpy (&audioOut[j++], &tmp, 1);
		tmp = (pop32 >> 8) & 0xFF;
		memcpy (&audioOut[j++], &tmp, 1);
		tmp = (pop32) & 0xFF;
		memcpy (&audioOut[j++], &tmp, 1);
		i++;
	}
	FILE *fp = fopen ("sampleFinal33.wav", "wb");
	fwrite (audioOut, size, 3, fp);
	fclose(fp);
	free(audioOut);

	// FILE *fp1 = fopen ("sampleFinal44.wav", "wb");
	// fwrite (audioOut11, size, 3, fp);
	// fclose(fp);
	// free(audioOut11);
}

void AudioSampleFile3 (){
	int8_t* audioOut;

	int size = audioIn_fifo.size();

	audioOut = (int8_t *) malloc (sizeof(int32_t) * size);

	int i = 0;
	int j = 0;

	while (i<size){
		int32_t pop = audioIn_fifo.front();
		audioIn_fifo.pop();

		int16_t tmp = (pop >> 16) & (int16_t) 0xFF;
		memcpy (&audioOut[j++], &tmp, 1);

		tmp = (pop >> 8) & 0xFF;
		memcpy (&audioOut[j++], &tmp, 1);

		tmp = (pop) & 0xFF;
		memcpy (&audioOut[j++], &tmp, 1);

		i++;
	}
	FILE *fp = fopen ("sampleFinal3.wav", "wb");
	fwrite (audioOut, size, 3, fp);
	fclose(fp);

	free(audioOut);
}
void AudioSampleFile4 (){
	uint8_t* audioOut;
	int size = audioIn_fifo8.size();
	audioOut = (uint8_t*)malloc (sizeof(uint8_t)*size);
	int i = 0;
	int j = 0;
	while (i<size){
		uint8_t pop = audioIn_fifo8.front();
		audioIn_fifo8.pop();
		memcpy (&audioOut[j++], &pop, 1);
		i++;
	}
	FILE *fp = fopen ("sampleFinal5.wav", "wb");
	fwrite (audioOut, size, 1, fp);
	fclose (fp);
	free(audioOut);
}
std::vector<double> createDomain (const double start, const double end, const double step);
std::vector<double> sin_func(const std::vector<double> domain);


std::vector<double> createDomain(const double start, const double end, const double step){
	std::vector<double> domain;
	for (double i=start; i<end; i+=step){
		domain.push_back(i);
	}
	return domain;
}

std::vector<double> sin_func(const std::vector<double> domain){
	std::vector <double> range;
	for (int i=0; i<domain.size(); ++i){
		range.push_back(domain[i]);
	}
	return range;
}





bool GetTimecodeDataPacket(uint16_t* puv){
	uint16_t w[23];
	for (int i=0; i<23; i++){
		w[i] = * (puv + i);
	}
	enum {
		LTC,
		VITC1,
		VITC2,
	};

	if ((w[3] == 0x260) &&
		(w[4] == 0x260) &&
		(w[5] == 0x110)){
		ancillaryTimecodePacket_s ATC;	
		ATC.ADF[0] = w[0];
		ATC.ADF[1] = w[1];
		ATC.ADF[2] = w[2];
		ATC.DID = w[3];
		ATC.SDID = w[4]; //study required
		ATC.DC = w[5]; //number of UDW

		for (int i=0; i<16; i++){
			ATC.UDW[i] = w[i+6];
		}
		ATC.CS = w[22];
#if TIMECODEDATA
		std::cout << std::dec << "$" << std::setfill('0') << std::setw(4) << ++tlines << " ";
		std::cout << std::hex << "ADF: " << "0x" << ATC.ADF[0] << ATC.ADF[1] << ATC.ADF[2] << " ";
		std::cout << std::hex << "DID: " << "0x" << ATC.DID << " ";

		if (((ATC.DID >> 7) & 0x1) == 0x1)
			std::cout << "Type 1" << " ";
		else //it shall be type 2
			std::cout << "Type 2" << " ";
		std::cout << std::hex << "SDID: " << "0x" << ATC.SDID << " ";
		std::cout << std::hex << " DC: " << "0x" << ATC.DC << " ";
		std::cout << std::hex << " CS: " << "0x" << ATC.CS << " ";
		std::cout << std::endl;



		bool DropFrameFlag = 0;
		bool ColorFrameFlag = 0;

		bool binaryGroupFlag0 = 0;
		bool binaryGroupFlag1 = 0;
		bool binaryGroupFlag2  = 0;
		bool polarityCorrection = 0;
		bool fieldIdentification = 0;

		int8_t tenHour = 0;
		int8_t unitHour = 0;

		int8_t tenMinute = 0;
		int8_t unitMinute = 0;

		int8_t tenSecond = 0;
		int8_t unitSecond = 0;

		int8_t tenFrame = 0;
		int8_t unitFrame = 0;

		int8_t binaryGroup1 = 0;
		int8_t binaryGroup2 = 0;
		int8_t binaryGroup3 = 0;
		int8_t binaryGroup4 = 0;
		int8_t binaryGroup5 = 0;
		int8_t binaryGroup6 = 0; 
		int8_t binaryGroup7 = 0; 
		int8_t binaryGroup8 = 0; 
		int8_t binaryGroupFlag = 0;

		


		int8_t DBB1 = 0;

		for (int i = 0; i<8; i++){
			DBB1 |= ((ATC.UDW[i] >> 3) & 0x1) << i;
		}
		
		switch (DBB1){
			case LTC:
				std::cout << "Time Code Type : LTC" << std::endl;
				polarityCorrection = (ATC.UDW[6] >> 6) & 0x1;
				break;
			case VITC1:
				std::cout << "Time Code Type : VITC1" << std::endl;
				fieldIdentification = (ATC.UDW[6] >> 6) & 0x1;
				break;
			case VITC2:
				std::cout << "Time Code Type : VITC2" << std::endl;
				fieldIdentification = (ATC.UDW[6] >> 6) & 0x1;
				break;
		}

		tenHour = (ATC.UDW[14] >> 4) & 0x3;
		unitHour = (ATC.UDW[12] >> 4) &0xF;

		tenMinute = (ATC.UDW[10] >> 4) & 0x7;
		unitMinute = (ATC.UDW[8] >> 4) & 0xF;

		tenSecond = (ATC.UDW[6] >> 4) & 0x7;
		unitSecond = (ATC.UDW[4] >> 4) & 0xF;

		tenFrame = (ATC.UDW[2] >> 4) & 0x2;
		unitFrame = (ATC.UDW[0] >> 4) & 0xF;

		DropFrameFlag = (ATC.UDW[2] >> 6) & 0x1;
		ColorFrameFlag = (ATC.UDW[2] >> 7) & 0x1;
		

		
		binaryGroup1 = (ATC.UDW[1] >> 4) & 0xF;
		binaryGroup2 = (ATC.UDW[3] >> 4) & 0xF;
		binaryGroup3 = (ATC.UDW[5] >> 4) & 0xF;
		binaryGroup4 = (ATC.UDW[7] >> 4) & 0xF;
		binaryGroup5 = (ATC.UDW[9] >> 4) & 0xF;
		binaryGroup6 = (ATC.UDW[11] >> 4) & 0xF;
		binaryGroup7 = (ATC.UDW[13] >> 4) & 0xF;
		binaryGroup8 = (ATC.UDW[15] >> 4) & 0xF;

		binaryGroupFlag0 = (ATC.UDW[10] >> 7) & 0x1;
		binaryGroupFlag1 = (ATC.UDW[14] >> 6) & 0x1;
		binaryGroupFlag2 = (ATC.UDW[14] >> 7) & 0x1;


		binaryGroupFlag = (binaryGroupFlag2 << 2) | (binaryGroupFlag1 << 1) | (binaryGroupFlag0);

		if ((w[3] & 0xFF) == 0x80){
			std::cout <<"mark Deletion"<<std::endl;
			return true;
		}
		std::cout << "Time Code : ";
		std::cout << static_cast<int16_t> (tenHour) << static_cast <int16_t> (unitHour) << ":";
		std::cout << static_cast<int16_t> (tenMinute) << static_cast <int16_t> (unitMinute) << ":";
		std::cout << static_cast<int16_t> (tenSecond) << static_cast<int16_t> (unitSecond) << ":";
		std::cout << static_cast<int16_t> (tenFrame) << static_cast<int16_t> (unitFrame)<< std::endl;

		switch (binaryGroupFlag)
		{
		case 0:
			break;
		case 1:
			std::cout << "Binary Group : 001" <<std::endl;
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			std::cout << "Binary Group : Unspecified" << std::endl;
			std::cout << "Date and Time Zone :: " << std::endl;
			std::cout << "Day : " << static_cast<int16_t> (binaryGroup2) << static_cast <int16_t> (binaryGroup1) << std::endl;
			std::cout << "Month : " << static_cast<int16_t> (binaryGroup4) << static_cast <int16_t> (binaryGroup3) << std::endl;
			std::cout << "Year : "<< static_cast<int16_t> (binaryGroup6) << static_cast <int16_t> (binaryGroup5) << std::endl;
			break;
		case 5:
			break;
		case 6:
			std::cout << "Binary Group : Clock" << std::endl;
			std::cout << "Date and Time Zone :: " << std::endl;
			std::cout << "Day : " << static_cast<int16_t> (binaryGroup2) << static_cast <int16_t> (binaryGroup1) << std::endl;
			std::cout << "Month : " << static_cast<int16_t> (binaryGroup4) << static_cast <int16_t> (binaryGroup3) << std::endl;
			std::cout << "Year : "<< static_cast<int16_t> (binaryGroup6) << static_cast <int16_t> (binaryGroup5) << std::endl;
			break;
		case 7:
			break;
		default:
			break;
		}



		


#endif
		return true;
	}
	else {
		return false;
	}
}

bool GetAudioControlPacket (uint16_t* py){
	uint16_t w[18];
	for (int i= 0; i<18; i++){
		w[i] = * (py + i);
	}


	if ((w[3] == group0controlDID || w[3] == group1controlDID || w[3] == group2controlDID || w[3] == group3controlDID)&&
		(w[4] == 0x200) &&
		(w[5] == 0x10B)){ // 11 (UDW0 - UDW10)
		audioControlPacket_s ACP;
		ACP.ADF[0] = w[0];
		ACP.ADF[1] = w[1]; 
		ACP.ADF[2] = w[2];
		ACP.DID = w[3];
		ACP.DBN = w[4]; //study required
		ACP.DC = w[5]; //number of UDW

		ACP.AF = w[6];
		ACP.RATE.asx = (w[7] >> 0) & 0x01;
		ACP.RATE.code = (w[7] >> 1) & 0x07;
		ACP.ACT[0] = (w[8] >> 0) & 0x01;
		ACP.ACT[1] = (w[8] >> 1) & 0x01;
		ACP.ACT[2] = (w[8] >> 2) & 0x01;
		ACP.ACT[3] = (w[8] >> 3) & 0x01;

		uint16_t* p =(py + 9);
		for (int i = 0; i<AUDIO_GROUP_NUM/2; i++){
			int16_t d0 = 0x1FF & *p++;
			int16_t d1 = 0x1FF & *p++;
			int16_t d2 = 0x1FF & *p++;

			ACP.DEL[i].valid = d0 & 0x01;
			ACP.DEL[i].delay |= ((d0 >> 1) & 0xFF);
			ACP.DEL[i].delay |= (d1 << 8);
			ACP.DEL[i].delay |= ((d2 & 0xFF) << 17);
			ACP.DEL[i].delay |= (((d2 >>8) & 0x01) << 31); //sign bit
		}

		ACP.RSRV[0] = 0x1FF & w[15];
		ACP.RSRV[1] = 0x1FF & w[16];

		ACP.CS = w[17];
#if AUDIOCONTROL 
		std::cout << std::dec << "%" << std::setfill('0') << std::setw(4) << ++clines << " ";
		std::cout << std::hex << "ADF: " << "0x" << ACP.ADF[0] << ACP.ADF[1] << ACP.ADF[2] << " ";
		std::cout << std::hex << "DID: " << "0x" << ACP.DID << " ";
		std::cout << std::hex << "DBN: " << "0x" << ACP.DBN << " ";
		std::cout << std::hex << " DC: " << "0x" << ACP.DC << " ";
		std::cout << std::hex << " CS: " << "0x" << ACP.CS << " ";
		std::cout << std::endl;


		if ((w[3] & 0xFF) == 0x80){
			std::cout <<"mark for Deletion"<<std::endl;
			return true;
		}
		if (!evenParity(ACP.DBN)){
			bool a = false;
			std::cerr << "DBN: even parity bit error"<<std::endl;
		}
		if (!parityCheck(ACP.DBN)){
			std::cerr << "DBN: parity bit error"<<std::endl;
		}

		if (!parityCheck(ACP.DC)){
			std::cerr << "DC: parity bit error"<<std::endl;
		}
		if ((ACP.DC & 0xFF) != 0x0B){
			std::cerr << std::dec << "DC: invalid UDW count: "<< (ACP.DC & 0xFF)<<std::endl;
		}

		//CS check
		int32_t b08 = (ACP.CS & 0x1FF); 
		if (!parityCheck(ACP.CS)){
			std::cerr << "CS: parity bit error"<<std::endl;
		}

		int cs = 0;
		for (int i = 3; i < 17; i++ ){
			cs += (w[i] & 0x1FF);
		}

		if (b08 != (cs & 0x1FF)){
			std::cout <<std::hex << "CS: check sum error: "<<"0x"<<b08;
			std::cout <<std::endl;
			celines++;
		}

		// //UDW check
		// for (int i = 6; i < 17; i++){
		// 	uint8_t b8 = (w[i] >> 8) & 0x01;
		// 	uint8_t b9 = (w[i] >> 9);
		// 	if (b8 == b9)
		// 		std::cerr << "UDW: parity bit error"<<std::endl;
		// 	if (i == 8 || i ==12 || i == 16){
		// 		if (w[i] & 0x01 != 0x00 ||
		// 			w[i] >> 1 & 0x01 != 0x00 ||
		// 			w[i] >> 2 & 0x01 != 0x00
		// 			)
		// 			std::cerr << "UDW" <<int(i) << ": Bits b0 through b2 are not zero"<<std::endl;
		// 		if (i == 12) 
		// 			if (w[i] >> 3 & 0x01 != 0x00)
		// 				std::cerr << "UDW" << int(i) << ": Bit b3 is not zero"<<std::endl;
		// 	}
		// }
#endif
		return true;
	}
	else {
		return false;
	}
}

bool GetAudioDataPacket(uint16_t* puv){
	uint16_t w[31];
	for (int i=0; i<31; i++){
		w[i] = * (puv + i);
	}
	if ((w[3] == group0dataDID || w[3] == group1dataDID || w[3] == group2dataDID || w[3] == group3dataDID) &&
		(w[5] == 0x218)){
		audioDataPacket_s ADP;
		ADP.ADF[0] = w[0];
		ADP.ADF[1] = w[1];
		ADP.ADF[2] = w[2];
		ADP.DID = w[3];
		ADP.DBN = w[4]; //study required
		ADP.DC = w[5]; //number of UDW

		// ADP.UDW.CLK[0] = w[6];
		// ADP.UDW.CLK[1] = w[7];

		// ADP.UDW.ECC[0] = w[24];
		// ADP.UDW.ECC[1] = w[25];
		// ADP.UDW.ECC[2] = w[26];
		// ADP.UDW.ECC[3] = w[27];
		// ADP.UDW.ECC[4] = w[28];
		// ADP.UDW.ECC[5] = w[29];

		ADP.CS = w[30];
#if AUDIODATA
		std::cout << std::dec << "#" << std::setfill('0') << std::setw(4) << ++slines << " ";
		std::cout << std::hex << "ADF: " << "0x" << ADP.ADF[0] << ADP.ADF[1] << ADP.ADF[2] << " ";
		std::cout << std::hex << "DID: " << "0x" << ADP.DID << " ";
		if ((ADP.DID >> 7) && 0x1)
			std::cout << "Type 1" << " ";
		else 
			std::cout << "Type 2" << " ";
		std::cout << std::hex << "DBN: " << "0x" << ADP.DBN << " ";
		std::cout << std::hex << " DC: " << "0x" << ADP.DC << " ";
		std::cout << std::hex << " CS: " << "0x" << ADP.CS << " ";
		
		std::cout << std::endl;

		if ((w[3] & 0xFF) == 0x80){
			std::cout <<"mark for Deletion"<<std::endl;
			return true;
		}
		uint16_t clock = 0;
		clock = (w[7] & 0x3F) << 8 | (w[6] & 0xFF);
		std::cout << std::dec<<" CLOCK: " << clock << " ";
		if (((w[7] >> 4) & 0x01) == 0x01){
			std::cout << "mpf set" <<std::endl;
		}

		//DBN
		if (!evenParity(ADP.DBN)){
			std::cerr << "DBN: even parity bit error"<<std::endl;
		}
		if (!parityCheck(ADP.DBN)){
			std::cerr << "DBN: parity bit error"<<std::endl;
		}
		
		if (!evenParity(ADP.DID)){
			std::cerr << "DID: even parity bit error"<<std::endl;
		}
		uint8_t b07 = ADP.DBN & 0xFF;
		if (prevDBN[ADP.DID] != -1 && b07 != (prevDBN[ADP.DID] + 1)){
			std::cerr << "DBN: discontinuity occurs"<<std::endl;
		}

		if (b07 == 0xFF){
			prevDBN[ADP.DID] = 0;
		}
		else {
			prevDBN[ADP.DID] = b07;	
		}

		//DC (DC == 0x218)
		if (!evenParity(ADP.DC)){
			std::cerr <<"DC: even parity bit error"<<std::endl;
		}
		if (!parityCheck(ADP.DC)){
			std::cerr <<"DC: parity bit error"<<std::endl;
		}
		if ( (b07=ADP.DC & 0xFF) != 0x18)/*b07*/{
			std::cerr << std::dec<<"DC invalid UDW count"<<b07;
			std::cerr << std::endl;
		}

		//CS check
		int32_t b08 = (ADP.CS & 0x1FF); 
		if (!parityCheck(ADP.CS)){
			std::cerr << "CS: parity bit error"<<std::endl;
		}

		int cs = 0;
		for (int i = 3; i < 30; i++ ){
			cs += (*(puv + i) & 0x1FF);
		}

		if (b08 != (cs & 0x1FF)){
			std::cerr <<std::hex << "CS: check sum error: "<<"0x"<<b08;
			std::cerr <<std::endl;
		}

		//UDW check
		for (int i = 6; i < 30; i++){
			uint8_t b8 = (w[i] >> 8) & 0x01;
			uint8_t b9 = (w[i] >> 9);
			if (b8 == b9)
				std::cerr << "UDW: parity bit error"<<std::endl;
			if (i == 8 || i ==12 || i == 16 || i == 20){
				if (w[i] & 0x01 != 0x00 ||
					w[i] >> 1 & 0x01 != 0x00 ||
					w[i] >> 2 & 0x01 != 0x00
					)
					std::cerr << "UDW" <<int(i) << ": Bits b0 through b2 are not zero"<<std::endl;
				if (i == 12 || i == 20) 
					if (w[i] >> 3 & 0x01 != 0x00)
						std::cerr << "UDW" << int(i) << ": Bit b3 is not zero"<<std::endl;
			}
		}

		//ECC check
		for (int i=24; i<30; i++){
			if (!parityCheck(w[i]))
				std::cerr << "ECC: parity bit error"<<std::endl;
		}

		
		
			
#endif
		ADP.PCM = GetAudioGroup (puv + 8, (ADP.DC & 0xFF));
		for (int i=0; i< AUDIO_UDW_NUM; i++){
			#if AUDIODATA
			numAudioSample++;
			std::cout 	<< (i%2? "0:" : "Z:") << int (ADP.PCM.CH[i].Z)
						<< ", P:" << int(ADP.PCM.CH[i].P) 
						<< ", C:" << int(ADP.PCM.CH[i].C)
						<< ", U:" << int(ADP.PCM.CH[i].U)
						<< ", V:" << int(ADP.PCM.CH[i].V)
						<< std::endl;
			std::cout << std::hex << "AUD: " << "0x" << ADP.PCM.CH[i].aud<<std::endl;
			#endif
			#if AUDIOSAMPLE1
					FILE * fp1;
					if ((access("sample1.wav", 0) != -1)){
						fp1 = fopen ("sample1.wav", "ab");
					}
					else {
						fp1 = fopen ("sample1.wav", "ab");
					}
					for (int i=0; i<4; i++){
						fwrite(&ADP.PCM.CH[i].aud, 1, 3, fp1);
						audioIn_fifo.push (ADP.PCM.CH[i].aud);
					}
					fclose(fp1);
			#endif
			#if AUDIOSAMPLE11
				FILE * fp11;
				if ((access("sample11.wav", 0) != -1)){
					fp11 = fopen ("sample11.wav", "ab");
				}
				else {
					fp11 = fopen ("sample11.wav"m "ab");
				}
				fwrite (&ADP.PCM.CH[0].aud, 1, 3, fp1);
			#endif
			#if AUDIOSAMPLE2
							FILE * fp2;
							fp2 = fopen ("sample2.wav", "ab");
							// audioIn_fifo.push (ADP.PCM.CH[i].aud);
							//fwrite(&ADP.PCM.CH[i].aud, 1, 3, fp2);
							fwrite(&ADP.PCM.CH[i].aud, 3, 1, fp2);
							fclose(fp2);
			#endif
				
			#if AUDIOSAMPLE22
					// FILE * fp3;
					//fp3 = fopen ("sample22.wav", "ab");
					audioIn_fifo.push (ADP.PCM.CH[i].aud);
					int8_t tmp;
					tmp = (ADP.PCM.CH[i].aud >> 16) & 0xFF;
					audioIn_fifo8.push (tmp);

					tmp = (ADP.PCM.CH[i].aud >> 8) & 0xFF;
					audioIn_fifo8.push (tmp);

					tmp = (ADP.PCM.CH[i].aud) & 0xFF;
					audioIn_fifo8.push (tmp);
					//fwrite(&ADP.PCM.CH[i].aud, 1, 3, fp2);
					//fwrite(&ADP.PCM.CH[i].aud, 3, 1, fp3);
					//fclose(fp3);
			#endif
			
			#if AUDIOSAMPLE3
				FILE * fp3;
				if ((access("sample3.wav", 0) != -1)){
					fp3 = fopen ("sample3.wav", "ab");
				}
				else {
					fp3 = fopen ("sample3.wav", "ab");
				}
				fwrite(&ADP.PCM.CH[0].aud, 1, 3, fp3);
				fclose(fp3);
			#endif
			#if AUDIOSAMPLE33
				FILE *fp33;
				if ((access("sample3.wav", 0) != -1)){
					fp33 = fopen ("sample33.wav", "ab");
				}
				else {
					fp33 = fopen ("sample33.wav", "ab");
				}
				int32_t test = 0;
				fwrite (&ADP.PCM.CH[0].aud, 1, 3, fp33);
			#endif
		}
		return true;
	}
	else if ((w[3] & 0xFF) == 0x80){
		std::cout << "Mark for Deletion"<<std::endl;
	}
	else {
		return false;
	}
}

bool PacketAnalyze(YUV yuv){

	bool isEndofFrame = false;
	int height = yuv.height;
	int width = yuv.width;

	for (int y = 0; y<height; y++){
		uint16_t *puv = (uint16_t *)(yuv.UV + y * width *2);
		bool empty = true;

		for (int x = 0; x < width;){
			if ((*(puv) == 0x0000 && *(puv + 1) == 0x03ff && *(puv + 2) == 0x03ff)){
					howManyPacket[y]++;
				if (GetAudioDataPacket (puv)){
#if AUDIODATA
					std::cout << std::dec << "(frame, line, offset) = (" << std::setfill(' ') << std::setw(4) <<frameIndex <<"," << y << "," << std::setw(4) << x << ")"<<std::endl;
					std::cout<<""<<std::endl;
#endif
					puv += AUDIO_PACKET_DATA_WORD_LENGTH;
					x += AUDIO_PACKET_DATA_WORD_LENGTH;
					empty = false;
				}
				else {
					puv++;
					x++;
					--howManyPacket[y];
				}
			}
			else {

				puv++;
				x++;
			}
		}

		uint16_t *py = (uint16_t *)(yuv.Y + y * width *2);
		for (int x = 0; x < width;){
			if (((*(py) == 0x0000) && (*(py + 1) == 0x03ff) && (*(py + 2) == 0x03ff))){
				if (GetAudioControlPacket(py)){
#if AUDIOCONTROL
					std::cout << std::dec << "(frame, line, offset) = (" << std::setfill(' ') << std::setw(4) <<frameIndex <<"," << y << "," << std::setw(4) << x << ")"<<std::endl;
					std::cout<<""<<std::endl;
#endif
					empty = false;
					py += AUDIO_CONTROL_PACKET_DATA_WORD_LENGTH;
					x += AUDIO_CONTROL_PACKET_DATA_WORD_LENGTH;
				}
				if (GetTimecodeDataPacket(py)){
#if TIMECODEDATA
					std::cout << std::dec << "(frame, line, offset) = (" << std::setfill(' ') << std::setw(4) <<frameIndex <<"," << y << "," << std::setw(4) << x << ")"<<std::endl;
					std::cout<<""<<std::endl;
#endif
					py += TIMECODE_PACKET_DATA_WORD_LENGTH;
					x += TIMECODE_PACKET_DATA_WORD_LENGTH;
					empty = false;
				}
				else {
					py++;
					x++;
				}
			}
			else {
				py++;
				x++;
			}
		}
#if EMPTY
		if (empty){
			std::cout<< "empty line : "<<y<<std::endl;
		}
#endif
	}
	isEndofFrame = true;
	return isEndofFrame;
}	

void OnlyHANC(uint8_t* p, YUV yuv){

	int width = yuv.width;
	int height = yuv.height;

	int qsize = width * height  * 2; // 16bit devided by 8byte = 2;
	
	int rowBytes = width * 2 * 10 / 8;

	for (int y=0; y<height; y++){
		uint8_t* ps = p + y * rowBytes;
		uint16_t* py = (uint16_t*)(yuv.Y + y * width * 2);
		uint16_t* puv = (uint16_t*)(yuv.UV + y * width * 2);

		for (int x=0; x<width; x+=2){
			uint8_t b0 = *ps++;
			uint8_t b1 = *ps++;
			uint8_t b2 = *ps++;
			uint8_t b3 = *ps++;
			uint8_t b4 = *ps++;

			*py++ = ((b1&0x3F) << 4) | (b2 >> 4);
			*py++ = ((b3&0x03) << 8) | b4;
			*puv++ = (b0 << 2) | (b1 >> 6);
			*puv++ = ((b2&0x0F) << 6) | (b3 >> 2);
		}
	}
}

void UYVY10BitToUV16Bit(uint8_t* p, YUV yuv){

	int width = yuv.width;
	int height = yuv.height;

	int qsize = width * height  * 2; // 16bit devided by 8byte = 2;
	
	int rowBytes = width * 2 * 10 / 8;

	for (int y=0; y<height; y++){
		uint8_t* ps = p + y * rowBytes;
		uint16_t* py = (uint16_t*)(yuv.Y + y * width * 2);
		uint16_t* puv = (uint16_t*)(yuv.UV + y * width * 2);

		for (int x=0; x<width; x+=2){
			uint8_t b0 = *ps++;
			uint8_t b1 = *ps++;
			uint8_t b2 = *ps++;
			uint8_t b3 = *ps++;
			uint8_t b4 = *ps++;

			*py++ = ((b1&0x3F) << 4) | (b2 >> 4);
			*py++ = ((b3&0x03) << 8) | b4;
			*puv++ = (b0 << 2) | (b1 >> 6);
			*puv++ = ((b2&0x0F) << 6) | (b3 >> 2);
		}
	}
}

int main (int argc, char **argv){ //UYVYfile videoSize
	FILE* sFile;
	string path ("./");
	YUV yuv;
	long int frameNumber =1;
	if (argc < 3){
	#if FRAME7501980
		sFile = fopen ("/root/work/YUVPacket/UYVY_leo/frame_750p50_1980.yuv", "rb");
		yuv.width = HORIZONTAL_SAMPLES_HD720_50;
		yuv.height = VERTICAL_LINES_HD720;
	#endif

	
	#if FRAME7501650
		sFile = fopen ("/root/work/YUVPacket/UYVY_leo/frame_750p60_1650.yuv", "rb");
		yuv.width = HORIZONTAL_SAMPLES_HD720_50;
		yuv.height = VERTICAL_LINES_HD720;
	#endif
		
	#if FRAME11252200
		sFile = fopen ("/root/work/YUVPacket/UYVY_leo/frame_1125p30_2200.yuv", "rb");
		yuv.width = HORIZONTAL_SAMPLES_HD1080_60;
		yuv.height = VERTICAL_LINES_HD1080;
	#endif 

	#if FRAME750591650
		sFile = fopen ("/root/work/YUVPacket/UYVY_leo/frame_750p59o94_1650.yuv", "rb");
		yuv.width = HORIZONTAL_SAMPLES_HD720_60;
		yuv.height = VERTICAL_LINES_HD720;
	#endif

	#if IFRAME11252200
		sFile = fopen ("/root/work/YUVPacket/UYVY_leo/frame_1125i30_2200.yuv", "rb");
		yuv.width = HORIZONTAL_SAMPLES_HD1080_60;
		yuv.height = VERTICAL_LINES_HD1080;
	#endif
	#if FRAMEMANUAL
		string fileName = "ST-104/lifeTime/ST2022_6_frame_858x525i@29.97.yuv";
		sFile = fopen (("/zzz/Evertz/"+fileName).c_str(), "rb");
		//sFile = fopen ("/root/work/YUVPacket/frame_1125p30_2200.yuv", "rb");
		yuv.width = 858;
		yuv.height = 525;
	#endif
	printf("Usage:%s input width\n", argv[0]);
	printf((fileName+"\n").c_str());
	}
	else {
		path+=argv[1];
		if ((sFile = fopen (path.c_str(), "rb")) == NULL)
			cout<<"Could not open data file\n";
		else {
			switch (atoi(argv[2])){
				case HORIZONTAL_SAMPLES_HD1080_48:
					yuv.width = HORIZONTAL_SAMPLES_HD1080_48;
					yuv.height = VERTICAL_LINES_HD1080;
					break;
				case HORIZONTAL_SAMPLES_HD1080_50:
					yuv.width = HORIZONTAL_SAMPLES_HD1080_50;
					yuv.height = VERTICAL_LINES_HD1080;
					break;
				case HORIZONTAL_SAMPLES_HD1080_60:
					yuv.width = HORIZONTAL_SAMPLES_HD1080_60;
					yuv.height = VERTICAL_LINES_HD1080;
					break;
				case HORIZONTAL_SAMPLES_HD720_50:
					yuv.width = HORIZONTAL_SAMPLES_HD720_50;
					yuv.height = VERTICAL_LINES_HD720;
					break;
				case HORIZONTAL_SAMPLES_HD720_60:
					yuv.width = HORIZONTAL_SAMPLES_HD720_60;
					yuv.height = VERTICAL_LINES_HD720;
					break;
				case HORIZONTAL_SAMPLES_PAL:
					yuv.width = HORIZONTAL_SAMPLES_PAL;
					yuv.height = VERTICAL_LINES_PAL;
					break;
				case HORIZONTAL_SAMPLES_NTSC:
					yuv.width = HORIZONTAL_SAMPLES_NTSC;
					yuv.height = VERTICAL_LINES_NTSC;
					break;
				default:
					break;
			}
		}
	}
	
	long int frameSize = yuv.width * yuv.height * 2 * 10 / 8;
	uint8_t *frame = NULL;
	
	#if FRAMES
    // fseek(sFile, 0, SEEK_END); 
	// long int fileSize = 0;
    // fileSize = ftell(sFile);

	// frameNumber = fileSize / frameSize;

	frame = (uint8_t*) malloc (frameSize);
	//uint8_t* pFrame = frame;
	// fseek(sFile,0,SEEK_SET);

	#elif
	frame = (uint8_t*) malloc (frameSize);
	#endif
	frameNumber =60;
	int ret = 0;
	
	remove("sample2.wav");
	remove("sample22.wav");
	remove("sampleFinal5.wav");
	long int qsize = yuv.width * yuv.height  * 2; // 16bit devided by 8byte = 2;
	yuv.Y = (uint8_t*) malloc (qsize);
	yuv.UV = (uint8_t*) malloc (qsize);
	for (int i=0; i<frameNumber; i++){
		if ((ret = fread(frame, frameSize, 1, sFile)) && ret != 0){
			UYVY10BitToUV16Bit(frame, yuv);
			PacketAnalyze(yuv);
			frameIndex++;
		}
        
		/*
		AudioSample genuine first 24 20 16 practical
		*/

		
	}
	fclose(sFile);

		#if FINAL4
		AudioSampleFile4();
		#endif
	
	printResult(yuv.height);
	free(frame);
	free(yuv.Y);
	free(yuv.UV);
    return 0;
}