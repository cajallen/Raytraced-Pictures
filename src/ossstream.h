#ifndef _SPACED_SS_H
#define _SPACED_SS_H

#include <sstream>

using namespace std;

// Out Spaced String Stream
// If you want to understand any of the Encode functions, this is a prerequisite.
struct ossstream {
    ossstream(ostringstream& sstream) : sstream(sstream) {}
    ostringstream& sstream;
};
// Prepend a newline if we're <<'ing a string literal (ALWAYS a key for scene files)
inline ossstream& operator<<(ossstream& inp_stream, string start) {
    inp_stream.sstream << endl << start;
    return inp_stream;
}
inline ossstream& operator<<(ossstream& inp_stream, const char* start) {
    inp_stream.sstream << endl << start;
    return inp_stream;
}
// Prepend a space if we're <<'ing anything else (ALWAYS a value for scene files)
template <class T>
ossstream& operator<<(ossstream& inp_stream, const T& x) {
    inp_stream.sstream << " " << x;
    return inp_stream;
}

#endif