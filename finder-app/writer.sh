#!/bin/bash

# Author: Dana Marble
# Description: Homework Assignment 1

writefile=$1
writestr=$2

#########################################################
# Check to see if the first argument is specified
#########################################################

if [ -z ${writefile} ]
then
echo "You have not specified 'writefile' argument"
exit 1
fi

#########################################################
# Check to see if the second argument is specified
#########################################################

if [ -z ${writestr} ]
then
echo "You have not specified 'writestr' argument"
exit 1
fi

#########################################################
# Create the directory path asked for
#########################################################

directory_setup(){
    # Capture the string and reverse it
    # Cut off the very first element which is the file name
    # Reverse the string again

    CLIPPED_END=$(echo $writefile|rev | cut -f1 -d '/'|rev)

    # Get sanitized path
    MY_DIRECTORY=$(echo $writefile|sed s/$CLIPPED_END//)
    
    # Make the path and all subsequent dirs
    mkdir -p $MY_DIRECTORY
}


#########################################################
# Build the composed operation
#########################################################

main(){

    directory_setup
    # echo the content into the file
    echo $writestr > $writefile

}

#########################################################
# Actually call the main function
#########################################################

main
