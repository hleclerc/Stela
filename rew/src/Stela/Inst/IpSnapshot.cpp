#include "IpSnapshot.h"

PI64 IpSnapshot::cur_date = 0;

IpSnapshot::IpSnapshot() : date( cur_date++ ) {
}
