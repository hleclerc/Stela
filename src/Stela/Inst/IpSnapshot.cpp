#include "IpSnapshot.h"

int IpSnapshot::cur_date = 0;

IpSnapshot::IpSnapshot() : date( ++cur_date ) {
}
