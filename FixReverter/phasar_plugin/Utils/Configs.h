#ifndef CONFIGS_H_
#define CONFIGS_H_

/* 
print debug logs that track how taint flows
dramatically increases running time
*/
extern short LOG_DEBUG_INFO;

/* 
flow counter increments by 1 on each IFDS flow
used as break point condition to help debugging
only valid when DEBUG_MODE is 1
*/
#define FLOW_COUNTER 1

#endif
