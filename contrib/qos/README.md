### QoS (Quality of service) ###

This is a Linux bash script that will set up tc to limit the outgoing bandwidth for connections to the BLAST network. It limits outbound TCP traffic with a source or destination port of 64640, but not if the destination IP is within a LAN.

This means one can have an always-on blastd instance running, and another local blastd/blast-qt instance which connects to this node and receives blocks from it.
