// The PushApp is a sample app for the concept of a personal trainer. The App will count the number of push-ups performed, but more importantly will
// announce "No Rep", for a poorly performed push-up (when the chest doesn't go low enough to count as a push-up).
// At the beginning of the workout, you need to choose the level of difficulty (how deep you need to go during a push-up for it to count as a rep)
// After that, just follow the spoken orders

#include "stdafx.h"
#include "WalabotAPI.h"
#include <assert.h>
#include <iostream>
#include <conio.h>
#define WINVER 0x0500
#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <MMSystem.h>
#pragma comment(lib, "Winmm.lib")
using namespace std;

double gPosMax = 0.0;     // Upmost position value (maximum distance recorded by the device at the beginning of the workout)
double gPosMin = 0.0;     // Minimal position value (This value will be updated continuasly)
double gDeltaRep = 0.0;   // The difference between the upmost position value and the downmost position value, that will count as a rep
double gDeltaRest = 0.0;  // The amount of change between upmost position value and current position value that is associated with resting (Change due to breathing or fidgeting)
double gLastDist = 0.0;   // The last recorded distance value
int gRepNum = 0;         // Number of reps performed

double gDeltaUP = 0.0;   // The total distance moved up
double gDeltaDN = 0.0;  // The total distance moved down

int flag = 0;           // Flag to mark if the rep was completed ( User is back to his upmost position)

// These values determine how deep you need to go during a push-up ,as a percentage  of your upmost position, for it to count as a rep
double gBeginnerRep = 0.3;
double gMediumRep = 0.45;
double gAdvancedRep = 0.7;
double gChosenLevel = 0.0;

// Let user choose workout difficulty level
void GetLevel() {
	printf("Choose level of difficulty:\n");
	printf("Type 'b' for Beginner\n");
	printf("Type 'm' for Medium\n");
	printf("Type 'a' for Advanced\n");
	printf("And aftrwards press Enter\n");
	char Level;
	scanf_s(" %c", &Level);
	switch (Level)
	{
	case 'b':
		gChosenLevel = gBeginnerRep;
		break;
	case 'm':
		gChosenLevel = gMediumRep;
		break;
	case 'a':
		gChosenLevel = gAdvancedRep;
		break;
	default:
		printf("Character doesn't match any option. Type desired level again\n\n");
		GetLevel();
	}
}

// Announce "No Rep"
void NoRep() {
	PlaySoundA("sounds/NO_REP.wav", NULL, SND_FILENAME | SND_ASYNC);
	Sleep(900);
}

// Say the number of reps
void CountOutLoud(int counter) {
	if (counter>25)
	{
		return;
	}
	else
	{
		string fileName = "sounds/say_" + to_string(counter) + ".wav";
		LPCSTR fileNameNew = fileName.c_str();
		PlaySoundA(fileNameNew, NULL, SND_FILENAME | SND_ASYNC);
		Sleep(100);
	}
}

// Get  distance of closest target (Only considering the y and z coordinates)
double GetTargetDistOfClosestTarget(SensorTarget *targets, int numTargets) {
	vector<int> ValidIndexes = vector<int>(numTargets);
	int counterTemp = 0;
	for (int k = 0; k<numTargets; k++)
	{
		if (!(isnan(targets[k].zPosCm)) && !(isinf(targets[k].zPosCm)) && !(isnan(targets[k].yPosCm)) && !(isinf(targets[k].yPosCm)))
		{
			ValidIndexes[counterTemp] = k;
			counterTemp++;
		}
	}
	if (counterTemp>0)
	{
		double SizeOfClosestTargetYZ = sqrt(pow(targets[ValidIndexes[0]].yPosCm, 2) + pow(targets[ValidIndexes[0]].zPosCm, 2));
		for (int k = 1; k < counterTemp; k++)
		{
			double SizeOfTargetYZ = sqrt(pow(targets[ValidIndexes[k]].zPosCm, 2) + pow(targets[ValidIndexes[k]].yPosCm, 2));
			if (SizeOfTargetYZ< SizeOfClosestTargetYZ)
			{
				SizeOfClosestTargetYZ = SizeOfTargetYZ;
			}
		}
		return SizeOfClosestTargetYZ;
	}
	else
	{
		return 6017; // If there wasn't a true target detected
	}
}

// Get target pos and calculate its distance from the device (Considering y and z coordinates only)
double GetTargetDist() {
	WALABOT_RESULT res;
	SensorTarget *targets;
	int numTargets;
	//  Trigger: Scan(sense) according to profile and record signals to be
	//  available for processing and retrieval.
	//  ====================================================================
	res = Walabot_Trigger();
	assert(res == WALABOT_SUCCESS);
	//   Get action : retrieve the last completed triggered recording
	//  ================================================================
	res = Walabot_GetSensorTargets(&targets, &numTargets);
	assert(res == WALABOT_SUCCESS);
	if (numTargets > 1)
	{
		double distTemp = GetTargetDistOfClosestTarget(targets, numTargets);
		if (distTemp == 6017)
		{
			GetTargetDist();
		}
		else
		{
			return distTemp;
		}
	}
	else if (numTargets == 1)
	{
		if (!(isnan(targets->zPosCm)) && !(isinf(targets->zPosCm)) && !(isnan(targets->yPosCm)) && !(isinf(targets->yPosCm)))
		{
			double Ztemp = targets->zPosCm;
			double Ytemp = targets->yPosCm;
			return sqrt(pow(Ztemp, 2) + pow(Ytemp, 2));
		}
		else
		{
			GetTargetDist();
		}
	}
	else
	{
		GetTargetDist();
	}
}

// For calibration purposes- Get the target distance at the upmost position
void GetTargetPosMax() {
	double tempMaxDist = 0.0;
	int counter = 30;
	for (int k = 0; k < counter; k++)
	{
		double PosTemp = GetTargetDist();
		if (PosTemp >tempMaxDist)
		{
			tempMaxDist = PosTemp;
		}
	}
	gPosMax = tempMaxDist;
	gPosMin = tempMaxDist;
	gDeltaRep = gChosenLevel *(tempMaxDist - 13); // 13 is a value by trial and error. Under the current arena definition, it is the value when the chest is almost touching the device.
	gDeltaRest = 0.2*(tempMaxDist - 13);
	PlaySoundA("sounds/Begin.wav", NULL, SND_FILENAME | SND_ASYNC);
	Sleep(1500);
}

// Wait till user completes the round (returns to the upmost position)
void WaitForRepFinish() {
	gLastDist = GetTargetDist();
	if (flag == 1)
	{
		flag = 0;
		gPosMin = gPosMax;
		gDeltaUP = 0.0;
		gDeltaDN = 0.0;
		return;
	}
	else if (gPosMax - 3.0 < gLastDist)
	{
		flag = 1;
		WaitForRepFinish();
	}
	else
	{
		WaitForRepFinish();
	}
}

// Determine if user is moving up or down
void GoingUpOrDn() {
	gLastDist = GetTargetDist();
	if (gLastDist < gPosMin)
	{
		gDeltaDN = gDeltaDN + (gPosMin - gLastDist);
		gPosMin = gLastDist;
		if (gDeltaDN - gDeltaUP > 3.0)
		{
			gDeltaDN = 0.0;
			gDeltaUP = 0.0;
			return;
		}
		else if (gDeltaUP - gDeltaDN > 5)
		{
			NoRep();
			WaitForRepFinish();
		}
		else
		{
			GoingUpOrDn();
		}
	}
	else
	{

		gDeltaUP = gDeltaUP + (gLastDist - gPosMin);
		gPosMin = gLastDist;
		if (gDeltaDN - gDeltaUP > 3)
		{
			gDeltaDN = 0.0;
			gDeltaUP = 0.0;
			return;
		}
		else if (gDeltaUP - gDeltaDN > 3)
		{
			NoRep();
			WaitForRepFinish();
		}
		else
		{
			GoingUpOrDn();
		}
	}
}

// Count reps -determine if there was a rep/ user is resting/ no rep
void CountReps() {
	if (gPosMax - gPosMin > gDeltaRep) //We have a rep
	{
		gRepNum++;
		CountOutLoud(gRepNum);
		WaitForRepFinish();
	}
	else if (gLastDist < gPosMin)
	{
		gPosMin = gLastDist;
		if (gPosMax - gPosMin < gDeltaRest) //Resting
		{
			return;
		}
		else
		{
			GoingUpOrDn(); // Determine if there is a Rep or No Rep
		}
	}
	else
	{
		return;
	}
}

void PushApp() {
	WALABOT_RESULT res;
	APP_STATUS appStatus;
	double calibrationProcess;
	double rArenaMin = 5.0;
	double rArenaMax = 40.0;
	double rArenaRes = 4.0;
	double thetaArenaMin = -20.0;
	double thetaArenaMax = 20.0;
	double thetaArenaRes = 3.0;
	double phiArenaMin = -30.0;
	double phiArenaMax = 30.0;
	double phiArenaRes = 3.0;
	double threshold = 8.0;
	res = Walabot_SetSettingsFolder("C:/ProgramData/Walabot/WalabotSDK");
	assert(res == WALABOT_SUCCESS);
	res = Walabot_ConnectAny();
	assert(res == WALABOT_SUCCESS);
	res = Walabot_SetProfile(PROF_SENSOR);
	assert(res == WALABOT_SUCCESS);
	res = Walabot_SetArenaR(rArenaMin, rArenaMax, rArenaRes);
	assert(res == WALABOT_SUCCESS);
	res = Walabot_SetArenaTheta(thetaArenaMin, thetaArenaMax, thetaArenaRes);
	assert(res == WALABOT_SUCCESS);
	res = Walabot_SetArenaPhi(phiArenaMin, phiArenaMax, phiArenaRes);
	assert(res == WALABOT_SUCCESS);
	res = Walabot_SetThreshold(threshold);
	assert(res == WALABOT_SUCCESS);
	res = Walabot_SetDynamicImageFilter(FILTER_TYPE_MTI);
	assert(res == WALABOT_SUCCESS);
	res = Walabot_Start();
	assert(res == WALABOT_SUCCESS);
	res = Walabot_StartCalibration();
	assert(res == WALABOT_SUCCESS);
	res = Walabot_GetStatus(&appStatus, &calibrationProcess);
	assert(res == WALABOT_SUCCESS);
	GetLevel(); // Get level of difficulty from user
	PlaySoundA("sounds/WELLCOME.wav", NULL, SND_FILENAME | SND_ASYNC);
	Sleep(7000); // announce the starting of the calibration
	GetTargetPosMax();
	while (true)
	{
		gLastDist = GetTargetDist();
		CountReps();
	}
	res = Walabot_Stop(); // Stop and disconnect
	assert(res == WALABOT_SUCCESS);
	res = Walabot_Disconnect();
	assert(res == WALABOT_SUCCESS);
}

void main()
{
	PushApp();
}
