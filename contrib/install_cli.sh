 #!/usr/bin/env bash

 # Execute this file to install the blast cli tools into your path on OS X

 CURRENT_LOC="$( cd "$(dirname "$0")" ; pwd -P )"
 LOCATION=${CURRENT_LOC%Blast-Qt.app*}

 # Ensure that the directory to symlink to exists
 sudo mkdir -p /usr/local/bin

 # Create symlinks to the cli tools
 sudo ln -s ${LOCATION}/Blast-Qt.app/Contents/MacOS/blastd /usr/local/bin/blastd
 sudo ln -s ${LOCATION}/Blast-Qt.app/Contents/MacOS/blast-cli /usr/local/bin/blast-cli
