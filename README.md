# multiplexer

#### Compilation steps
g++ mux.cpp -o mux

#### Running steps
./mux input.txt

#### Additional Info
A brief summary of what this program does is it reads the source information from a file and stores information 
about each source. It calculates the transmission rate of the mux based on the combined average transmission rates
of the sources. Then, based on the start/end time of each datablock in the source, the program simulates the 
datablocks reaching the input buffer of the mux. The program will output what the input buffer looks like, meaning
what datablocks arrive at each timeframe. As each datablock arrives, the mux will add it to a frame unless the frame
is full. When the frame is full, it will transmit it and start creating the next time frame. The output buffer
simulating what each frame looks like is printed to the screen as:

	startFlag | address:datablock address:datablock ... | endflag

where the start and end flags are the start and end timing indicators of the frame and the addresses are the indexes
of the datablocks in the frame.
