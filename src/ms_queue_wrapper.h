#pragma once
#include <cds/container/msqueue.h>
#include <cds/init.h>
#include <cds/gc/hp.h>

template<typename T>
using MSQueue = cds::container::MSQueue<cds::gc::HP, T>;
