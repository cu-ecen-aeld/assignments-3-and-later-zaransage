#!/bin/ash

# Author: Dana Marble
# Description: Adjusted Homework 1 for Homework 3

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
# Count the number of matching lines
# awk defails from https://stackoverflow.com/questions/28905083/how-to-sum-a-column-in-awk
#########################################################

get_line_count() {
    grep -Risc "${searchstr}" "${filesdir}" | grep -v ':0' | awk -F: '{ sum += $2 } END { print sum }'
}

#########################################################
# Count the number of files containing the search string
#########################################################

count_files() {
    grep -Risc "${searchstr}" "${filesdir}" | grep -v ':0' | wc -l
}

#########################################################
# Main function
#########################################################

main() {
    NUMBER_OF_FILES=$(count_files)
    NUMBER_OF_LINES=$(get_line_count)
    echo "The number of files are ${NUMBER_OF_FILES} and the number of matching lines are ${NUMBER_OF_LINES}"
}

main