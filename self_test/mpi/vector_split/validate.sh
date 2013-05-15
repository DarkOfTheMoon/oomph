#! /bin/sh


#Set the number of tests to be checked
NUM_TESTS=1

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


# Validation for vector split test
#-----------------------------------------

echo "Running vector split test "
# one processor
MPI_RUN_ON_NP_COMMAND=`echo $MPI_RUN_COMMAND | sed -e "s/2/1/g"`
$MPI_RUN_ON_NP_COMMAND ../vector_split

# two processors
MPI_RUN_ON_NP_COMMAND=`echo $MPI_RUN_COMMAND | sed -e "s/2/2/g"`
$MPI_RUN_ON_NP_COMMAND ../vector_split

# three processors
MPI_RUN_ON_NP_COMMAND=`echo $MPI_RUN_COMMAND | sed -e "s/2/3/g"`
$MPI_RUN_ON_NP_COMMAND ../vector_split

# four processors
MPI_RUN_ON_NP_COMMAND=`echo $MPI_RUN_COMMAND | sed -e "s/2/4/g"`
$MPI_RUN_ON_NP_COMMAND ../vector_split

echo "done"
echo " " >> validation.log
echo "Distributed vector split test" >> validation.log
echo "------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log

# Cat all the outputs into one file.
cat out_NP1R0 out_NP2R0 out_NP2R1 out_NP3R0 out_NP3R1 out_NP3R2 out_NP4R0 out_NP4R1 out_NP4R2 out_NP4R3 > vector_split.dat

# Remove the individual outputs.
rm -rf out_NP1R0 out_NP2R0 out_NP2R1 out_NP3R0 out_NP3R1 out_NP3R2 out_NP4R0 out_NP4R1 out_NP4R2 out_NP4R3

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/vector_split.dat.gz  \
         vector_split.dat >> validation.log
fi

#----------------------------------------------------------------------

# Append log to main validation log
cat validation.log >> ../../../../validation.log

cd ..

#######################################################################

# validate_ok_count will exit with status
# 0 if all tests has passed.
# 1 if some tests failed.
# 2 if there are more 'OK' than expected.
. ../../../bin/validate_ok_count

# Never get here
exit 10
