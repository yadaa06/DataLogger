// timeset.h

#ifndef TIMESET_H
#define TIMESET_H

#define MAX_RETRY_COUNT 10

void initialize_sntp(void);
void wait_for_time_sync(void);
void set_timezone(void);

#endif