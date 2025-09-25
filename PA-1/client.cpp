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
	int p = 0;
	double t = 0.0;
	int e = 0;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}


    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
    char buf[MAX_MESSAGE]; // 256
	double interv = 0.004;

	if(p != 0 && e == 0 && t == 0.0) {
		ofstream x1file("received/x1.csv");
		if (!x1file.is_open()) {
			cerr << "Cannot open x1.csv for writing\n";
			return 1;
		}

		for(int i = 0; i < 1000; ++i) {
			t = interv * i;
			datamsg x1(p, t, 1);
			memcpy(buf, &x1, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply1;
			chan.cread(&reply1, sizeof(double)); //answer

			datamsg x2(p, t, 2);
			memcpy(buf, &x2, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply2;
			chan.cread(&reply2, sizeof(double)); //answer

			x1file << t << "," << reply1 << "," << reply2 << endl;
		}
	}
	else{
		datamsg x(p, t, e);
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;

	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

}
