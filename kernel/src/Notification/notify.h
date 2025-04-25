#pragma once

#include <stdint.h>
#include <gpx1.h>

void notify_message(const char* message);
void notify_kernel_log(const char* kern_log);
void notify_non_fatal_panic(const char* nf_panic);