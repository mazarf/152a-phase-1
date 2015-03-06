#include <iostream>
#include <cstdlib> // drand48()
#include <cmath>   // log(), pow()
#include <climits> // INT_MAX
#include <cfloat> // DBL_MIN
#include <queue> // queue

using namespace std;

// Global Vars
double timeElapsed = 0;
double utilizationTime = 0;
double packetsDropped = 0; // count of all lost packets
double area = 0; // area under the curve, used to get average queue lengths
bool isProcessing = false; // is something being serviced?

// Constants
const bool pareto = true; // are we doing pareto distributions?

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
 * generate random pareto variable using
 * formula from Wikipedia
 */
double rpareto(double rate) {
    double u;
	u = drand48();
    return (1e-100)/(pow(u, 1 / rate));
} // rpareto

/*
 * Event objects hold a the type of event and the time of the event
 * as well as a pointer to prev/next event
 */
class Event {
public:
	Event(EventType type, double rate) {
		this->type = type;
		if(pareto && type == ARRIVAL) {
            this->eventTime = timeElapsed + rpareto(rate);
		} else {
            this->eventTime = timeElapsed + rexp(rate);
        }
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
                } // if
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

/*
 * creates new arrival packet and processes old one
 * updates the global variable timeElapsed
 * deletes the old arrival event iff it is to be handled immmediately
 */
void processArrivalEvent(Event* evt, EventList& gel, queue<Event*>& buffer,
                         double lambda, double mu, int MAXBUFFER) {
    double prevTime = timeElapsed; // used to calculate stats
    timeElapsed = evt->eventTime; // update time
    area += (timeElapsed - prevTime) * (buffer.size() + isProcessing); // might need a +1
    gel.insertEvent(new Event(ARRIVAL, lambda)); // new event

    if(isProcessing) {
            utilizationTime += timeElapsed - prevTime;
    } // if still processing

    if(buffer.size() == 0 && !isProcessing) { // handle immediately
        gel.insertEvent(new Event(DEPARTURE, mu));
        isProcessing = true;
        delete evt; // maybe?
    } else if((int)buffer.size() < MAXBUFFER) {
        buffer.push(evt); // delete event later
    } else { // buffer full, drop packet
        packetsDropped++;
        delete evt;
    } // if-elseif-else
} // processArrivalEvent

void processServiceCompletion(Event* evt, EventList& gel, queue<Event*>& buffer,
                              double lambda, double mu, int MAXBUFFER) {
    double prevTime = timeElapsed; // used to calculate stats
    timeElapsed = evt->eventTime;
    area += (timeElapsed - prevTime) * (buffer.size()+isProcessing); // might need a +1
    delete evt;
    // update stats
    if(buffer.size() == 0) {
        isProcessing = false;
    } else {
        Event *nextEvt = buffer.front();
        buffer.pop();
        delete nextEvt;
        gel.insertEvent(new Event(DEPARTURE, mu));
    } // if
    // calculate stats
    utilizationTime += timeElapsed - prevTime;
} // processServiceCompletion

void outputStatistics(double l, double m) {
    cout << "Packet loss: " << packetsDropped << endl;
    cout << "Average queue size: " << area / timeElapsed << endl;
    cout << "Mean utilization: " << utilizationTime / timeElapsed << endl;
} // outputStatistics

void queueSim(double lambda, double mu, int MAXBUFFER) {
    timeElapsed = 0;
    utilizationTime = 0;
    packetsDropped = 0;
    area = 0;           // reset global vars. TODO: make local. may be hard b/c Event uses them
    isProcessing = false;

	EventList gel; // Global Event List
	queue<Event*> buffer; // events waiting to be processed
	Event *firstEvt = new Event(ARRIVAL, lambda);
	gel.insertEvent(firstEvt);

	for(int i = 0; i < 100000; i++) {
        Event *evt = gel.getNextEvent();
        //cout << evt->eventTime << ' ';
        if(evt == NULL) { // failsafe. should never happen
            cout << "NULL event" << endl;
            return;
        }
        if(evt->type == ARRIVAL) {
            processArrivalEvent(evt, gel, buffer, lambda, mu, MAXBUFFER);
        } else { // DEPARTURE
            processServiceCompletion(evt, gel, buffer, lambda, mu, MAXBUFFER);
        } // if-else
        //cout << buffer.size() << endl;
	} // main loop

	outputStatistics(lambda, mu);
} // queueSim

/*
 * Queue modeling program
 */
int main() {
    // unbounded buffer
    queueSim(0.1, 1.0, INT_MAX);
    queueSim(0.25, 1.0, INT_MAX);
    queueSim(0.4, 1.0, INT_MAX);
    queueSim(0.55, 1.0, INT_MAX);
    queueSim(0.65, 1.0, INT_MAX);
    queueSim(0.80, 1.0, INT_MAX);
    queueSim(0.90, 1.0, INT_MAX);

    queueSim(0.20, 1.0, 1);
    queueSim(0.40, 1.0, 1);
    queueSim(0.60, 1.0, 1);
    queueSim(0.80, 1.0, 1);
    queueSim(0.90, 1.0, 1);

    queueSim(0.20, 1.0, 20);
    queueSim(0.40, 1.0, 20);
    queueSim(0.60, 1.0, 20);
    queueSim(0.80, 1.0, 20);
    queueSim(0.90, 1.0, 20);

    queueSim(0.20, 1.0, 50);
    queueSim(0.40, 1.0, 50);
    queueSim(0.60, 1.0, 50);
    queueSim(0.80, 1.0, 50);
    queueSim(0.90, 1.0, 50);

} // main
