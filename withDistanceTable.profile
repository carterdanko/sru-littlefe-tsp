Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 61.18     96.51    96.51    10000     0.01     0.01  fixIntermediate
 20.08    128.19    31.68 928200054     0.00     0.00  lookup_distance
 13.09    148.84    20.64 5218117350     0.00     0.00  INIT_EDGE
  1.93    151.89     3.05    10000     0.00     0.00  generateABCycles
  1.25    153.86     1.97        1     1.97     1.97  construct_distTable
  0.72    154.99     1.13    10000     0.00     0.02  performEAX
  0.61    155.95     0.96                             frame_dummy
  0.34    156.48     0.53    10000     0.00     0.00  applyESet
  0.18    156.77     0.29    10000     0.00     0.00  mergeTours
  0.18    157.05     0.28    77629     0.00     0.00  mergeSubTours
  0.08    157.17     0.12    10000     0.00     0.00  createGraph
  0.05    157.25     0.08    20000     0.00     0.00  roulette_select
  0.02    157.28     0.03  1915413     0.00     0.00  frand
  0.02    157.31     0.03    10100     0.00     0.00  set_tour_fitness
  0.01    157.32     0.02  1125832     0.00     0.00  sizeOfTour
  0.00    157.33     0.01                             merge_recursive
  0.00    157.33     0.00   124750     0.00     0.00  dist
  0.00    157.33     0.00   124750     0.00     0.00  get_distance_between
  0.00    157.33     0.00    10000     0.00     0.00  mergeTourToPop
  0.00    157.33     0.00      101     0.00     0.00  print_tour
  0.00    157.33     0.00      101     0.00     1.53  serial_listener
  0.00    157.33     0.00      100     0.00     0.00  create_tour_rand
  0.00    157.33     0.00      100     0.00     0.00  mergeToursToPop
  0.00    157.33     0.00      100     0.00     1.54  run_genalg
  0.00    157.33     0.00      100     0.00     0.00  trackTours
  0.00    157.33     0.00        2     0.00     0.00  loadCities
  0.00    157.33     0.00        2     0.00     0.00  load_cities
  0.00    157.33     0.00        1     0.00     0.00  MPI_init
  0.00    157.33     0.00        1     0.00     0.00  dumpBestTours
  0.00    157.33     0.00        1     0.00     0.00  initBestTourTracking
  0.00    157.33     0.00        1     0.00     0.00  populate_tours
  0.00    157.33     0.00        1     0.00     0.00  terminate_program

 %         the percentage of the total running time of the
time       program used by this function.

cumulative a running sum of the number of seconds accounted
 seconds   for by this function and those listed above it.

 self      the number of seconds accounted for by this
seconds    function alone.  This is the major sort for this
           listing.

calls      the number of times this function was invoked, if
           this function is profiled, else blank.
 
 self      the average number of milliseconds spent in this
ms/call    function per call, if this function is profiled,
	   else blank.

 total     the average number of milliseconds spent in this
ms/call    function and its descendents per call, if this 
	   function is profiled, else blank.

name       the name of the function.  This is the minor sort
           for this listing. The index shows the location of
	   the function in the gprof listing. If the index is
	   in parenthesis it shows where it would appear in
	   the gprof listing if it were to be printed.
