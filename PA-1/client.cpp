/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Jack Anderson	
	UIN: 833007410
	Date: 09/17/2025
*/
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;


int main (int argc, char *argv[]) {

	pid_t pid = fork(); // fork and store the pid
	if(pid == 0) { // if shild
		char* args[] = {(char*)"./server", nullptr}; // set up entry to server
		execvp(args[0], args); // exec into server
		cerr << "exec failed\n"; // shouldnt run unless exec didnt work
		return 1;
	}

	int opt;
	int p = 1; // initializing default values
	double t = 0.0;
	int e = 1;

	bool is_p = false; // initializing use of characters for which type (p for patient)
	bool is_ti = false; // here ti is t or time
	bool is_e = false;
	bool is_f = false;
	bool is_c = false;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:c")) != -1) { // gets opt checking for cases in string
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				is_p = true; // makes sure to make it known this parameter was passed
				break;
			case 't':
				t = atof (optarg);
				is_ti = true;
				break;
			case 'e':
				e = atoi (optarg);
				is_e = true;
				break;
			case 'f':
				filename = optarg;
				is_f = true;
				break;
			case 'c':
				is_c = true; // if these parameters arent passed these bools remain false
				break;
		}
	} 


    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE); // launching control
	FIFORequestChannel* curr_chan = &chan; // creates pointer for which is the current channel
	
    char buf[MAX_MESSAGE]; // 256
	double interv = 0.004; // time interval

	if(is_c) { // if c parameter passed
		MESSAGE_TYPE new_channel_mes = NEWCHANNEL_MSG; // set the msg to create a new channel
		chan.cwrite(&new_channel_mes, sizeof(MESSAGE_TYPE)); // write to server for new channel name
		char buf4[256]; // make place to store name
		chan.cread(buf4, sizeof(buf4)); // read channel name
		curr_chan = new FIFORequestChannel(buf4, FIFORequestChannel::CLIENT_SIDE); // set the current channel to the new channel the server made
	}

	if(is_p && !is_ti && !is_e) { // if p parameter pass and no time or e passed
		ofstream x1file("received/x1.csv"); // make output file x1 and store it in recieved
		if (!x1file.is_open()) { // if it isnt open exit w error
			cerr << "Cannot open x1.csv for writing";
			return 1;
		}

		for(int i = 0; i < 1000; ++i) { // for the first 1000 lines
			t = interv * i; // time goes up in intervals of interv so time accurate to iteration
			datamsg x1(p, t, 1); // set datamsg for the first ecg1
			memcpy(buf, &x1, sizeof(datamsg)); // copy the memory
			curr_chan->cwrite(buf, sizeof(datamsg)); // question
			double reply1; // stores ecg1
			curr_chan->cread(&reply1, sizeof(double)); //answer

			datamsg x2(p, t, 2); // set to ecg2
			memcpy(buf, &x2, sizeof(datamsg));
			curr_chan->cwrite(buf, sizeof(datamsg)); // question
			double reply2; // stores ecg2
			curr_chan->cread(&reply2, sizeof(double)); //answer

			x1file << t << "," << reply1 << "," << reply2 << endl; //writes to file the output from eahc iteration in correct format
		}
	} 
	else if(is_f && !is_e && !is_ti && !is_p) { // if file passed, no e or t or p is passed
		// sending a non-sense message, you need to change this

		ofstream file("received/" + filename, ios::binary); // create file with filename in recieved
		if (!file.is_open()) { // if doesnt work say something
			cerr << "Cannot open file for writing";
			return 1;
		}

		filemsg fm(0, 0); // send 0 and 0 so server knows it needs to send length
		string fname = filename;
		
		int len = sizeof(filemsg) + (fname.size() + 1); // sets up buf2 to ask for length
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		curr_chan->cwrite(buf2, len);  // I want the file length;

		__int64_t fs;
		curr_chan->cread(&fs, sizeof(__int64_t)); // reads the file length (i denote it as size even though it is length)

		int offset = 0; // set offset to start at 0
		int length = MAX_MESSAGE; // size of each chunk copied over
		while (offset < fs) { // while the offset is less than the entire file length
			if(offset + MAX_MESSAGE > fs) { // if the next chunk is less than their is left to copy then do this
				length = fs - offset; // set the length instead to the length left to copy
			}
			filemsg fm(offset, length); // create filemsg for the offset and length to get the file chunk

			int len = sizeof(filemsg) + (fname.size() + 1); // sets up to ask for file chunk
			char* buf3 = new char[len];
			memcpy(buf3, &fm, sizeof(filemsg));
			strcpy(buf3 + sizeof(filemsg), fname.c_str());
			curr_chan->cwrite(buf3, len); // asks for file chunk

			delete[] buf3; // cleanup

			char* line = new char[length]; // creates spot to fore file chunk
			curr_chan->cread(line, length); // reads the chunk from server
			file.write(line, length); // writes the chunk to the file in recieved

			delete[] line; // cleanup
			offset += length; // offset increases by chunksize
		}

		delete[] buf2; // cleanup
	} 	
	else { // then just wants point
		datamsg x(p, t, e); // passes arguments
		memcpy(buf, &x, sizeof(datamsg)); 
		curr_chan->cwrite(buf, sizeof(datamsg)); // question
		double reply;
		curr_chan->cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	

	// closing the channel    
	if (is_c) delete curr_chan; // delete dynamically allocated channel
    MESSAGE_TYPE m = QUIT_MSG; // store quit message
    chan.cwrite(&m, sizeof(MESSAGE_TYPE)); // send quit message

}
