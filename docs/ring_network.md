# Ring Network
Boards are connected in a ring, with each TX connected to the RX of the following board.

The communication between adjacent nodes is subdivided in packets, that need to be transferred all at once.

Each board can transmit its own packets and must pass along the packets received from the incoming node, if it's not the recipient.



There are the following concerns:
1. Address assignment
1. Flood control (packet time to live)

# Packet structure

 Start - Control - Addresses - Data - Crc - End

### Start
1x Byte with value 85
### Control
1x Byte that specifies the kind of packet<br>
Control = 1: Protocol packet, more details in Data<br>
Control = 2: Data packet for upper layer, Data must be passed to upper layer<br>
### Addresses
1x Byte with the address of the source<br>
1x Byte with the address of the destination
### Data
[0-256]x Bytes with the data of this packet
### Crc
4x Bytes with the hash of this packet, containing everything except Start, Crc and End
### End
1x Byte with value 170

## Escaping
Any byte in the packet, between Start and End, that has the value 85 or 170 or 27 must be escaped using the byte 27

The sequence <br>
10 65 <span style="color:red">85</span> 72 <span style="color:red">27</span><br>
Becomes <br>
10 65 <span style="color:green">27</span> <span style="color:red">85</span> 72 <span style="color:green">27</span> <span style="color:red">27</span>