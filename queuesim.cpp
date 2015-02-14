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
	Event(EventType type = ARRIVAL) {
		this->type = type;
		this->eventTime = timeElapsed + rexp(lambda);
		this->prevEvent = NULL;
		this->nextEvent = NULL;
	} // constructor

	double eventTime;
	EventType type;
	Event * prevEvent; // why are these necessary? just use PQ
	Event * nextEvent;
}; // Event

class EventList {
public:
    EventList() {
        size = 0;
    }

    /*
     * Return the topmost event, next event becomes the front
     * If no event left, returns nullptr
     */
    Event *getNextEvent() {
        Event *nextEvt = front;
        if(front != NULL) {
            front = front->nextEvent; // traverse
        }
        size--;
        return nextEvt;
    }

    /*
     * insert a new event in order based on time
     * prevEvent and nextEvent members modified
     */
    void insertEvent(Event *evt) {
        if(front == NULL) {
            front = evt;
        } else {
            Event *curEvt = front;
            Event *prevEvt = NULL;

            // traverse until a spot is found
            while(curEvt != NULL && curEvt->eventTime < evt->eventTime) {
                prevEvt = curEvt;
                curEvt = curEvt->nextEvent;
            } // while

            if(prevEvt == NULL) // front of list
            {
                front = evt;
                evt->nextEvent = curEvt;
            } else {
                prevEvt->nextEvent = evt;
                evt->prevEvent = prevEvt;
                evt->nextEvent = curEvt;
                if(curEvt != NULL) {
                    curEvt->prevEvent = evt;
                }
            } // if-else
        } // if-else
        size++;
    } // insert

    bool empty() {
        return size == 0;
    } // empty

    int getSize() {
        return size;
    }

    void printList() {
        if(front == NULL) {
            cout << "Empty!" << endl;
        } else {
            Event *curEvt = front;
            while(curEvt != NULL) {
                cout << curEvt->eventTime << endl;
                curEvt = curEvt->nextEvent;
            } // while
        } // if-else
    } // printList

private:
    Event *front = NULL;
    int size; // could be useful
}; // EventList


/*
 * Queue modeling program
 */
int main() {
	cout << "Hello" << endl;

	EventList elist;
	 Event evts[100];
    for(int i = 0;i  < 100; i++) {
        elist.insertEvent(evts + i);
    }
	elist.printList();
} // main
