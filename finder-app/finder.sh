#!/bin/bash

# Author: Dana Marble
# Description: Homework Assignment 1

filesdir=$1
searchstr=$2

#########################################################
# Check to see if the first argument is specified
#########################################################

if [ -z ${filesdir} ]
then
echo "You have not specified 'filesdir' argument"
exit 1
fi

#########################################################
# Check to see if the directory to find exists
#########################################################

if [ ! -d ${filesdir} ]
then
echo "This directory does not exist"
exit 1
fi

#########################################################
# Check to see if the search is specified
#########################################################

if [ -z ${searchstr} ]
then
echo "You have not specified 'searchstr' argument"
exit 1
fi

#########################################################
# Build array of files with a count of the search string only
#########################################################

list_build(){

LineArray=($( grep -risc ${searchstr} ${filesdir} | grep -v ':0'))

}

#########################################################
# Using the data in the array, get the line count
#########################################################

list_get_lines(){

LINES=$(for i in ${LineArray[@]}; do echo $i;done| wc -l)
echo $LINES

}

#########################################################
# Capture only the quantity of the search tearm and sum them
# Probably, there is a better way to do this.
#########################################################

list_get_quantity(){
TOTAL=$(for i in ${LineArray[@]}; do echo $i | cut -f2 -d ":";done)
COUNT=0; for i in $TOTAL; do COUNT=$(expr $i + $COUNT) ;done

echo $COUNT

}

#########################################################
# Call the list and format the output.
# Arguably I should use a seperate method for this part too.
#########################################################

main(){
    
    list_build
    echo "The number of files are $(list_get_lines) and the number of matching lines are $(list_get_quantity)"    

}

#########################################################
# Actually call the main function
#########################################################

main
