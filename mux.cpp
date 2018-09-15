#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <math.h>       
#include <iterator>

using namespace std;

int numTimeSlots = 0;
int numSources = 0;

struct DataBlock {
	string data; 
    int start;   
    int end;  
};

struct Frame {
	int startFlag;
	int endFlag;
	vector<DataBlock> dataBlocks;
};

struct Source {
	string name;
	vector<DataBlock> dataBlocks;
};

void printSourceDataBlocks(Source source){
	printf("Printing info about %s\n", source.name.c_str());
	for (int i = 0; i < source.dataBlocks.size(); i++){
		DataBlock db = source.dataBlocks[i];
		printf("start: %d, end: %d, data: %s\n", db.start, db.end, db.data.c_str());
	}
}

float getAverageRate(vector<Source> sources){
	int numDataBlocks, numTimeSlots;
	Source source;
	int numSources = 0;
	float averageSourceRate = 0.0;
	float avgRatesSum = 0;

	for (int i = 0; i < sources.size(); i++){
		source = sources[i];
		numDataBlocks = source.dataBlocks.size();
		numTimeSlots = source.dataBlocks[numDataBlocks-1].end;
		averageSourceRate = (float)numDataBlocks / (float)numTimeSlots;
		
		if (numTimeSlots > ::numTimeSlots){
			::numTimeSlots = numTimeSlots;
		}
		avgRatesSum += averageSourceRate;
		numSources++;
		//printf("%s has %d blocks\n", source.name.c_str(), numDataBlocks);
		//printf("%s transmits for %d seconds\n", source.name.c_str(), numTimeSlots);
		//printf("%s average rate is %f\n", source.name.c_str(), averageSourceRate);
	}

	::numSources = numSources;
	printf("Average combined transmission rate of sources: %f\n", avgRatesSum);
	return avgRatesSum;
}

int main(int argc, char *argv[]){

	vector<Source> sources;
	vector<DataBlock> allDataBlocks;
	vector<Frame> frames;

	//get data from file
	ifstream infile;
	string filename = argv[1];
	infile.open(filename.c_str());
	//printf("reading file %s\n", filename.c_str());
	
	string lineString;
	while (getline(infile, lineString)) {
		if (lineString.length() == 0){
			break;
		}

		Source source;
 		string sourcename;	

		char lineArray[1024];
		strcpy(lineArray, lineString.c_str());

  		char * tok;
		tok = strtok (lineArray, ",");
		int first = true;
  		while (tok != NULL) {
   			istringstream iss(tok);
			vector<string> results((istream_iterator<string>(iss)), istream_iterator<string>());
			iss.clear();

			DataBlock db;
			int counter = 0;
			for (int i = 0; i < results.size(); i++){
				istringstream iss(results[i]);
				//cout << results[i] << endl;

				if (first && counter == 0){
					iss >> sourcename;
					source.name = sourcename.substr(0, sourcename.length() - 1);
					first = false;
					counter --;
					//printf("source name: %s\n", source.name.c_str());
				} else if (counter == 0){
					iss >> db.start;
					//printf("start: %d\n", db.start);
				} else if (counter == 1){
					iss >> db.end;
					//printf("end: %d\n", db.end);
				} else if (counter == 2){
					iss >> db.data;
					//printf("data: %s\n", db.data.c_str());
				}
				iss.clear();
				counter++;
			}
			source.dataBlocks.push_back(db);
   			tok = strtok (NULL, ",");
 		}
 		sources.push_back(source);
	}

	int i, j, k;
	for (i = 0; i < sources.size(); i++){
		//printSourceDataBlocks(sources[i]);
		//cout << endl;
	}
	infile.close();

	int muxRate = ceil(getAverageRate(sources));
	printf("Mux transmission rate: %d\n", muxRate);
	printf("Number of time slots: %d\n", numTimeSlots);
	printf("Number of sources: %d\n\n", numSources);

	//assemble all the data blocks into an ordered vector
	Source source;
	for (i = 0; i < numTimeSlots; i++) {
		//printf("checking time slot %d\n", i);
		for (j = 0; j < sources.size(); j++){
			source = sources[j];
			//printf("checking source %s\n", source.name.c_str());
			for (k = 0; k < source.dataBlocks.size(); k++){
				DataBlock db = source.dataBlocks[k];
				if (db.start == i) {
					allDataBlocks.push_back(db);
				}
			}
		}
	}

	//printf("Ordered datablocks: ");
	for (i = 0; i < allDataBlocks.size(); i++){
		DataBlock db = allDataBlocks[i];
		//printf("%s ", db.data.c_str());
	}
	//printf("\n\n");
	printf("Mux Input:\n");
	for (i = 0; i < numTimeSlots; i++){
		printf("Timeslot %d->%d: ", i, i+1);
		for (j = 0; j < allDataBlocks.size(); j++){
			DataBlock db = allDataBlocks[j];
			if (db.start == i){
				printf("%s ", db.data.c_str());
			}
		}
		printf("\n");
	}
	printf("\n");

	int timeSlotCounter = 0;
	int dataBlockCounter = 0;
	DataBlock db;
	Frame frame;
	for (i = 0; i < allDataBlocks.size(); i++) {
		db = allDataBlocks[i];
		if (dataBlockCounter == 0){
			//cout << "start of frame" << endl;
			frame = Frame();
			frame.startFlag = timeSlotCounter;
		}
		
		if (dataBlockCounter == muxRate) {
			//cout << "frame full" << endl;
			frame.endFlag = timeSlotCounter+1;
			frames.push_back(frame);
			//set counters
			timeSlotCounter++;
			dataBlockCounter = 0;
			i--;
		} else if (db.start <= timeSlotCounter){
			//printf("Adding %s to frame\n", db.data.c_str());
			frame.dataBlocks.push_back(db);
			dataBlockCounter++;
			if (i == allDataBlocks.size() - 1){
				int fillerBlocksNeeded =  muxRate - dataBlockCounter ;
				for (int filler = 0; filler < fillerBlocksNeeded; filler++){
					//cout << "adding filler block to frame" << endl;
					DataBlock fillerdb;
					fillerdb.start = timeSlotCounter;
					fillerdb.end = timeSlotCounter+1;
					fillerdb.data = "*";
					frame.dataBlocks.push_back(fillerdb);
				}
				frame.endFlag = timeSlotCounter+1;
				frames.push_back(frame);
				break;
			}
		} else {
			//add filler datablock
			//cout << "adding filler block to frame" << endl;
			DataBlock fillerdb;
			fillerdb.start = timeSlotCounter;
			fillerdb.end = timeSlotCounter+1;
			fillerdb.data = "*";
			frame.dataBlocks.push_back(fillerdb);
			dataBlockCounter++;
			i--;
		}
	}

	printf("Mux Output:\n");
	printf("Format is startFlag | address:datablock address:datablock ... | endflag\n");
	for (i = 0; i < frames.size(); i++){
		Frame frame = frames[i];
		printf("Frame%d = ", i);
		printf("%d | ", frame.startFlag);
		for (j = 0; j < frame.dataBlocks.size(); j++){
			DataBlock db = frame.dataBlocks[j];
			printf("%d:%s ", j, db.data.c_str());
		}
		printf("| %d\n", frame.endFlag);
	}
}
