# Ring Network
Boards are connected in a ring, with each TX connected to the RX of the following board.

The communication between adjacent nodes is subdivided in packets, that need to be transferred all at once.

Each board can transmit its own packets and must pass along the packets received from the incoming node, if it's not the recipient.



There are the following concerns:
1. Address assignment
1. Flood control (packet time to live)

# Packet structure

 Start[1] - Header[5] - Data[0-256] - Footer[4] - End[1]

Max packet size: 5+256+4 = 265, plus start and end it's 267

### Start
1x Byte with value 85
### Header
1x Byte with length of Data block<br>
1x Byte with the following bits:<br>
bit 0: <br>
 0=Protocol packet, more details in Data<br>
 1=Data packet for upper layer, Data must be passed to upper layer<br>
bit 1-7: reserved<br>
1x Byte with the address of the source<br>
1x Byte with the address of the destination<br>
1x Byte with Time to live: when it reaches 0 the packet is dropped. This value prevents packet from going around forever<br>

### Data
[0-256]x Bytes with the data of this packet
### Footer
4x Bytes with the hash of this packet, containing everything except Start, hash and End
### End
1x Byte with value 170

## Escaping
Any byte in the packet, between Start and End, that has the value 85 or 170 or 27 must be escaped using the byte 27

The sequence <br>
10 65 <span style="color:red">85</span> 72 <span style="color:red">27</span><br>
Becomes <br>
10 65 <span style="color:green">27</span> <span style="color:red">85</span> 72 <span style="color:green">27</span> <span style="color:red">27</span>