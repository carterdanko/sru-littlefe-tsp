Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 35.54    103.67   103.67 532475658     0.00     0.00  dist
 32.32    197.94    94.27    10000     0.01     0.03  fixIntermediate
 12.50    234.40    36.46 532475658     0.00     0.00  get_distance_between
  8.42    258.97    24.57 532475658     0.00     0.00  lookup_distance
  8.33    283.27    24.31 4822392954     0.00     0.00  INIT_EDGE
  1.14    286.58     3.31    10000     0.00     0.00  generateABCycles
  0.77    288.84     2.26    10000     0.00     0.03  performEAX
  0.33    289.79     0.95                             frame_dummy
  0.31    290.69     0.90                             construct_distTable
  0.15    291.14     0.45    10000     0.00     0.00  applyESet
  0.07    291.34     0.20    10000     0.00     0.00  mergeTours
  0.07    291.54     0.20    74534     0.00     0.00  mergeSubTours
  0.04    291.67     0.13    10000     0.00     0.00  createGraph
  0.01    291.69     0.02    20000     0.00     0.00  roulette_select
  0.01    291.71     0.02      100     0.00     0.00  create_tour_rand
  0.00    291.72     0.01  1951368     0.00     0.00  frand
  0.00    291.73     0.01  1137623     0.00     0.00  sizeOfTour
  0.00    291.74     0.01    10000     0.00     0.00  mergeTourToPop
  0.00    291.75     0.01                             merge_recursive
  0.00    291.75     0.00    10100     0.00     0.00  set_tour_fitness
  0.00    291.75     0.00      101     0.00     0.00  print_tour
  0.00    291.75     0.00      101     0.00     2.87  serial_listener
  0.00    291.75     0.00      100     0.00     0.00  mergeToursToPop
  0.00    291.75     0.00      100     0.00     2.90  run_genalg
  0.00    291.75     0.00      100     0.00     0.00  trackTours
  0.00    291.75     0.00        2     0.00     0.00  loadCities
  0.00    291.75     0.00        2     0.00     0.00  load_cities
  0.00    291.75     0.00        1     0.00     0.00  MPI_init
  0.00    291.75     0.00        1     0.00     0.00  dumpBestTours
  0.00    291.75     0.00        1     0.00     0.00  initBestTourTracking
  0.00    291.75     0.00        1     0.00     0.04  populate_tours
  0.00    291.75     0.00        1     0.00     0.00  terminate_program

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
