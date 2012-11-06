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
#define PRINT_INTERMEDIATE_INFO 0   // information about intermediate individuals generated
#define PRINT_EDGES 0               // prints the list of edges
#define PRINT_CHILD_TOURS 1         // information about child tours
#define PRINT_GRAPHS 1              // information about graphs
#define PRINT_CYCLES 1              // information about AB cycles, sub cycles, and other cycles
// per method
#define PRINT_GENERATE_AB_CYCLES 0   // generateABCycles() output
#define PRINT_FIX_INTERMEDIATE 0    // fixIntermediate() output
#define PRINT_MERGE_SUB_TOURS 0     // mergeSubTours() output
#define PRINT_APPLY_ESET 0          // applyESET() output
#define PRINT_GENERATE_ESET 0       // generateESET functions (rand and heuristic)
#define PRINT_EDGE_OPERATIONS 0     // prints information about edge operations (REMOVE_EDGE and RESTORE_EDGE)
// main control
#define PRINT_TOURS_DURING_MERGING 1 // before and after doing merge operations, prints the list of tours
// other stuff I guess?
#define PRINT_CYCLE_POINTERS 1      // prints what the cycle pointers are at various points, used for debugging the identical cycle pointers bug

#endif // OPC_ENABLE
/////////////////////////////////////////////////////////////////////


#endif // header guard
