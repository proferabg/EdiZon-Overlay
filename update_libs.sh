#!/bin/bash
################################################################################
# File: update_libs.sh
# Author: ppkantorski
# Description: 
#   This script automates the process of updating the "libultra" and "libtesla"
#   libraries. It performs the following steps:
#   - Navigates to the directory where the script is located.
#   - Removes any existing "libultra" and "libtesla" directories in the "/lib" folder.
#   - Initializes a temporary Git repository to selectively clone only the necessary
#     directories from the Ultrahand-Overlay repository using sparse checkout.
#   - Moves the downloaded directories to the correct locations in the project's "/lib" folder.
#   - Cleans up temporary files and directories.
#
# Usage:
#   1. Place this script in the root directory of the overlay project.
#   2. Make sure the script has execute permissions:
#      chmod +x update_libs.sh
#   3. Run the script:
#      ./update_libs.sh
#
# Notes:
#   - Ensure you have an active internet connection, as this script pulls from a remote Git repository.
#   - Requires Git to be installed and properly configured on your system.
#
# License:
#   This script was created for overlay development by ppkantorski and is
#   distributed under the GPLv2 license.
################################################################################

# Navigate to the directory where this script is located
cd "$(dirname "$0")"

# Create the /lib directory if it doesn't exist
mkdir -p libs

# Remove existing libultra and libtesla directories if they exist
rm -rf libs/libultra
rm -rf libs/libtesla

# Initialize a new git repository
git init temp-lib

# Add the Ultrahand-Overlay repository as a remote
cd temp-lib
git remote add -f origin https://github.com/ppkantorski/Ultrahand-Overlay.git

# Enable sparse-checkout
git config core.sparseCheckout true

# Specify the directories to checkout
echo "lib/libultra/*" >> .git/info/sparse-checkout
echo "lib/libtesla/*" >> .git/info/sparse-checkout

# Checkout the specified directories
git pull origin main

# Move the directories to the correct place
cd ..
mkdir -p libs/libultra libs/libtesla
mv temp-lib/lib/libultra/* libs/libultra/
mv temp-lib/lib/libtesla/* libs/libtesla/

# Clean up
rm -rf temp-lib
