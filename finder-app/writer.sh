# Accepts the following arguments: 
# the first argument is a full path to a file (including filename) on the filesystem, 
# referred to below as writefile; the second argument is a text string which will 
# be written within this file, referred to below as writestr

# Exits with value 1 error and print statements if any of the 
# arguments above were not specified

# Creates a new file with name and path writefile with content writestr, 
# overwriting any existing file and creating the path if it doesn’t exist. 
# Exits with value 1 and error print statement if the file could not be created.


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
