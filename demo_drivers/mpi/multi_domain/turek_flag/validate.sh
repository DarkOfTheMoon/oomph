#! /bin/sh


#Set the number of tests to be checked
NUM_TESTS=2

# Doc what we're using to run tests on two processors
echo " " 
echo "Running mpi tests with mpi run command: " $MPI_RUN_COMMAND
echo " " 

# Setup validation directory
#---------------------------
touch Validation
rm -r -f Validation
mkdir Validation

cd Validation
cp ../*partition.dat .



#---------------------------------------------------------------------

# Validation for Turek flag problem (with load balancing)
#-------------------------------------------------------------------------

echo "Running Turek flag validation with load balancing"
mkdir RESLT_TUREK_LOAD_BALANCE

# Wait for a bit to allow parallel file systems to realise
# the existence of the new directory
sleep 5

$MPI_RUN_COMMAND ../turek_flag_load_balance > OUTPUT_turek_flag_load_balance
echo "done"
echo " " >> validation.log
echo "Turek flag validation" >> validation.log
echo "------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT_TUREK_LOAD_BALANCE/soln1_on_proc0.dat RESLT_TUREK_LOAD_BALANCE/soln1_on_proc1.dat RESLT_TUREK_LOAD_BALANCE/soln2_on_proc0.dat RESLT_TUREK_LOAD_BALANCE/soln2_on_proc1.dat RESLT_TUREK_LOAD_BALANCE/solid_soln1_on_proc0.dat RESLT_TUREK_LOAD_BALANCE/solid_soln1_on_proc1.dat RESLT_TUREK_LOAD_BALANCE/solid_soln2_on_proc0.dat RESLT_TUREK_LOAD_BALANCE/solid_soln2_on_proc1.dat > turek_flag_load_balance_external_results.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../../bin/fpdiff.py ../validata/turek_flag_load_balance_external_results.dat.gz  \
         turek_flag_load_balance_external_results.dat  0.5 1.0e-14 >> validation.log
fi

#---------------------------------------------------------------------


# Validation for Turek flag problem (parallel, with refineable solid mesh)
#-------------------------------------------------------------------------

echo "Running Turek flag validation "
mkdir RESLT_TUREK

# Wait for a bit to allow parallel file systems to realise
# the existence of the new directory
sleep 5

$MPI_RUN_COMMAND ../turek_flag > OUTPUT_turek_flag
echo "done"
echo " " >> validation.log
echo "Turek flag validation" >> validation.log
echo "------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT_TUREK/soln1_on_proc0.dat RESLT_TUREK/soln1_on_proc1.dat RESLT_TUREK/soln2_on_proc0.dat RESLT_TUREK/soln2_on_proc1.dat RESLT_TUREK/solid_soln1_on_proc0.dat RESLT_TUREK/solid_soln1_on_proc1.dat RESLT_TUREK/solid_soln2_on_proc0.dat RESLT_TUREK/solid_soln2_on_proc1.dat > turek_flag_external_results.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../../bin/fpdiff.py ../validata/turek_flag_external_results.dat.gz  \
         turek_flag_external_results.dat 0.5 1.0e-14 >> validation.log
fi

# Append log to main validation log
cat validation.log >> ../../../../../validation.log

#---------------------------------------------------------------------

cd ..



#######################################################################


#Check that we get the correct number of OKs
OK_COUNT=`grep -c 'OK' Validation/validation.log`
if  [ $OK_COUNT -eq $NUM_TESTS ]; then
 echo " "
 echo "======================================================================"
 echo " " 
 echo "All tests in" 
 echo " " 
 echo "    `pwd`    "
 echo " "
 echo "passed successfully."
 echo " "
 echo "======================================================================"
 echo " " 
else
  if [ $OK_COUNT -lt $NUM_TESTS ]; then
   echo " "
   echo "======================================================================"
   echo " " 
   echo "Only $OK_COUNT of $NUM_TESTS test(s) passed; see"
   echo " " 
   echo "    `pwd`/Validation/validation.log"
   echo " " 
   echo "for details" 
   echo " " 
   echo "======================================================================"
   echo " "
  else 
   echo " "
   echo "======================================================================"
   echo " " 
   echo "More OKs than tests! Need to update NUM_TESTS in"
   echo " " 
   echo "    `pwd`/validate.sh"
   echo " "
   echo "======================================================================"
   echo " "
  fi
fi
