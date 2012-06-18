/*
 * Ping Scan
 *
 * This program performs a ping scan on a network specified by the user.
 */
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <crafter.h>

/* Collapse namespaces */
using namespace std;
using namespace Crafter;

int main() {

	/* Init the library */
	InitCrafter();

	/* Set the interface */
	string iface = "wlan0";

	/* Get the IP address associated to the interface */
	string MyIP = GetMyIP(iface);
	string MyMAC = GetMyMAC(iface);
	cout << "[@] My IP address is  : " << MyIP  << endl;
	cout << "[@] My MAC address is  : " << MyMAC  << endl;

	/* --------- Common data to all headers --------- */

	/* Create an IP header */
	IP ip_header;

	/* Set the Source and Destination IP address */
	ip_header.SetSourceIP(MyIP);                         // <-- Set a source IP address.

	/* Create an ICMP header */
	ICMP icmp_header;

	icmp_header.SetType(ICMP::EchoRequest);              // <-- Echo request (a ping)
	icmp_header.SetPayload("ThisIsThePayloadOfAPing\n"); // <-- Set an arbitrary payload

    /* ---------------------------------------------- */

	/* Define the network to scan */
	vector<string> net = GetIPs("74.125.134.*");    // <-- Create a container of IP addresses from a "wildcard"
	vector<string>::iterator it_IP;                      // <-- Iterator

	/* Create a PacketContainer to hold all the ICMP packets (is just a typedef for vector<Packet*>) */
	typedef boost::shared_ptr<Packet> packet_ptr;
	vector<packet_ptr> pings_packets;

	/* Iterate to access each string that defines an IP address */
	for(it_IP = net.begin() ; it_IP != net.end() ; it_IP++) {

		ip_header.SetDestinationIP(*it_IP);              // <-- Set a destination IP address
		icmp_header.SetIdentifier(RNG16());              // <-- Set a random ID for the ICMP packet

		/* Finally, push the packet into the container */
		pings_packets.push_back(packet_ptr(new Packet(ip_header / icmp_header)));
	}

	/*
	 * At this point, we have all the packets into the
	 * pings_packets container. Now we can Send 'Em All.
	 *
	 * 48 (nthreads) -> Number of threads for distributing the packets
	 *                  (tunable, the best value depends on your
	 *                   network an processor). 32 is good :-)
	 * 0.1 (timeout) -> Timeout in seconds for waiting an answer
	 * 2  (retry)    -> Number of times we send a packet until a response is received
	 */
	cout << "[@] Sending the ICMP echoes. Wait..." << endl;
	vector<packet_ptr> pongs_packets(pings_packets.size());
	SendRecv(pings_packets.begin(),pings_packets.end(),pongs_packets.begin(),iface,1,3,48);
	cout << "[@] SendRecv function returns :-) " << endl;

	/*
	 * pongs_packets is a pointer to a PacketContainer with the same size
	 * of pings_packets (first argument). So, at this point, (after
	 * the SendRecv functions returns) we can iterate over each
	 * reply packet, if any.
	 */
	vector<packet_ptr>::iterator it_pck;
	int counter = 0;
	for(it_pck = pongs_packets.begin() ; it_pck < pongs_packets.end() ; it_pck++) {
		/* Check if the pointer is not NULL */
		packet_ptr reply_packet = (*it_pck);
		if(reply_packet) {
            /* Get the ICMP layer */
            ICMP* icmp_layer = reply_packet->GetLayer<ICMP>();
            if(icmp_layer->GetType() == ICMP::EchoReply) {
				/* Get the IP layer of the replied packet */
				IP* ip_layer = reply_packet->GetLayer<IP>();
				/* Print the Source IP */
				cout << "[@] Host " << ip_layer->GetSourceIP() << " up." << endl;
				counter++;
            }
		}
	}

	cout << "[@] " << counter << " hosts up. " << endl;

	/* Now, because we are good programmers, clean everything before exit */

//	/* Delete the container with the PINGS packets */
//	ClearContainer(pings_packets);
//
//	/* Delete the container with the responses, if there is one (check the NULL pointer) */
//	ClearContainer(pongs_packets);

	/* Clean up library stuff */
	CleanCrafter();

	return 0;
}
