#include <iostream>
#include <cstdlib> // drand48()
#include <cmath>   // log
#include <climits> // INT_MAX
#include <queue> // GEL, a min PQ
#include <vector>

using namespace std;

// Global Vars
const double lambda = 0.1; // arrival rate
const double mu = 1.0; // service rate

double timeElapsed = 0;
double packetsDropped = 0; // count of all lost packets


// Constants
const int MAXBUFFER = INT_MAX; // maximum queue size

typedef enum { ARRIVAL, DEPARTURE } EventType;

/*
 * Randomly generated value
 * with exponential distribution
 */
double rexp(double rate) {
	double u;
	u = drand48();
	return ((-1/rate)*log(1-u));
} // rexp

/*
 * Event objects hold a the type of event and the time of the event
 * as well as a pointer to prev/next event
 */
class Event {
public:
	Event(EventType type) {
		cout << "Event created" << endl;
		this->type = type;
		this->eventTime = timeElapsed + rexp(lambda);
	} // constructor

	// implement operators so max PQ will work
	bool operator==(Event evt) {
		return this->eventTime == evt.eventTime; // should i compare refs or values?
	}
	bool operator>=(Event evt) {
		return this->eventTime >= evt.eventTime;
	}
	bool operator<=(Event evt) {
		return this->eventTime <= evt.eventTime;
	}
	bool operator>(Event evt) {
		return this->eventTime > evt.eventTime;
	}
	bool operator<(Event evt) {
		return this->eventTime < evt.eventTime;
	}
	// end of operators

	double eventTime;
	EventType type;
	Event * prevEvent; // why are these necessary? just use PQ
	Event * nextEvent;
};


// GEL. Temporarily here. STL min PQ
std::priority_queue<int> my_min_heap;

/*
 * Queue modeling program
 */
int main() {
	cout << "Hello" << endl;
	Event evt(ARRIVAL);
	Event evt2(ARRIVAL);
	cout << evt.eventTime << ' ' << evt2.eventTime << endl;
} // main
