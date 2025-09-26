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

	pid_t pid = fork();
	if(pid == 0) {
		char* args[] = {(char*)"./server", nullptr};
		execvp(args[0], args);
		cerr << "exec failed\n";
		return 1;
	}

	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;

	bool is_p = false;
	bool is_ti = false;
	bool is_e = false;
	bool is_f = false;
	bool is_c = false;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				is_p = true;
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
				is_c = true;
				break;
		}
	}


    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	FIFORequestChannel* curr_chan = &chan;
	
    char buf[MAX_MESSAGE]; // 256
	double interv = 0.004;

	if(is_c) {
		MESSAGE_TYPE new_channel_mes = NEWCHANNEL_MSG;
		chan.cwrite(&new_channel_mes, sizeof(MESSAGE_TYPE));
		char buf4[256];
		chan.cread(buf4, sizeof(buf4));
		curr_chan = new FIFORequestChannel(buf4, FIFORequestChannel::CLIENT_SIDE);
	}

	if(is_p && !is_ti && !is_e) {
		ofstream x1file("received/x1.csv");
		if (!x1file.is_open()) {
			cerr << "Cannot open x1.csv for writing\n";
			return 1;
		}

		for(int i = 0; i < 1000; ++i) {
			t = interv * i;
			datamsg x1(p, t, 1);
			memcpy(buf, &x1, sizeof(datamsg));
			curr_chan->cwrite(buf, sizeof(datamsg)); // question
			double reply1;
			curr_chan->cread(&reply1, sizeof(double)); //answer

			datamsg x2(p, t, 2);
			memcpy(buf, &x2, sizeof(datamsg));
			curr_chan->cwrite(buf, sizeof(datamsg)); // question
			double reply2;
			curr_chan->cread(&reply2, sizeof(double)); //answer

			x1file << t << "," << reply1 << "," << reply2 << endl;
		}
	} 
	else if(is_f && !is_e && !is_ti && !is_p) {
		// sending a non-sense message, you need to change this

		ofstream file("received/" + filename, ios::binary);
		if (!file.is_open()) {
			cerr << "Cannot open file for writing\n";
			return 1;
		}

		filemsg fm(0, 0);
		string fname = filename;
		
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		curr_chan->cwrite(buf2, len);  // I want the file length;

		__int64_t fs;
		curr_chan->cread(&fs, sizeof(__int64_t));

		int offset = 0;
		int length = MAX_MESSAGE;
		while (offset < fs) {
			if(offset + MAX_MESSAGE > fs) {
				length = fs - offset;
			}
			filemsg fm(offset, length);

			int len = sizeof(filemsg) + (fname.size() + 1);
			char* buf3 = new char[len];
			memcpy(buf3, &fm, sizeof(filemsg));
			strcpy(buf3 + sizeof(filemsg), fname.c_str());
			curr_chan->cwrite(buf3, len);

			delete[] buf3;

			char* line = new char[length];
			curr_chan->cread(line, length);
			file.write(line, length);

			delete[] line;
			offset += length;
		}

		delete[] buf2;
	} 	
	else {
		datamsg x(p, t, e);
		memcpy(buf, &x, sizeof(datamsg));
		curr_chan->cwrite(buf, sizeof(datamsg)); // question
		double reply;
		curr_chan->cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	

	// closing the channel    
	if (is_c) delete curr_chan;
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

}
