/**
 * contains definitions that control the flow and how much output
 * is sent to the screen. Mostly for debugging purposes
 */
#ifndef OPC_H // header guard
#define OPC_H

////////////////////////////////////////////////////////////////
#define OPC_ENABLE 0 // set to 0 to disable all of them
#if OPC_ENABLE
// debugging
// globals
#define PRINT_PARENT_TOURS 1        // parent tours of current generation
#define PRINT_INTERMEDIATE_INFO 1   // information about intermediate individuals generated
#define PRINT_EDGES 0               // prints the list of edges
#define PRINT_CHILD_TOURS 1         // information about child tours
#define PRINT_GRAPHS 0              // information about graphs
#define PRINT_CYCLES 1              // information about AB cycles, sub cycles, and other cycles
#define PRINT_MISC 1                // everything else I was too lazy to granularize. Mostly allocations, and mergeGraph/freeGraph stuff
// per method
#define PRINT_STEPS 1               // prints a line in performEAX before each step, thereby identifying each step
#define PRINT_GENERATE_AB_CYCLES 0  // generateABCycles() output
#define PRINT_FIX_INTERMEDIATE 1    // fixIntermediate() output
#define PRINT_MERGE_SUB_TOURS 1     // mergeSubTours() output
#define PRINT_APPLY_ESET 0          // applyESET() output
#define PRINT_GENERATE_ESET 1       // generateESET functions (rand and heuristic)
#define PRINT_EDGE_OPERATIONS 0     // prints information about edge operations (REMOVE_EDGE and RESTORE_EDGE)
// main control
#define PRINT_TOURS_DURING_MERGING 1 // before and after doing merge operations, prints the list of tours
#define PRINT_BEST_TOUR_EACH_ITERATION 1 // prints the best tour each iteration
// other stuff I guess?
#define PRINT_CYCLE_POINTERS 1      // prints what the cycle pointers are at various points, used for debugging the identical cycle pointers bug
#define PRINT_DISTANCE 0            // prints distance while generating the table, kind of cluttered, not really used
#define PRINT_DISTANCE_TABLE  1     // prints the distance table in main

#endif // OPC_ENABLE
/////////////////////////////////////////////////////////////////////

////////////////// BEST TOUR TRACKING /////////////////////////////////////////
#define BEST_TOUR_TRACKING 1        // if enabled, an array that tracks every time the best tour changed gets created
                                    // and its contents dumped to disk at program completion
#if BEST_TOUR_TRACKING
#define BTT_FILE "bestTours.txt"    // file that will contain the best tours
#define MAX_BEST_TOURS 1000         // maximum number of best tours
#endif // best tour tracking
///////////////////////////////////////////////////////////////////////////////


#endif // header guard