# netqi
## What Is Netqi ?

NetQi is a model checker which allows to verify time based games to infers the
optimal strategy over time for a given player.

## Applications

NetQi has been successfully used to analyze many network security threats including: Distributed Denial Of Service (DDOS), Network Exploit, Trust Relation Abuse, Information Leak, Password Cracking, Hard drive crash and DNS cache poisoning. That being said NetQi is not limited to network security and can be used to analyze most situation where interaction with a complex environment can be described as rules such as protein interaction in biology.

## Etymology

NetQi name come from the English word Net and the Chinese word Qi : 氣 which mean vital energy flow. Hence NetQi is a tool designed to analyze the “network vital energy flow” to prevent attacks and failures that can harm this flow (legitimate traffic). It is based on timed game and model-checking theory.

## Installation

To compile and install NetQi is /usr/bin simply type:

  make install

It will copy the binary NetQi to your /usr/bin directory.
If you prefers to install it in an other directory:

1. compile NetQi: make
2. copy NetQi in the directory: cp NetQi /whatever
