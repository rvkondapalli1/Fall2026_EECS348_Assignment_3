// Program: EECS 348 Assignment 3
// Description: This program prioritizes emails for a CEO using a custom MaxHeap.
// Emails are prioritized first by sender category and then by newest date.
// The program processes EMAIL, NEXT, READ, and COUNT commands from a test file.
//
// Input: A test file containing EMAIL, NEXT, READ, and COUNT commands.
//
// Output: Displays the next highest-priority email and the number of unread emails.
//
// Collaborators: None
//
// Sources: ChatGPT and Claude
//
// Author: Rithvik Kondapalli
//
// Creation Date: 10/1/2026
//
// Revision Date: 10/1/2026
//
// Revisions: Converted the email priority program to object-oriented C++ using a
// custom MaxHeap and added validation, memory, and comment improvements.







// CEO Email Prioritizer - object-oriented C++ using a hand-built MaxHeap
//
// Classes:
//   Email            - one message; knows its own priority rules
//   MaxHeap          - array-based heap (priority queue), built from scratch
//   CEOInbox         - the CEO's commands (EMAIL, NEXT, READ, COUNT)
//   CommandProcessor - reads the test file and calls the inbox
//
// Priority: Boss > Subordinate > Peer > ImportantPerson > OtherPerson;
// within a category the newest date is read first.
#include <iostream>
#include <fstream>
#include <string>
#include <utility>   // std::swap

using namespace std;

// ---------------------------------------------------------------
// Email: one message plus the logic that decides its priority
// ---------------------------------------------------------------
// Source: Claude-generated code, modified by Rithvik Kondapalli using Claude for assistance
class Email {
private:
    string sender;
    string subject;
    string date;      // MM-DD-YYYY
    int    rank;      // higher = read sooner
    long   dateKey;   // YYYYMMDD so newer dates compare larger
    long   sequence;  // arrival order, used to break exact ties

    // Rank 5 (Boss) down to 1 (OtherPerson); higher rank is read first
    static int rankFor(const string& s) {
        if (s == "Boss")            return 5;
        if (s == "Subordinate")     return 4;
        if (s == "Peer")            return 3;
        if (s == "ImportantPerson") return 2;
        return 1;                   // OtherPerson
    }

    static long keyFor(const string& d) {
        if (d.size() < 10) return 0;
        long mm = stol(d.substr(0, 2));
        long dd = stol(d.substr(3, 2));
        long yy = stol(d.substr(6, 4));
        return yy * 10000 + mm * 100 + dd;
    }
    // Source: Claude-generated code, modified by Rithvik Kondapalli using Claude
public:
    // Correctness: true only for the five known sender categories
    static bool isValidCategory(const string& s) {
        return s == "Boss" || s == "Subordinate" || s == "Peer" ||
               s == "ImportantPerson" || s == "OtherPerson";
    }

    Email() : rank(0), dateKey(0), sequence(0) {}
    Email(const string& s, const string& subj, const string& d, long seq)
        : sender(s), subject(subj), date(d),
          rank(rankFor(s)), dateKey(keyFor(d)), sequence(seq) {}

    // true if this email should be read before 'other':
    // 1) higher sender rank, 2) newer date, 3) arrived earlier
    bool higherPriorityThan(const Email& other) const {
        if (rank != other.rank)       return rank > other.rank;
        if (dateKey != other.dateKey) return dateKey > other.dateKey; // newest first
        return sequence < other.sequence;                             // earlier arrival first
    }

    void display() const {
        cout << "Sender: "  << sender  << endl;
        cout << "Subject: " << subject << endl;
        cout << "Date: "    << date    << endl;
    }
};

// ---------------------------------------------------------------
// MaxHeap: array (list) based, grows dynamically, built from scratch
// ---------------------------------------------------------------
// Source: Claude-generated code, modified by Rithvik Kondapalli using Claude for assistance
class MaxHeap {
private:
    Email* data;
    int    count;
    int    capacity;

    static int parent(int i) { return (i - 1) / 2; }
    static int left(int i)   { return 2 * i + 1; }
    static int right(int i)  { return 2 * i + 2; }

    void swapAt(int a, int b) {
        std::swap(data[a], data[b]);   // moves strings instead of copying them
    }

    // Doubles the array when it is full
    void grow() {
        int newCap = capacity * 2;
        Email* bigger = new Email[newCap];
        for (int i = 0; i < count; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCap;
    }

    // Space: halve the array when it is mostly empty (never below 16)
    void shrink() {
        if (capacity <= 16 || count > capacity / 4) return;
        int newCap = capacity / 2;
        Email* smaller = new Email[newCap];
        for (int i = 0; i < count; i++) smaller[i] = data[i];
        delete[] data;
        data = smaller;
        capacity = newCap;
    }

    void siftUp(int i) {
        while (i > 0 && data[i].higherPriorityThan(data[parent(i)])) {
            swapAt(i, parent(i));
            i = parent(i);
        }
    }

    void siftDown(int i) {
        while (true) {
            int best = i, l = left(i), r = right(i);
            if (l < count && data[l].higherPriorityThan(data[best])) best = l;
            if (r < count && data[r].higherPriorityThan(data[best])) best = r;
            if (best == i) break;
            swapAt(i, best);
            i = best;
        }
    }

public:
    MaxHeap() : data(new Email[16]), count(0), capacity(16) {}
    ~MaxHeap() { delete[] data; }
    MaxHeap(const MaxHeap&) = delete;
    MaxHeap& operator=(const MaxHeap&) = delete;

    bool isEmpty() const { return count == 0; }
    int  size() const    { return count; }

    void insert(const Email& e) {
        if (count == capacity) grow();
        data[count] = e;
        siftUp(count);
        count++;
    }

    // NEXT: copies the top email into 'out' without removing it (false if empty)
    bool peek(Email& out) const {
        if (count == 0) return false;
        out = data[0];
        return true;
    }

    // READ: removes the highest priority email (false if empty)
    bool removeMax() {
        if (count == 0) return false;
        data[0] = data[count - 1];
        count--;
        if (count > 0) siftDown(0);
        shrink();
        return true;
    }
};

// ---------------------------------------------------------------
// CEOInbox: wraps the heap and exposes the CEO's commands
// ---------------------------------------------------------------
// Source: Claude-generated code, modified by Rithvik Kondapalli using Claude for assistance
class CEOInbox {
private:
    MaxHeap heap;
    long    nextSequence;

public:
    CEOInbox() : nextSequence(0) {}

    void addEmail(const string& sender, const string& subject, const string& date) {
        heap.insert(Email(sender, subject, date, nextSequence++));
    }

    void next() const {
        Email e;
        if (heap.peek(e)) {
            cout << "Next email:" << endl;
            e.display();
        } else {
            cout << "No emails to read." << endl;
        }
    }

    void read() { heap.removeMax(); }

    void count() const {
        cout << "There are " << heap.size() << " emails to read." << endl;
    }
};

// ---------------------------------------------------------------
// CommandProcessor: parses the test file and drives the inbox
// ---------------------------------------------------------------
// Source: Claude-generated code, modified by Rithvik Kondapalli using Claude for assistance
class CommandProcessor {
private:
    CEOInbox& inbox;

    static string trim(const string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    void handleEmail(const string& rest) {
        size_t c1 = rest.find(',');
        size_t c2 = rest.rfind(',');
        if (c1 == string::npos || c2 == c1) return; // malformed line
        string sender  = trim(rest.substr(0, c1));
        string subject = trim(rest.substr(c1 + 1, c2 - c1 - 1));
        string date    = trim(rest.substr(c2 + 1));
        if (!Email::isValidCategory(sender) || date.size() != 10) return; // skip bad line
        inbox.addEmail(sender, subject, date);
    }

public:
    explicit CommandProcessor(CEOInbox& in) : inbox(in) {}

    void process(istream& in) {
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            if (line.compare(0, 6, "EMAIL ") == 0) handleEmail(line.substr(6));
            else if (line == "NEXT")  inbox.next();
            else if (line == "READ")  inbox.read();
            else if (line == "COUNT") inbox.count();
        }
    }
};

int main(int argc, char* argv[]) {
    string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        cout << "Enter test file name: ";
        getline(cin, filename);
    }

    ifstream file(filename);
    if (!file) {
        cerr << "Could not open file: " << filename << endl;
        return 1;
    }

    CEOInbox inbox;
    CommandProcessor processor(inbox);
    processor.process(file);
    return 0;
}
