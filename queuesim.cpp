#include <iostream>
#include <cstdlib> // drand48()
#include <cmath>   // log
#include <climits> // INT_MAX
#include <queue>

using namespace std;

// Global Vars

double timeElapsed = 0;
double packetsDropped = 0; // count of all lost packets
double area = 0; // area under the curve, used to get average queue lengths

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
	Event(EventType type, double rate) {
		this->type = type;
		this->eventTime = timeElapsed + rexp(rate);
		this->prevEvent = NULL;
		this->nextEvent = NULL;
	} // constructor

	double eventTime;
	EventType type;
	Event *prevEvent; // why are these necessary? just use PQ
	Event *nextEvent;
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

    /*
     * For debugging. Prints list times and size
     */
    void printList() {
        Event *curEvt = front;
        while(curEvt != NULL) {
            cout << curEvt->eventTime << endl;
            curEvt = curEvt->nextEvent;
        } // while
        cout << "Size: " << size << endl;
    } // printList

private:
    Event *front = NULL;
    int size; // could be useful
}; // EventList

bool isProcessing = false; // is something being serviced?

/*
 * creates new arrival packet and processes old one
 * updates the global variable timeElapsed
 * deletes the old arrival event iff it is to be handled immmediately
 */
void processArrivalEvent(Event* evt, EventList& gel, queue<Event*>& buffer,
                         double lambda, double mu) {
    timeElapsed = evt->eventTime; // update time
    gel.insertEvent(new Event(ARRIVAL, lambda)); // new event

    if(buffer.size() == 0 && !isProcessing) { // handle immediately
        gel.insertEvent(new Event(DEPARTURE, mu));
        isProcessing = true;
        delete evt; // maybe?
    } else if(buffer.size() < MAXBUFFER) {
        buffer.push(evt); // delete later
        // update statistics
    } else { // buffer full, drop packet
        packetsDropped++;
        delete evt;
    }
} // processArrivalEvent

void processServiceCompletion(Event* evt, EventList& gel, queue<Event*>& buffer,
                              double lambda, double mu) {
    timeElapsed = evt->eventTime;
    // update stats
    if(buffer.size() == 0) {
        isProcessing = false;
    } else {
        Event *nextEvt = buffer.front();
        buffer.pop();
        delete nextEvt;
        gel.insertEvent(new Event(DEPARTURE, mu));
    } // if
} // processServiceCompletion

void outputStatistics() {
    cout << "Packet loss: " << packetsDropped << endl;
} // outputStatistics

/*
 * Queue modeling program
 */
int main() {
    double lambda = 0.1; // i don't know how to deal with these. global consts?
	double mu = 1.0;

	EventList gel; // Global Event List
	queue<Event*> buffer; // events waiting to be processed
	Event *firstEvt = new Event(ARRIVAL, lambda);
	gel.insertEvent(firstEvt);

	for(int i = 0; i < 100000; i++) {
        Event *evt = gel.getNextEvent();
        if(evt == NULL) { // will not happen later on
            cout << "NULL event" << endl;
            return 1;
        }
        if(evt->type == ARRIVAL) {
            processArrivalEvent(evt, gel, buffer, lambda, mu);
        } else { // DEPARTURE
            processServiceCompletion(evt, gel, buffer, lambda, mu);
        } // if-else
	} // main loop

	outputStatistics();
} // main
